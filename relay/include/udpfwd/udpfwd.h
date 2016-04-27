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
#include "ovsdb-idl.h"

#include <stdio.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/udp.h>
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

/* statistics refresh interval key */
#define SYSTEM_OTHER_CONFIG_MAP_STATS_UPDATE_INTERVAL \
"stats-update-interval"

/* statistics refresh default interval  */
#define STATS_UPDATE_DEFAULT_INTERVAL    5000

#ifdef FTR_DHCP_RELAY
/* structure needed for statistics counters */
typedef struct DHCP_RELAY_PKT_COUNTER
{
    uint32_t    client_drops; /* number of dropped client requests */
    uint32_t    client_valids; /* number of valid client requests */
    uint32_t    serv_drops; /* number of dropped server responses */
    uint32_t    serv_valids; /* number of valid server responses */
    uint32_t    client_drops_with_option82; /* number of dropped client
                                               requests with option 82 */
    uint32_t    client_valids_with_option82; /* number of valid client
                                                requests with option 82 */
    uint32_t    serv_drops_with_option82; /* number of dropped server
                                             responses with option 82 */
    uint32_t    serv_valids_with_option82; /* number of valid server
                                              responses with option 82 */
} DHCP_RELAY_PKT_COUNTER;
#endif /* FTR_DHCP_RELAY */

/* Pseudo header for udp checksum computation */
struct pseudoheader {
    u_int32_t src_addr;
    u_int32_t dst_addr;
    u_int8_t padding;
    u_int8_t proto;
    u_int16_t length;
};

/* UDP checksum construct */
struct csum_construct {
    struct pseudoheader pshd;
    struct udphdr udph;
    char payload[RECV_BUFFER_SIZE];
};

/* UDP Forwarder Control Block. */
typedef struct UDPF_CTRL_CB
{
    int32_t udpSockFd;    /* Socket to send/receive UDP packets */
    sem_t waitSem;        /* Semaphore for concurrent access protection */
    struct shash intfHashTable; /* interface hash table handle */
    struct cmap serverHashMap;  /* server hash map handle */
    FEATURE_CONFIG feature_config;
    char *rcvbuff; /* Buffer which is used to store udp packet */
    int32_t stats_interval;    /* statistics refresh interval */
    struct csum_construct udp_csum_construct; /* UDP checksum construct */
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
#ifdef FTR_DHCP_RELAY
  DHCP_RELAY_PKT_COUNTER dhcp_relay_pkt_counters; /* Counts of dhcp-relay
                                                     statistics */
#endif /* FTR_DHCP_RELAY */
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
 * Function prototypes from udpfwd.c
 */
extern bool udpfwd_init(void);
extern void udpfwd_reconfigure(void);
extern void udpfwd_exit(void);

/*
 * Function prototypes from udpfwd_xmit.c
 */
void udpfwd_forward_packet (void *pkt, uint16_t udp_dport, int32_t size,
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
void refresh_dhcp_relay_stats(void);

#endif /* udpfwd.h */
