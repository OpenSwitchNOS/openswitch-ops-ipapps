/* DHCP-Relay CLI commands header file
 *
 * Copyright (C) 2016 Hewlett Packard Enterprise Development LP
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
 */

/****************************************************************************
 * @file dhcpv6_relay_vty.h
 * purpose: To add declarations required for
 * dhcpv6_relay_vty.c
 ***************************************************************************/

#ifndef DHCPV6_RELAY_VTY_H
#define DHCPV6_RELAY_VTY_H

#include <stdbool.h>

#define DHCPV6_RELAY_STR \
"Configure dhcpv6-relay\n"
#define SHOW_DHCPV6_RELAY_STR \
"Show dhcpv6-relay configuration\n"

/* DHCPv6-Relay configuration keys */
#define DHCPV6_RELAY_OPTION_STR \
"Configure DHCPv6 relay option\n"
#define OPTION_79_STR \
"Configure the DHCPv6 option 79 on the device\n"

/* FIXME: The declarations to be moved to idl.h */

#define SYSTEM_DHCP_CONFIG_MAP_V6RELAY_ENABLED    "v6relay_enabled"
#define SYSTEM_DHCP_CONFIG_MAP_V6RELAY_OPTION79_ENABLED  \
                                       "v6relay_option79_enabled"

/* DHCPv6-Relay statistics keys*/
#define PORT_DHCP_RELAY_STATISTICS_MAP_VALID_V6CLIENT_REQUESTS \
                                   "valid_v6client_requests"
#define PORT_DHCP_RELAY_STATISTICS_MAP_DROPPED_V6CLIENT_REQUESTS \
                                   "dropped_v6client_requests"
#define PORT_DHCP_RELAY_STATISTICS_MAP_VALID_V6SERVER_RESPONSES \
                                   "valid_v6server_responses"
#define PORT_DHCP_RELAY_STATISTICS_MAP_DROPPED_V6SERVER_RESPONSES \
                                   "dropped_v6server_responses"

/* DHCPv6 feature enumeration */
typedef enum DHCPV6_FEATURE
{
    GLOBAL_CONFIG = 0,
    OPTION79_CONFIG
} DHCPV6_FEATURE;

/* Structure needed for dhcpv6-relay statistics counters */
typedef struct DHCPV6_RELAY_PKT_COUNTER
{
    uint32_t    v6client_drops; /* Number of dropped client requests */
    uint32_t    v6client_valids; /* Number of valid client requests */
    uint32_t    v6serv_drops; /* Number of dropped server responses */
    uint32_t    v6serv_valids; /* Number of valid server responses */

} DHCPV6_RELAY_PKT_COUNTER;

/* Defuns for dhcp-relay */
extern struct cmd_element dhcpv6_relay_configuration_cmd;
extern struct cmd_element dhcpv6_relay_option_configuration_cmd;
extern struct cmd_element no_dhcpv6_relay_configuration_cmd;
extern struct cmd_element no_dhcpv6_relay_option_configuration_cmd;
extern struct cmd_element show_dhcpv6_relay_configuration_cmd;

#endif /* dhcpv6_relay_vty.h */
