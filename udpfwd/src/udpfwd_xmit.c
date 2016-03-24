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
 * File: udpf_xmit.c
 *
 */

/*
 * This file handles the following functionality:
 * - Relay Packet to client/server.
 */

#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include "udpfwd_util.h"

VLOG_DEFINE_THIS_MODULE(udpfwd_xmit);


/*
 * Function : udpf_send_pkt_through_socket
 * Responsiblity : To send a unicast packet to a known server address.
 * Parameters : pkt - IP packet
 *              size - size of udp payload
 *              in_pktinfo - pktInfo
 *              to - it has destination address and port number
 * Returns: true - packet is sent successfully.
 *          false - any failures.
 */
static bool udpfwd_send_pkt_through_socket(void *pkt,
                 int32_t size, struct in_pktinfo *pktInfo, struct sockaddr_in* to)
{
    struct msghdr msg;
    struct iovec iov[1];
    struct cmsghdr *cmptr;
    struct in_pktinfo p = *pktInfo;
    struct ip  *iph;
    struct udphdr *udph;
    union control_u ctrl;
    char result = false;

    /* Update IP and UDP header checksum fields */
    iph  = (struct ip *) pkt;
    udph = (struct udphdr *) ((char *)iph + (iph->ip_hl * 4));

    /* Set destination ip and udp port number in the packet */
    iph->ip_dst.s_addr = to->sin_addr.s_addr;
    udph->uh_dport = to->sin_port;

    iph->ip_sum = in_cksum((uint16_t *) pkt, iph->ip_len, 0);
    /* FIXME: Add working udp checksum function */
    udph->check = 0; //get_udpsum(iph, udph);

    iov[0].iov_base = pkt;
    iov[0].iov_len = size;

    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;
    msg.msg_name = to;
    msg.msg_namelen = sizeof(struct sockaddr_in);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    msg.msg_control = &ctrl;
    msg.msg_controllen = sizeof(union control_u);
    cmptr = CMSG_FIRSTHDR(&msg);
    memcpy(CMSG_DATA(cmptr), &p, sizeof(p));
    msg.msg_controllen = cmptr->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
    cmptr->cmsg_level = IPPROTO_IP;
    cmptr->cmsg_type = IP_PKTINFO;

    assert(udpfwd_ctrl_cb_p->udpSockFd);

    if (sendmsg(udpfwd_ctrl_cb_p->udpSockFd, &msg, 0) < 0 )
    {
        VLOG_ERR("errno = %d, sending packet failed", errno);
    }
    else
    {
        result = true;
    }

    return result;
}

/*
 * Function: udpfwd_forward_packet
 * Responsibilty : Send incoming UDP broadcast message to server UDP port.
 *                 This routine forwards the UDP broadcast message to the UDP port of
 *                 configured destination UDP server.
 * Parameters : pkt  - Ip packet
 *              udp_dport - destination udp port
 *              size - size of udp payload
 *              pktInfo - pktInfo structure
 * Returns: void
 *
 */
void udpfwd_forward_packet (void *pkt, uint16_t udp_dport, int size,
                            struct in_pktinfo *pktInfo)
{
    IP_ADDRESS interface_ip;
    uint32_t iter = 0;
    uint32_t ifIndex = -1;
    char ifName[IF_NAMESIZE + 1];
    struct sockaddr_in to;
    struct shash_node *node;
    UDPFWD_SERVER_T *server = NULL;
    UDPFWD_SERVER_T **serverArray = NULL;
    UDPFWD_INTERFACE_NODE_T *intfNode = NULL;

    ifIndex = pktInfo->ipi_ifindex;

    if ((-1 == ifIndex) ||
        (NULL == if_indextoname(ifIndex, ifName))) {
        VLOG_ERR("Failed to read input interface : %d", ifIndex);
        return;
    }

    /* Get IP address associated with the Interface. */
    interface_ip = getIpAddressfromIfname(ifName);

    /* If there is no IP address on the input interface do not proceed. */
    if(interface_ip == 0) {
        VLOG_ERR("interface IP address is 0. Discard packet");
        return;
    }

    /* Acquire db lock */
    sem_wait(&udpfwd_ctrl_cb_p->waitSem);
    node = shash_find(&udpfwd_ctrl_cb_p->intfHashTable, ifName);
    if (NULL == node) {
        VLOG_DBG("packet from client on interface %s without "
                 "UDP forward-protocol address\n", ifName);
        /* Release db lock */
        sem_post(&udpfwd_ctrl_cb_p->waitSem);
        return;
    }

    intfNode = (UDPFWD_INTERFACE_NODE_T *)node->data;
    serverArray = intfNode->serverArray;

    /* UDP Broadcast Forwarder request to each of the configured server. */
    for(iter = 0; iter < intfNode->addrCount; iter++) {
        server = serverArray[iter];
        if (server->udp_port != udp_dport) {
            continue;
        }

        if ( pktInfo->ipi_addr.s_addr == INADDR_ANY) {
            /* If the source IP address is 0, then replace the ip address with
             * IP addresss of the interface on which the packet is received.
             */
            pktInfo->ipi_spec_dst.s_addr = interface_ip;
        }

        pktInfo->ipi_ifindex = 0;

        to.sin_family = AF_INET;
        to.sin_addr.s_addr = server->ip_address;
        to.sin_port = htons(udp_dport);

        if (udpfwd_send_pkt_through_socket(pkt, size,
                                           pktInfo, &to) == true) {
            VLOG_INFO("packet sent to server successfully\n\n");
        }
    }
    /* Release db lock */
    sem_post(&udpfwd_ctrl_cb_p->waitSem);

    return;
}

/*
 * Function: udpfwd_relay_to_dhcp_server
 * Responsibilty : Send incoming DHCP message to client port.
 *                 This routine relays a DHCP message to the client port of
 *                 every DHCP server or relay agent whose IP address is
 *                 contained in the interface structure. All messages are
 *                 discarded after the hops field exceeds 16 to comply with
 *                 the relay agent behavior specified in RFC 1542.
 *                 The relay agent normally discards such messages before
 *                 this routine is called. They will only be received by this
 *                 routine if the user ignores the instructions in the
 *                 manual and sets the value of DHCP_MAX_HOPS higher than 16.
 * Parameters : pkt  - ip packet
 *              size - size of udp payload
 *              pktInfo - pktInfo structure
 * Returns: void
 *
 */
void udpfwd_relay_to_dhcp_server(void* pkt, int32_t size,
                                 struct in_pktinfo *pktInfo)
{
    struct ip *iph;              /* ip header */
    struct udphdr *udph;            /* udp header */
    struct dhcp_packet* dhcp;
    IP_ADDRESS interface_ip;
    int32_t iter = 0;
    uint32_t ifIndex = -1;
    struct sockaddr_in to;
    struct shash_node *node;
    UDPFWD_SERVER_T *server = NULL;
    UDPFWD_SERVER_T **serverArray = NULL;
    UDPFWD_INTERFACE_NODE_T *intfNode = NULL;
    char ifName[IF_NAMESIZE + 1];
    DHCP_OPTION_82_OPTIONS  option82_info;

    ifIndex = pktInfo->ipi_ifindex;

    if ((-1 == ifIndex) ||
        (NULL == if_indextoname(ifIndex, ifName))) {
        VLOG_ERR("Failed to read input interface : %d", ifIndex);
        return;
    }

    /* Get IP address associated with the Interface. */
    interface_ip = getIpAddressfromIfname(ifName);

    /* If there is no IP address on the input interface do not proceed. */
    if(interface_ip == 0) {
        VLOG_ERR("%s: Interface IP address is 0. Discard packet", ifName);
        return;
    }

    iph  = (struct ip *) pkt;
    udph = (struct udphdr *) ((char *)iph + (iph->ip_hl * 4));
    dhcp = (struct dhcp_packet *)
                        ((char *)iph + (iph->ip_hl * 4) + UDPHDR_LENGTH);

    if ((dhcp->hops) > UDPFWD_DHCP_MAX_HOPS) {
        VLOG_ERR("Hops field exceeds %d as a result packet is discarded\n",
                 UDPFWD_DHCP_MAX_HOPS);
        return;
    }

    /*
     * we need to preserve the giaddr in case of multi hop relays
     * so setting a giaddr should be done only when giaddr is zero.
     */

    if(dhcp->giaddr.s_addr == 0) {
        dhcp->giaddr.s_addr = interface_ip;
    }

    /* ========================================================================
       Make the appropriate port correction
       http://www.ietf.org/internet-drafts/draft-ietf-dhc-implementation-02.txt
         4.7.2 Relay Agent Port Usage
            Relay agents should use port 67 as the source port number. Relay
            agents always listen on port 67, but port 68 has sometimes used

            as the source port number probably because it was copied from the
            source port of the incoming packet.

            Cable modem vendors would like to install filters blocking outgoing
            packets with source port 67.

            RECOMMENDATIONS:
            O  Relay agents MUST use 67 as their source port number.
            O  Relay agents MUST NOT forward packets with non-zero giaddr
               unless the source port number on the packet is 67.

       ===================================================================== */
    if (udph->uh_sport == DHCPC_PORT)
         udph->uh_sport = DHCPS_PORT;

    /* Acquire db lock */
    sem_wait(&udpfwd_ctrl_cb_p->waitSem);
    node = shash_find(&udpfwd_ctrl_cb_p->intfHashTable, ifName);
    if (NULL == node) {
        /* Release db lock */
        sem_post(&udpfwd_ctrl_cb_p->waitSem);
        return;
    }

    if (ENABLE == get_feature_status(udpfwd_ctrl_cb_p->feature_config.config,
                  DHCP_RELAY_HOP_COUNT_INCREMENT)) {
        dhcp->hops++;
    }

    /* RFC prefers to decrement time to live */
    iph->ip_ttl--;

    intfNode = (UDPFWD_INTERFACE_NODE_T *)node->data;

    /* Check if Bootp Gateway configured on this interface is valid one and if yes,
     * use this for stamping the DHCP requests
     */

    if (intfNode->bootp_gw)
        dhcp->giaddr.s_addr = intfNode->bootp_gw;

    memset(&option82_info, 0, sizeof(option82_info));
    option82_info.ip_addr = interface_ip;

    if (process_dhcp_relay_option82_message(pkt, &option82_info, ifIndex,
                            ifName, intfNode->bootp_gw) == false)
    {
        VLOG_ERR("Option 82 check failed when relaying packet to server."
                  "Drop packet.");

         /* Release the semaphore and return */
         sem_post(&udpfwd_ctrl_cb_p->waitSem);
         return;
    }

    /* update value of size */
    size= ntohs(iph->ip_len);
    serverArray = intfNode->serverArray;

    /* Relay DHCP-Request to each of the configured server. */
    for(iter = 0; iter < intfNode->addrCount; iter++) {
        server = serverArray[iter];
        if (server->udp_port != DHCPS_PORT) {
            continue;
        }

        if ( iph->ip_src.s_addr == INADDR_ANY) {
            /*
             * If the source IP address is 0, then replace the ip address with
             * IP addresss of the interface on which the packet is received.
             */
            iph->ip_src.s_addr = interface_ip;
        }

        pktInfo->ipi_ifindex = 0;

        to.sin_family = AF_INET;
        to.sin_addr.s_addr = server->ip_address;
        to.sin_port = htons(DHCPS_PORT);

        if (udpfwd_send_pkt_through_socket((void*)pkt, size,
                                         pktInfo, &to) == true) {
            VLOG_INFO("packet sent to server successfully\n\n");
        }
    }

    /* Release db lock */
    sem_post(&udpfwd_ctrl_cb_p->waitSem);
    return;
}

/*
 * Function :  udpfwd_relay_to_dhcp_client
 * Responsibilty : Send DHCP replies to client.
 *                 This routine relays a DHCP reply to the client which
 *                 generated the initial request. The routine accesses
 *                 global pointers already set to indicate the reply.
 *                 All messages are discarded after the hops field exceeds
 *                 16 to comply with the relay agent behavior specified
 *                 in RFC 1542. The relay agent normally discards such messages
 *                 before this routine is called. They will only be received by
 *                 this routine if the user ignores the instructions
 *                 in the manual and sets the value of DHCP_MAX_HOPS higher than 16.
 *
 * Params: void* pkt - packet
 *         size - size of udp payload
 *         in_pktinfo *pktInfo - pktInfo structure
 *
 * Returns: void
 */
void udpfwd_relay_to_dhcp_client(void* pkt, int32_t size,
                                 struct in_pktinfo *pktInfo)
{
    struct ip *iph;              /* ip header */
    struct udphdr *udph;            /* udp header */
    struct dhcp_packet *dhcp;       /* dhcp header */
    struct arpreq arp_req;
    unsigned char *option = NULL; /* Dhcp options. */
    bool NAKReply = false;  /* Whether this is a NAK. */
    struct sockaddr_in dest;
    uint32_t ifIndex = -1;
    char ifName[IF_NAMESIZE + 1];
    struct in_addr interface_ip_address; /* Interface IP address. */
    DHCP_OPTION_82_OPTIONS  option82_info;

    iph  = (struct ip *) pkt;
    udph = (struct udphdr *) ((char *)iph + (iph->ip_hl * 4));
    dhcp = (struct dhcp_packet *)
                              ((char *)iph + (iph->ip_hl * 4) + UDPHDR_LENGTH);

    interface_ip_address.s_addr = dhcp->giaddr.s_addr;

    /* Get ifIndex associated with this Interface IP address. */
    ifIndex = getIfIndexfromIpAddress(interface_ip_address.s_addr);

    /* Get ifname from ifindex */
    if ((-1 == ifIndex) ||
        (NULL == if_indextoname(ifIndex, ifName))) {
        VLOG_ERR("Failed to read input interface : %d", ifIndex);
        return;
    }

    iph->ip_ttl--;

    /* initialize option82_info struct */
    memset(&option82_info, 0, sizeof(option82_info));
    if (process_dhcp_relay_option82_message(pkt, &option82_info, ifIndex,
                                            ifName, 0) == false)

    {
        VLOG_ERR("Option 82 check failed when relaying packet to client."
                 "Drop packet");
        return;
    }

    /* Check whether this packet is a NAK. */
    option = dhcpPickupOpt(dhcp, DHCP_PKTLEN(udph), DHCP_MSGTYPE);
    if (option != NULL)
        NAKReply = (*OPTBODY (option) == DHCPNAK);

    /* Examine broadcast flag. */
    if((ntohs(dhcp->flags) & UDPFWD_DHCP_BROADCAST_FLAG)|| NAKReply)
        /* Broadcast flag is set or a NAK, so set MAC address to broadcast. */
    {
        /* Set destination IP address to broadcast address. */
        dest.sin_addr.s_addr = IP_ADDRESS_BCAST;
        dest.sin_port = htons(DHCPC_PORT);
        dest.sin_family = AF_INET;
    }
    else
    {
        /*
         * By default use these following for dest IP and MAC
         * By default set MAC address to server MAC address.
         */

        /* By default set destination IP address offered address. */
        dest.sin_addr.s_addr = dhcp->yiaddr.s_addr;
        dest.sin_port = htons(DHCPC_PORT);
        dest.sin_family = AF_INET;

        /*
         * DHCP ACK message workaround:
         * Windows server uses DHCP Inform and ACK mssages to discover
         * other DHCP Servers on the network.
         * The ACK message will have the yiaddr set to 0.0.0.0 and
         * chaddr field set to 0.  In this case use the ciaddr field
         * and check for the ARP table for the client MAC address.
         */
        if ((option != NULL) && (*OPTBODY (option) == DHCPACK))
        {
            if (dhcp->yiaddr.s_addr == IP_ADDRESS_NULL)
            {
                if(dhcp->ciaddr.s_addr != IP_ADDRESS_NULL)
                    dest.sin_addr.s_addr = dhcp->ciaddr.s_addr;
            else
               /* ciaddr is 0.0.0.0, don't relay to client. */
               return;
            }
        }

        strncpy(arp_req.arp_dev, ifName, IF_NAMESIZE);
        memcpy(&arp_req.arp_pa, &dest, sizeof(struct sockaddr_in));
        arp_req.arp_ha.sa_family = dhcp->htype;
        memcpy(arp_req.arp_ha.sa_data, dhcp->chaddr, dhcp->hlen);
        arp_req.arp_flags = ATF_COM;
        if (ioctl(udpfwd_ctrl_cb_p->udpSockFd, SIOCSARP, &arp_req) == -1)
            VLOG_ERR("ARP Failed, errno value = %d", errno);
    }

    pktInfo->ipi_ifindex = ifIndex;
    pktInfo->ipi_spec_dst.s_addr = 0;

    /* update value of size */
    size= ntohs(iph->ip_len);

    if (udpfwd_send_pkt_through_socket((void*)pkt, size,
                                pktInfo, &dest) != true) {
        VLOG_ERR("Failed to send packet dhcp-client");
    }

    return;
}
