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
 * File: udpfwd_utils.c
 *
 * Purpose:  To provide the UDP broadcast forwarder
 *           utility functions.
 */
#include "udpfwd_utils.h"

static const struct ovsrec_vrf* udp_bcast_config_vrf_lookup(const char *);
extern struct ovsdb_idl *idl;
VLOG_DEFINE_THIS_MODULE(vtysh_udpfwd_utils);

/* Supported UDP protocols. */
udpProtocols
udp_protocol[MAX_UDP_PROTOCOL] = {
                                     { "dns", 53},
                                     { "ntp", 123},
                                     { "netbios-ns", 137},
                                     { "netbios-dgm", 138},
                                     { "radius", 1812},
                                     { "radius-old", 1645},
                                     { "rip", 520},
                                     { "snmp", 161},
                                     { "snmp-trap", 162},
                                     { "tftp", 69},
                                     { "timep", 37}
                                 };

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
|                    for the UDP forward-protocol CLI
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
    int pNum, i;
    char *pName;
    struct in_addr addr;
    bool validParams = false;

    memset (&addr, 0, sizeof (struct in_addr));

    /* Validate protocol server IP. */
    if (inet_pton (AF_INET, (char*)argv[0], &addr)<= 0)
    {
        vty_out(vty, "Invalid IPv4 address.%s", VTY_NEWLINE);
        return validParams;
    }

    if (!IS_VALID_IPV4(htonl(addr.s_addr)) )
    {
        if (IS_BROADCAST_IPV4(htonl(addr.s_addr)))
        {
            vty_out(vty,
                "Broadcast, multicast and loopback addresses "
                "are not allowed.%s",
                VTY_NEWLINE);
            return validParams;
        }
        else if (!IS_SUBNET_BROADCAST(htonl(addr.s_addr)))
        {
            vty_out(vty,
                "Broadcast, multicast and loopback addresses "
                "are not allowed.%s",
                VTY_NEWLINE);
            return validParams;
        }
    }

    udpfServer->ipAddr = (char *)argv[0];

    if (type == DHCP_RELAY)
        return true;

    /* Validate UDP port info. */
    if (isalpha((int) *argv[1]))
    {
        pName = (char*)argv[1];
        for (i = 0; i < MAX_UDP_PROTOCOL; i++)
        {
            if(!strcmp(pName, udp_protocol[i].name))
            {
                udpfServer->udpPort = udp_protocol[i].number;
                validParams = true;
                break;
            }
        }
    }
    else
    {
        pNum = atoi(argv[1]);
        for (i = 0; i < MAX_UDP_PROTOCOL; i++)
        {
            if(pNum == udp_protocol[i].number)
            {
                udpfServer->udpPort = pNum;
                validParams = true;
                break;
            }
        }
    }

    if (!validParams)
    {
        vty_out(vty, "Invalid UDP portname/portnumber entered. %s",
                        VTY_NEWLINE);
        VLOG_ERR("Invalid UDP portname/portnumber entered.");
    }

    return validParams;
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

    /* Identify if the operation is for UDP Bcast Forwarding */
    if (type == UDP_BCAST_FWD)
        keyValue = SYSTEM_OTHER_CONFIG_MAP_UDP_BCAST_FWD_ENABLED;

    smap_clone(&smap_status_value, &ovs_row->other_config);
    state_value = (char*)smap_get(&ovs_row->other_config,
                                  keyValue);

    if(state_value == NULL)
    {
        if (!strcmp(enable, TRUE))
            smap_add(&smap_status_value,
                        keyValue, TRUE);
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
| Responsibility   : To update the server IP information to UDP Server table
| Parameters       :
|        row_serv  : UDP Bcast Forwarder Server row
|        action    : Operation to be performed add/delete the server IP
|        *udpfServ : Pointer containing user input details
-----------------------------------------------------------------------------*/
void
udpFwd_serverUpdate (const struct
                     ovsrec_udp_bcast_forwarder_server *row_serv,
                     int8_t action, udpf_server *udpfServ)
{
    char **servers;
    size_t i, n, server_length;

    if (action == 1)
    {
        server_length = row_serv->n_ipv4_ucast_server + 1;
        servers = xmalloc(IP_ADDRESS_LENGTH*server_length);
        /* add a server IP to UDP Bcast Forwarder table */
        for (i = 0; i < row_serv->n_ipv4_ucast_server; i++)
            servers[i] = row_serv->ipv4_ucast_server[i];

        servers[row_serv->n_ipv4_ucast_server] = udpfServ->ipAddr;
        ovsrec_udp_bcast_forwarder_server_set_ipv4_ucast_server(row_serv,
                                servers, row_serv->n_ipv4_ucast_server+1);
    }
    else
    {
        server_length = row_serv->n_ipv4_ucast_server - 1;
        servers = xmalloc(IP_ADDRESS_LENGTH*server_length);
        /* delete the server IP from UDP Bcast Forwarder table */
        for (i = n = 0; i < row_serv->n_ipv4_ucast_server; i++)
        {
            if (strcmp (udpfServ->ipAddr,
                        row_serv->ipv4_ucast_server[i]) != 0)
                servers[n++] = row_serv->ipv4_ucast_server[i];
        }
        ovsrec_udp_bcast_forwarder_server_set_ipv4_ucast_server
                            (row_serv, servers, n);
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
    const struct ovsrec_udp_bcast_forwarder_server *row_serv = NULL;
    const struct ovsrec_vrf *vrf_row = NULL;
    const struct ovsrec_port *port_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status txn_status;
    size_t i;
    bool isAddrMatch = false;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    OVSREC_UDP_BCAST_FORWARDER_SERVER_FOR_EACH (row_serv, idl)
    {
        if(!strcmp(row_serv->src_port->name, (char*)vty->index)
           && !strcmp(row_serv->dest_vrf->name, DEFAULT_VRF_NAME))
        {
            if (row_serv->udp_dport == udpfServ->udpPort)
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
                    if (!isAddrMatch)
                    {
                        /* set action. */
                        udpFwd_serverUpdate(row_serv, 1, udpfServ);
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
                        udpFwd_serverUpdate(row_serv, 0, udpfServ);
                        /* If this is the last entry then after unset remove
                           the complete row. */
                        if (row_serv->n_ipv4_ucast_server == 0)
                            ovsrec_udp_bcast_forwarder_server_delete(row_serv);
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
            row_serv = ovsrec_udp_bcast_forwarder_server_insert(status_txn);
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
                    ovsrec_udp_bcast_forwarder_server_set_src_port
                                          (row_serv, port_row);
                    break;
                }
            }
            if (!port_row)
            {
                return CMD_OVSDB_FAILURE;
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
            ovsrec_udp_bcast_forwarder_server_set_dest_vrf(row_serv, vrf_row);

            /* Update the dst udp port. */
            ovsrec_udp_bcast_forwarder_server_set_udp_dport
                        (row_serv, udpfServ->udpPort);
            /* Update the protocol server IP. */
            udpFwd_serverUpdate(row_serv, 1, udpfServ);
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
    const struct ovsrec_udp_bcast_forwarder_server *row_serv = NULL;
    const struct ovsrec_system *ovs_row = NULL;
    const struct ovsdb_datum *datum = NULL;
    char *udp_status = NULL, *fwd_server_ip = NULL;
    int index = 0;
    size_t i = 0;

    ovs_row = ovsrec_system_first(idl);
    if (ovs_row == NULL)
    {
        VLOG_ERR("%s SYSTEM table did not have any rows. Ideally it "
                 "should have just one entry.", __func__);
        return CMD_SUCCESS;
    }

    if (type == UDP_BCAST_FWD)
    {
        udp_status = (char *)smap_get(&ovs_row->other_config,
                             SYSTEM_OTHER_CONFIG_MAP_UDP_BCAST_FWD_ENABLED);
        if (!udp_status)
        {
            udp_status = "disabled";
        }
        else
        {
            if (!strcmp(udp_status, TRUE))
                udp_status = "enabled";
            else
                udp_status = "disabled";
        }

        vty_out(vty, "%sIP Forwarder Addresses%s",
                     VTY_NEWLINE, VTY_NEWLINE);
        vty_out(vty, "%sUDP Broadcast Forwarder%s",
                      VTY_NEWLINE, VTY_NEWLINE);
        vty_out(vty, "-------------------------%s",
                      VTY_NEWLINE);
        vty_out(vty, "UDP Bcast Forwarder : %s%s", udp_status, VTY_NEWLINE);
        vty_out(vty, "%s", VTY_NEWLINE);

        if (ifname)
        {
            vty_out(vty, "Interface: %s%s", ifname, VTY_NEWLINE);
            vty_out(vty, "%2sIP Forward Addresses%4sUDP Port %s", "", "",
                         VTY_NEWLINE);
            vty_out(vty, "%2s------------------------------- %s", "",
                         VTY_NEWLINE);
        }
    }

    row_serv = ovsrec_udp_bcast_forwarder_server_first(idl);
    if (!row_serv)
    {
        return CMD_SUCCESS;
    }

    OVSREC_UDP_BCAST_FORWARDER_SERVER_FOR_EACH (row_serv, idl)
    {
        /* get the interface details. */
        if(row_serv->src_port)
        {
            if (ifname)
            {
                if(!strcmp(row_serv->src_port->name, ifname))
                {
                    if(row_serv->n_ipv4_ucast_server)
                    {
                        for (i = 0; i < row_serv->n_ipv4_ucast_server; i++)
                        {
                            /* get the configured server IP.*/
                            fwd_server_ip = row_serv->ipv4_ucast_server[i];
                            /* get the UDP port number. */
                            datum =
                            ovsrec_udp_bcast_forwarder_server_get_udp_dport
                                    (row_serv, OVSDB_TYPE_INTEGER);
                            if ((NULL!=datum) && (datum->n >0))
                            {
                                index = datum->keys[0].integer;
                            }
                            if (type == UDP_BCAST_FWD)
                            {
                                if (index != DHCP_RELAY_UDP_PORT)
                                    vty_out(vty, "%2s%s%16s%d%s", "",
                                            fwd_server_ip, "", index,
                                            VTY_NEWLINE);
                            }
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
                if(type == UDP_BCAST_FWD)
                {
                    if (row_serv->udp_dport != DHCP_RELAY_UDP_PORT)
                    {
                            vty_out(vty, "Interface: %s%s",
                                     row_serv->src_port->name, VTY_NEWLINE);
                            vty_out(vty, "%2sIP Forward Address%4sUDP Port%s",
                                         "", "", VTY_NEWLINE);
                            vty_out(vty, "%2s-----------------------------%s",
                                         "", VTY_NEWLINE);
                    }
                }

                for (i = 0; i < row_serv->n_ipv4_ucast_server; i++)
                {
                    /* get the configured server IP.*/
                    fwd_server_ip = row_serv->ipv4_ucast_server[i];

                    /* get the UDP port number. */
                    datum = ovsrec_udp_bcast_forwarder_server_get_udp_dport
                                                (row_serv, OVSDB_TYPE_INTEGER);
                    if ((NULL!=datum) && (datum->n >0))
                    {
                        index = datum->keys[0].integer;
                    }
                    if (type == UDP_BCAST_FWD)
                    {
                        if (index != DHCP_RELAY_UDP_PORT)
                            vty_out(vty, "%2s%s%16s%d%s", "",
                                         fwd_server_ip, "",
                                         index, VTY_NEWLINE);
                    }
                }
            }
        }
    }
    return CMD_SUCCESS;
}
