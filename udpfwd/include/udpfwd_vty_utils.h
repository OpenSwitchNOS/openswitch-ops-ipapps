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

#ifndef _UDPFWD_VTY_UTILS_H
#define _UDPFWD_VTY_UTILS_H

/* IP Address length. */
#define IP_ADDRESS_LENGTH   15

/* Maximum entries allowed on an interface. */
#define MAX_HELPER_ADDRESS_PER_INTERFACE   16

#define TRUE    "true"
#define FALSE   "false"
#define SET     1
#define UNSET   0

/* Store the UDP Forwarder details. */
typedef struct
udpfwd_feature_t {
    char *ipAddr;       /* IP address of the protocol server. */
    int  udpPort;       /* UDP port number. */
} udpfwd_feature;

/* Enums for UDP Bcast Forwarding and dhcp-relay */
typedef enum
config_type_t {
    DHCP_RELAY = 0,
    UDP_BCAST_FWD
} config_type;

/* Handler functions. */

extern bool decode_server_param (udpfwd_feature *, const char **,
                                 config_type );
extern bool
find_helper_address (const struct
                     ovsrec_dhcp_relay *, udpfwd_feature *);
extern const struct
ovsrec_dhcp_relay *dhcp_relay_row_lookup(const struct
                                         ovsrec_dhcp_relay *);
extern int8_t udpfwd_globalconfig (const char *, config_type );
extern void udpfwd_serverupdate (const struct
                                 ovsrec_dhcp_relay *,
                                 int8_t , udpfwd_feature *);
extern int8_t udpfwd_serverconfig (udpfwd_feature *, bool );
extern int8_t show_ip_helper_address_config (const char *, config_type );

#endif /* UDPFWD_VTY_UTILS_H */
