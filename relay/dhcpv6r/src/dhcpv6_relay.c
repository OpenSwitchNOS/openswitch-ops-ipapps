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
 * File: dhcpv6_relay.c
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

#include "relay_common.h"
#include "dhcpv6_relay.h"

/* FIXME: Temporary addition. Once macro is merged following shall be removed */
#define SYSTEM_DHCP_CONFIG_MAP_V6RELAY_ENABLED    "v6relay_enabled"
#define SYSTEM_DHCP_CONFIG_MAP_V6RELAY_OPTION79_ENABLED "dhcpv6_relay_option79_enabled"

/*
 * Global variable declarations.
 */

/* DHCPv6 relay global configuration context */
DHCPV6_RELAY_CTRL_CB dhcpv6_relay_ctrl_cb;
DHCPV6_RELAY_CTRL_CB *dhcpv6_relay_ctrl_cb_p = &dhcpv6_relay_ctrl_cb;

VLOG_DEFINE_THIS_MODULE(dhcpv6_relay);

/*
 * Function      : dhcpv6_relay_module_init
 * Responsiblity : Initialization routine for dhcpv6 relay feature
 * Parameters    : none
 * Return        : true, on success
 *                 false, on failure
 */
bool dhcpv6_relay_module_init(void)
{
    int32_t retVal;
    memset(dhcpv6_relay_ctrl_cb_p, 0, sizeof(DHCPV6_RELAY_CTRL_CB));

    /* DB access semaphore initialization */
    retVal = sem_init(&dhcpv6_relay_ctrl_cb_p->waitSem, 0, 1);
    if (0 != retVal) {
        VLOG_FATAL("Failed to initilize the semaphore, retval : %d (errno : %d)",
                   retVal, errno);
        return false;
    }

    /* Initialize server hash table */
    shash_init(&dhcpv6_relay_ctrl_cb_p->intfHashTable);

    /* Initialize server hash map */
    cmap_init(&dhcpv6_relay_ctrl_cb_p->serverHashMap);

    /* default values */
    dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_enable = false;
    dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_option79_enable = false;

    return true;
}

/*
 * Function      : dhcpv6_relay_update_stats_refresh_interval
 * Responsiblity : Check for statistics refresh interval update.
 * Parameters    : value - statistics refresh interval
 * Return        : none
 */
void dhcpv6_relay_update_stats_refresh_interval(const char *value)
{
    if (atoi(value) != dhcpv6_relay_ctrl_cb_p->stats_interval) {
        VLOG_INFO("statistics refresh interval changed. old : %d, new : %d",
                  dhcpv6_relay_ctrl_cb_p->stats_interval,
                  atoi(value));

        /* update global structure with the new statistics refresh interval. */
        dhcpv6_relay_ctrl_cb_p->stats_interval = atoi(value);
    }

    return;
}


/*
 * Function      : dhcpv6_relay_process_globalconfig_update
 * Responsiblity : Process system table update notifications related to
 *                 dhcpv6 relay from OVSDB.
 * Parameters    : none
 * Return        : none
 */
void dhcpv6_relay_process_globalconfig_update(void)
{
    const struct ovsrec_system *system_row = NULL;

#if defined(FTR_DHCPV6_RELAY)
    bool dhcpv6_relay_enabled = false, dhcpv6_relay_option79 = false;
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

#ifdef FTR_DHCPV6_RELAY
    /* Check if dhcpv6-relay global configuration is changed */
    if (OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_system_col_dhcp_config,
                                   idl_seqno)) {
        value = (char *)smap_get(&system_row->dhcp_config,
                                 SYSTEM_DHCP_CONFIG_MAP_V6RELAY_ENABLED);

        /* DHCPv6-Relay is disabled by default.*/
        if (value && (!strncmp(value, "true", strlen(value)))) {
            dhcpv6_relay_enabled = true;
        }

        /* Check if dhcpv6-relay global configuration is changed */
        if (dhcpv6_relay_enabled != dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_enable) {
            VLOG_INFO("DHCPv6-Relay global config change. old : %d, new : %d",
                     dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_enable, dhcpv6_relay_enabled);
            dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_enable = dhcpv6_relay_enabled;
        }

        /* Check if dhcpv6-relay option 79 value is changed */
        value = (char *)smap_get(&system_row->dhcp_config,
                                 SYSTEM_DHCP_CONFIG_MAP_V6RELAY_OPTION79_ENABLED);
        if (value && (!strncmp(value, "true", strlen(value)))) {
            dhcpv6_relay_option79 = true;
        }

        if (dhcpv6_relay_option79 != dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_option79_enable) {
            VLOG_INFO("DHCPv6-Relay option 79 global config change. old : %d, new : %d",
                     dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_option79_enable, dhcpv6_relay_option79);
            dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_option79_enable = dhcpv6_relay_option79;
        }
    }

    /* Check if there is a change in system table other config */
    if (OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_system_col_other_config,
                                   idl_seqno)) {
        /* Check if there is a change in statistics refresh interval */
        value = (char *)smap_get(&system_row->other_config,
                                 SYSTEM_OTHER_CONFIG_MAP_STATS_UPDATE_INTERVAL);
        dhcpv6_relay_update_stats_refresh_interval(value);
    }
#endif /* FTR_DHCPV6_RELAY */
    return;
}


#ifdef FTR_DHCPV6_RELAY
/*
 * Function      : dhcpv6_relay_server_config_update
 * Responsiblity : Process dhcp_relay table update notifications from OVSDB for the
 *                 dhcpv6 relay configuration changes.
 * Parameters    : none
 * Return        : none
 */
void dhcpv6_relay_server_config_update(void)
{
    const struct ovsrec_dhcp_relay *rec_first = NULL;
    const struct ovsrec_dhcp_relay *rec = NULL;

    /* Check for server configuration changes */
    rec_first = ovsrec_dhcp_relay_first(idl);
    if (NULL == rec_first) {
        /* Check if last entry from the table is deleted */
        dhcpv6_relay_handle_row_delete(idl);
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
      dhcpv6_relay_handle_row_delete(idl);
    }

    /* Process row insert/modify notifications */
    OVSREC_DHCP_RELAY_FOR_EACH (rec, idl) {
        if (OVSREC_IDL_IS_ROW_INSERTED(rec, idl_seqno)
            || OVSREC_IDL_IS_ROW_MODIFIED(rec, idl_seqno)) {
            dhcpv6_relay_handle_config_change(rec, idl_seqno);
        }
    }

    /* FIXME: Handle the case of NULL port column value. Currently
     * "no interface" command doesn't seem to be working */

    return;
}
#endif /* FTR_DHCPV6_RELAY */

#ifdef FTR_DHCPV6_RELAY
/*
 * Function      : dhcpv6_relay_run_stats_update
 * Responsiblity : To update interface dhcp-relay statistics if necessary.
 * Parameters    : none
 * Return        : none
 */
static void
dhcpv6_relay_run_stats_update(void)
{
    int stats_interval;
    static int stats_timer_interval;
    static long long int stats_timer = LLONG_MIN;

    /* Statistics update interval should always be greater than or equal to
     * 5000 ms. */
    stats_interval = dhcpv6_relay_ctrl_cb_p->stats_interval;

    if (stats_timer_interval != stats_interval) {
        stats_timer_interval = stats_interval;
        stats_timer = LLONG_MIN;
    }

    /* Rate limit the update. */
    if (time_msec() >= stats_timer) {

        /*FIXME: refresh_dhcpv6_relay_stats(); */
        stats_timer = time_msec() + stats_timer_interval;
    }
}
#endif /* FTR_DHCPV6_RELAY */

/*
 * Function      : dhcpv6_relay_reconfigure
 * Responsiblity : Process the table update notifications from OVSDB for the
 *                 configuration changes.
 * Parameters    : none
 * Return        : none
 */
void dhcpv6_relay_reconfigure(void)
{
    uint32_t new_idl_seqno = ovsdb_idl_get_seqno(idl);

    if (new_idl_seqno == idl_seqno){
        VLOG_DBG("No config change for dhcpv6_relay in ovs");
        return;
    }

#ifdef FTR_DHCPV6_RELAY

    /* Update statistics */
    dhcpv6_relay_run_stats_update();

    /* Check for global configuration changes in system table */
    dhcpv6_relay_process_globalconfig_update();

    /* Process dhcp_relay table updates */
    dhcpv6_relay_server_config_update();
#endif /* FTR_DHCPV6_RELAY */

    /* Cache the lated idl sequence number */
    idl_seqno = new_idl_seqno;
    return;
}

/*
 * Function      : dhcpv6_relay_exit
 * Responsiblity : dhcpv6_relay module cleanup before exit
 * Parameters    : -
 * Return        : none
 */
void dhcpv6_relay_exit(void)
{

}

#ifdef FTR_DHCPV6_RELAY
/*
 * Function      : dhcpv6_relay_interface_dump
 * Responsiblity : Function dumps information about server IPv6 address,
 *                 outgoing interface name for each
 *                 interface into dynamic string ds.
 * Parameters    : ds - output buffer
 *                 node - interface node.
 * Return        : none
 */
static void dhcpv6_relay_interface_dump(struct shash_node *node,
                                  struct ds *ds)
{
    DHCPV6_RELAY_SERVER_T *server = NULL, **serverArray = NULL;;
    DHCPV6_RELAY_INTERFACE_NODE_T *intfNode = NULL;
    int32_t iter = 0;
    char ipv6[INET6_ADDRSTRLEN];

    memset(ipv6, 0, sizeof(ipv6));

    intfNode = (DHCPV6_RELAY_INTERFACE_NODE_T *)node->data;
    serverArray = intfNode->serverArray;

    /* Print all configured server IPv6 addresses */
    ds_put_format(ds, "Interface %s: %d\n", intfNode->portName,
                      intfNode->addrCount);

    for(iter = 0; iter < intfNode->addrCount; iter++)
    {
        server = serverArray[iter];
        inet_ntop(AF_INET6, server->ipv6_address, ipv6, INET6_ADDRSTRLEN);
        ds_put_format(ds, "%s,%d\n", ipv6, server->ref_count);
        ds_put_format(ds, "Outgoing interface %s",server->egressIfName);
    }
    return;
}
#endif /* FTR_DHCPV6_RELAY */


/*
 * Function      : dhcpv6_relay_unixctl_dump
 * Responsiblity : DHCPv6 Relay module core dump callback
 * Parameters    : conn - unixctl socket connection
 *                 argc, argv - function parameters
 *                 aux - aux connection data
 * Return        : none
 */
void dhcpv6_relay_unixctl_dump(struct unixctl_conn *conn, int argc OVS_UNUSED,
                   const char *argv[] OVS_UNUSED, void *aux OVS_UNUSED)
{
    struct ds ds = DS_EMPTY_INITIALIZER;
    struct shash_node *node, *temp;

    /*
     * Parse unxictl arguments.
     * second argument is always interface name.
     * fourth argument is udp destination port number.
     * ex : ovs-appctl -t ops-relay dhcpv6_relay/dump int 1
     */

#ifdef FTR_DHCPV6_RELAY
    ds_put_format(&ds, "DHCPv6 Relay : %d\n", dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_enable);
    ds_put_format(&ds, "DHCPv6 Relay Option79 : %d\n",
        dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_option79_enable);


    if (!argv[2]) {
        /* dump all interfaces */
        SHASH_FOR_EACH(temp, &dhcpv6_relay_ctrl_cb_p->intfHashTable)
            dhcpv6_relay_interface_dump(temp, &ds);
    }
    else {
        node = shash_find(&dhcpv6_relay_ctrl_cb_p->intfHashTable, argv[2]);
        if (NULL == node) {
            ds_put_format(&ds, "No servers are configured on"
            " this interface :%s\n", argv[2]);
            return;
        }
        dhcpv6_relay_interface_dump(node, &ds);
    }
#endif /* FTR_DHCPV6_RELAY */

    unixctl_command_reply(conn, ds_cstr(&ds));
    ds_destroy(&ds);
}

#ifdef FTR_DHCPV6_RELAY
/*
 * Function      : dhcpv6_relay_init
 * Responsiblity : idl create/registration, module initialization and
 *                 unixctl registrations
 * Parameters    : none
 * Return        : true - on success
 *                 false - on failure
 */
bool dhcpv6_relay_init()
{
    /* Register for DHCP_Relay table updates */
    ovsdb_idl_add_table(idl, &ovsrec_table_dhcp_relay);
    ovsdb_idl_add_column(idl, &ovsrec_dhcp_relay_col_port);
    ovsdb_idl_add_column(idl,
                        &ovsrec_dhcp_relay_col_vrf);
    ovsdb_idl_add_column(idl,
                        &ovsrec_dhcp_relay_col_ipv6_ucast_server);
    ovsdb_idl_add_column(idl,
                        &ovsrec_dhcp_relay_col_ipv6_mcast_server);

    /* Register for port table for dhcp_relay_statistics update */
    ovsdb_idl_add_table(idl, &ovsrec_table_port);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_name);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_dhcp_relay_statistics);

    /* Initialize module data structures */
    if (true != dhcpv6_relay_module_init())
    {
        VLOG_FATAL("DHCPv6 relay module failed to initialize");
        return false;
    }

    unixctl_command_register("dhcpv6r/dump", "", 0, 2,
                             dhcpv6_relay_unixctl_dump, NULL);
    return true;
}

#endif /* FTR_DHCPV6_RELAY */
