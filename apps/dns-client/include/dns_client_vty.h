/* DNS Client CLI commands header file
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
 * File: dns_client_vty.h
 *
 * Purpose:  To add declarations required for dns_client_vty.c
 */

#ifndef DNS_CLIENT_VTY_H
#define DNS_CLIENT_VTY_H

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
#include "vtysh/vtysh_utils.h"
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

#define DNS_STR \
"Enable/disable DNS Client\n"
#define DOMAIN_NAME_STR \
"Define the default domain name\n"
#define DOMAIN_NAME_WORD_STR \
"Default domain name (Max Length 64)\n"
#define DOMAIN_LIST_STR \
"Domain name to complete unqualified host names\n"
#define DOMAIN_LIST_WORD_STR \
"A domain name\n"
#define SERVER_ADDR_STR \
"Configure DNS server IP address\n"
#define HOST_STR \
"Add an entry to the ip hostname table\n"
#define HOST_NAME_STR \
"Name of host\n"
#define SERVER_IPv4 \
"Domain name server IP address (maximum of 3)\n"
#define SERVER_IPv6 \
"Domain server IPv6 address (maximum of 3)\n"

/* Max lengths */
#define MAX_DOMAIN_NAME_LEN  64
#define MAX_DOMAIN_LIST_LEN  64
#define MAX_HOST_NAME_LEN    64
#define MAX_HOST_ADDR_LEN    64
#define MAX_NAME_SERV_LEN    64
#define MAX_DOMAIN_LIST      6
#define MAX_HOST_NAME        6
#define MAX_NAME_SERVER      3

#define IS_VALID_IPV6(i) !(IN6_IS_ADDR_UNSPECIFIED(i) | IN6_IS_ADDR_LOOPBACK(i) | \
                           IN6_IS_ADDR_SITELOCAL(i)  |  IN6_IS_ADDR_MULTICAST(i))

/* Common Globals */
#define DNS_DOMAIN_NAME     "domain_name"

/* enum for DNS Client */
typedef enum DNS_CLIENT_FIELDS
{
    domList = 0,
    nameServ
} DNS_CLIENT_FIELDS;

/* struct to hold the hosts information */
typedef struct dnsHost_t
{
    char *hostname;
    char *hostIpv4;
    char *hostIpv6;
} dnsHost;

#if 0
/* Defun prototypes */
extern struct cmd_element cli_dns_client_enable_cmd;
extern struct cmd_element cli_dns_client_disable_cmd;
extern struct cmd_element cli_dns_client_domain_name_cmd;
extern struct cmd_element cli_no_dns_client_domain_name_cmd;
extern struct cmd_element cli_dns_client_domain_list_cmd;
extern struct cmd_element cli_no_dns_client_domain_list_cmd;
extern struct cmd_element cli_dns_client_server_address_cmd;
extern struct cmd_element cli_no_dns_client_server_address_cmd;
extern struct cmd_element cli_dns_client_host_name_cmd;
extern struct cmd_element cli_no_dns_client_host_name_cmd;
extern struct cmd_element cli_show_dns_client_cmd;
#endif

#endif /* dns_client_vty.h */