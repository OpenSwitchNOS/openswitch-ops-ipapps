/* UDP Broadcast Forwarder Utility header file
 *
 * Copyright (C) 2016 Hewlett Packard Enterprise Development LP
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * File: udpfwd_vty_utils.h
 *
 * Purpose:  To add declarations required for udpfwd_vty_utils.c
 */
#ifndef _UDPFWD_VTY_UTILS_H_
#define _UDPFWD_VTY_UTILS_H_

#include <sys/un.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <pwd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "vtysh/lib/version.h"
#include "getopt.h"
#include "vtysh/command.h"
#include "memory.h"
#include "vtysh/vtysh.h"
#include "vtysh/vtysh_user.h"
#include "vswitch-idl.h"
#include "ovsdb-idl.h"
#include "smap.h"
#include "openvswitch/vlog.h"
#include "openswitch-idl.h"
#include "vtysh/vtysh_ovsdb_if.h"
#include "vtysh/vtysh_ovsdb_config.h"
#include <arpa/inet.h>
#include <string.h>

/* Max protocols supported. */
#define MAX_UDP_PROTOCOL    11

/* IP Address length. */
#define IP_ADDRESS_LENGTH   18

/* dhcp-relay UDP port number. */
#define DHCP_RELAY_UDP_PORT 67

/* Maximum entries allowed on an interface. */
#define MAX_PROTOCOL_ADDR   16

#define TRUE    "true"
#define FALSE   "false"
#define SET     1
#define UNSET   0

/* Struct to hold the UDP protocol name and number. */
typedef struct {
    char *name; /* UDP protocol name */
    int number; /* UDP protocol port number */
} udpProtocols;

/* Store the forward protocol details. */
typedef struct
udpfwd_server_t {
    char *ipAddr;       /* IP address of the protocol server. */
    int  udpPort;       /* UDP port number. */
} udpfwd_server;

/* Enums for UDP Bcast Forwarding and dhcp-relay */
typedef enum
udpfwd_feature_t {
    DHCP_RELAY = 0,
    UDP_BCAST_FWD
} udpfwd_feature;

extern udpProtocols udp_protocol[MAX_UDP_PROTOCOL];

/* Handler functions. */

extern bool decode_server_param (udpfwd_server *, const char **,
                                 udpfwd_feature );
extern int8_t udpfwd_globalconfig (const char *, udpfwd_feature );
extern void udpfwd_serverupdate (const struct
                                 ovsrec_udp_bcast_forwarder_server *,
                                 int8_t , udpfwd_server *, char **);
extern int8_t udpfwd_serverconfig (udpfwd_server *, bool );
extern int8_t show_udp_forwarder_configuration (const char *, udpfwd_feature );

#endif /* _UDPFWD_VTY_UTILS_H_ */
