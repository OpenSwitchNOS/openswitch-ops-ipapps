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

#include "udpfwd_common.h"
#include "dhcp_relay.h"
#include "udpfwd.h"
#include <netinet/ip.h>
#include <netinet/udp.h>

#define MAX_UINT32 4294967295 /*255.255.255.255.255 */

/* Function to retrieve lowest IP address from interface name. */
IP_ADDRESS getLowestIpOnInterface(char *ifName);

/* Function to check if ip address exists on an interface. */
bool ipExistsOnInterface(char *ifName, IP_ADDRESS ip);

/* Function to retrieve interface index from IP address. */
uint32_t getIfIndexfromIpAddress(IP_ADDRESS ip);

/* Function to retrieve mac address. */
void getMacfromIfname(MAC_ADDRESS mac, char*ifName);

/* Set get routines for feature configuration */
FEATURE_STATUS get_feature_status(uint16_t value, UDPFWD_FEATURE feature);
void set_feature_status(uint16_t *value, UDPFWD_FEATURE feature,
                        FEATURE_STATUS status);
/* Checksum computation function */
uint16_t in_cksum(const uint16_t *addr, register int32_t len, uint16_t csum);

#endif /* udpfwd_util.h */
