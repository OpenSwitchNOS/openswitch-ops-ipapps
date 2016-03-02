/* UDP Broadcast Forwarder CLI commands
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
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * File: udpfwd_vty.c
 *
 * Purpose:  To add UDP Broadcast Forwarder CLI configuration and
 *           display commands.
 */
#include "udpfwd_vty.h"

extern struct ovsdb_idl *idl;
VLOG_DEFINE_THIS_MODULE(vtysh_udpfwd_cli);

/*-----------------------------------------------------------------------------
| Defun for UDP broadcast forwarding
| Responsibility: UDP broadcast forwarding enable
-----------------------------------------------------------------------------*/
DEFUN ( cli_udp_bcast_fwd_enable,
        cli_udp_bcast_fwd_enable_cmd,
        "ip udp-bcast-forward",
        IP_STR
        UDPFWD_BCAST_STR )
{
      udpfwd_feature type = UDP_BCAST_FWD;
      return udpfwd_globalconfig(TRUE, type);
}

/*-----------------------------------------------------------------------------
| Defun for UDP broadcast forwarding
| Responsibility: UDP broadcast forwarding disable
-----------------------------------------------------------------------------*/
DEFUN ( cli_udp_bcast_fwd_disable,
        cli_udp_bcast_fwd_disable_cmd,
        "no ip udp-bcast-forward",
        NO_STR
        IP_STR
        UDPFWD_BCAST_STR )
{
      udpfwd_feature type = UDP_BCAST_FWD;
      return udpfwd_globalconfig(FALSE, type);
}

/*-----------------------------------------------------------------------------
| Defun UDP forward-protocol configuration
| Responsibility: Set UDP forward-protocol config
-----------------------------------------------------------------------------*/
DEFUN ( cli_udpf_bcast_set_proto,
        intf_set_udpf_proto_cmd,
        "ip forward-protocol udp A.B.C.D (INTEGER | WORD)",
        IP_STR
        UDPFWD_PROTO_STR
        UDPFWD_PROTO_STR
        UDPFWD_SERV_ADDR_STR
        UDPFWD_PORT_NUM_STR
        UDPFWD_PROTO_NAME_STR )
{
    udpfwd_server udpfServer;
    udpfwd_feature type = UDP_BCAST_FWD;
    memset(&udpfServer, 0, sizeof(udpfwd_server));

    /* Validate the input parameters. */
    if (decode_server_param(&udpfServer, argv, type))
    {
        return udpfwd_serverconfig(&udpfServer,
                                    SET);
    }
    else
    {
        return CMD_SUCCESS;
    }
}

/*-----------------------------------------------------------------------------
| Defun UDP forward-protocol configuration
| Responsibility: Unset UDP forward-protocol config
-----------------------------------------------------------------------------*/
DEFUN ( cli_udpf_bcast_no_set_proto,
        intf_no_set_udpf_proto_cmd,
        "no ip forward-protocol udp A.B.C.D (INTEGER | WORD)",
        NO_STR
        IP_STR
        UDPFWD_PROTO_STR
        UDPFWD_PROTO_STR
        UDPFWD_SERV_ADDR_STR
        UDPFWD_PORT_NUM_STR
        UDPFWD_PROTO_NAME_STR )
{
    udpfwd_server udpfServer;
    udpfwd_feature type = UDP_BCAST_FWD;
    memset(&udpfServer, 0, sizeof(udpfwd_server));

    /* Validate the input parameters. */
    if (decode_server_param(&udpfServer, argv, type))
    {
        return udpfwd_serverconfig(&udpfServer,
                                    UNSET);
    }
    else
    {
        return CMD_SUCCESS;
    }
}

/*-----------------------------------------------------------------------------
| Defun show ip forward-protocol
| Responsibility: Show the UDP broadcast forward configurations
-----------------------------------------------------------------------------*/
DEFUN ( cli_show_udpf_forward_protocol,
        cli_show_udpf_forward_protocol_cmd,
        "show ip forward-protocol {interface (IFNAME | A.B)}",
        SHOW_STR
        IP_STR
        UDPFWD_PROTO_STR
        UDPFWD_INT_STR
        UDPFWD_IFNAME_STR
        SUBIFNAME_STR )
{
    udpfwd_feature type = UDP_BCAST_FWD;
    if (argv[0])
    {
        return show_udp_forwarder_configuration(argv[0], type);
    }
    else
    {
        return show_udp_forwarder_configuration(NULL, type);
    }
}

/*******************************************************************
 * @func        : udpfwd_vty_init
 * @detail      : Add UDP Broadcast Forwarder related table & columns
 *                to ops-cli idl cache
 *******************************************************************/
static void
udpfwd_vty_init(void)
{
    ovsdb_idl_add_table(idl, &ovsrec_table_udp_bcast_forwarder_server);
    ovsdb_idl_add_column(idl, &ovsrec_udp_bcast_forwarder_server_col_src_port);
    ovsdb_idl_add_column(idl, &ovsrec_udp_bcast_forwarder_server_col_dest_vrf);
    ovsdb_idl_add_column(idl,
                &ovsrec_udp_bcast_forwarder_server_col_udp_dport);
    ovsdb_idl_add_column(idl,
                &ovsrec_udp_bcast_forwarder_server_col_ipv4_ucast_server);

    return;
}

/* Initialize udpfwd cli node.
 */
void
cli_pre_init (void)
{
    vtysh_ret_val retval = e_vtysh_error;

    udpfwd_vty_init();
    retval = install_show_run_config_context
                        (e_vtysh_udp_forwarder_context,
                         &vtysh_udp_forwarder_context_clientcallback,
                         NULL, NULL);
    if (e_vtysh_ok != retval)
    {
        vtysh_ovsdb_config_logmsg(VTYSH_OVSDB_CONFIG_ERR,
                           "UDP  Broadcast Forwarder context unable "\
                           "to add config callback");
        assert(0);
    }

    return;
}

/* Initialize udpfwd cli element.
 */
void cli_post_init(void)
{
    install_element(CONFIG_NODE, &cli_udp_bcast_fwd_enable_cmd);
    install_element(CONFIG_NODE, &cli_udp_bcast_fwd_disable_cmd);
    install_element(INTERFACE_NODE, &intf_set_udpf_proto_cmd);
    install_element(INTERFACE_NODE, &intf_no_set_udpf_proto_cmd);
    install_element(SUB_INTERFACE_NODE, &intf_set_udpf_proto_cmd);
    install_element(SUB_INTERFACE_NODE, &intf_no_set_udpf_proto_cmd);
    install_element(ENABLE_NODE, &cli_show_udpf_forward_protocol_cmd);

    return;
}
