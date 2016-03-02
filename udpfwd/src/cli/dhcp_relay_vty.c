/* dhcp-relay CLI commands
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
 * File: dhcp_relay_vty.c
 *
 * Purpose: To add dhcp-relay configuration CLI commands.
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
#include "dhcp_relay_vty.h"
#include "vtysh_ovsdb_dhcp_relay_context.h"
#include "udpfwd_vty_utils.h"

static int show_dhcp_relay_config (void);

extern struct ovsdb_idl *idl;
VLOG_DEFINE_THIS_MODULE(dhcp_relay_vty);

/*-----------------------------------------------------------------------------
| Function       : show_dhcp_relay_config
| Responsibility : To show the dhcp-relay configuration.
| Return         : Returns CMD_SUCCESS
-----------------------------------------------------------------------------*/
static int
show_dhcp_relay_config (void)
{
    const struct ovsrec_system *ovs_row = NULL;
    char *dhcp_relay_disabled = NULL;

    ovs_row = ovsrec_system_first(idl);
    if (ovs_row == NULL)
    {
        VLOG_ERR("%s SYSTEM table did not have any rows.%s",
                 VTY_NEWLINE, VTY_NEWLINE);
        return CMD_SUCCESS;
    }

    vty_out (vty, "\n DHCP Relay Agent :");
    dhcp_relay_disabled = (char *)smap_get(&ovs_row->other_config,
                             SYSTEM_OTHER_CONFIG_MAP_DHCP_RELAY_DISABLED);

    if (!dhcp_relay_disabled)
    {
        dhcp_relay_disabled = "enabled";
    }
    else
    {
        if (!strcmp(dhcp_relay_disabled, "false"))
            dhcp_relay_disabled = "enabled";
        else
            dhcp_relay_disabled = "disabled";
    }

    if (dhcp_relay_disabled)
    {
        vty_out(vty, "%s%s", dhcp_relay_disabled, VTY_NEWLINE);
    }
    return CMD_SUCCESS;
}

/*-----------------------------------------------------------------------------
| Function       : dhcp_relay_ovsdb_init
| Responsibility : Initialise dhcp-relay table tables.
| Parameters     :
|      idl       : Pointer to idl structure
-----------------------------------------------------------------------------*/
static void
dhcp_relay_ovsdb_init()
{
    ovsdb_idl_add_table(idl, &ovsrec_table_dhcp_relay);
    ovsdb_idl_add_column(idl, &ovsrec_dhcp_relay_col_port);
    ovsdb_idl_add_column(idl, &ovsrec_dhcp_relay_col_vrf);
    ovsdb_idl_add_column(idl, &ovsrec_dhcp_relay_col_ipv4_ucast_server);

    return;
}

/*-----------------------------------------------------------------------------
| Defun for dhcp-relay configuration
| Responsibility: Enable dhcp-relay
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_configuration,
      dhcp_relay_configuration_cmd,
      "dhcp-relay",
      DHCP_RELAY_STR)
{
    return udpfwd_globalconfig("false", DHCP_RELAY);
}

/*-----------------------------------------------------------------------------
| Defun for dhcp-relay unconfiguration
| Responsibility: Disable dhcp-relay
-----------------------------------------------------------------------------*/
DEFUN(no_dhcp_relay_configuration,
      no_dhcp_relay_configuration_cmd,
      "no dhcp-relay",
      NO_STR
      DHCP_RELAY_STR)
{
    return udpfwd_globalconfig("true", DHCP_RELAY);
}

/*-----------------------------------------------------------------------------
| Defun for show dhcp-relay
| Responsibility: Displays the dhcp-relay configuration
-----------------------------------------------------------------------------*/
DEFUN(show_dhcp_relay_configuration,
      show_dhcp_relay_configuration_cmd,
      "show dhcp-relay",
      SHOW_STR
      SHOW_DHCP_RELAY_STR)
{
     return show_dhcp_relay_config();
}

/*-----------------------------------------------------------------------------
| Defun ip helper-address configuration
| Responsibility: Set a helper-addresses for a dhcp-relay
-----------------------------------------------------------------------------*/
DEFUN(ip_helper_address_configuration,
      ip_helper_address_configuration_cmd,
      "ip helper-address A.B.C.D ",
      IP_STR
      HELPER_ADDRESS_STR
      HELPER_ADDRESS_INPUT_STR)
{
    udpfwd_server udpfwdServer;
    memset(&udpfwdServer, 0, sizeof(udpfwd_server));

    /* Validate the input parameters. */
    if (decode_server_param(&udpfwdServer, argv, DHCP_RELAY))
    {
        return udpfwd_serverconfig(&udpfwdServer, SET);
    }
    else
    {
        return CMD_SUCCESS;
    }

}

/*-----------------------------------------------------------------------------
| Defun ip helper-address unconfiguration
| Responsibility: Unset a helper-addresses for a dhcp-relay
-----------------------------------------------------------------------------*/
DEFUN(no_ip_helper_address_configuration,
      no_ip_helper_address_configuration_cmd,
      "no ip helper-address A.B.C.D ",
      NO_STR
      IP_STR
      HELPER_ADDRESS_STR
      HELPER_ADDRESS_INPUT_STR)
{
    udpfwd_server udpfwdServer;
    memset(&udpfwdServer, 0, sizeof(udpfwd_server));

    /* Validate the input parameters. */
    if (decode_server_param(&udpfwdServer, argv, DHCP_RELAY))
    {
        return udpfwd_serverconfig(&udpfwdServer, UNSET);
    }
    else
    {
        return CMD_SUCCESS;
    }
}

/*-----------------------------------------------------------------------------
| Defun for show ip helper-address
| Responsibility: Displays the helper-addresses of a dhcp-relay
-----------------------------------------------------------------------------*/
DEFUN(show_ip_helper_address_configuration,
      show_ip_helper_address_configuration_cmd,
      "show ip helper-address {interface (IFNAME | A.B )} ",
      SHOW_STR
      IP_STR
      SHOW_HELPER_ADDRESS_STR
      INTERFACE_STR
      IFNAME_STR
      SUBIFNAME_STR)
{
    if (argv[0])
    {
        return show_ip_helper_address_config(argv[0], DHCP_RELAY);
    }
    else
    {
        return show_ip_helper_address_config(NULL, DHCP_RELAY);
    }
}

/* Install dhcp-relay related vty commands */
void cli_pre_init(void)
{
    vtysh_ret_val retval = e_vtysh_error;

    dhcp_relay_ovsdb_init();

    retval = install_show_run_config_context(e_vtysh_dhcp_relay_context,
                                     &vtysh_dhcp_relay_context_clientcallback,
                                     NULL, NULL);
    if(e_vtysh_ok != retval)
    {
       vtysh_ovsdb_config_logmsg(VTYSH_OVSDB_CONFIG_ERR,
                           "dhcp-relay context unable "\
                           "to add config callback");
        assert(0);
    }
    return;
}

/* Install dhcp-relay related vty commands */
void
cli_post_init(void)
{
    install_element (CONFIG_NODE, &dhcp_relay_configuration_cmd);
    install_element (CONFIG_NODE, &no_dhcp_relay_configuration_cmd);
    install_element (INTERFACE_NODE, &ip_helper_address_configuration_cmd);
    install_element (INTERFACE_NODE, &no_ip_helper_address_configuration_cmd);
    install_element (SUB_INTERFACE_NODE, &ip_helper_address_configuration_cmd);
    install_element (SUB_INTERFACE_NODE, &no_ip_helper_address_configuration_cmd);
    install_element (ENABLE_NODE, &show_dhcp_relay_configuration_cmd);
    install_element (ENABLE_NODE, &show_ip_helper_address_configuration_cmd);
}
