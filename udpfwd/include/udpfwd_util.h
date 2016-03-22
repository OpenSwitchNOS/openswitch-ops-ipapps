/*
 * Copyright (C) 2016 Hewlett Packard Enterprise Development LP
 * All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may
 *   not use this file except in compliance with the License. You may obtain
 *   a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 *
 * File: udpfwd_util.h
 */

/*
 * This file has all the utility functions
 * related to DHCP relay functionality and also functions
 * to get ifIndex from IP address, get IpAddress from ifName.
 */

#ifndef UDPFWD_UTIL_H
#define UDPFWD_UTIL_H 1

#include "dhcp_relay.h"
#include "udpfwd_common.h"
#include <netinet/ip.h>
#include <netinet/udp.h>

/* Pseudo header for UDP checksum computation */
struct ipovly {
    caddr_t  ih_next, ih_prev; /* for protocol sequence q's */
    u_char   ih_x1;            /* (unused) */
    u_char   ih_pr;            /* protocol */
    short    ih_len;           /* protocol length */
    struct   in_addr ih_src;   /* source internet address */
    struct   in_addr ih_dst;   /* destination internet address */
};


/* Function to retrieve IP address from interface name. */
IP_ADDRESS getIpAddressfromIfname(char *ifName);

/* Function to retrieve interface index from IP address. */
uint32_t getIfIndexfromIpAddress(IP_ADDRESS ip);

/* Set get routines for feature configuration */
FEATURE_STATUS get_feature_status(uint16_t value, UDPFWD_FEATURE feature);
void set_feature_status(uint16_t *value, UDPFWD_FEATURE feature,
                        FEATURE_STATUS status);
/* Functions to search for a specified option tag in the dhcp packet. */
uint8_t *dhcpPickupOpt(struct dhcp_packet *dhcp, int32_t len, uint8_t tag);
uint8_t * dhcpScanOpt(uint8_t *opt, uint8_t *optend,
                        uint8_t tag, uint8_t *ovld_opt);

/* Checksum computation function */
uint16_t in_cksum(const uint16_t *addr, register int32_t len, uint16_t csum);


#endif /* udpfwd_util.h */
