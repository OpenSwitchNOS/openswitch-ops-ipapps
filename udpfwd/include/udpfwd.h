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

#define UDPHDR_LENGTH 8 /* UDP packet header length */

#define RECV_BUFFER_SIZE 9228 /* Jumbo frame size */
//#define   ETHERMTU    9198 /* FIXME : check with thimma once */

#define IDL_POLL_INTERVAL 5

#define IP_ADDRESS_NULL   ((IP_ADDRESS)0L)
#define IP_ADDRESS_BCAST  ((IP_ADDRESS)0xffffffff)

#define IPHL 	sizeof(struct ip)
#define SIZEOF_ETHERHEADER  sizeof(struct ether_header)


/* DHCP client and server port numbers. */
#define DHCPS_PORT        67
#define DHCPC_PORT        68

#define UDPFWD_DHCP_MAX_HOPS     16 /* RFC limit */

#define UDPFWD_DHCP_BROADCAST_FLAG    0x8000

/* structures needed for statistics */

typedef struct UDPF_PKT_COUNTER
{
    uint32_t     dhcp_client_drops;
    uint32_t     dhcp_client_out_pkts;
    uint32_t     dhcp_serv_drops;
    uint32_t     dhcp_serv_out_pkts;
    uint32_t     bcast_fwd_drops;
    uint32_t     bcast_fwd_out_pkts;
} UDPF_PKT_COUNTER;

typedef struct UDPF_RECV_PKT_COUNTER
{
   uint32_t h3c_dhcp_bad_pkts;
   uint32_t h3c_dhcp_received_client_pkts;
   uint32_t h3c_dhcp_received_server_pkts;
   uint32_t h3c_dhcp_discover_pkts;
   uint32_t h3c_dhcp_request_pkts;
   uint32_t h3c_dhcp_inform_pkts;
   uint32_t h3c_dhcp_release_pkts;
   uint32_t h3c_dhcp_decline_pkts;
   uint32_t h3c_dhcp_bootprequest_pkts;
   uint32_t h3c_dhcp_offer_pkts;
   uint32_t h3c_dhcp_ack_pkts;
   uint32_t h3c_dhcp_nak_pkts;
   uint32_t h3c_dhcp_bootpreply_pkts;
} UDPF_RECV_PKT_COUNTER;

typedef struct UDPF_RELAY_PKT_COUNTER
{
   uint32_t h3c_dhcp_relay_servers_pkts; /* DHCP packets relayed to servers */
   uint32_t h3c_dhcp_relay_discover_pkts;
   uint32_t h3c_dhcp_relay_request_pkts;
   uint32_t h3c_dhcp_relay_inform_pkts;
   uint32_t h3c_dhcp_relay_release_pkts;
   uint32_t h3c_dhcp_relay_decline_pkts;
   uint32_t h3c_dhcp_relay_bootprequest_pkts;
   uint32_t h3c_dhcp_relay_clients_pkts; /* DHCP packets relayed to clients */
   uint32_t h3c_dhcp_relay_offer_pkts;
   uint32_t h3c_dhcp_relay_ack_pkts;
   uint32_t h3c_dhcp_relay_nak_pkts;
   uint32_t h3c_dhcp_relay_bootpreply_pkts;
} UDPF_RELAY_PKT_COUNTER;

/* UDP Forwarder Control Block. */
typedef struct UDPF_CTRL_CB
{
    int32_t udpSockFd;    /* Socket to send/receive UDP packets */
    sem_t waitSem;        /* Semaphore for concurrent access protection */
    struct shash intfHashTable; /* interface hash table handle */
    struct cmap serverHashMap;  /* server hash map handle */
    FEATURE_CONFIG feature_config;
    char *rcvbuff; /* Buffer which is used to store udp packet */
    UDPF_PKT_COUNTER udp_counters; /* Packet counter for staticstics*/
    UDPF_RECV_PKT_COUNTER udp_counters_recv;    /*counter for packet received from server and client */
    UDPF_RELAY_PKT_COUNTER udp_counters_relay;  /*counter  packet relayed to server and client */
} UDPFWD_CTRL_CB;

/* The following macros will return pkt counters values  */
#define UDPF_DHCPR_CLIENT_DROPS(ifIndex)  \
            udpfwd_ctrl_cb_p->udp_counters.dhcp_client_drops
#define UDPF_DHCPR_CLIENT_SENT(ifIndex)  \
            udpfwd_ctrl_cb_p->udp_counters.dhcp_client_out_pkts
#define UDPF_DHCPR_SERVER_DROPS(ifIndex)  \
            udpfwd_ctrl_cb_p->udp_counters.dhcp_serv_drops
#define UDPF_DHCPR_SERVER_SENT(ifIndex)  \
            udpfwd_ctrl_cb_p->udp_counters.dhcp_serv_out_pkts
#define UDPF_BCAST_FORWARD_DROPS(ifIndex)  \
            udpfwd_ctrl_cb_p->udp_counters.bcast_fwd_drops
#define UDPF_BCAST_FORWARD_SENT(ifIndex)  \
            udpfwd_ctrl_cb_p->udp_counters.bcast_fwd_out_pkts

#define UDPF_DHCPRECV_BAD_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_bad_pkts
#define UDPF_DHCPRECV_CLIENT_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_received_client_pkts
#define UDPF_DHCPRECV_SERVER_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_received_server_pkts
#define UDPF_DHCPRECV_DISCOVER_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_discover_pkts
#define UDPF_DHCPRECV_REQUEST_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_request_pkts
#define UDPF_DHCPRECV_INFORM_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_inform_pkts
#define UDPF_DHCPRECV_RELEASE_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_release_pkts
#define UDPF_DHCPRECV_DECLINE_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_decline_pkts
#define UDPF_DHCPRECV_BOOTPREQUEST_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_bootprequest_pkts
#define UDPF_DHCPRECV_OFFER_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_offer_pkts
#define UDPF_DHCPRECV_ACK_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_ack_pkts
#define UDPF_DHCPRECV_NAK_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_nak_pkts
#define UDPF_DHCPRECV_BOOTPREPLY_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_bootpreply_pkts

#define UDPF_DHCPRELAY_SERVERS_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_servers_pkts
#define UDPF_DHCPRELAY_DISCOVER_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_discover_pkts
#define UDPF_DHCPRELAY_REQUEST_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_request_pkts
#define UDPF_DHCPRELAY_INFORM_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_inform_pkts
#define UDPF_DHCPRELAY_RELEASE_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_release_pkts
#define UDPF_DHCPRELAY_DECLINE_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_decline_pkts
#define UDPF_DHCPRELAY_BOOTPREQUEST_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_bootprequest_pkts
#define UDPF_DHCPRELAY_CLIENTS_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_clients_pkts
#define UDPF_DHCPRELAY_OFFER_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_offer_pkts
#define UDPF_DHCPRELAY_ACK_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_ack_pkts
#define UDPF_DHCPRELAY_NAK_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_nak_pkts
#define UDPF_DHCPRELAY_BOOTPREPLY_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_bootpreply_pkts


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
              const struct ovsrec_dhcp_relay *rec);
void udpfwd_handle_dhcp_relay_row_delete(struct ovsdb_idl *idl);
void udpfwd_handle_udp_bcast_forwarder_row_delete(struct ovsdb_idl *idl);
void udpfwd_handle_udp_bcast_forwarder_config_change(
              const struct ovsrec_udp_bcast_forwarder_server *rec);

#endif /* udpfwd.h */
