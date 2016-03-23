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
 * File: udpfwd_util.h
 */

/*
 * This file has all the utility functions
 * related to DHCP relay functionality and also functions
 * to get ifIndex from IP address, get IpAddress from ifName.
 */

#ifndef UDPFWD_UTIL_H
#define UDPFWD_UTIL_H 1

#include "dhcp_relay.h"
#include "udpfwd_common.h"
#include <netinet/ip.h>
#include <netinet/udp.h>


extern DHCP_OPTION_82_COUNTERS dhcp_option_82_counters;

/* Invalid Server Index */
#define UDPF_INVALID_SERVER_INDEX -1

/* Macros for statistics counters */

#define INC_UDPF_DHCPR_CLIENT_DROPS(ifIndex)  \
            udpfwd_ctrl_cb_p->udp_counters.dhcp_client_drops++
#define INC_UDPF_DHCPR_CLIENT_SENT(ifIndex)  \
            udpfwd_ctrl_cb_p->udp_counters.dhcp_client_out_pkts++
#define INC_UDPF_DHCPR_SERVER_DROPS(ifIndex)  \
            udpfwd_ctrl_cb_p->udp_counters.dhcp_serv_drops++
#define INC_UDPF_DHCPR_SERVER_SENT(ifIndex)  \
            udpfwd_ctrl_cb_p->udp_counters.dhcp_serv_out_pkts++
#define INC_UDPF_BCAST_FORWARD_DROPS(ifIndex)  \
            udpfwd_ctrl_cb_p->udp_counters.bcast_fwd_drops++
#define INC_UDPF_BCAST_FORWARD_SENT(ifIndex)  \
            udpfwd_ctrl_cb_p->udp_counters.bcast_fwd_out_pkts++


/* Option 82 counters */
#define INC_UDPF_OPT82_CLIENT_DROPS \
        dhcp_option_82_counters.client_drops++
#define INC_UDPF_OPT82_CLIENT_VALIDS \
        dhcp_option_82_counters.client_valids++
#define INC_UDPF_OPT82_SERVER_DROPS \
        dhcp_option_82_counters.server_drops++
#define INC_UDPF_OPT82_SERVER_VALIDS \
        dhcp_option_82_counters.server_valids++

/*********************************************************************************************/
/* The following macros will be used to get and set value of h3c dhcp packet counter         */
/*********************************************************************************************/

/* SET DHCP RELAY COUNTERS */

#define INC_UDPF_DHCPRECV_BAD_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_bad_pkts++
#define INC_UDPF_DHCPRECV_CLIENT_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_received_client_pkts++
#define INC_UDPF_DHCPRECV_SERVER_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_received_server_pkts++
#define INC_UDPF_DHCPRECV_DISCOVER_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_discover_pkts++
#define INC_UDPF_DHCPRECV_REQUEST_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_request_pkts++
#define INC_UDPF_DHCPRECV_INFORM_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_inform_pkts++
#define INC_UDPF_DHCPRECV_RELEASE_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_release_pkts++
#define INC_UDPF_DHCPRECV_DECLINE_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_decline_pkts++
#define INC_UDPF_DHCPRECV_BOOTPREQUEST_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_bootprequest_pkts++
#define INC_UDPF_DHCPRECV_OFFER_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_offer_pkts++
#define INC_UDPF_DHCPRECV_ACK_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_ack_pkts++
#define INC_UDPF_DHCPRECV_NAK_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_nak_pkts++
#define INC_UDPF_DHCPRECV_BOOTPREPLY_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_recv.h3c_dhcp_bootpreply_pkts++

#define INC_UDPF_DHCPRELAY_SERVERS_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_servers_pkts++
#define INC_UDPF_DHCPRELAY_DISCOVER_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_discover_pkts++
#define INC_UDPF_DHCPRELAY_REQUEST_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_request_pkts++
#define INC_UDPF_DHCPRELAY_INFORM_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_inform_pkts++
#define INC_UDPF_DHCPRELAY_RELEASE_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_release_pkts++
#define INC_UDPF_DHCPRELAY_DECLINE_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_decline_pkts++
#define INC_UDPF_DHCPRELAY_BOOTPREQUEST_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_bootprequest_pkts++
#define INC_UDPF_DHCPRELAY_CLIENTS_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_clients_pkts++
#define INC_UDPF_DHCPRELAY_OFFER_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_offer_pkts++
#define INC_UDPF_DHCPRELAY_ACK_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_ack_pkts++
#define INC_UDPF_DHCPRELAY_NAK_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_nak_pkts++
#define INC_UDPF_DHCPRELAY_BOOTPREPLY_PKTS(ifIndex)  \
        udpfwd_ctrl_cb_p->udp_counters_relay.h3c_dhcp_relay_bootpreply_pkts++

/* end of  SET DHCP RELAY COUNTERS */

/* Pseudo header for UDP checksum computation */
struct ipovly {
    caddr_t  ih_next, ih_prev; /* for protocol sequence q's */
    u_char   ih_x1;            /* (unused) */
    u_char   ih_pr;            /* protocol */
    short    ih_len;           /* protocol length */
    struct   in_addr ih_src;   /* source internet address */
    struct   in_addr ih_dst;   /* destination internet address */
};


/* Function to retrieve IP address from ifIndex name. */
IP_ADDRESS getIpAddressfromIfname(char *ifName);

/* Function to retrieve ifIndex index from IP address. */
uint32_t getIfIndexfromIpAddress(IP_ADDRESS ip);

/* Function to retrieve mac address from ifname. */
uint8_t* getMacfromIfname(char *ifName);


/* Set get routines for feature configuration */
FEATURE_STATUS get_feature_status(uint16_t value, UDPFWD_FEATURE feature);
void set_feature_status(uint16_t *value, UDPFWD_FEATURE feature,
                        FEATURE_STATUS status);
/* Functions to search for a specified option tag in the dhcp packet. */
uint8_t *dhcpPickupOpt(struct dhcp_packet *dhcp, int32_t len, uint8_t tag);
uint8_t * dhcpScanOpt(uint8_t *opt, uint8_t *optend,
                        uint8_t tag, uint8_t *ovld_opt);

/* Checksum computation function */
uint16_t in_cksum(const uint16_t *addr, register int32_t len, uint16_t csum);

/* statistics calculation functions */
void udpfwd_client_packet_type_received(int32_t ifIndex, int32_t dhcp_msg_type);
void udpfwd_client_packet_type_relayed(int32_t ifIndex, int32_t dhcp_msg_type);
void udpfwd_server_packet_type_received(int32_t ifIndex, int32_t dhcp_msg_type);
void udpfwd_server_packet_type_relayed(int32_t ifIndex, int32_t dhcp_msg_type);

#endif /* udpfwd_util.h */
