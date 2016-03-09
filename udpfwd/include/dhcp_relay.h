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
 * File: dhcp_relay.h
 */

/*
 * This file has the all the definitions of structures, enums
 * and declaration of functions related to DHCP Relay.
 */

#ifndef DHCP_RELAY_H
#define DHCP_RELAY_H 1

#include "udpfwd.h"

/* BOOTP operation types. */
#define BOOTREQUEST  1
#define BOOTREPLY    2

/*
 * DHCP-Relay macro definitions
 */
#define DHCPOPTLEN(TAGP)   (*(((char *)TAGP) + 1)) /* Get DHCP option length */
#define DFLTDHCPLEN   sizeof(struct dhcp_packet)   /* Default DHCP message size */
#define OPTBODY(TAGP)  (((char *)TAGP) + 2)        /* Get contents of DHCP option */

#define DHCP_MSGTYPE         ((uint8_t)  53)
#define DHCP_SERVER_ID       ((uint8_t)  54)
#define PAD                  ((uint8_t)   0)
#define END                  ((uint8_t) 255)
#define OPT_OVERLOAD         ((uint8_t)  52)
#define NO_RELAY              3

/* This flag indicate whether the module is enabled or disabled.*/
#define FILE_ISOPT  1
#define SNAME_ISOPT  2
#define BOTH_AREOPT FILE_ISOPT + SNAME_ISOPT
#define MAGIC_LEN   4        /* length of the magic cookie */

/* DHCP packet field lengths. */
#define DFLTOPTLEN   312    /* Default Option fields length */
#define DHCP_CHADDR_MAX          16
#define DHCP_BOOT_FILENAME_LEN   128
#define DHCP_SERVER_HOSTNAME_LEN 64

/* Option values for DHCP message type (DHCP_OPT_MESSAGE_TYPE). */
typedef enum DHCP_MSG_TYPE_t {
   DHCPDISCOVER = 1,
   DHCPOFFER,
   DHCPREQUEST,
   DHCPDECLINE,
   DHCPACK,
   DHCPNAK,
   DHCPRELEASE,
   DHCPINFORM,
   DHCPFORCERENEW,
   DHCPMSGTYPEMAX
} DHCP_MSG_TYPE_t;

/* DHCP message format. */
struct dhcp_packet {
  uint8_t   op;      /* Packet type. */
  uint8_t   htype;   /* Hardware address type. */
  uint8_t   hlen;    /* Hardware address length. */
  uint8_t   hops;    /* Gateway hops. */
  uint32_t  xid;     /* Transaction ID. */
  uint16_t  secs;    /* Seconds since boot began. */
  uint16_t  flags;   /* DHCP flags. */
  struct in_addr  ciaddr;   /* Client IP address. */
  struct in_addr  yiaddr;   /* 'Your' IP address. */
  struct in_addr  siaddr;   /* Server IP address. */
  struct in_addr  giaddr;   /* Relay agent IP address. */
  uint8_t   chaddr[DHCP_CHADDR_MAX];     /* Client hardware address. */
  uint8_t   sname[DHCP_SERVER_HOSTNAME_LEN];     /* Server host name. */
  uint8_t   file[DHCP_BOOT_FILENAME_LEN];       /* Boot file name. */
  uint8_t   options[DFLTOPTLEN];  /* Optional parameters field. */
};

/*
 * Function prototypes from udpfwd_xmit.c
 */
bool udpfwd_relay_to_dhcp_server(struct dhcp_packet* pkt_dhcp, int32_t size,
                   struct in_pktinfo *pktInfo);
bool udpfwd_relay_to_dhcp_client(struct dhcp_packet* pkt_dhcp, int32_t size,
                                   struct in_pktinfo *pktInfo);

#endif /* dhcp_relay.h */
