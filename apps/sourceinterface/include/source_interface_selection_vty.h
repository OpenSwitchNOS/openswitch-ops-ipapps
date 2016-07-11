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

#define TFTP                        "tftp"
#define ALL                         "all"

#define SOURCE_STRING               "Specify source-interface utility\n"
#define ADDRESS_STRING              "Specify an IP address\n"
#define TFTP_STRING                 "TFTP protocol\n"
#define ALL_STRING                  "All the defined protocols\n"

/* FIXME: Will be removed once the macros got merged into openswitch-idl.h */
#define VRF_SOURCE_IP_MAP_ALL               "all"
#define VRF_SOURCE_INTERFACE_MAP_ALL        "all"
#define VRF_SOURCE_IP_MAP_TFTP              "tftp"
#define VRF_SOURCE_INTERFACE_MAP_TFTP       "tftp"

/* Structure to store the source-interface and type, wether it is
 ip address or interface */
typedef struct PROTO_SOURCE_T {
    char *source;
    bool isIp;
} PROTO_SOURCE;

/* Defining the type of arguments passing through the cli */
typedef enum {
    TFTP_PROTOCOL,
    ALL_PROTOCOL
}SOURCE_INTERFACE_PROTOCOL;

bool
get_configured_protocol_source_local(PROTO_SOURCE *,
                           SOURCE_INTERFACE_PROTOCOL,
                           const char *);
bool
get_common_protocol_source_local(PROTO_SOURCE *,
                           const char *);
bool
get_protocol_source_local(PROTO_SOURCE *,
                           SOURCE_INTERFACE_PROTOCOL,
                           const char *);
#endif /*_SOURCE_INTERFACE_SELECTION_VTY_H_H*/
