/*
 * Copyright (C) 2016 Hewlett-Packard Development Company, L.P.
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
 * File: udpfwd.h
 */

#ifndef UDPFWD_H_
#define UDPFWD_H_

#include "hmap.h"
#include "shash.h"
#include "openvswitch/types.h"

#include <stdio.h>


/**
 * Macro definitions
 */
#define DHCP_SERVER_PORT (67)
#define RECV_BUFFER_SIZE (1024)

#define IDL_POLL_INTERVAL 5

/**
 * Structure declarations
 */
typedef struct thread_data
{
    unsigned short udpDstPort;
} thread_data_t;


#define DHCP_CHADDR_MAX          16
#define DHCP_OPTIONS_LEN         312
#define DHCP_BOOT_FILENAME_LEN   128
#define DHCP_SERVER_HOSTNAME_LEN 64
struct dhcp_packet {
  uint8_t op, htype, hlen, hops;
  uint32_t xid;
  uint16_t secs, flags;
  struct in_addr ciaddr, yiaddr, siaddr, giaddr;
  uint8_t chaddr[DHCP_CHADDR_MAX];
  uint8_t sname[DHCP_SERVER_HOSTNAME_LEN], file[DHCP_BOOT_FILENAME_LEN];
  uint8_t options[312];
};

#endif /* UDPFWD_H_ */
