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
 * File: dhcpv6_relay.h
 */

/*
 * This file has the all the definitions of structures, enums
 * and declaration of functions related to DHCPv6 Relay feature.
 */

#ifndef DHCPV6_RELAY_H
#define DHCPV6_RELAY_H 1

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
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <net/if.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/udp.h>

#ifdef FTR_DHCPV6_RELAY

#define ETHER_ADDR_TYPE 1
#define UDP_HEADER 8

/* Mac address definition */
typedef uint8_t MAC_ADDRESS[6];

/* structure needed for statistics counters */
typedef struct DHCPV6_RELAY_PKT_COUNTER
{
    uint32_t    client_drops; /* number of dropped client requests */
    uint32_t    client_valids; /* number of valid client requests */
    uint32_t    serv_drops; /* number of dropped server responses */
    uint32_t    serv_valids; /* number of valid server responses */
} DHCPV6_RELAY_PKT_COUNTER;

/* DHCPV6 Relay Control Block. */
typedef struct DHCPV6_RELAY_CTRL_CB
{
    int32_t dhcpv6r_recvSock;    /* Socket to receive DHCP packets */
    int32_t dhcpv6r_sendSock; /* Socket to send DHCP packets */
    sem_t waitSem;        /* Semaphore for concurrent access protection */
    bool dhcpv6_relay_enable; /* Flag to store dhcpv6-relay global status */
    bool dhcpv6_relay_option79_enable; /* Flag to store dhcpv6-relay option 79 status */
    struct shash intfHashTable; /* interface hash table handle */
    struct cmap serverHashMap;  /* server hash map handle */
    char *rcvbuff; /* Buffer which is used to store ipv6 packet */
    int32_t stats_interval;    /* statistics refresh interval */
    struct in6_addr agentIpv6Address; /* Store the DHCPv6 Relay Agents and Servers IPv6 address */
} DHCPV6_RELAY_CTRL_CB;

/* Server Address structure. */
typedef struct DHCPV6_RELAY_SERVER_T {
    struct cmap_node cmap_node; /* cmap Node, used for hashing */
    char* ipv6_address; /* Server ipv6 address */
    uint16_t   ref_count;  /* Counts how many interfaces are using the serverIP.
                            This field helps in deleting a server entry */
    char *egressIfName; /* Name of outgoing interface */
} DHCPV6_RELAY_SERVER_T;

/* Interface Table Structure. */
typedef struct DHCPV6_RELAY_INTERFACE_NODE_T
{
    char  *portName; /* Name of the Interface */
    uint8_t addrCount; /* Counts of configured servers */
    DHCPV6_RELAY_SERVER_T **serverArray; /* Pointer to the array server configs */
    DHCPV6_RELAY_PKT_COUNTER dhcpv6_relay_pkt_counters; /* Counts of dhcp-relay
                                                       statistics */
    char *ip6_address; /*ip address configured on the interface */
} DHCPV6_RELAY_INTERFACE_NODE_T;

/* Macros for dhcpv6-relay statistics counters */
#define INC_DHCPV6R_CLIENT_DROPS(intfNode)  \
            intfNode->dhcpv6_relay_pkt_counters.client_drops++
#define INC_DHCPV6R_CLIENT_SENT(intfNode)  \
            intfNode->dhcpv6_relay_pkt_counters.client_valids++
#define INC_DHCPV6R_SERVER_DROPS(intfNode)  \
            intfNode->dhcpv6_relay_pkt_counters.serv_drops++
#define INC_DHCPV6R_SERVER_SENT(intfNode)  \
            intfNode->dhcpv6_relay_pkt_counters.serv_valids++

/* The following macros will return pkt counters values  */
#define DHCPV6R_CLIENT_DROPS(intfNode)  \
            intfNode->dhcpv6_relay_pkt_counters.client_drops
#define DHCPV6R_CLIENT_SENT(intfNode)  \
            intfNode->dhcpv6_relay_pkt_counters.client_valids
#define DHCPV6R_SERVER_DROPS(intfNode)  \
            intfNode->dhcpv6_relay_pkt_counters.serv_drops
#define DHCPV6R_SERVER_SENT(intfNode)  \
            intfNode->dhcpv6_relay_pkt_counters.serv_valids

/* Maximum number of entries allowed per INTERFACE. */
#define MAX_SERVERS_PER_INTERFACE 8

/* DHCPv6 base message structure */
typedef struct dhcpv6_basepkt {
    union {
        uint8_t msgtype;
        uint32_t transid;
    } dhcpv6c_msgtypexid;
   /* Options follow */
} dhcpv6_basepkt;

#pragma pack(1)
/* DHCPv6 relay base packet structure */
typedef struct dhcpv6r_basePkt_t {
    uint8_t msgType;
    uint8_t hopCount;
    struct in6_addr linkAddress;
    struct in6_addr peerAddress;
} dhcpv6r_basePkt_t;
#pragma pack(1)

/* DHCPv6 options base format */
typedef struct dhcpv6_opt_t {
    uint16_t opt_type;
    uint16_t opt_len;
   /* Type dependent data follows */
} dhcpv6_opt_t;

/*DHCPv6 client MAC option format RFC6939*/
typedef struct {
    uint16_t hwtype;
    MAC_ADDRESS macaddr;
} dhcpv6_clientmac_Opt_t;

/* DHCPv6 client information for relaying to Clients */
typedef struct {
    uint32_t clientIntf;
    struct in6_addr clientAddress;
} dhcpv6r_clientInfo_t;

/* DHCPv6 multicast destination address */
#define DHCPV6_ALLAGENTS    "ff02::1:2"
#define DHCPV6_ALLSERVERS   "FF05::1:3"

/* DHCPv6 relay port */
#define DHCPV6_RELAY_PORT 547

/* Size of Relay messages that we allow */
#define DHCPV6_RELAY_MSG_SIZE   1024

/* Maximum number of helper addresses supported. */
#define DHCPV6R_MAX_HELPER_ADDRS  32

/* Maximum hop count limit. */
#define DHCPV6R_MAX_HOPCOUNT 32

#define DHCPV6R_MULTICAST_HOPLIMIT 32
#define OPT_RELAY_MSG      9

#define OPT_INTF_ID        18

#define DHCPV6_CL_PORT    546

/* OPTION_CLIENT_LINKLAYER_ADDR defined in RFC6939 */
#define OPT_CLIENT_LINKLAYER_ADDR 79

/* DHCPv6 message types as per IANA. */
#define SOLICIT_MSG          0x01000000
#define ADVERTISE_MSG        0x02000000
#define REQUEST_MSG          0x03000000
#define CONFIRM_MSG          0x04000000
#define RENEW_MSG            0x05000000
#define REBIND_MSG           0x06000000
#define REPLY_MSG            0x07000000
#define RELEASE_MSG          0x08000000
#define DECLINE_MSG          0x09000000
#define RECONFIGURE_MSG      0x0a000000
#define INFOREQ_MSG          0x0b000000
#define RELAY_FORW_MSG       0x0c000000
#define RELAY_REPLY_MSG      0x0d000000
#define LEASEQUERY_MSG       0x0e000000
#define LEASEQUERY_REPLY_MSG 0x0f000000

#define DHCPV6_XID_MASK   0x00ffffff
#define DHCPV6_MSG_MASK   0xff000000
#define DHCPV6_IAID_IFINDEX_MASK 0xffff0000
#define DHCPV6_DEF_XID    0x11223344

/* Global variable declaration */
extern DHCPV6_RELAY_CTRL_CB *dhcpv6_relay_ctrl_cb_p;

/* Function prototypes from dhcpv6r_common.c */
bool dhcpv6r_isEnabled();
bool dhcpv6r_isValidMsg(uint32_t msgType);
bool dhcpv6r_isMsgFromClient(uint32_t msgType);

/* Function prototypes from dhcpv6_relay.c */
extern void dhcpv6r_exit(void);
extern bool dhcpv6r_init(void);
extern void dhcpv6r_reconfigure(void);

/*
 * Function prototypes from dhcpv6_relay_config.c
 */
void dhcpv6r_handle_config_change(
              const struct ovsrec_dhcp_relay *rec, uint32_t idl_seqno);
void dhcpv6r_handle_row_delete(struct ovsdb_idl *idl);

/* Function prototypes from dhcpv6r_xmit.c */
bool dhcpv6r_relay_to_server (dhcpv6r_basePkt_t *rxPktPtr, int rxPktLen,
                           struct sockaddr_in6 srcAddr,
                           uint32_t srcRecv_ifindex);
bool dhcpv6r_send (char *sBuf, uint32_t pktLen, bool relayToServer,
                  uint32_t ifIndex, void *data, bool sendToRelay,
                  DHCPV6_RELAY_INTERFACE_NODE_T *intfNode, struct sockaddr_in6 srcAddr);

bool dhcpv6r_relay_to_client (dhcpv6r_basePkt_t *pktPtr, int pktLen);

/* Function prototypes from dhcpv6r_util.c */
bool dhcpv6r_is_valid_msg (uint32_t msgType);
bool dhcpv6r_is_msg_from_client (uint32_t msgType);
bool dhcpv6r_is_msg_from_server (uint32_t msgType);

/* Function prototypes from dhcpv6r_recv.c */
void * dhcpv6r_recv(void *args);
bool dhcpv6_relay_join_or_leave_mcast_group
    (DHCPV6_RELAY_INTERFACE_NODE_T *intfNode, bool joinFlag);

bool dhcpv6r_get_client_mac
    (struct in6_addr *in6Addr, dhcpv6_clientmac_Opt_t *clientMacOpt);

#endif /* FTR_DHCPV6_RELAY */
#endif /* dhcpv6_relay.h */
