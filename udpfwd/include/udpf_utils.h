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
 * File: udpf_utils.h
 *
 * Purpose:  To add declarations required for udpf_utils.c
 */

 #ifndef _UDPFWD_UTILS_H
#define _UDPFWD_UTILS_H

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

/* IP Address length. */
#define IP_ADDRESS_LENGTH   18

/* Maximum entries allowed on an interface. */
#define MAX_PROTOCOL_ADDR   16

#define TRUE    "true"
#define FALSE   "false"
#define SET     1
#define UNSET   0

/* Store the forward protocol details. */
typedef struct
udpf_server_t {
    char *ipAddr;       /* IP address of the protocol server. */
    int  udpPort;       /* UDP port number. */
} udpf_server;

/* Enums for UDP Bcast Forwarding and dhcp-relay */
typedef enum
config_type_t {
    DHCP_RELAY = 0,
    UDP_BCAST_FWD
} config_type;

/* Handler functions. */

extern bool decode_server_param (udpf_server *, const char **,
                                 config_type );
extern int8_t udpFwd_globalConfig (const char *, config_type );
extern void udpFwd_serverUpdate (const struct
                                 ovsrec_dhcp_relay *,
                                 int8_t , udpf_server *, char **);
extern int8_t udpFwd_serverConfig (udpf_server *, bool );
extern int8_t show_udp_forwarder_configuration (const char *, config_type );

#endif /* UDPFWD_UTILS_H */
