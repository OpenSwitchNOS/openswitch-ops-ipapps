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
 * File: udpfwd.h
 */

/*
 * This file has the all the definitions of structures, enums
 * and declaration of functions related to DHCP Relay and UDP Broadcast
 * forwarder features.
 */

#ifndef UDPFWD_H
#define UDPFWD_H 1

#include "shash.h"
#include "cmap.h"
#include "semaphore.h"
#include "openvswitch/types.h"
#include "openvswitch/vlog.h"
#include "vswitch-idl.h"
#include "openswitch-idl.h"

#include <stdio.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <net/if.h>
#include <assert.h>

typedef uint32_t IP_ADDRESS;     /* IP Address. */

#define IFNAME_LEN 16  /* Maximum length of an interface name */

#define RECV_BUFFER_SIZE 1024
#define IDL_POLL_INTERVAL 5

#define IP_ADDRESS_NULL   ((IP_ADDRESS)0L)
#define IP_ADDRESS_BCAST  ((IP_ADDRESS)0xffffffff)

/**
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
#define NO_RELAY                 3

/* DHCP client and server port numbers. */
#define DHCPS_PORT        67
#define DHCPC_PORT        68

/* BOOTP operation types. */
#define BOOTREQUEST  1
#define BOOTREPLY    2

/* DHCP packet field lengths. */
#define DFLTOPTLEN   312    /* Default Option fields length */
#define DHCP_CHADDR_MAX          16
#define DHCP_BOOT_FILENAME_LEN   128
#define DHCP_SERVER_HOSTNAME_LEN 64

/* Maximum number of entries allowed per INTERFACE. */
#define MAX_UDP_BCAST_SERVER_PER_INTERFACE 16

/* This flag indicate whether the module is enabled or disabled.*/
#define FILE_ISOPT  1
#define SNAME_ISOPT  2
#define BOTH_AREOPT FILE_ISOPT + SNAME_ISOPT
#define MAGIC_LEN   4        /* length of the magic cookie */

#define UDPFWD_DHCP_MAX_HOPS     16 /* RFC limit */

#define UDPFWD_DHCP_BROADCAST_FLAG    0x8000

/* UDP Forwarder Control Block. */
typedef struct UDPF_CTRL_CB
{
    int32_t send_sockFd;  /* Socket ID used for sending packets */
    sem_t waitSem;        /* Semaphore for concurrent access protection */
    struct shash intfHashTable; /* interface hash table handle */
    struct cmap serverHashMap;  /* server hash map handle */
    bool dhcp_relay_enable;     /* Flag to store DHCP_Relay global status */
} UDPFWD_CTRL_CB;

/* Server Address structure. */
typedef struct UDPFWD_SERVER_T {
  struct cmap_node cmap_node; /* cmap Node, used for hashing */
  IP_ADDRESS ip_address; /* Server IP address */
  uint16_t   udp_port;   /* UDP Port Number */
  uint16_t   ref_count;  /* Counts how many interfaces are using the serverIP.
                            This field helps in deleting a server entry */
} UDPFWD_SERVER_T;

/* Interface Table Structure. */
typedef struct UDPFWD_INTERFACE_NODE_T
{
  char portName[IFNAME_LEN + 1]; /* Name of the Interface */
  uint8_t   addrCount;           /* Counts of configured servers */
  UDPFWD_SERVER_T **serverArray; /* Pointer to the array server configs */
} UDPFWD_INTERFACE_NODE_T;

typedef enum DB_OP_TYPE_t {
    TABLE_OP_INSERT = 1,
    TABLE_OP_DELETE,
    TABLE_OP_MODIFIED,
    TABLE_OP_MAX
} TABLE_OP_TYPE_t;

/* Option values for DHCP message type (DHCP_OPT_MESSAGE_TYPE). */
typedef enum DHCP_MSG_TYPE_t {
   DHCPDISCOVER=1,
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
 * Global variable declaration
 */
extern UDPFWD_CTRL_CB *udpfwd_ctrl_cb_p;

/*
 * Function prototypes from udpfwd_xmit.c
 */
bool udpfwd_relay_to_dhcp_server(struct dhcp_packet* pkt_dhcp, int32_t size,
                   struct in_pktinfo *pktInfo);
bool udpfwd_relay_to_dhcp_client(struct dhcp_packet* pkt_dhcp, int32_t size,
                                   struct in_pktinfo *pktInfo);

/*
 * Function prototypes form udpfwd_config.c
 */
void udpfwd_handle_dhcp_relay_config_change(
              const struct ovsrec_dhcp_relay *rec);
void udpfwd_handle_dhcp_relay_row_delete(struct ovsdb_idl *idl);

#endif /* udpfwd.h */
