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
 * File: dhcpv6r_recv.c
 *
 */

/*
 * This file handles the following functionality:
 * Receive dhcp packet and pass it to appropriate
 * handler.
 */

#include "dhcpv6_relay.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/select.h>
#include <netinet/in.h>

VLOG_DEFINE_THIS_MODULE(dhcpv6r_recv);

#ifdef FTR_DHCPV6_RELAY

/*
 * Function      : dhcpv6_relay_join_or_leave_mcast_group
 * Responsiblity : To join or leave the All_Dhcp_Relay_Agents_And_Servers
 *                 multicast group on an interface.
 * Parameters    : intfNode - Interface node
 *                 joinFlag - Indicates whether to join or leave the group.
 * Return        : true - success
 *                 false - failure
 */
bool dhcpv6_relay_join_or_leave_mcast_group(DHCPV6_RELAY_INTERFACE_NODE_T *intfNode,
                                        bool joinFlag)
{
    struct ipv6_mreq multicastReq;

    memcpy(&(multicastReq.ipv6mr_multiaddr),
          &(dhcpv6_relay_ctrl_cb_p->agentIpv6Address),
          sizeof(multicastReq.ipv6mr_multiaddr));

    /* Join or Leave multicast from this interface. */
    if (0 != if_nametoindex(intfNode->portName))
        multicastReq.ipv6mr_interface = if_nametoindex(intfNode->portName);
    else
    {
        VLOG_ERR("Failed to read ifIndex, errno = %d", errno);
        return false;
    }

    /* Join or Leave the All_Dhcp_Relay_Agents_And_Server multicast IPv6 address. */
    if (joinFlag)
    {
        if (setsockopt(dhcpv6_relay_ctrl_cb_p->dhcpv6_relaySockFd, IPPROTO_IPV6,
                     IPV6_JOIN_GROUP, (char*)&multicastReq,
                     sizeof(multicastReq)) < 0)
        {
            VLOG_ERR("Join all dhcp relay agents and"
            " servers mcast group failed, errno = %d", errno);
            return false;
        }
    }
    else
    {
        if (setsockopt(dhcpv6_relay_ctrl_cb_p->dhcpv6_relaySockFd, IPPROTO_IPV6,
                     IPV6_LEAVE_GROUP, (char*)&multicastReq,
                     sizeof(multicastReq)) < 0)
        {
            VLOG_ERR("Leave all dhcp relay agents and"
             " servers mcast group failed, errno = %d", errno);
            return false;
        }
    }
    return true;
}

/*
 * Function      : dhcpv6r_recv
 * Responsiblity : Entry point of the DHCPv6 relay receive task.
 *                 Receive packet and pass it to appropriate handler.
 * Parameters    : none
 * Return        : none
 */
void * dhcpv6r_recv(void *args)
{
    VLOG_INFO("DHCPv6 Packet receive routine");
    uint32_t dhcpv6_msgType = 0, recv_ifindex = 0;
    char cmsgbuf[DHCPV6_RELAY_MSG_SIZE];
    struct msghdr mhdr;
    struct iovec iov;
    struct sockaddr_in6 from;
    struct cmsghdr *cm;
    struct in6_pktinfo *pi = NULL;
    dhcpv6_basepkt *dhcpv6r_pktPtr = NULL;
    int size = 0;
    char ifName[IF_NAMESIZE + 1];
    struct udphdr *udph;            /* udp header */

    memset(&iov, 0, sizeof(iov));
    memset(&mhdr, 0, sizeof(mhdr));

    iov.iov_base = (void *)dhcpv6_relay_ctrl_cb_p->rcvbuff;
    iov.iov_len = DHCPV6_RELAY_MSG_SIZE - 1;
    mhdr.msg_name = &from;
    mhdr.msg_namelen = sizeof(from);
    mhdr.msg_iov = &iov;
    mhdr.msg_iovlen = 1;
    mhdr.msg_control = (char*)cmsgbuf;
    mhdr.msg_controllen = sizeof(cmsgbuf);

    assert(dhcpv6_relay_ctrl_cb_p->dhcpv6_relaySockFd);

    while (true)
    {
        size = recvmsg(dhcpv6_relay_ctrl_cb_p->dhcpv6_relaySockFd, &mhdr, 0);
        if (size < 0) {
            VLOG_FATAL("Failed to recvmsg :%d, errno:%d", size, errno);
            return NULL;
        }
        /* Check if the message has been sent by a node with an IPv6 address. */
        if (from.sin6_family != AF_INET6)
        {
            VLOG_ERR("Incorrect address family");
            continue;
        }
        /* Make sure that the packet received is not IPv4 mapped IPv6 address. */
        if (IN6_IS_ADDR_V4MAPPED(&(from.sin6_addr)))
        {
            VLOG_ERR("IPv4 mapped IPv6 address");
            continue;
        }

        /* Check if message received is of min. DHCPv6 packet size. */
        if (size < (int)sizeof(dhcpv6_basepkt))
        {
            VLOG_ERR("size is less than minimum DHCPv6 pkt");
            continue;
        }

        /* check msg has udp dest port as 547 */
        udph = (struct udphdr *)iov.iov_base;
        if (ntohs(udph->dest) == DHCPV6_RELAY_PORT)
        {
            /* Validate the msg type and decide relay agent action. */
            dhcpv6r_pktPtr = (dhcpv6_basepkt*)((char*)iov.iov_base + UDP_HEADER);
            dhcpv6_msgType = ntohl((dhcpv6r_pktPtr->dhcpv6c_msgtypexid).transid);
            dhcpv6_msgType = (dhcpv6_msgType & DHCPV6_MSG_MASK);
            if (!dhcpv6r_is_valid_msg(dhcpv6_msgType))
            {
                VLOG_ERR("Invalid msg");
                continue;
            }
            if (dhcpv6r_is_msg_from_client(dhcpv6_msgType))
            {
                /* Detect receiving interface. */
                for (cm = (struct cmsghdr *)CMSG_FIRSTHDR(&mhdr); cm;
                    cm = (struct cmsghdr *)CMSG_NXTHDR(&mhdr, cm))
                {
                    if (cm->cmsg_level == IPPROTO_IPV6 &&
                        cm->cmsg_type == IPV6_PKTINFO &&
                        cm->cmsg_len == CMSG_LEN(sizeof(struct in6_pktinfo)))
                    {
                        pi = (struct in6_pktinfo *)(CMSG_DATA(cm));
                        break;
                    }
                }
                if (pi == NULL)
                {
                    VLOG_ERR("No data received for pktinfo option");
                    continue;
                }
                recv_ifindex = pi->ipi6_ifindex;
                if ((-1 == recv_ifindex) ||
                    (NULL == if_indextoname(recv_ifindex, ifName))) {
                    printf("Failed to read input interface : %d", recv_ifindex);
                    return NULL;
                }
                if (!(dhcpv6r_relay_to_server
                ((void*)(iov.iov_base), size, from, recv_ifindex)))
                {
                    VLOG_ERR("Packet received on interface: %s failed"
                    " to relay to server", ifName);
                }
            }
            else
            {
                if (!(dhcpv6r_is_msg_from_server(dhcpv6_msgType)))
                {
                    continue;
                }
                if (!(dhcpv6r_relay_to_client((void *)(iov.iov_base), size, from)))
                {
                VLOG_ERR("Packet received on interface: %s failed to relay to client", ifName);
                }
            }
        }
    }
}

#endif /* FTR_DHCPV6_RELAY */
