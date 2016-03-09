/* UDP Broadcast Forwarder CLI commands header file
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
 * File: udpfwd_vty.h
 *
 * Purpose:  To add declarations required for udpfwd_vty.c
 */

#ifndef _UDPFWD_VTY_H_
#define _UDPFWD_VTY_H_

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
#include "udpfwd_vty_utils.h"
#include "dhcp_relay_vty.h"
#include "vtysh_ovsdb_udpfwd_context.h"
#include <arpa/inet.h>
#include <string.h>

#define UDPFWD_BCAST_STR \
"Enable/disable UDP broadcast forwarding\n"
#define UDPFWD_PROTO_STR \
"Configure a UDP broadcast server for the interface\n"
#define UDPFWD_SERV_ADDR_STR \
"IP address of the protocol server\n"
#define UDPFWD_PORT_NUM_STR \
"UDP port number corresponding to a UDP application supported\n"
#define UDPFWD_PROTO_NAME_STR \
"Supported UDP protocol names: " \
"dns (53)," \
"ntp (123)," \
"netbios-ns (137)," \
"netbios-dgm (138)," \
"radius (1812)," \
"radius-old (1645)," \
"rip (520)," \
"snmp (161)," \
"snmp-trap (162)," \
"tftp (69), " \
"timep (37)\n"
#define UDPFWD_INT_STR \
"Select an interface to configure\n"
#define UDPFWD_IFNAME_STR \
"Interface's name\n"

/* Defun prototypes for UDP Forwarder */
extern struct cmd_element cli_udp_bcast_fwd_enable_cmd;
extern struct cmd_element cli_udp_bcast_fwd_disable_cmd;
extern struct cmd_element intf_set_udpf_proto_cmd;
extern struct cmd_element intf_no_set_udpf_proto_cmd;
extern struct cmd_element cli_show_udpf_forward_protocol_cmd;

#endif /* _UDPFWD_VTY_H_ */
