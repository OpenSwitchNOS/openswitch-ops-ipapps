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
| Responsibility : To enable/disable udp broadcast forwarding and dhcp-relay.
| Parameters     :
|        status  : If true, enable UDP broadcast forwarding/dhcp-relay and
|                  if false disable the UDP broadcast forwarding/dhcp-relay
|        type    : Determines the udpfwd feature type
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
udpfwd_globalconfig (const char *status, UDPFWD_FEATURE type)
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

    key = SYSTEM_OTHER_CONFIG_MAP_UDP_BCAST_FWD_ENABLED;

    smap_clone(&smap_status_value, &ovs_row->other_config);

    /* Update the latest config status. */
    smap_replace(&smap_status_value, key, status);

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
|                    for the  UDP forward-protocol and helper-address CLI.
| Parameters       :
|        *udpfwdServ : Pointer containing user input details
|        *argv     : argv from the CLI
|        type      : Determines the udpfwd feature type
| Return           : On success returns true,
|                    On failure returns false
-----------------------------------------------------------------------------*/
bool
decode_server_param (udpfwd_server *udpfwdServer, const char *argv[],
                     UDPFWD_FEATURE type)
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
        if (type == UDP_BCAST_FORWARDER)
        {
            if ((IS_BROADCAST_IPV4(htonl(addr.s_addr))) ||
                (!IS_SUBNET_BROADCAST(htonl(addr.s_addr))))
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
|                    present in UDP forwarder table or dhcp-relay table.
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
|                    default VRF in the dhcp-relay table.
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
        if (((row_serv->port) && (!strcmp(row_serv->port->name, portname))) &&
           ((row_serv->vrf) && (!strcmp(row_serv->vrf->name, vrf_name))))
        {
            return row_serv;
        }
    }
    return row_serv;
}

/*-----------------------------------------------------------------------------
| Function         : udp_bcast_server_row_lookup
| Responsibility   : To lookup for the record with port and
|                    default VRF in the UDP broadcast server table.
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
        if ((row_serv->src_port) &&
            (!strcmp(row_serv->src_port->name, portname)))
        {
            if ((row_serv->dest_vrf) &&
                (!strcmp(row_serv->dest_vrf->name, vrf_name)))
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
| Function         : server_address_maxcount_reached
| Responsibility   : To check the UDP server IP or dhcp-relay helper address
|                    count on an interface.
| Parameters       :
|      portname    : Name of the port
|      type        : Determines the udpfwd feature type
| Return           : On success returns true,
|                    On failure returns false
-----------------------------------------------------------------------------*/
bool
server_address_maxcount_reached (const char *portname, UDPFWD_FEATURE type)
{
    const struct ovsrec_udp_bcast_forwarder_server *udp_row_serv = NULL;
    const struct ovsrec_dhcp_relay *dhcp_row_serv = NULL;
    size_t entries = 0;

    if(type == UDP_BCAST_FORWARDER)
    {
        OVSREC_UDP_BCAST_FORWARDER_SERVER_FOR_EACH (udp_row_serv, idl)
        {
            if ((udp_row_serv->src_port) &&
                (!strcmp(udp_row_serv->src_port->name, portname)))
            {
                entries += udp_row_serv->n_ipv4_ucast_server;

                /*
                 * Verify if the entries on an interface has
                 * exceeded the limit.
                 */
                if (entries >= MAX_UDPFWD_SERVER_ADDRESSES_PER_INTERFACE)
                {
                    return true;
                }
            }
        }
    }
    else if (type == DHCP_RELAY)
    {
        OVSREC_DHCP_RELAY_FOR_EACH (dhcp_row_serv, idl)
        {
            if ((dhcp_row_serv->port) &&
                (!strcmp(dhcp_row_serv->port->name, portname)))
            {
                entries += dhcp_row_serv->n_ipv4_ucast_server;

                /*
                 * Verify if the entries on an interface has
                 * exceeded the limit.
                 */
                if (entries >= MAX_HELPER_ADDRESSES_PER_INTERFACE)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

/*-----------------------------------------------------------------------------
| Function         : udpfwd_serverupdate
| Responsibility   : To update the server IP information to UDP Bcast table.
| Parameters       :
|      row_serv    : This will point to the specific udpfwd feature row
|      action      : Operation to be performed add/delete the server IP
|      *udpfwdServ : Pointer containing user input details
-----------------------------------------------------------------------------*/
void
udpfwd_serverupdate (void *row_serv, bool action, udpfwd_server *udpfwdServ,
                     UDPFWD_FEATURE type)
{
    const struct ovsrec_udp_bcast_forwarder_server *udp_row_serv = NULL;
    const struct ovsrec_dhcp_relay *dhcp_row_serv = NULL;
    char **servers;
    char **serverArray;
    size_t i, n, server_length = 0, serverCount = 0;

    if (type == UDP_BCAST_FORWARDER)
    {
        udp_row_serv = (const struct
                        ovsrec_udp_bcast_forwarder_server *)row_serv;
        serverCount = udp_row_serv->n_ipv4_ucast_server;
        serverArray = udp_row_serv->ipv4_ucast_server;
    }
    else if (type == DHCP_RELAY)
    {
        dhcp_row_serv = (const struct
                         ovsrec_dhcp_relay *)row_serv;
        serverCount = dhcp_row_serv->n_ipv4_ucast_server;
        serverArray = dhcp_row_serv->ipv4_ucast_server;
    }

    if (action)
    {
        server_length = serverCount + 1;
        servers = xmalloc(IPADDRESS_STRING_MAX_LENGTH * server_length);

        /* Add a server IP to the respective table */
        for (i = 0; i < serverCount; i++)
            servers[i] = serverArray[i];

        servers[serverCount] = udpfwdServ->ipAddr;
        if (type == UDP_BCAST_FORWARDER)
        {
            ovsrec_udp_bcast_forwarder_server_set_ipv4_ucast_server
                        (udp_row_serv, servers, server_length);
        }
        else if (type == DHCP_RELAY)
        {
            ovsrec_dhcp_relay_set_ipv4_ucast_server
                        (dhcp_row_serv, servers, server_length);
        }
    }
    else
    {
        server_length = serverCount - 1;
        servers = xmalloc(IPADDRESS_STRING_MAX_LENGTH * server_length);

        /* Delete the server IP from the respective table */
        for (i = n = 0; i < serverCount; i++)
        {
            if (strcmp (udpfwdServ->ipAddr, serverArray[i]) != 0)
                servers[n++] = serverArray[i];
        }
        if (type == UDP_BCAST_FORWARDER)
        {
            ovsrec_udp_bcast_forwarder_server_set_ipv4_ucast_server
                                (udp_row_serv, servers, n);
        }
        else if (type == DHCP_RELAY)
        {
            ovsrec_dhcp_relay_set_ipv4_ucast_server
                                (dhcp_row_serv, servers, n);
        }
    }

    free(servers);
    return;
}

/*-----------------------------------------------------------------------------
| Function         : udpfwd_setcommoncolumn
| Responsibility   : To update the common VRF and port columns in UDP Bcast
|                    or dhcp-relay table.
| Parameters       :
|      row_serv    : This will point to the specific udpfwd feature row
|      type        : Determines the udpfwd feature type
| Return           : On success returns true,
|                    On failure returns false
-----------------------------------------------------------------------------*/
bool
udpfwd_setcommoncolumn (void *row_serv, UDPFWD_FEATURE type)
{
    const struct ovsrec_port *port_row = NULL;
    const struct ovsrec_vrf *vrf_row = NULL;

    /* Update the interface name */
    OVSREC_PORT_FOR_EACH(port_row, idl)
    {
        if (strcmp(port_row->name, (char*)vty->index) == 0)
        {
            if (type == UDP_BCAST_FORWARDER)
            {
                ovsrec_udp_bcast_forwarder_server_set_src_port
                ((const struct ovsrec_udp_bcast_forwarder_server *)row_serv,
                  port_row);
            }
            else if (type == DHCP_RELAY)
            {
                ovsrec_dhcp_relay_set_port
                    ((const struct ovsrec_dhcp_relay *)row_serv,
                      port_row);
            }
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
        return false;
    }

    if (!strcmp(vrf_row->name, DEFAULT_VRF_NAME))
    {
        if (type == UDP_BCAST_FORWARDER)
        {
            ovsrec_udp_bcast_forwarder_server_set_dest_vrf
                ((const struct ovsrec_udp_bcast_forwarder_server *)row_serv,
                  vrf_row);
        }
        else if (type == DHCP_RELAY)
        {
            ovsrec_dhcp_relay_set_vrf
                ((const struct ovsrec_dhcp_relay *)row_serv,
                  vrf_row);
        }
    }
    return true;
}

/*-----------------------------------------------------------------------------
| Function         : udpfwd_helperaddressconfig
| Responsibility   : Set/unset dhcp-relay.
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
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status txn_status;
    bool isAddrMatch = false;
    bool isMaxEntries;
    UDPFWD_FEATURE type = DHCP_RELAY;

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
        isMaxEntries = server_address_maxcount_reached
                            ((char*)vty->index, type);
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

            /* Update the dhcp-relay table. */
            if (!udpfwd_setcommoncolumn((void *)row_serv, type))
            {
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }

            /* Update the protocol server IP. */
            udpfwd_serverupdate((void *)row_serv, true, udpfwdServ, type);
        }
        else
        {
            if (!isAddrMatch)
            {
                /* Update the protocol server IP. */
                udpfwd_serverupdate((void *)row_serv, true, udpfwdServ, type);
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
                udpfwd_serverupdate((void *)row_serv, false, udpfwdServ, type);
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
| Responsibility   : set/unset UDP forward-protocol.
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
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status txn_status;
    bool isAddrMatch = false, isMaxEntries = false;
    UDPFWD_FEATURE type = UDP_BCAST_FORWARDER;

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
        isMaxEntries = server_address_maxcount_reached
                            ((char*)vty->index, type);
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

            /* Update the UDP broadcast forwarder server table. */
            if (!udpfwd_setcommoncolumn((void *)row_serv, type))
            {
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }

            /* Update the dst udp port */
            ovsrec_udp_bcast_forwarder_server_set_udp_dport
                        (row_serv, udpfwdServ->udpPort);

            /* Update the protocol server IP */
            udpfwd_serverupdate((void *)row_serv, true, udpfwdServ, type);
        }
        else
        {
            if (!isAddrMatch)
            {
                /* set action */
                udpfwd_serverupdate((void *)row_serv, true, udpfwdServ, type);
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
                    udpfwd_serverupdate((void *)row_serv, false, udpfwdServ, type);
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
                            vty_out(vty, "%2s%s%s", "",
                                    row_serv->ipv4_ucast_server[i],
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
                vty_out(vty, "%s Interface: %s%s", VTY_NEWLINE,
                             row_serv->port->name, VTY_NEWLINE);
                vty_out(vty, "%2sIP Helper Address%s", "", VTY_NEWLINE);
                vty_out(vty, "%2s-----------------%s", "", VTY_NEWLINE);
                for (i = 0; i < row_serv->n_ipv4_ucast_server; i++)
                {
                    vty_out(vty, "%2s%s%s", "",
                            row_serv->ipv4_ucast_server[i], VTY_NEWLINE);
                }
            }
        }
    }
    return CMD_SUCCESS;
}

/*-----------------------------------------------------------------------------
| Responsibility : To show the UDP Broadcast Forwarder configurations.
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
    char *udp_status = NULL ;
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
                            /* get the UDP port number. */
                            datum =
                            ovsrec_udp_bcast_forwarder_server_get_udp_dport
                                    (row_serv, OVSDB_TYPE_INTEGER);
                            if ((NULL!=datum) && (datum->n >0))
                            {
                                index = datum->keys[0].integer;
                            }
                            vty_out(vty, "%2s%s%16s%d%s", "",
                                    row_serv->ipv4_ucast_server[i], "",
                                    index, VTY_NEWLINE);
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
                vty_out(vty, "Interface: %s%s",
                        row_serv->src_port->name, VTY_NEWLINE);
                vty_out(vty, "%2sIP Forward Address%4sUDP Port%s",
                        "", "", VTY_NEWLINE);
                vty_out(vty, "%2s-----------------------------%s",
                        "", VTY_NEWLINE);

                for (i = 0; i < row_serv->n_ipv4_ucast_server; i++)
                {
                    /* get the UDP port number. */
                    datum = ovsrec_udp_bcast_forwarder_server_get_udp_dport
                                                (row_serv, OVSDB_TYPE_INTEGER);
                    if ((NULL!=datum) && (datum->n >0))
                    {
                        index = datum->keys[0].integer;
                    }
                    vty_out(vty, "%2s%s%16s%d%s", "",
                                 row_serv->ipv4_ucast_server[i], "",
                                 index, VTY_NEWLINE);
                }
            }
        }
    }

    return CMD_SUCCESS;
}
