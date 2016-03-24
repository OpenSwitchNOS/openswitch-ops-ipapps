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
#include "udpfwd_common.h"

typedef uint32_t IP_ADDRESS;     /* IP Address. */

/* Mac address definition */
typedef uint8_t MAC_ADDRESS[6];

#define UDPHDR_LENGTH 8 /* UDP packet header length */

#define RECV_BUFFER_SIZE 9228 /* Jumbo frame size */

#define IDL_POLL_INTERVAL 5

#define IP_ADDRESS_NULL   ((IP_ADDRESS)0L)
#define IP_ADDRESS_BCAST  ((IP_ADDRESS)0xffffffff)

/* DHCP client and server port numbers. */
#define DHCPS_PORT        67
#define DHCPC_PORT        68

#define UDPFWD_DHCP_MAX_HOPS     16 /* RFC limit */

#define UDPFWD_DHCP_BROADCAST_FLAG    0x8000

/* UDP Forwarder Control Block. */
typedef struct UDPF_CTRL_CB
{
    int32_t udpSockFd;    /* Socket to send/receive UDP packets */
    sem_t waitSem;        /* Semaphore for concurrent access protection */
    struct shash intfHashTable; /* interface hash table handle */
    struct cmap serverHashMap;  /* server hash map handle */
    FEATURE_CONFIG feature_config;
    char *rcvbuff; /* Buffer which is used to store udp packet */
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
  char  *portName; /* Name of the Interface */
  uint8_t addrCount; /* Counts of configured servers */
  UDPFWD_SERVER_T **serverArray; /* Pointer to the array server configs */
  IP_ADDRESS bootp_gw; /* store bootp gateway IP address */
} UDPFWD_INTERFACE_NODE_T;

typedef enum DB_OP_TYPE_t {
    TABLE_OP_INSERT = 1,
    TABLE_OP_DELETE,
    TABLE_OP_MODIFIED,
    TABLE_OP_MAX
} TABLE_OP_TYPE_t;

/* union to store socket ancillary data */
union control_u {
    struct cmsghdr align; /* this ensures alignment */
    char control[CMSG_SPACE(sizeof(struct in_pktinfo))];
};

/* union to store pktinfo meta data */
union packet_info {
    unsigned char *c;
    struct in_pktinfo *pktInfo;
};

/*
 * Global variable declaration
 */
extern UDPFWD_CTRL_CB *udpfwd_ctrl_cb_p;

/*
 * Function prototypes from udpfwd_xmit.c
 */
void udpfwd_forward_packet (void *pkt, uint16_t udp_dport, int size,
                                 struct in_pktinfo *pktInfo);

/*
 * Function prototypes form udpfwd_config.c
 */
void udpfwd_handle_dhcp_relay_config_change(
              const struct ovsrec_dhcp_relay *rec, uint32_t idl_seqno);
void udpfwd_handle_dhcp_relay_row_delete(struct ovsdb_idl *idl);
void udpfwd_handle_udp_bcast_forwarder_row_delete(struct ovsdb_idl *idl);
void udpfwd_handle_udp_bcast_forwarder_config_change(
              const struct ovsrec_udp_bcast_forwarder_server *rec);


#endif /* udpfwd.h */
