/*
 * (c) Copyright 2016 Hewlett Packard Enterprise Development LP
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

#ifndef UDPFWD_UTIL_H
#define UDPFWD_UTIL_H

#include "udpfwd.h"

IP_ADDRESS getIpAddressfromIfname(char *ifName);
uint32_t getIfIndexfromIpAddress(IP_ADDRESS ip);
unsigned char *dhcpPickupOpt(struct dhcp_packet *dhcp,
                                   int len, unsigned char tag);
unsigned char * dhcpScanOpt(unsigned char *opt, unsigned char *optend,
                                 unsigned char tag,unsigned char *ovld_opt);

#endif
