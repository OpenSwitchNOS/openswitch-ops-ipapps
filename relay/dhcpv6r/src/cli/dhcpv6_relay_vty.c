/* DHCPV6-Relay CLI commands
 *
 * Copyright (C) 2016 Hewlett Packard Enterprise Development LP
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the FreeAC
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * File: dhcpv6_relay_vty.c
 *
 * Purpose: To add dhcpv6-relay configuration CLI commands.
 */

#include <sys/un.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <pwd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "vtysh/lib/version.h"
#include "getopt.h"
#include "vtysh/command.h"
#include "vtysh/memory.h"
#include <stdbool.h>
#include <stdlib.h>
#include "vtysh/vtysh.h"
#include "vswitch-idl.h"
#include "ovsdb-idl.h"
#include "openvswitch/vlog.h"
#include "openswitch-idl.h"
#include "vtysh/prefix.h"
#include "vtysh/vtysh_ovsdb_if.h"
#include "vtysh/vtysh_ovsdb_config.h"
#include "vtysh_ovsdb_dhcpv6_relay_context.h"
#include "dhcpv6_relay_vty.h"
#include "dhcp_relay_vty.h"

int32_t show_dhcpv6_relay_config ();

extern struct ovsdb_idl *idl;
VLOG_DEFINE_THIS_MODULE(dhcpv6_relay_vty);

/*-----------------------------------------------------------------------------
| Function       : show_dhcpv6_relay_config
| Responsibility : To show the dhcpv6-relay configuration.
| Return         : Returns CMD_SUCCESS
-----------------------------------------------------------------------------*/
int32_t
show_dhcpv6_relay_config (void)
{
    const struct ovsrec_system *ovs_row = NULL;
    const struct ovsrec_dhcp_relay *row = NULL;
    const struct ovsdb_datum *datum = NULL;
    union ovsdb_atom atom;
    uint32_t index;
    DHCPV6_RELAY_PKT_COUNTER dhcpv6_relay_pkt_counters;
    memset(&dhcpv6_relay_pkt_counters, 0, sizeof(DHCPV6_RELAY_PKT_COUNTER));

    row = ovsrec_dhcp_relay_first(idl);
    char *status = NULL;

    ovs_row = ovsrec_system_first(idl);
    if (ovs_row == NULL)
    {
        VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
        return CMD_OVSDB_FAILURE;
    }

    /* Display the dhcpv6-relay global configuration */
    status = (char *)smap_get(&ovs_row->dhcp_config,
                             SYSTEM_DHCP_CONFIG_MAP_V6RELAY_ENABLED);

    if (status && !strcmp(status, "true"))
        vty_out(vty, "\n DHCPV6 Relay Agent : %s%s",
                "enabled", VTY_NEWLINE);
    else
        vty_out(vty, "\n DHCPV6 Relay Agent : %s%s",
                "disabled", VTY_NEWLINE);

    /* Display the dhcpv6-relay option 79 configuration */
    status = NULL;
    status = (char *)smap_get(&ovs_row->dhcp_config,
                SYSTEM_DHCP_CONFIG_MAP_V6RELAY_OPTION79_ENABLED);

    if (status && !strcmp(status, "true"))
        vty_out(vty, " Option 79          : %s%s",
                "enabled", VTY_NEWLINE);
    else
        vty_out(vty, " Option 79          : %s%s",
                "disabled", VTY_NEWLINE);

    OVSREC_DHCP_RELAY_FOR_EACH (row, idl)
    {
        datum = ovsrec_port_get_dhcp_relay_statistics(row->port,
                                OVSDB_TYPE_STRING, OVSDB_TYPE_INTEGER);
        if (NULL == datum)
            continue;

        atom.string =
            PORT_DHCP_RELAY_STATISTICS_MAP_VALID_V6CLIENT_REQUESTS;
        index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
        dhcpv6_relay_pkt_counters.v6client_valids +=
            ((index == UINT_MAX)? 0 : datum->values[index].integer);

        atom.string =
            PORT_DHCP_RELAY_STATISTICS_MAP_DROPPED_V6CLIENT_REQUESTS;
        index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
        dhcpv6_relay_pkt_counters.v6client_drops +=
            ((index == UINT_MAX)? 0 : datum->values[index].integer);

        atom.string =
            PORT_DHCP_RELAY_STATISTICS_MAP_VALID_V6SERVER_RESPONSES;
        index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
        dhcpv6_relay_pkt_counters.v6serv_valids +=
            ((index == UINT_MAX)? 0 : datum->values[index].integer);

        atom.string =
            PORT_DHCP_RELAY_STATISTICS_MAP_DROPPED_V6SERVER_RESPONSES;
        index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
        dhcpv6_relay_pkt_counters.v6serv_drops +=
            ((index == UINT_MAX)? 0 : datum->values[index].integer);
    }
    /* Display the dhcpv6-relay statistics */
    vty_out(vty, "%s DHCPV6 Relay Statistics:%s",
                VTY_NEWLINE, VTY_NEWLINE);
    vty_out(vty, "%s  Client Requests       Server Responses%s",
                VTY_NEWLINE, VTY_NEWLINE);
    vty_out(vty, "%s  Valid      Dropped    Valid      Dropped%s",
                VTY_NEWLINE, VTY_NEWLINE);
    vty_out(vty, "  ---------- ---------- ---------- ----------%s",
            VTY_NEWLINE);
    vty_out(vty, "  %-10d %-10d %-10d %-10d%s",
            dhcpv6_relay_pkt_counters.v6client_valids,
            dhcpv6_relay_pkt_counters.v6client_drops,
            dhcpv6_relay_pkt_counters.v6serv_valids,
            dhcpv6_relay_pkt_counters.v6serv_drops, VTY_NEWLINE);

    return CMD_SUCCESS;
}

/*-----------------------------------------------------------------------------
| Function       : dhcpv6_relay_config
| Responsibility : To configure dhcpv6-relay and option 79.
| Parameters     :
|     status     : If true, enable dhcpv6-relay and option 79 and
|                  if false disable the dhcpv6-relay and option 79
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
dhcpv6_relay_config (const char *status, DHCPV6_FEATURE type)
{
    const struct ovsrec_system *ovs_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    struct smap smap_status_value;
    enum ovsdb_idl_txn_status txn_status;
    char *key = NULL;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    ovs_row = ovsrec_system_first(idl);
    if (!ovs_row) {
        VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    smap_clone(&smap_status_value, &ovs_row->dhcp_config);

    if (type == GLOBAL_CONFIG)
        key = SYSTEM_DHCP_CONFIG_MAP_V6RELAY_ENABLED;
    else
        key = SYSTEM_DHCP_CONFIG_MAP_V6RELAY_OPTION79_ENABLED;

    /* Update the latest config status. */
    smap_replace(&smap_status_value, key, status);

    ovsrec_system_set_dhcp_config(ovs_row, &smap_status_value);
    smap_destroy(&smap_status_value);
    txn_status = cli_do_config_finish(status_txn);

    if (txn_status == TXN_SUCCESS || txn_status == TXN_UNCHANGED)
    {
        return CMD_SUCCESS;
    }
    else
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
        return CMD_OVSDB_FAILURE;
    }
}

/*-----------------------------------------------------------------------------
| Defun for dhcpv6-relay configuration
| Responsibility: Enable dhcpv6-relay
-----------------------------------------------------------------------------*/
DEFUN(dhcpv6_relay_configuration,
      dhcpv6_relay_configuration_cmd,
      "dhcpv6-relay",
      DHCPV6_RELAY_STR)
{
    return dhcpv6_relay_config("true", GLOBAL_CONFIG);
}
/*-----------------------------------------------------------------------------
| Defun for dhcpv6-relay unconfiguration
| Responsibility: Disable dhcpv6-relay
-----------------------------------------------------------------------------*/
DEFUN(no_dhcpv6_relay_configuration,
      no_dhcpv6_relay_configuration_cmd,
      "no dhcpv6-relay",
      NO_STR
      DHCPV6_RELAY_STR)
{
    return dhcpv6_relay_config("false", GLOBAL_CONFIG);
}

/*-----------------------------------------------------------------------------
| Defun for dhcpv6-relay option 79 configuration
| Responsibility: Enable dhcpv6-relay option 79
-----------------------------------------------------------------------------*/
DEFUN(dhcpv6_relay_option_configuration,
      dhcpv6_relay_option_configuration_cmd,
      "dhcpv6-relay option 79",
      DHCPV6_RELAY_STR
      DHCPV6_RELAY_OPTION_STR
      OPTION_79_STR)
{
    return dhcpv6_relay_config("true", OPTION79_CONFIG);
}

/*-----------------------------------------------------------------------------
| Defun for dhcpv6-relay option 79 unconfiguration
| Responsibility: Disable dhcpv6-relay option 79
-----------------------------------------------------------------------------*/
DEFUN(no_dhcpv6_relay_option_configuration,
      no_dhcpv6_relay_option_configuration_cmd,
      "no dhcpv6-relay option 79",
      NO_STR
      DHCPV6_RELAY_STR
      DHCPV6_RELAY_OPTION_STR
      OPTION_79_STR)
{
    return dhcpv6_relay_config("false", OPTION79_CONFIG);
}

/*-----------------------------------------------------------------------------
| Defun for show dhcpv6-relay
| Responsibility: Displays the dhcpv6-relay configuration
-----------------------------------------------------------------------------*/
DEFUN(show_dhcpv6_relay_configuration,
      show_dhcpv6_relay_configuration_cmd,
      "show dhcpv6-relay {interface (IFNAME | A.B)}",
      SHOW_STR
      SHOW_DHCPV6_RELAY_STR
      INTERFACE_STR
      IFNAME_STR
      SUBIFNAME_STR)
{
    return show_dhcpv6_relay_config();
}

/*-----------------------------------------------------------------------------
| Function       : dhcpv6_relay_ovsdb_init
| Responsibility : Initialise dhcpv6-relay table details.
| Parameters     :
|      idl       : Pointer to idl structure
-----------------------------------------------------------------------------*/
static void
dhcpv6_relay_ovsdb_init(void)
{
    ovsdb_idl_add_table(idl, &ovsrec_table_system);
    ovsdb_idl_add_column(idl, &ovsrec_system_col_dhcp_config);
    ovsdb_idl_add_table(idl, &ovsrec_table_port);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_dhcp_relay_statistics);
    ovsdb_idl_add_table(idl, &ovsrec_table_dhcp_relay);
    ovsdb_idl_add_column(idl, &ovsrec_dhcp_relay_col_port);
    ovsdb_idl_add_column(idl, &ovsrec_dhcp_relay_col_vrf);
    ovsdb_idl_add_column(idl, &ovsrec_dhcp_relay_col_other_config);

    return;
}
/* Install dhcpv6-relay related vty commands */
void
cli_pre_init(void)
{
    vtysh_ret_val dhcpv6_retval = e_vtysh_error;

    dhcpv6_relay_ovsdb_init();

    /* dhcpv6-relay */
    dhcpv6_retval = install_show_run_config_context(e_vtysh_dhcpv6_relay_context,
                                     &vtysh_dhcpv6_relay_context_clientcallback,
                                     NULL, NULL);
    if(e_vtysh_ok != dhcpv6_retval)
    {
       vtysh_ovsdb_config_logmsg(VTYSH_OVSDB_CONFIG_ERR,
                           "dhcpv6-relay context unable "\
                           "to add config callback");
        assert(0);
    }
    return;
}

/* Install dhcpv6-relay related vty commands */
void
cli_post_init(void)
{
    install_element (CONFIG_NODE, &dhcpv6_relay_configuration_cmd);
    install_element(CONFIG_NODE, &dhcpv6_relay_option_configuration_cmd);
    install_element (CONFIG_NODE, &no_dhcpv6_relay_configuration_cmd);
    install_element(CONFIG_NODE, &no_dhcpv6_relay_option_configuration_cmd);
    install_element (ENABLE_NODE, &show_dhcpv6_relay_configuration_cmd);

    return;
}
