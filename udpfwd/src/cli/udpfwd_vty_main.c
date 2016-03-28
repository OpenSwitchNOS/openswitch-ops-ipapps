/* udpfwd CLI commands
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
 * File: udpfwd_main_vty.c
 *
 * Purpose:  To initialise the tables required for udpfwd.
 */

#include <stdint.h>
#include "vtysh/command.h"
#include "dhcp_relay_vty.h"
#include "udpfwd_vty.h"
#include "vtysh_ovsdb_udpfwd_context.h"
#include "vtysh_ovsdb_dhcp_relay_context.h"

extern struct ovsdb_idl *idl;
VLOG_DEFINE_THIS_MODULE(vtysh_udpfwd_cli);

/*-----------------------------------------------------------------------------
| Function       : dhcp_relay_ovsdb_init
| Responsibility : Initialise dhcp-relay table details.
| Parameters     :
|      idl       : Pointer to idl structure
-----------------------------------------------------------------------------*/
static void
dhcp_relay_ovsdb_init(void)
{
    ovsdb_idl_add_table(idl, &ovsrec_table_system);
    ovsdb_idl_add_column(idl, &ovsrec_system_col_dhcp_config);
    ovsdb_idl_add_table(idl, &ovsrec_table_dhcp_relay);
    ovsdb_idl_add_column(idl, &ovsrec_dhcp_relay_col_port);
    ovsdb_idl_add_column(idl, &ovsrec_dhcp_relay_col_vrf);
    ovsdb_idl_add_column(idl, &ovsrec_dhcp_relay_col_ipv4_ucast_server);
    ovsdb_idl_add_column(idl, &ovsrec_dhcp_relay_col_other_config);

    return;
}

/*******************************************************************
| Function       : udpfwd_ovsdb_init
| Responsibility : Initialise UDP broadcast server table details.
| Parameters     :
|      idl       : Pointer to idl structure
 *******************************************************************/
static void
udpfwd_ovsdb_init(void)
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

/* Install dhcp-relay and UDP fowarder related vty commands */
void
cli_pre_init(void)
{
    vtysh_ret_val dhcp_retval = e_vtysh_error, udp_retval = e_vtysh_error;

    /* dhcp-relay */
    dhcp_relay_ovsdb_init();
    dhcp_retval = install_show_run_config_context(e_vtysh_dhcp_relay_context,
                                     &vtysh_dhcp_relay_context_clientcallback,
                                     NULL, NULL);
    if(e_vtysh_ok != dhcp_retval)
    {
       vtysh_ovsdb_config_logmsg(VTYSH_OVSDB_CONFIG_ERR,
                           "dhcp-relay context unable "\
                           "to add config callback");
        assert(0);
    }

    /* UDP forwarder */
    udpfwd_ovsdb_init();
    udp_retval = install_show_run_config_context
                        (e_vtysh_udp_forwarder_context,
                         &vtysh_udp_forwarder_context_clientcallback,
                         NULL, NULL);
    if (e_vtysh_ok != udp_retval)
    {
        vtysh_ovsdb_config_logmsg(VTYSH_OVSDB_CONFIG_ERR,
                           "UDP  Broadcast Forwarder context unable "\
                           "to add config callback");
        assert(0);
    }
    return;
}

/* Install dhcp-relay and UDP forwarder related vty commands */
void
cli_post_init(void)
{
    /* dhcp-relay */
    install_element (CONFIG_NODE, &dhcp_relay_configuration_cmd);
    install_element(CONFIG_NODE, &dhcp_relay_options_configuration_cmd);
    install_element(CONFIG_NODE, &dhcp_relay_keep_option_configuration_cmd);
    install_element(CONFIG_NODE, &dhcp_relay_drop_option_configuration_cmd);
    install_element(CONFIG_NODE,
                    &dhcp_relay_drop_validate_option_configuration_cmd);
    install_element(CONFIG_NODE,
                    &dhcp_relay_drop_remote_id_mac_option_configuration_cmd);
    install_element(CONFIG_NODE,
                    &dhcp_relay_drop_remote_id_ip_option_configuration_cmd);
    install_element(CONFIG_NODE,
                    &dhcp_relay_replace_option_configuration_cmd);
    install_element(CONFIG_NODE,
                    &dhcp_relay_replace_validate_option_configuration_cmd);
    install_element(CONFIG_NODE,
            &dhcp_relay_replace_remote_id_mac_option_configuration_cmd);
    install_element(CONFIG_NODE,
                    &dhcp_relay_replace_remote_id_ip_option_configuration_cmd);
    install_element(CONFIG_NODE,
                    &dhcp_relay_validate_policy_option_configuration_cmd);
    install_element(CONFIG_NODE,
                    &dhcp_relay_validate_drop_option_configuration_cmd);
    install_element(CONFIG_NODE,
                    &dhcp_relay_validate_replace_option_configuration_cmd);
    install_element (CONFIG_NODE, &no_dhcp_relay_configuration_cmd);
    install_element(CONFIG_NODE, &no_dhcp_relay_option_configuration_cmd);
    install_element (INTERFACE_NODE, &ip_helper_address_configuration_cmd);
    install_element (INTERFACE_NODE, &no_ip_helper_address_configuration_cmd);
    install_element (SUB_INTERFACE_NODE, &ip_helper_address_configuration_cmd);
    install_element (SUB_INTERFACE_NODE,
                     &no_ip_helper_address_configuration_cmd);
    install_element(INTERFACE_NODE, &ip_bootp_gateway_configuration_cmd);
    install_element(INTERFACE_NODE, &no_ip_bootp_gateway_configuration_cmd);
    install_element(SUB_INTERFACE_NODE, &ip_bootp_gateway_configuration_cmd);
    install_element(SUB_INTERFACE_NODE,
                     &no_ip_bootp_gateway_configuration_cmd);
    install_element (ENABLE_NODE, &show_dhcp_relay_configuration_cmd);
    install_element (ENABLE_NODE, &show_ip_helper_address_configuration_cmd);
    install_element(ENABLE_NODE,
                    &show_dhcp_relay_bootp_gateway_configuration_cmd);

    /* UDP Forwarder */
    install_element(CONFIG_NODE, &cli_udp_bcast_fwd_enable_cmd);
    install_element(CONFIG_NODE, &cli_udp_bcast_fwd_disable_cmd);
    install_element(INTERFACE_NODE, &intf_set_udpf_proto_cmd);
    install_element(INTERFACE_NODE, &intf_no_set_udpf_proto_cmd);
    install_element(SUB_INTERFACE_NODE, &intf_set_udpf_proto_cmd);
    install_element(SUB_INTERFACE_NODE, &intf_no_set_udpf_proto_cmd);
    install_element(ENABLE_NODE, &cli_show_udpf_forward_protocol_cmd);

    return;
}
