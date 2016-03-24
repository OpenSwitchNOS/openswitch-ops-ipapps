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
#define BOOTPOPTLEN   64
#define DHCPOPTLEN(TAGP) (*(((char *)TAGP) + 1)) /* Get DHCP option length */
#define DFLTDHCPLEN      sizeof(struct dhcp_packet) /* Default DHCP message size */
#define OPTBODY(TAGP)    (((char *)TAGP) + 2) /* Get contents of DHCP option */
#define DHCP_PKTLEN(UDP)      (ntohs(UDP->uh_ulen) - UDPHDR_LENGTH)
#define DFLTBOOTPLEN  (DFLTDHCPLEN - DFLTOPTLEN + BOOTPOPTLEN)

/*
 * Min bootp len is the default dhcp pkt struct size, less the option buffer
 * length, plus room for a cookie (4 bytes) and an end marker (1 byte).
 */
#define MINBOOTPLEN  (DFLTDHCPLEN - DFLTOPTLEN + 5)

#define PAD                  ((uint8_t)   0)
#define OPT_OVERLOAD         ((uint8_t)  52)
#define DHCP_MSGTYPE         ((uint8_t)  53)
#define DHCP_SERVER_ID       ((uint8_t)  54)
#define DHCP_MAXMSGSIZE      ((uint8_t)  57)
#define DHCP_AGENT_OPTIONS   ((uint8_t)  82)
#define END                  ((uint8_t) 255)

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
#define RFC1048_MAGIC { 99, 130, 83, 99 }   /* magic cookie for RFC1048 */

/* DHCP msg maximum size */
#define MAX_DHCP_MESSAGE_SIZE 576

#define MAC_HEADER_LENGTH        6

typedef uint32_t CIRCUIT_ID_t;
typedef IP_ADDRESS REMOTE_ID_IP_ADDR_t;

typedef struct DHCP_OPTION_82_OPTIONS
{
   CIRCUIT_ID_t  circuit_id;
   REMOTE_ID_IP_ADDR_t ip_addr;
} DHCP_OPTION_82_OPTIONS;

/* invalid message type or options */
#define DHCPR_INVALID_PKT -1
#define DHCP_RELAY_INVALID_OPTION_82 -1
#define DHCP_RELAY_OPTION_82_MISMATCH 0
#define DHCP_RELAY_OPTION_82_OK 1

#define DHCP_OPTION_HEADER_LENGTH 2

/* DHCP Relay Agent Information Sub Option Tag Values */

/* Dhcp Relay Agent Circuit ID field */
#define DHCP_RAI_CIRCUIT_ID   ((unsigned char) 1)

/* Dhcp Relay Agent Remote ID field */
#define DHCP_RAI_REMOTE_ID   ((unsigned char) 2)

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

/* pseudo udp header for checksum computation */
struct ps_udph {
  struct in_addr srcip;
  struct in_addr dstip;
  int8_t  zero;
  int8_t  proto;
  int16_t ulen;
};


/* Function prototypes from dhcp_options.c */
int32_t dhcp_relay_get_option82_len(DHCP_RELAY_OPTION82_REMOTE_ID remote_id);

int32_t dhcp_relay_validate_agent_option(const uint8_t *buf, int32_t buflen,
                               char* ifName, DHCP_OPTION_82_OPTIONS *pkt_info,
                               DHCP_RELAY_OPTION82_REMOTE_ID remote_id);

bool process_dhcp_relay_option82_message(void *pkt, DHCP_OPTION_82_OPTIONS *pkt_info,
                               uint32_t ifIndex, char *ifName,
                               IP_ADDRESS bootp_gw);

/*
 * Function prototypes from udpfwd_xmit.c
 */
void udpfwd_relay_to_dhcp_server(void* pkt, int32_t size,
                   struct in_pktinfo *pktInfo);
void udpfwd_relay_to_dhcp_client(void* pkt, int32_t size,
                                   struct in_pktinfo *pktInfo);

#endif /* dhcp_relay.h */
