/* UDP Forwarder Utility functions.
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
 * File: udpfwd_vty_utils.c
 *
 * Purpose:  To provide the UDP broadcast forwarder and dhcp-relay
 *           utility functions.
 */

#include <sys/un.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <pwd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "vtysh/lib/version.h"
#include <unistd.h>
#include "getopt.h"
#include "vtysh/command.h"
#include "vtysh/memory.h"
#include "vtysh/vtysh.h"
#include "vtysh/vtysh_user.h"
#include "vswitch-idl.h"
#include "ovsdb-idl.h"
#include "smap.h"
#include "openvswitch/vlog.h"
#include "openswitch-idl.h"
#include "vtysh/vtysh_ovsdb_config.h"
#include <arpa/inet.h>
#include <string.h>
#include "udpfwd_vty_utils.h"
#include "udpfwd_common.h"

static const struct ovsrec_vrf* udp_bcast_config_vrf_lookup(const char *);
extern struct ovsdb_idl *idl;
VLOG_DEFINE_THIS_MODULE(udpfwd_vty_utils);

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
| Function       : udpfwd_globalconfig
| Responsibility : To enable/disable udp broadcast forwarding and dhcp-relay
| Parameters     :
|        status  : If true, enable UDP broadcast forwarding/dhcp-relay and
|                  if false disable the UDP broadcast forwarding/dhcp-relay
|        type    : Determines thhe udpfwd feature type
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
udpfwd_globalconfig (const char *status, udpfwd_feature type)
{
    const struct ovsrec_system *ovs_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    struct smap smap_status_value;
    enum ovsdb_idl_txn_status txn_status;
    char *state_value = NULL, *key = NULL;

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
        key = SYSTEM_OTHER_CONFIG_MAP_DHCP_RELAY_DISABLED;
    else if (type == UDP_BCAST_FWD)
        key = SYSTEM_OTHER_CONFIG_MAP_UDP_BCAST_FWD_ENABLED;

    smap_clone(&smap_status_value, &ovs_row->other_config);
    state_value = (char*)smap_get(&ovs_row->other_config,
                                  key);

    if(state_value == NULL)
    {
        if (!strcmp(status, "true"))
            smap_add(&smap_status_value, key, status);
    }
    else
    {
        smap_replace(&smap_status_value, key, status);
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

/*
 * FIXME: This will be removed once the same function
 * is available from vrf_util
 */
/*-----------------------------------------------------------------------------
| Function         : udp_bcast_config_vrf_lookup
| Responsibility   : To lookup for the default VRF.
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
| Responsibility   : Validates the user input parameters
|                    for the  UDP forward-protocol and helper-address CLI
| Parameters       :
|        *udpfwdServ : Pointer containing user input details
|        *argv     : argv from the CLI
|        type      : Determines thhe udpfwd feature type
| Return           : On success returns true,
|                    On failure returns false
-----------------------------------------------------------------------------*/
bool
decode_server_param (udpfwd_server *udpfwdServer, const char *argv[],
                     udpfwd_feature type)
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

    if (!IS_VALID_IPV4(htonl(addr.s_addr)))
    {
        if (type == UDP_BCAST_FWD)
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
        else if (type == DHCP_RELAY)
        {
            vty_out(vty,
                "Broadcast, multicast and loopback addresses "
                "are not allowed.%s",
                VTY_NEWLINE);
                return validParams;
        }
    }

    udpfwdServer->ipAddr = (char *)argv[0];

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
                udpfwdServer->udpPort = udp_protocol[i].number;
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
                udpfwdServer->udpPort = pNum;
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
| Function         : find_udpfwd_server_ip
| Responsibility   : To check the server IP on the specified row is
|                    present in UDP forwarder table or dhcp-relay table
| Parameters       :
|      servers     : Array of server IP
|      count       : Number of server IPs
|      *udpfwdServ : Pointer containing user input details
| Return           : On success returns true,
|                    On failure returns false
-----------------------------------------------------------------------------*/
bool
find_udpfwd_server_ip (char **servers, int8_t count,
                       udpfwd_server *udpfwdServ)
{
    size_t i;

    for (i = 0; i < count; i++)
    {
        if (!strcmp (udpfwdServ->ipAddr, servers[i]))
        {
            return true;
        }
    }
    return false;
}

/*-----------------------------------------------------------------------------
| Function         : dhcp_relay_row_lookup
| Responsibility   : To lookup for the record with port and
|                    default VRF in the dhcp-relay table
| Parameters       :
|        portname  : Name of the port
|        vrf_name  : Name of the VRF
| Return           : On success returns the dhcp-relay row,
|                    On failure returns NULL
-----------------------------------------------------------------------------*/
const struct
ovsrec_dhcp_relay *dhcp_relay_row_lookup(const char *portname,
                                         const char *vrf_name)
{
    const struct ovsrec_dhcp_relay *row_serv = NULL;
    OVSREC_DHCP_RELAY_FOR_EACH (row_serv, idl)
    {
        if (strcmp(row_serv->port->name, portname) == 0 &&
                             strcmp(row_serv->vrf->name, vrf_name) == 0)
        {
            return row_serv;
        }
    }
    return row_serv;
}

/*-----------------------------------------------------------------------------
| Function         : udp_bcast_server_row_lookup
| Responsibility   : To lookup for the record with port and
|                    default VRF in the UDP broadcast server table
| Parameters       :
|        portname  : Name of the port
|        vrf_name  : Name of the VRF
| Return           : On success returns the UDP broadcast server row,
|                    On failure returns NULL
-----------------------------------------------------------------------------*/
const struct
ovsrec_udp_bcast_forwarder_server *udp_bcast_server_row_lookup
        (const char *portname, const char *vrf_name, udpfwd_server *udpfwdServ)
{
    const struct ovsrec_udp_bcast_forwarder_server *row_serv = NULL;

    OVSREC_UDP_BCAST_FORWARDER_SERVER_FOR_EACH (row_serv, idl)
    {
        if (strcmp(row_serv->src_port->name, portname) == 0)
        {
            if (strcmp(row_serv->dest_vrf->name, vrf_name) == 0)
            {
                if (row_serv->udp_dport == udpfwdServ->udpPort)
                {
                    return row_serv;
                }
            }
        }
    }

    return row_serv;
}

/*-----------------------------------------------------------------------------
| Function         : helper_address_maxcount_reached
| Responsibility   : To check the helper-address count on an interface
|                    of dhcp-relay table
| Parameters       :
|      portname    : Name of the port
| Return           : On success returns true,
|                    On failure returns false
-----------------------------------------------------------------------------*/
bool
helper_address_maxcount_reached(const char *portname)
{
    const struct ovsrec_dhcp_relay *row_serv = NULL;
    size_t entries = 1;

    OVSREC_DHCP_RELAY_FOR_EACH (row_serv, idl)
    {
        if (strcmp(row_serv->port->name, portname) == 0)
        {
            entries += row_serv->n_ipv4_ucast_server;

            /*
             * Verify if the entries on an interface has
             * exceeded the limit.
             */
            if (entries > MAX_HELPER_ADDRESSES_PER_INTERFACE)
            {
                return true;
            }
        }
    }
    return false;
}

/*-----------------------------------------------------------------------------
| Function         : server_address_maxcount_reached
| Responsibility   : To check the UDP server IP count on an interface
|                    of UDP broadcast server table
| Parameters       :
|      portname    : Name of the port
| Return           : On success returns true,
|                    On failure returns false
-----------------------------------------------------------------------------*/
bool
server_address_maxcount_reached (const char *portname)
{
    const struct ovsrec_udp_bcast_forwarder_server *row_serv = NULL;
    size_t entries = 1;

    OVSREC_UDP_BCAST_FORWARDER_SERVER_FOR_EACH (row_serv, idl)
    {
        if (strcmp(row_serv->src_port->name, portname) == 0)
        {
            entries += row_serv->n_ipv4_ucast_server;

            /*
             * Verify if the entries on an interface has
             * exceeded the limit.
             */
            if (entries > MAX_SERVER_ADDRESS_PER_INTERFACE)
            {
                return true;
            }
        }
    }
    return false;
}

/*-----------------------------------------------------------------------------
| Function         : udpfwd_dhcpserverupdate
| Responsibility   : To update the server IP information to dhcp-relay table
| Parameters       :
|      row         : udpfwd feature row
|      action      : Operation to be performed add/delete the server IP
|      *udpfwdServ : Pointer containing user input details
-----------------------------------------------------------------------------*/
void
udpfwd_dhcpserverupdate (const struct ovsrec_dhcp_relay *row_serv, bool action,
                         udpfwd_server *udpfwdServ, udpfwd_feature type)
{
    char **servers;
    size_t i, n, server_length = 0;

    if (action)
    {
        server_length = row_serv->n_ipv4_ucast_server + 1;
        servers = xmalloc(IPADDRESS_STRING_MAX_LENGTH * server_length);

        /* Add a server IP to dhcp-relay table */
        for (i = 0; i < row_serv->n_ipv4_ucast_server; i++)
            servers[i] = row_serv->ipv4_ucast_server[i];

        servers[row_serv->n_ipv4_ucast_server] = udpfwdServ->ipAddr;
        ovsrec_dhcp_relay_set_ipv4_ucast_server(row_serv,
                                servers, server_length);
    }
    else
    {
        server_length = row_serv->n_ipv4_ucast_server - 1;
        servers = xmalloc(IPADDRESS_STRING_MAX_LENGTH * server_length);

        /* Delete the server IP from dhcp-relay table */
        for (i = n = 0; i < row_serv->n_ipv4_ucast_server; i++)
        {
            if (strcmp (udpfwdServ->ipAddr,
                        row_serv->ipv4_ucast_server[i]) != 0)
                servers[n++] = row_serv->ipv4_ucast_server[i];
        }
        ovsrec_dhcp_relay_set_ipv4_ucast_server (row_serv, servers, n);
    }

    free(servers);
    return;
}

/*-----------------------------------------------------------------------------
| Function         : udpfwd_udpserverupdate
| Responsibility   : To update the server IP information to UDP Bcast table
| Parameters       :
|      row         : udpfwd feature row
|      action      : Operation to be performed add/delete the server IP
|      *udpfwdServ : Pointer containing user input details
-----------------------------------------------------------------------------*/
void
udpfwd_udpserverupdate (const struct ovsrec_udp_bcast_forwarder_server *row_serv,
                        bool action, udpfwd_server *udpfwdServ,
                        udpfwd_feature type)
{
    char **servers;
    size_t i, n, server_length = 0;

    if (action)
    {
        server_length = row_serv->n_ipv4_ucast_server + 1;
        servers = xmalloc(IPADDRESS_STRING_MAX_LENGTH * server_length);

        /* Add a server IP to dhcp-relay table */
        for (i = 0; i < row_serv->n_ipv4_ucast_server; i++)
            servers[i] = row_serv->ipv4_ucast_server[i];

        servers[row_serv->n_ipv4_ucast_server] = udpfwdServ->ipAddr;
        ovsrec_udp_bcast_forwarder_server_set_ipv4_ucast_server
                        (row_serv, servers, server_length);
    }
    else
    {
        server_length = row_serv->n_ipv4_ucast_server - 1;
        servers = xmalloc(IPADDRESS_STRING_MAX_LENGTH * server_length);

        /* Delete the server IP from dhcp-relay table */
        for (i = n = 0; i < row_serv->n_ipv4_ucast_server; i++)
        {
            if (strcmp (udpfwdServ->ipAddr,
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
| Function         : udpfwd_helperaddressconfig
| Responsibility   : Set/unset dhcp-relay
|                    helper-address.
| Parameters       :
|      *udpfwdServ : Pointer containing user input details
|      set         : Flag to set or unset
| Return           : On success returns CMD_SUCCESS,
|                    On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
udpfwd_helperaddressconfig (udpfwd_server *udpfwdServ, bool set)
{
    const struct ovsrec_dhcp_relay *row_serv = NULL;
    const struct ovsrec_port *port_row = NULL;
    const struct ovsrec_vrf *vrf_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status txn_status;
    bool isAddrMatch = false;
    bool isMaxEntries;
    udpfwd_feature type = DHCP_RELAY;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    /* Lookup for the record in the dhcp-relay table */
    row_serv = dhcp_relay_row_lookup((char*)vty->index, DEFAULT_VRF_NAME);
    if (row_serv)
    {
        isAddrMatch = find_udpfwd_server_ip(row_serv->ipv4_ucast_server,
                                            row_serv->n_ipv4_ucast_server,
                                            udpfwdServ);
    }

    if (set)
    {
        isMaxEntries = helper_address_maxcount_reached((char*)vty->index);
        if (isMaxEntries)
        {
            vty_out(vty, "Maximum allowed helper addresses already "
                         "configured on the interface.%s", VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }

        if (NULL == row_serv)
        {
            /* First set of dhcp-relay helper-address. */

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
                if (strcmp(port_row->name, (char*)vty->index) == 0)
                {
                    ovsrec_dhcp_relay_set_port(row_serv, port_row);
                    break;
                }
            }

            /* Update the vrf name of the port. */
            vrf_row = udp_bcast_config_vrf_lookup(DEFAULT_VRF_NAME);
            if (!vrf_row)
            {
                vty_out(vty, "Error: Could not fetch "
                        "default VRF data.%s", VTY_NEWLINE);
                VLOG_ERR("%s VRF table did not have any rows. "
                         "Ideally it should have just one entry.", __func__);
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }

            if (strcmp(vrf_row->name, DEFAULT_VRF_NAME) == 0)
            {
                ovsrec_dhcp_relay_set_vrf(row_serv, vrf_row);
            }

            /* Update the protocol server IP. */
            udpfwd_dhcpserverupdate(row_serv, true, udpfwdServ, type);
        }
        else
        {
            if (!isAddrMatch)
            {
                /* Update the protocol server IP. */
                udpfwd_dhcpserverupdate(row_serv, true, udpfwdServ, type);
            }
            else
            {
                /* Existing entry. */
                vty_out(vty, "This entry already exists.%s", VTY_NEWLINE);
                cli_do_config_abort(status_txn);
                return CMD_SUCCESS;
            }
        }
    }
    else
    {
        if (NULL == row_serv || (!isAddrMatch))
        {
            vty_out(vty, "Helper-address is not present.%s", VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }
        else
        {
            /*
             * If this is the last entry then after unset remove
             * the complete row.
             */

            if (row_serv->n_ipv4_ucast_server == 1)
            {
                ovsrec_dhcp_relay_delete(row_serv);
            }
            else
            {
                /* Update the protocol server IP. */
                udpfwd_dhcpserverupdate(row_serv, false, udpfwdServ, type);
            }
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
| Function         : udpfwd_serverconfig
| Responsibility   : set/unset UDP forward-protocol
| Parameters       :
|      *udpfwdServ : Pointer containing user input details
|      set         : Flag to set or unset
| Return           : On success returns CMD_SUCCESS,
|                    On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
udpfwd_serverconfig (udpfwd_server *udpfwdServ, bool set)
{
    const struct ovsrec_udp_bcast_forwarder_server *row_serv = NULL;
    const struct ovsrec_vrf *vrf_row = NULL;
    const struct ovsrec_port *port_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status txn_status;
    bool isAddrMatch = false, isMaxEntries = false;
    udpfwd_feature type = UDP_BCAST_FWD;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    /* lookup for the record in the UDP broadcast server table */
    row_serv = udp_bcast_server_row_lookup((char*)vty->index,
                                           DEFAULT_VRF_NAME, udpfwdServ);

    if (row_serv)
    {
        isAddrMatch = find_udpfwd_server_ip(row_serv->ipv4_ucast_server,
                                            row_serv->n_ipv4_ucast_server,
                                            udpfwdServ);
    }

    if (set)
    {
        isMaxEntries = server_address_maxcount_reached((char*)vty->index);
        if (isMaxEntries)
        {
            vty_out(vty, "%s: Entry not allowed as maximum "
                         "entries per interface exceeded.%s",
                         udpfwdServ->ipAddr, VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }

        if (NULL == row_serv)
        {
            /*
             * First set of UDP forward-protocol
             */

            row_serv = ovsrec_udp_bcast_forwarder_server_insert(status_txn);

            if (!row_serv)
            {
                VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }

            /* Update the interface name */
            OVSREC_PORT_FOR_EACH(port_row, idl)
            {
                if (strcmp(port_row->name, (char*)vty->index) == 0)
                {
                    ovsrec_udp_bcast_forwarder_server_set_src_port
                                (row_serv, port_row);
                    break;
                }
            }

            /* Update the vrf name of the port */
            vrf_row = udp_bcast_config_vrf_lookup(DEFAULT_VRF_NAME);
            if (!vrf_row)
            {
                vty_out(vty, "Error: Could not fetch "
                             "default VRF data.%s", VTY_NEWLINE);
                VLOG_ERR("%s VRF table did not have any rows. "
                         "Ideally it should have just one entry.", __func__);
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }

            if (strcmp(vrf_row->name, DEFAULT_VRF_NAME) == 0)
            {
                ovsrec_udp_bcast_forwarder_server_set_dest_vrf
                                (row_serv, vrf_row);
            }

            /* Update the dst udp port */
            ovsrec_udp_bcast_forwarder_server_set_udp_dport
                        (row_serv, udpfwdServ->udpPort);

            /* Update the protocol server IP */
            udpfwd_udpserverupdate(row_serv, true, udpfwdServ, type);
        }
        else
        {
            if (!isAddrMatch)
            {
                /* set action */
                udpfwd_udpserverupdate(row_serv, true, udpfwdServ, type);
            }
            else
            {
                /* Existing entry */
                vty_out(vty, "This entry already exists.%s", VTY_NEWLINE);
                cli_do_config_abort(status_txn);
                return CMD_SUCCESS;
            }
        }
    }
    else
    {
        if (NULL == row_serv)
        {
            vty_out(vty, "UDP forward-protocol is not present. %s", VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }
        else
        {
            if (!isAddrMatch)
            {
                vty_out(vty, "No such entries are present. %s", VTY_NEWLINE);
                cli_do_config_abort(status_txn);
                return CMD_SUCCESS;
            }
            else
            {
                /*
                 * If this is the last entry then after unset remove
                 * the complete row.
                 */
                if (row_serv->n_ipv4_ucast_server == 1)
                {
                    ovsrec_udp_bcast_forwarder_server_delete(row_serv);
                }
                else
                {
                     /* No set action */
                    udpfwd_udpserverupdate(row_serv, false, udpfwdServ, type);
                }
            }
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
| Responsibility : To show the dhcp-relay helper-address configurations.
| Parameters     :
|      *portname : Name of the Port
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
show_ip_helper_address_config (const char *portname)
{
    const struct ovsrec_dhcp_relay *row_serv = NULL;
    char *fwd_server_ip = NULL;
    size_t i = 0;

    vty_out(vty, " IP Helper Addresses%s", VTY_NEWLINE);
    if (portname)
    {
        vty_out(vty, "%s Interface: %s%s", VTY_NEWLINE,
                     portname, VTY_NEWLINE);
        vty_out(vty, "%2sIP Helper Address%s", "", VTY_NEWLINE);
        vty_out(vty, "%2s-----------------%s", "", VTY_NEWLINE);
    }

    row_serv = ovsrec_dhcp_relay_first(idl);
    if (!row_serv)
    {
        return CMD_SUCCESS;
    }

    OVSREC_DHCP_RELAY_FOR_EACH (row_serv, idl)
    {
        /* Get the interface details. */
        if (row_serv->port)
        {
            if (portname)
            {
                if (strcmp(row_serv->port->name, portname) == 0)
                {
                    if(row_serv->n_ipv4_ucast_server)
                    {
                        for (i = 0; i < row_serv->n_ipv4_ucast_server; i++)
                        {
                            /* Get the configured server IP.*/
                            fwd_server_ip = row_serv->ipv4_ucast_server[i];
                            vty_out(vty, "%2s%s%s", "",
                                    fwd_server_ip, VTY_NEWLINE);
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
                vty_out(vty, "%s Interface: %s%s", VTY_NEWLINE,
                             row_serv->port->name, VTY_NEWLINE);
                vty_out(vty, "%2sIP Helper Address%s", "", VTY_NEWLINE);
                vty_out(vty, "%2s-----------------%s", "", VTY_NEWLINE);
                for (i = 0; i < row_serv->n_ipv4_ucast_server; i++)
                {
                    /* Get the configured server IP.*/
                    fwd_server_ip = row_serv->ipv4_ucast_server[i];
                    vty_out(vty, "%2s%s%s", "", fwd_server_ip, VTY_NEWLINE);
                }
            }
        }
    }
    return CMD_SUCCESS;
}

/*-----------------------------------------------------------------------------
| Responsibility : To show the UDP Broadcast Forwarder and dhcp-relay
|                  helper-address configurations.
| Parameters     :
|       *ifname  : Interface name
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
show_udp_forwarder_configuration (const char *ifname)
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

    udp_status = (char *)smap_get(&ovs_row->other_config,
                         SYSTEM_OTHER_CONFIG_MAP_UDP_BCAST_FWD_ENABLED);
    if (!udp_status)
    {
        udp_status = "disabled";
    }
    else
    {
        if (!strcmp(udp_status, "true"))
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
                            if (index != DHCP_RELAY_UDP_PORT)
                                vty_out(vty, "%2s%s%16s%d%s", "",
                                        fwd_server_ip, "", index,
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
                if (row_serv->udp_dport != DHCP_RELAY_UDP_PORT)
                {
                        vty_out(vty, "Interface: %s%s",
                                row_serv->src_port->name, VTY_NEWLINE);
                        vty_out(vty, "%2sIP Forward Address%4sUDP Port%s",
                                "", "", VTY_NEWLINE);
                        vty_out(vty, "%2s-----------------------------%s",
                                "", VTY_NEWLINE);
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
                    if (index != DHCP_RELAY_UDP_PORT)
                        vty_out(vty, "%2s%s%16s%d%s", "",
                                     fwd_server_ip, "",
                                     index, VTY_NEWLINE);
                }
            }
        }
    }

    return CMD_SUCCESS;
}
