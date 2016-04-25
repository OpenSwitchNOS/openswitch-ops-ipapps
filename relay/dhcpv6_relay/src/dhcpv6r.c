/*
 * Copyright (C) 2016 Hewlett Packard Enterprise Development LP
 * All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may
 *   not use this file except in compliance with the License. You may obtain
 *   a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 *
 * File: dhcpv6r.c
 *
 */

/*
 * This daemon handles the following functionality:
 * - Dhcpv6-relay agent for IPv6.
 */

/* Linux includes */
#include <errno.h>
#include <timeval.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <semaphore.h>

/* Dynamic string */
#include <dynamic-string.h>

/* OVSDB Includes */
#include "config.h"
#include "command-line.h"
#include "stream.h"
#include "daemon.h"
#include "fatal-signal.h"
#include "dirs.h"
#include "poll-loop.h"
#include "unixctl.h"
#include "openvswitch/vconn.h"
#include "openvswitch/vlog.h"
#include "openswitch-dflt.h"
#include "coverage.h"
#include "svec.h"

#include "dhcpv6r.h"

/* FIXME: Temporary addition. Once macro is merged following shall be removed */
#define SYSTEM_DHCP_CONFIG_MAP_V6RELAY_DISABLED "dhcpv6_relay_disabled"
#define SYSTEM_DHCP_CONFIG_MAP_V6RELAY_OPTION79_ENABLED "dhcpv6_relay_option79_disabled"

/*
 * Global variable declarations.
 */
/* Cached idl sequence number */
extern uint32_t idl_seqno;
/* idl pointer */
extern struct ovsdb_idl *idl;

/* DHCPv6 relay global configuration context */
DHCPV6R_CTRL_CB dhcpv6r_ctrl_cb;
DHCPV6R_CTRL_CB *dhcpv6r_ctrl_cb_p = &dhcpv6r_ctrl_cb;

VLOG_DEFINE_THIS_MODULE(dhcpv6r);

/*
 * Function      : dhcpv6r_module_init
 * Responsiblity : Initialization routine for dhcpv6 relay feature
 * Parameters    : none
 * Return        : true, on success
 *                 false, on failure
 */
bool dhcpv6r_module_init(void)
{
    return true;
}

/*
 * Function      : dhcpv6r_update_stats_refresh_interval
 * Responsiblity : Check for statistics refresh interval update.
 * Parameters    : value - statistics refresh interval
 * Return        : none
 */
void dhcpv6r_update_stats_refresh_interval(const char *value)
{
    if (atoi(value) != dhcpv6r_ctrl_cb_p->stats_interval) {
        VLOG_INFO("statistics refresh interval changed. old : %d, new : %d",
                  dhcpv6r_ctrl_cb_p->stats_interval,
                  atoi(value));

        /* update global structure with the new statistics refresh interval. */
        dhcpv6r_ctrl_cb_p->stats_interval = atoi(value);
    }

    return;
}


/*
 * Function      : dhcpv6r_process_globalconfig_update
 * Responsiblity : Process system table update notifications related to
 *                 dhcpv6 relay from OVSDB.
 * Parameters    : none
 * Return        : none
 */
void dhcpv6r_process_globalconfig_update(void)
{
    const struct ovsrec_system *system_row = NULL;

#if defined(FTR_DHCPV6_RELAY)
    bool dhcpv6r_enabled, dhcpv6r_option79;
    char *value;
#endif /* FTR_DHCPV6_RELAY */

    system_row = ovsrec_system_first(idl);
    if (NULL == system_row) {
        VLOG_ERR("Unable to find system record in idl cache");
        return;
    }

    /* If system table row is not modified do nothing */
    if (!OVSREC_IDL_IS_ROW_MODIFIED(system_row, idl_seqno))
    {
        return;
    }

#if defined(FTR_DHCPV6_RELAY)
    /* Check if dhcpv6-relay global configuration is changed */
    if (OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_system_col_dhcp_config,
                                   idl_seqno)) {
        value = (char *)smap_get(&system_row->dhcp_config,
                                 SYSTEM_DHCP_CONFIG_MAP_V6RELAY_DISABLED);

        /* DHCPv6-Relay is enabled by default.*/
        if (NULL == value) {
            dhcpv6r_enabled = true;
        } else {
            if (!strncmp(value, "false", strlen(value))) {
                dhcpv6r_enabled = true;
            }
        }

        /* Check if dhcpv6-relay global configuration is changed */
        if (dhcpv6r_enabled != dhcpv6r_ctrl_cb_p->dhcpv6r_enable) {
            VLOG_INFO("DHCPv6-Relay global config change. old : %d, new : %d",
                     dhcpv6r_ctrl_cb_p->dhcpv6r_enable, dhcpv6r_enabled);
            dhcpv6r_ctrl_cb_p->dhcpv6r_enable = dhcpv6r_enabled;
        }

        /* Check if dhcpv6-relay option 79 value is changed */
        value = (char *)smap_get(&system_row->dhcp_config,
                                 SYSTEM_DHCP_CONFIG_MAP_V6RELAY_OPTION79_ENABLED);
        if (value && (!strncmp(value, "true", strlen(value)))) {
            dhcpv6r_option79 = true;
        }

        if (dhcpv6r_option79 != dhcpv6r_ctrl_cb_p->dhcpv6r_option79_enable) {
            VLOG_INFO("DHCPv6-Relay option 79 global config change. old : %d, new : %d",
                     dhcpv6r_ctrl_cb_p->dhcpv6r_option79_enable, dhcpv6r_option79);
            dhcpv6r_ctrl_cb_p->dhcpv6r_option79_enable = dhcpv6r_option79;
        }
    }

    /* Check if there is a change in system table other config */
    if (OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_system_col_other_config,
                                   idl_seqno)) {
        /* Check if there is a change in statistics refresh interval */
        value = (char *)smap_get(&system_row->other_config,
                                 SYSTEM_OTHER_CONFIG_MAP_STATS_UPDATE_INTERVAL);
        dhcpv6r_update_stats_refresh_interval(value);
    }
#endif /* FTR_DHCPV6_RELAY */
    return;
}


#if defined(FTR_DHCPV6_RELAY)
/*
 * Function      : dhcpv6r_server_config_update
 * Responsiblity : Process dhcp_relay table update notifications from OVSDB for the
 *                 dhcpv6 relay configuration changes.
 * Parameters    : none
 * Return        : none
 */
void dhcpv6r_server_config_update(void)
{
    const struct ovsrec_dhcp_relay *rec_first = NULL;
    const struct ovsrec_dhcp_relay *rec = NULL;

    /* Check for server configuration changes */
    rec_first = ovsrec_dhcp_relay_first(idl);
    if (NULL == rec_first) {
        /* Check if last entry from the table is deleted */
        dhcpv6r_handle_row_delete(idl);
        return;
    }

    if (!OVSREC_IDL_ANY_TABLE_ROWS_INSERTED(rec_first, idl_seqno)
        && !OVSREC_IDL_ANY_TABLE_ROWS_DELETED(rec_first, idl_seqno)
        && !OVSREC_IDL_ANY_TABLE_ROWS_MODIFIED(rec_first, idl_seqno)) {
        /* FIXME: Log to be removed after initial round of testing */
        VLOG_DBG("No table changes updates");
        return;
    }

    /* Process row delete notification */
    if (OVSREC_IDL_ANY_TABLE_ROWS_DELETED(rec_first, idl_seqno)) {
      dhcpv6r_handle_row_delete(idl);
    }

    /* Process row insert/modify notifications */
    OVSREC_DHCP_RELAY_FOR_EACH (rec, idl) {
        if (OVSREC_IDL_IS_ROW_INSERTED(rec, idl_seqno)
            || OVSREC_IDL_IS_ROW_MODIFIED(rec, idl_seqno)) {
            dhcpv6r_handle_config_change(rec, idl_seqno);
        }
    }

    /* FIXME: Handle the case of NULL port column value. Currently
     * "no interface" command doesn't seem to be working */

    return;
}
#endif /* FTR_DHCPV6_RELAY */


/*
 * Function      : dhcpv6r_reconfigure
 * Responsiblity : Process the table update notifications from OVSDB for the
 *                 configuration changes.
 * Parameters    : none
 * Return        : none
 */
void dhcpv6r_reconfigure(void)
{
    uint32_t new_idl_seqno = ovsdb_idl_get_seqno(idl);

    if (new_idl_seqno == idl_seqno){
        VLOG_DBG("No config change for dhcpv6r in ovs");
        return;
    }


#if defined(FTR_DHCPV6_RELAY)
    /* Check for global configuration changes in system table */
    dhcpv6r_process_globalconfig_update();

    /* Process dhcp_relay table updates */
    dhcpv6r_server_config_update();
#endif /* FTR_DHCPV6_RELAY */

    /* Cache the lated idl sequence number */
    idl_seqno = new_idl_seqno;
    return;
}

/*
 * Function      : dhcpv6r_exit
 * Responsiblity : dhcpv6r module cleanup before exit
 * Parameters    : -
 * Return        : none
 */
void dhcpv6r_exit(void)
{

}
