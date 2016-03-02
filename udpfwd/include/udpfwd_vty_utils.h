/* UDP Forwarder Utility header file
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

#ifndef UDPFWD_VTY_UTILS_H
#define UDPFWD_VTY_UTILS_H

#define SET     1
#define UNSET   0

/* Store the UDP Forwarder details. */
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

/* Handler functions. */

extern bool decode_server_param (udpfwd_server *, const char **,
                                 udpfwd_feature);
extern bool
find_udpfwd_server_ip (char **, int8_t,
                       udpfwd_server *);
extern bool
helper_address_maxcount_reached(const char * );
extern const struct
ovsrec_dhcp_relay *dhcp_relay_row_lookup(const char *, const char *);
extern int8_t udpfwd_globalconfig (const char *, udpfwd_feature );
extern void udpfwd_serverupdate (const struct
                                 ovsrec_dhcp_relay *,
                                 bool, udpfwd_server *);
extern int8_t udpfwd_serverconfig (udpfwd_server *, bool );
extern int8_t show_ip_helper_address_config (const char *, udpfwd_feature );

#endif /* udpfwd_vty_utils.h */
