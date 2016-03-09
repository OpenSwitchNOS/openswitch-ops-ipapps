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
 * Parameters : payload - udp payload
 *              size - size of udp payload
 *              in_pktinfo - pktInfo
 *              to - it has destination address and port number
 * Returns: true - packet is sent successfully.
 *          false - any failures.
 */
static bool udpfwd_send_pkt_through_socket(void *payload,
                 int32_t size, struct in_pktinfo *pktInfo, struct sockaddr_in* to)
{
    struct msghdr msg;
    struct iovec iov[1];
    struct cmsghdr *cmptr;
    struct in_pktinfo p = *pktInfo;
    char result = false;

    /* union to store ancillary data */
    union {
        struct cmsghdr align; /* this ensures alignment */
        char control[CMSG_SPACE(sizeof(struct in_pktinfo))];
    } control_u;

    iov[0].iov_base = payload;
    iov[0].iov_len = size;

    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;
    msg.msg_name = to;
    msg.msg_namelen = sizeof(struct sockaddr_in);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    msg.msg_control = &control_u;
    msg.msg_controllen = sizeof(control_u);
    cmptr = CMSG_FIRSTHDR(&msg);
    memcpy(CMSG_DATA(cmptr), &p, sizeof(p));
    msg.msg_controllen = cmptr->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
    cmptr->cmsg_level = IPPROTO_IP;
    cmptr->cmsg_type = IP_PKTINFO;

    assert(udpfwd_ctrl_cb_p->send_sockFd);

    if (sendmsg(udpfwd_ctrl_cb_p->send_sockFd, &msg, 0) < 0 ) {
            VLOG_ERR(" errno =%d sending packet failed\n", errno);
            result = false;
    }
    else
        result = true;

    return result;
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
 * Parameters : dhcp_packet* pkt - dhcp packet
 *              size - size of udp payload
 *              pktInfo - pktInfo structure
 * Returns: true - packet is sent successfully.
 *          false - any failures.
 *
 */
bool udpfwd_relay_to_dhcp_server(struct dhcp_packet* pkt_dhcp, int32_t size,
                                 struct in_pktinfo *pktInfo)
{
    IP_ADDRESS interface_ip;
    int32_t iter = 0;
    uint32_t ifIndex = -1;
    char ifName[IFNAME_LEN];
    struct sockaddr_in to;
    struct shash_node *node;
    UDPFWD_SERVER_T *server = NULL;
    UDPFWD_SERVER_T **serverArray = NULL;
    UDPFWD_INTERFACE_NODE_T *intfNode = NULL;

    ifIndex = pktInfo->ipi_ifindex;

    if ((-1 == ifIndex) ||
        (NULL == if_indextoname(ifIndex, ifName))) {
        VLOG_ERR("Failed to read input interface : %d", ifIndex);
        return false;
    }

    /* Get IP address associated with the Interface. */
    interface_ip = getIpAddressfromIfname(ifName);

    /* If there is no IP address on the input interface do not proceed. */
    if(interface_ip == 0) {
        VLOG_ERR("Interface IP address is 0. Discard packet\n");
        return false;
    }

    if ((pkt_dhcp->hops++) > UDPFWD_DHCP_MAX_HOPS) {
        VLOG_ERR("Hops field exceeds %d as a result packet is discarded\n",
                 UDPFWD_DHCP_MAX_HOPS);
        return false;
    }

    /*
     * we need to preserve the giaddr in case of multi hop relays
     * so setting a giaddr should be done only when giaddr is zero.
     */

    if(pkt_dhcp->giaddr.s_addr == 0) {
        pkt_dhcp->giaddr.s_addr = interface_ip;
    }

    /* Acquire db lock */
    sem_wait(&udpfwd_ctrl_cb_p->waitSem);
    node = shash_find(&udpfwd_ctrl_cb_p->intfHashTable, ifName);
    if (NULL == node) {
        VLOG_ERR("udpf_relay_to_dhcp_server: "
                 "packet from client on interface %s without"
                 " helper address\n", ifName);
        return false;
    }

    intfNode = (UDPFWD_INTERFACE_NODE_T *)node->data;
    serverArray = intfNode->serverArray;

    /* Relay DHCP-Request to each of the configured server. */
    for(iter = 0; iter < intfNode->addrCount; iter++) {
        server = serverArray[iter];
        if (server->udp_port != DHCPS_PORT) {
            continue;
        }

        if ( pktInfo->ipi_addr.s_addr == INADDR_ANY) {
            /*
             * If the source IP address is 0, then replace the ip address with
             * IP addresss of the interface on which the packet is received.
             */
            pktInfo->ipi_spec_dst.s_addr = interface_ip;
        }

        pktInfo->ipi_ifindex = 0;

        to.sin_family = AF_INET;
        to.sin_addr.s_addr = server->ip_address;
        to.sin_port = htons(DHCPS_PORT);

        if (udpfwd_send_pkt_through_socket((void*)pkt_dhcp, size,
                                         pktInfo, &to) == true) {
            VLOG_INFO("packet sent to server successfully\n\n");
        }
    }

    /* Release db lock */
    sem_post(&udpfwd_ctrl_cb_p->waitSem);
    return true;
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
 * Params: dhcp_packet* pkt - dhcp packet
 *         size - size of udp payload
 *         in_pktinfo *pktInfo - pktInfo structure
 *
 * Returns: true - packet is sent successfully.
 *          false - any failures.
 */
bool udpfwd_relay_to_dhcp_client(struct dhcp_packet* pkt_dhcp, int32_t size,
                                 struct in_pktinfo *pktInfo)
{
    struct dhcp_packet *dhcp = (struct dhcp_packet*) pkt_dhcp;
    struct in_addr interface_ip_address; /* Interface IP address. */
    struct arpreq arp_req;
    unsigned char *option = NULL; /* Dhcp options. */
    bool NAKReply = false;  /* Whether this is a NAK. */
    uint32_t ifIndex = -1;
    struct sockaddr_in dest;
    char ifName[IFNAME_LEN];

    interface_ip_address.s_addr = dhcp->giaddr.s_addr;

    /* Get ifIndex associated with this Interface IP address. */
    ifIndex = getIfIndexfromIpAddress(interface_ip_address.s_addr);

    /* Get ifname from ifindex */
    if ((-1 == ifIndex) ||
        (NULL == if_indextoname(ifIndex, ifName))) {
        VLOG_ERR("Failed to read input interface : %d", ifIndex);
        return false;
    }

    /* Check whether this packet is a NAK. */
    option = dhcpPickupOpt(dhcp, size, DHCP_MSGTYPE);
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
               return false;
            }
        }

        strncpy(arp_req.arp_dev, ifName, IFNAME_LEN);
        memcpy(&arp_req.arp_pa, &dest, sizeof(struct sockaddr_in));
        arp_req.arp_ha.sa_family = dhcp->htype;
        memcpy(arp_req.arp_ha.sa_data, dhcp->chaddr, dhcp->hlen);
        arp_req.arp_flags = ATF_COM;
        if (ioctl(udpfwd_ctrl_cb_p->send_sockFd, SIOCSARP, &arp_req) == -1)
        VLOG_ERR(" ARP Failed, errno value = %d\n", errno);
    }

    pktInfo->ipi_ifindex = ifIndex;
    pktInfo->ipi_spec_dst.s_addr = 0;

    if (udpfwd_send_pkt_through_socket((void*)pkt_dhcp, size,
                                pktInfo, &dest) == true) {
        VLOG_INFO("\n packet sent to client successfully\n ");
    }
    return true;
}
