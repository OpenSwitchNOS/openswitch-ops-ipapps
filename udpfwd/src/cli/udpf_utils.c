/* UDP Broadcast Forwarder Utility functions.
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
 * File: udpf_utils.c
 *
 * Purpose:  To provide the UDP broadcast forwarder and dhcp-relay
 *           utility functions.
 */
#include "udpf_utils.h"

static const struct ovsrec_vrf* udp_bcast_config_vrf_lookup(const char *);
extern struct ovsdb_idl *idl;
VLOG_DEFINE_THIS_MODULE(udpf_utils);

/*-----------------------------------------------------------------------------
| Function         : udp_bcast_config_vrf_lookup
| Responsibility   : to lookup for the default VRF.
| Parameters       :
|        *vrf_name : VRF name that needs to be looked up.
| Return           : On success returns the VRF row,
|                    On failure returns NULL
-----------------------------------------------------------------------------*/
static const struct
ovsrec_vrf* udp_bcast_config_vrf_lookup (const char *vrf_name)
{
    const struct ovsrec_vrf *vrf_row = NULL;
    OVSREC_VRF_FOR_EACH (vrf_row, idl)
    {
        if (strcmp(vrf_row->name, vrf_name) == 0) {
            return vrf_row;
        }
    }
    return NULL;
}

/*-----------------------------------------------------------------------------
| Function         : decode_server_param
| Responsibility   : validates the user input parameters
|                    for the  UDP forward-protocol and helper-address CLI
| Parameters       :
|        *udpfServ : Pointer containing user input details
|        *argv     : argv from the CLI
|        type      : determines the forwarder protocol
| Return           : On success returns true,
|                    On failure returns false
-----------------------------------------------------------------------------*/
bool
decode_server_param (udpf_server *udpfServer, const char *argv[],
                     config_type type)
{
    struct in_addr addr;
    bool validParams = false;

    memset (&addr, 0, sizeof (struct in_addr));

    /* Validate protocol server IP. */
    if (inet_pton (AF_INET, (char*)argv[0], &addr)<= 0)
    {
        vty_out(vty, "Invalid IPv4 address.%s", VTY_NEWLINE);
        return validParams;
    }

    if (!IS_VALID_IPV4(htonl(addr.s_addr)))
    {
        vty_out(vty,
                "Broadcast, multicast and loopback addresses "
                "are not allowed.%s",
                VTY_NEWLINE);
        return validParams;
    }

    udpfServer->ipAddr = (char *)argv[0];

    if (type == DHCP_RELAY)
        return true;
    return true;
}

/*-----------------------------------------------------------------------------
| Function       : udpFwd_globalConfig
| Responsibility : To enable/disable udp broadcast forwarding and dhcp-relay
| Parameters     :
|         enable : If true, enable UDP Broadcast Forwarder and if false
|                  disable the UDP Broadcast Forwarder
|        type    : determines the forwarder protocol
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
udpFwd_globalConfig (const char *enable, config_type type)
{
    const struct ovsrec_system *ovs_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    struct smap smap_status_value;
    enum ovsdb_idl_txn_status txn_status;
    char *state_value = NULL, *keyValue = NULL;

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

    /* Identify if the operation is for dhcp-relay. */
    if (type == DHCP_RELAY)
        keyValue = SYSTEM_OTHER_CONFIG_MAP_DHCP_RELAY_DISABLED;

    smap_clone(&smap_status_value, &ovs_row->other_config);
    state_value = (char*)smap_get(&ovs_row->other_config,
                                  keyValue);

    if(state_value == NULL)
    {
        smap_add(&smap_status_value,
                 keyValue, enable);
    }
    else
    {
        smap_replace(&smap_status_value,
                     keyValue, enable);
    }

    ovsrec_system_set_other_config(ovs_row, &smap_status_value);
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
| Function         : udpFwd_serverUpdate
| Responsibility   : To update the server IP information to Dhcp-relay table
| Parameters       :
|        row_serv  : Dhcp-relay row
|        action    : Operation to be performed add/delete the server IP
|        *udpfServ : Pointer containing user input details
-----------------------------------------------------------------------------*/
void
udpFwd_serverUpdate (const struct
                     ovsrec_dhcp_relay *row_serv,
                     int8_t action, udpf_server *udpfServ, char **servers)
{
    size_t i, n;

    if (action == 1)
    {
        /* add a server IP to dhcp-relay table */
        for (i = 0; i < row_serv->n_ipv4_ucast_server; i++)
            servers[i] = row_serv->ipv4_ucast_server[i];

        servers[row_serv->n_ipv4_ucast_server] = udpfServ->ipAddr;
        ovsrec_dhcp_relay_set_ipv4_ucast_server(row_serv,
                                servers, row_serv->n_ipv4_ucast_server+1);
    }
    else
    {
        /* delete the server IP from dhcp-relay table */
        for (i = n = 0; i < row_serv->n_ipv4_ucast_server; i++)
        {
            if (strcmp (udpfServ->ipAddr,
                        row_serv->ipv4_ucast_server[i]) != 0)
                servers[n++] = row_serv->ipv4_ucast_server[i];
        }
        ovsrec_dhcp_relay_set_ipv4_ucast_server (row_serv, servers, n);
    }
    free(servers);
    return;
}

/*-----------------------------------------------------------------------------
| Function         : udpFwd_serverConfig
| Responsibility   : set/unset UDP forward-protocol and dhcp-relay
|                    helper-address.
| Parameters       :
|        *udpfServ : Pointer containing user input details
|        set       : Flag to set or unset
| Return           : On success returns CMD_SUCCESS,
|                    On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
udpFwd_serverConfig (udpf_server *udpfServ, bool set)
{
    const struct ovsrec_dhcp_relay *row_serv = NULL;
    const struct ovsrec_port *port_row = NULL;
    const struct ovsrec_vrf *vrf_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status txn_status;
    char **server_addresses;
    size_t i, entries = 1;
    bool isAddrMatch = false;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    OVSREC_DHCP_RELAY_FOR_EACH (row_serv, idl)
    {
        if(strcmp(row_serv->port->name, (char*)vty->index) == 0)
        {
            /* set action. */
            server_addresses =
                              xmalloc (IP_ADDRESS_LENGTH *
                                        (row_serv->n_ipv4_ucast_server + 1));
            if(strcmp(row_serv->vrf->name, DEFAULT_VRF_NAME) == 0)
            {
                for (i = 0; i < row_serv->n_ipv4_ucast_server; i++)
                {
                    if (!strcmp (udpfServ->ipAddr,
                                 row_serv->ipv4_ucast_server[i]))
                    {
                        isAddrMatch = true;
                        break;
                    }
                }
                if (set)
                {
                    /* Verify if the entries on an interface has
                       exceeded the limit. */
                    entries += row_serv->n_ipv4_ucast_server;
                    if(entries > MAX_PROTOCOL_ADDR)
                    {
                        vty_out(vty, "%s: Entry not allowed as maximum "
                                     "entries per interface exceeded.%s",
                                     udpfServ->ipAddr, VTY_NEWLINE);
                        cli_do_config_abort(status_txn);
                        return CMD_SUCCESS;
                    }

                    if (!isAddrMatch)
                    {
                        /* set action. */
                        udpFwd_serverUpdate(row_serv, 1, udpfServ, server_addresses);
                        break;
                    }
                    else
                    {
                        /* Existing entry. */
                        vty_out(vty, "This entry already exists.%s",
                                     VTY_NEWLINE);
                        cli_do_config_abort(status_txn);
                        return CMD_SUCCESS;

                    }
                }
                else
                {
                    if (!isAddrMatch)
                    {
                        vty_out(vty, "No such entries are present. %s",
                                     VTY_NEWLINE);
                        cli_do_config_abort(status_txn);
                        return CMD_SUCCESS;
                    }
                    else
                    {
                        /* no set action. */
                        server_addresses =
                                xmalloc (IP_ADDRESS_LENGTH *
                                        (row_serv->n_ipv4_ucast_server - 1));
                        udpFwd_serverUpdate(row_serv, 0, udpfServ, server_addresses);
                        /* If this is the last entry then after unset remove
                           the complete row. */
                        if (row_serv->n_ipv4_ucast_server == 0)
                            ovsrec_dhcp_relay_delete(row_serv);
                        break;
                    }
                }
            }
        }
    }

    if (!row_serv)
    {
        if (set)
        {
            /* first set of UDP forward-protocol
               or dhcp-relay helper-address. */
            row_serv = ovsrec_dhcp_relay_insert(status_txn);
            if (!row_serv)
            {
                VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }

            /* Update the interface name. */
            OVSREC_PORT_FOR_EACH(port_row, idl)
            {
                if(strcmp(port_row->name, (char*)vty->index) == 0)
                {
                    ovsrec_dhcp_relay_set_port
                                          (row_serv, port_row);
                    break;
                }
            }

            /* Update the vrf name of the port. */
            vrf_row = udp_bcast_config_vrf_lookup(DEFAULT_VRF_NAME);
            if (!vrf_row)
            {
                vty_out(vty, "Error: Could not fetch default VRF data.%s",
                     VTY_NEWLINE);
                VLOG_ERR("%s VRF table did not have any rows. Ideally it "
                     "should have just one entry.", __func__);
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }

            if(strcmp(vrf_row->name, DEFAULT_VRF_NAME) == 0)
            {
                ovsrec_dhcp_relay_set_vrf(row_serv, vrf_row);
            }

            /* Update the protocol server IP. */
            server_addresses =
                        xmalloc(IP_ADDRESS_LENGTH *
                               (row_serv->n_ipv4_ucast_server + 1));
            udpFwd_serverUpdate(row_serv, 1, udpfServ, server_addresses);
        }
        else
        {
            vty_out(vty, "No such entries are present. %s",
                         VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }
    }

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
| Responsibility : To show the UDP Broadcast Forwarder and dhcp-relay
|                  helper-address configurations.
| Parameters     :
|       *ifname  : Interface name
|        type    : determines the forwarder protocol
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
show_udp_forwarder_configuration (const char *ifname, config_type type)
{
    const struct ovsrec_dhcp_relay *row_serv = NULL;
    const struct ovsrec_vrf *vrf_row = NULL;
    char *fwd_server_ip = NULL;
    size_t i = 0;

    vrf_row = ovsrec_vrf_first(idl);

    if (!vrf_row)
    {
        vty_out(vty, "Error: Could not fetch default VRF data.%s",
                     VTY_NEWLINE);
        VLOG_ERR("%s VRF table did not have any rows. Ideally it "
                 "should have just one entry.", __func__);
        return CMD_SUCCESS;
    }

    if (type == DHCP_RELAY)
    {
        vty_out(vty, " IP Helper Addresses%s %s", VTY_NEWLINE, VTY_NEWLINE);
        if (ifname)
            {
                vty_out(vty, " Interface: %s%s", ifname,
                                                 VTY_NEWLINE);
                vty_out(vty, "%2sIP Helper Address%s", "",
                                                 VTY_NEWLINE);
                vty_out(vty, "%2s-----------------%s", "",
                                                 VTY_NEWLINE);
            }
    }

    row_serv = ovsrec_dhcp_relay_first(idl);
    if (!row_serv)
    {
        return CMD_SUCCESS;
    }

    OVSREC_DHCP_RELAY_FOR_EACH (row_serv, idl)
    {
        /* get the interface details. */
        if(row_serv->port)
        {
            if (ifname)
            {
                if(strcmp(row_serv->port->name, ifname) == 0)
                {
                    if(row_serv->n_ipv4_ucast_server)
                    {
                        for (i = 0; i < row_serv->n_ipv4_ucast_server; i++)
                        {
                            /* get the configured server IP.*/
                            fwd_server_ip = row_serv->ipv4_ucast_server[i];
                            vty_out(vty, "%2s%s%s", "", fwd_server_ip,
                                         VTY_NEWLINE);
                        }
                    }
                    else
                    {
                        return CMD_SUCCESS;
                    }

                }
            }
            else
            {
                vty_out(vty, " Interface: %s%s",
                                    row_serv->port->name, VTY_NEWLINE);
                vty_out(vty, "%2sIP Helper Address%s", "", VTY_NEWLINE);
                vty_out(vty, "%2s-----------------%s", "", VTY_NEWLINE);
                for (i = 0; i < row_serv->n_ipv4_ucast_server; i++)
                {
                    /* get the configured server IP.*/
                    fwd_server_ip = row_serv->ipv4_ucast_server[i];
                    vty_out(vty, "%2s%s%s%s", "", fwd_server_ip,
                                 VTY_NEWLINE, VTY_NEWLINE);
                }
            }
        }
    }
    return CMD_SUCCESS;
}
