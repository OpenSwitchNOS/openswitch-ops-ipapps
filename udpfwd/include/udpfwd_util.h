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
#include <netinet/ip.h>
#include <netinet/udp.h>

#define getShortFromPacket(shortPtr)                   \
   (uint16_t)(((*(uint8_t *)(shortPtr)) << 8) |        \
             (*(((uint8_t *)(shortPtr)) + 1)))

/* Function to retrieve IP address from interface name. */
IP_ADDRESS getIpAddressfromIfname(char *ifName);

/* Function to retrieve interface index from IP address. */
uint32_t getIfIndexfromIpAddress(IP_ADDRESS ip);

/* Functions to search for a specified option tag in the dhcp packet. */
uint8_t *dhcpPickupOpt(struct dhcp_packet *dhcp, int32_t len, uint8_t tag);
uint8_t * dhcpScanOpt(uint8_t *opt, uint8_t *optend,
                        uint8_t tag, uint8_t *ovld_opt);

/* Checksum computation function */
uint32_t get_ipsum(uint16_t *data, int32_t len, uint32_t initialValue);
uint16_t get_udpsum (struct ip *iph, struct udphdr *udph);

#endif /* udpfwd_util.h */
