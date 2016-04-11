/* UDP Forwarder common header file
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
 * File: udpfwd_common.h
 *
 * Purpose:  To add common declarations for UDP Forwarder
 */

#ifndef UDPFWD_COMMON_H
#define UDPFWD_COMMON_H

#include "openvswitch/types.h"

/* Feature bit map */
typedef uint16_t feature_bmap;

/* Feature enumeration bit positions */
typedef enum FEATURE_BIT
{
    /* UDP broadcast forwarder feature bit */
    UDP_BCAST_FORWARDER_BIT           = 0x0001,
    /* DHCP relay feature bit */
    DHCP_RELAY_BIT                    = 0x0002,
    /* DHCP packet hop count increment feature bit */
    DHCP_RELAY_HOP_COUNT_INCR_BIT     = 0x0004,
    /* DHCP relay option 82 feature bit */
    DHCP_RELAY_OPTION82_BIT           = 0x0008,
    /* DHCP relay option 82 server resonse validate bit */
    DHCP_RELAY_OPTION82_VALIDATE_BIT  = 0x0020,
    /* Invalid feature */
    INVALID_FEATURE_BIT               = 0x0040
} FEATURE_BIT;

/* Feature enumeration */
typedef enum UDPFWD_FEATURE
{
    UDP_BCAST_FORWARDER = 0,
    DHCP_RELAY,
    DHCP_RELAY_HOP_COUNT_INCREMENT,
    DHCP_RELAY_OPTION82,
    DHCP_RELAY_OPTION82_VALIDATE
} UDPFWD_FEATURE;

/* Feature to name mapping. There should be exact one-to-one mapping
 * between UDPFWD_FEATURE enum and feature_name array */
extern char *feature_name[];

/* DHCP-Relay option 82 policy types */
typedef enum DHCP_RELAY_OPTION82_POLICY
{
    KEEP = 0,
    DROP,
    REPLACE,
    INVALID
} DHCP_RELAY_OPTION82_POLICY;

/* Policy to name mapping. There should be strict one-to-one mapping
 * between DHCP_RELAY_OPTION82_POLICY and policy_name array */
extern char *policy_name[];

typedef enum dhcp_relay_option82_remote_id
{
    REMOTE_ID_IP = 0,
    REMOTE_ID_MAC,
    REMOTE_ID_INVALID
} DHCP_RELAY_OPTION82_REMOTE_ID;

/* remote-id to name mapping. There should be strict one-to-one mapping
 * between DHCP_RELAY_OPTION82_REMOTE_ID and policy_name array */
extern char *remote_id_name[];

/* Feature configuration status */
typedef enum feature_status
{
    DISABLE,
    ENABLE
} FEATURE_STATUS;

/* Status of the feature configurations */
typedef struct FEATURE_CONFIG
{
    feature_bmap config;
    DHCP_RELAY_OPTION82_POLICY    policy;
    DHCP_RELAY_OPTION82_REMOTE_ID r_id;
} FEATURE_CONFIG;

/* DHCP-Relay statistics types */
typedef enum RELAY_STATISTICS
{
    VALID_V4CLIENT_REQUESTS = 0,
    DROPPED_V4CLIENT_REQUESTS,
    VALID_V4SERVER_RESPONSES,
    DROPPED_V4SERVER_RESPONSES,
    VALID_V4CLIENT_REQUESTS_WITH_OPTION82,
    DROPPED_V4CLIENT_REQUESTS_WITH_OPTION82,
    VALID_V4SERVER_RESPONSES_WITH_OPTION82,
    DROPPED_V4SERVER_RESPONSES_WITH_OPTION82
}RELAY_STATISTICS;
/* IP Address string maximum length(Ex : 255.255.255.255). */
#define IPADDRESS_STRING_MAX_LENGTH   (15)

/* Maximum entries allowed on an interface. */
#define MAX_HELPER_ADDRESSES_PER_INTERFACE   (8)

/* Maximum UDP server IPs on an interface. */
#define MAX_UDPFWD_SERVER_ADDRESSES_PER_INTERFACE   (8)

/* Sub interface string. */
#define SUBIFNAME_STR \
"Subinterface name\n"

/* Maximum number of entries allowed per INTERFACE. */
#define MAX_UDP_BCAST_SERVER_PER_INTERFACE \
            (MAX_HELPER_ADDRESSES_PER_INTERFACE + \
             MAX_UDPFWD_SERVER_ADDRESSES_PER_INTERFACE)

#endif /* udpfwd_common.h */
