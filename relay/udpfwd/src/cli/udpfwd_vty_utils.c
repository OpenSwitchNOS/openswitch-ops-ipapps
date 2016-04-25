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

extern struct ovsdb_idl *idl;
VLOG_DEFINE_THIS_MODULE(udpfwd_vty_utils);

#ifdef FTR_UDP_BCAST_FWD
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
#endif /* FTR_UDP_BCAST_FWD */

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
const struct
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
#ifdef FTR_UDP_BCAST_FWD
    int pNum, i;
    char *pName;
#endif /* FTR_UDP_BCAST_FWD */
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
#ifdef FTR_UDP_BCAST_FWD
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
#endif /* FTR_UDP_BCAST_FWD */
#ifdef FTR_DHCP_RELAY
        if (type == DHCP_RELAY)
        {
            if ((!IS_BROADCAST_IPV4(htonl(addr.s_addr))) ||
                (!IS_SUBNET_BROADCAST(htonl(addr.s_addr))))
            {
                vty_out(vty,
                        "Multicast and loopback addresses "
                        "are not allowed.%s",
                        VTY_NEWLINE);
                return validParams;
            }
        }
#endif /* FTR_DHCP_RELAY */
    }

    udpfwdServer->ipAddr = (char *)argv[0];

#ifdef FTR_DHCP_RELAY
    if (type == DHCP_RELAY)
        return true;
#endif /* FTR_DHCP_RELAY */

#ifdef FTR_UDP_BCAST_FWD
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
#endif /* FTR_UDP_BCAST_FWD */

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
#ifdef FTR_UDP_BCAST_FWD
    const struct ovsrec_udp_bcast_forwarder_server *udp_row_serv = NULL;
#endif /* FTR_UDP_BCAST_FWD */
#ifdef FTR_DHCP_RELAY
    const struct ovsrec_dhcp_relay *dhcp_row_serv = NULL;
#endif /* FTR_DHCP_RELAY */
#if defined(FTR_DHCP_RELAY) || defined(FTR_UDP_BCAST_FWD)
    size_t entries = 0;
#endif /* (FTR_DHCP_RELAY | FTR_UDP_BCAST_FWD) */

#ifdef FTR_UDP_BCAST_FWD
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
#endif /* FTR_UDP_BCAST_FWD */
#ifdef FTR_DHCP_RELAY
    if (type == DHCP_RELAY)
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
#endif /* FTR_DHCP_RELAY */

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
#ifdef FTR_UDP_BCAST_FWD
    const struct ovsrec_udp_bcast_forwarder_server *udp_row_serv = NULL;
#endif /* FTR_UDP_BCAST_FWD */
#ifdef FTR_DHCP_RELAY
    const struct ovsrec_dhcp_relay *dhcp_row_serv = NULL;
#endif /* FTR_DHCP_RELAY */

    char **servers = NULL;
    char **serverArray = NULL;
    size_t i, n, server_length = 0, serverCount = 0;

#ifdef FTR_UDP_BCAST_FWD
    if (type == UDP_BCAST_FORWARDER)
    {
        udp_row_serv = (const struct
                        ovsrec_udp_bcast_forwarder_server *)row_serv;
        serverCount = udp_row_serv->n_ipv4_ucast_server;
        serverArray = udp_row_serv->ipv4_ucast_server;
    }
#endif /* FTR_UDP_BCAST_FWD */
#ifdef FTR_DHCP_RELAY
    if (type == DHCP_RELAY)
    {
        dhcp_row_serv = (const struct
                         ovsrec_dhcp_relay *)row_serv;
        serverCount = dhcp_row_serv->n_ipv4_ucast_server;
        serverArray = dhcp_row_serv->ipv4_ucast_server;
    }
#endif /* FTR_DHCP_RELAY */

    if (action)
    {
        server_length = serverCount + 1;
        servers = xmalloc(IPADDRESS_STRING_MAX_LENGTH * server_length);

        /* Add a server IP to the respective table */
        for (i = 0; i < serverCount; i++)
            servers[i] = serverArray[i];

        servers[serverCount] = udpfwdServ->ipAddr;
#ifdef FTR_UDP_BCAST_FWD
        if (type == UDP_BCAST_FORWARDER)
        {
            ovsrec_udp_bcast_forwarder_server_set_ipv4_ucast_server
                        (udp_row_serv, servers, server_length);
        }
#endif /* FTR_UDP_BCAST_FWD */
#ifdef FTR_DHCP_RELAY
        if (type == DHCP_RELAY)
        {
            ovsrec_dhcp_relay_set_ipv4_ucast_server
                        (dhcp_row_serv, servers, server_length);
        }
#endif /* FTR_DHCP_RELAY */
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
#ifdef FTR_UDP_BCAST_FWD
        if (type == UDP_BCAST_FORWARDER)
        {
            ovsrec_udp_bcast_forwarder_server_set_ipv4_ucast_server
                                (udp_row_serv, servers, n);
        }
#endif /* FTR_UDP_BCAST_FWD */
#ifdef FTR_DHCP_RELAY
        if (type == DHCP_RELAY)
        {
            ovsrec_dhcp_relay_set_ipv4_ucast_server
                                (dhcp_row_serv, servers, n);
        }
#endif /* FTR_DHCP_RELAY */
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
#ifdef FTR_UDP_BCAST_FWD
            if (type == UDP_BCAST_FORWARDER)
            {
                ovsrec_udp_bcast_forwarder_server_set_src_port
                ((const struct ovsrec_udp_bcast_forwarder_server *)row_serv,
                  port_row);
            }
#endif /* FTR_UDP_BCAST_FWD */
#ifdef FTR_DHCP_RELAY
            if (type == DHCP_RELAY)
            {
                ovsrec_dhcp_relay_set_port
                    ((const struct ovsrec_dhcp_relay *)row_serv,
                      port_row);
            }
#endif /* FTR_DHCP_RELAY */
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
#ifdef FTR_UDP_BCAST_FWD
        if (type == UDP_BCAST_FORWARDER)
        {
            ovsrec_udp_bcast_forwarder_server_set_dest_vrf
                ((const struct ovsrec_udp_bcast_forwarder_server *)row_serv,
                  vrf_row);
        }
#endif /* FTR_UDP_BCAST_FWD */
#ifdef FTR_DHCP_RELAY
        if (type == DHCP_RELAY)
        {
            ovsrec_dhcp_relay_set_vrf
                ((const struct ovsrec_dhcp_relay *)row_serv,
                  vrf_row);
        }
#endif /* FTR_DHCP_RELAY */
    }
    return true;
}
