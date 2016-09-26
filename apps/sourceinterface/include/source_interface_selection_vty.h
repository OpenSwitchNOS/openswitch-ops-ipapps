/* Source Interface Selection CLI commands header file
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

/****************************************************************************
 * @ingroup cli/vtysh
 *
 * @file source_interface_selection_vty.h
 * purpose: To add declarations required for source_interface_selection_vty.c
 ***************************************************************************/

#ifndef _SOURCE_INTERFACE_SELECTION_VTY_H
#define _SOURCE_INTERFACE_SELECTION_VTY_H

#define SOURCE_INTERFACE_STRING     "Specify source-interface utility\n"
#define ADDRESS_STRING              "Specify an IP address\n"
#define TFTP_STRING                 "TFTP protocol\n"
#define TACACS_STRING               "TACACS protocol\n"
#define RADIUS_STRING               "RADIUS protocol\n"
#define ALL_STRING                  "All the defined protocols\n"


/* Defining the type of configuration */
typedef enum {
    SOURCE_IP,
    SOURCE_INTERFACE,
    SOURCE_MAX
}CONFIG_TYPE;

/* Structure to store source-interface configuration and type. */
typedef struct protoSource_t {
    char *source;
    CONFIG_TYPE config_type;
} protoSource;

/* Defining the type of Source interface protocols passing through the cli */
typedef enum {
    TFTP_PROTOCOL,
    TACACS_PROTOCOL,
    RADIUS_PROTOCOL,
    ALL_PROTOCOL,
    PROTOCOL_MAX
}SOURCE_INTERFACE_PROTOCOL;

/* source-interface protocol keys */
static char *protocol_keys[PROTOCOL_MAX] = {
    VRF_SOURCE_INTERFACE_MAP_TFTP,
    VRF_SOURCE_INTERFACE_MAP_TACACS,
    VRF_SOURCE_INTERFACE_MAP_RADIUS,
    VRF_SOURCE_INTERFACE_MAP_ALL
};

bool
get_configured_protocol_source_local(SOURCE_INTERFACE_PROTOCOL type,
                               const char *vrf_name,
                               protoSource *proto_source);
bool
get_protocol_source_local(SOURCE_INTERFACE_PROTOCOL type,
                          const char *vrf_name,
                          protoSource *proto_source);
#endif /*_SOURCE_INTERFACE_SELECTION_VTY_H_H*/
