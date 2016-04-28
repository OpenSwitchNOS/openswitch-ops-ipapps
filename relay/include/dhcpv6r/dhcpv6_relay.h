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
 * and declaration of functions related to DHCPV6 Relay feature.
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
#include <sys/socket.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <net/if.h>
#include <assert.h>


#ifdef FTR_DHCPV6_RELAY
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
    int32_t dhcpv6_relaySockFd;    /* Socket to send/receive DHCP packets */
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
} DHCPV6_RELAY_INTERFACE_NODE_T;

/*
 * Global variable declaration
 */
extern DHCPV6_RELAY_CTRL_CB *dhcpv6_relay_ctrl_cb_p;

/* Maximum number of entries allowed per INTERFACE. */
#define MAX_SERVERS_PER_INTERFACE 8

/* DHCPv6 multicast destination address */
#define DHCPV6_ALLAGENTS    "ff02::1:2"
#define DHCPV6_ALLSERVERS   "FF05::1:3"

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

#endif /* FTR_DHCPV6_RELAY */
#endif /* dhcpv6_relay.h */
