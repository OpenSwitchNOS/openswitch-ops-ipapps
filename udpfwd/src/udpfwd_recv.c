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
 * File: udpfwd_recv.c
 *
 */

/*
 * This file handles the following functionality:
 * - Receive UDP packet from client/server.
 * - Decode the packet.
 * - Pass it on to the right handler.
 */

#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/select.h>
#include "dhcp_relay.h"

VLOG_DEFINE_THIS_MODULE(udpfwd_recv);

/*
 * Function      : create_udp_bcast_socket
 * Responsiblity : Create a socket to receive UDP packets.
 * Parameters    : none
 * Return        : On success socket descriptor is returned
 *                 otherwise -1.
 */
int32_t create_udp_bcast_socket(void)
{
    int32_t sockfd = -1;
    struct sockaddr_in saddr;
    int32_t oneopt = 1;

    VLOG_INFO("Creating UDP receive socket\n");

    /* Create socket to receive UDP packets */
    sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (-1 == sockfd) {
        VLOG_ERR("Failed to allocate socket");
        return -1;
    }

    /* Set socket options for broadcast receive and packet info. */
    if ((setsockopt(sockfd, IPPROTO_IP, IP_PKTINFO,
                              &oneopt, sizeof(oneopt)) == -1)
        || (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,
                              &oneopt, sizeof(oneopt)) == -1)) {
        VLOG_ERR("Failed to set bcast/pktinfo socket options");
        close(sockfd);
        return -1;
    }

    if (((setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT,
                             &oneopt, sizeof(oneopt))) == -1)
        || (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                             &oneopt, sizeof(oneopt)) == -1)) {
        VLOG_ERR("Failed to set reuse port/addr socket options");
        close(sockfd);
        return -1;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;

    /*FIXME :dont specify port and receive all udp packets */
    saddr.sin_port = htons(DHCPS_PORT);

    if (bind(sockfd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in))) {
        VLOG_ERR("Failed to bind socket to port");
        close(sockfd);
        return -1;
    }

    VLOG_INFO("UDP receive socket created successfully \n");
    return sockfd;
}

/*
 * Function      : udpfwd_ctrl
 * Responsiblity : Depending on type of request(BOOTP REQUEST/BOOTP REPLY),
 *                 this function relays packet to client/server.
 * Parameters    : payload - udp payload
 *                 size - size of payload
 *                 dstPort - destination udp port
 *                 pktInfo - pktInfo
 * Return        : none
 */
void udpfwd_ctrl(void *payload, int32_t size,
                 uint16_t dstPort, struct in_pktinfo *pktInfo)
{
    if ((NULL == payload) || (NULL == pktInfo))
    {
        VLOG_ERR("\n Invalid input parameters. pkt : %p, pktInfo : %p",
                  payload, pktInfo);
        return;
    }

    switch (dstPort) {
    case DHCPS_PORT:
    case DHCPC_PORT:
        {
            if (!udpfwd_ctrl_cb_p->dhcp_relay_enable) {
                VLOG_ERR("Since DHCP relay is disabled. Discard the packet\n");
                return;
            }
            struct dhcp_packet *dhcp = (struct dhcp_packet *) payload;
            if(dhcp->op == BOOTREQUEST) { /* Packet must be relayed to DHCP servers. */
                udpfwd_relay_to_dhcp_server(dhcp, size, pktInfo);
            }  else if(dhcp->op == BOOTREPLY) { /* Packet must be relayed to DHCP client. */
                udpfwd_relay_to_dhcp_client(dhcp, size, pktInfo);
            } else {
                VLOG_ERR("\n udpf_ctrl: Invalid DHCP operation type : %p", dhcp);
            }
            break;
        }
    default:
        {
            break;
        }
    }
}

/*
 * Function      : udp_packet_recv
 * Responsiblity : Thread to receive UDP packets to a
 *                 specified destination port.
 * Parameters    : args - arguments
 * Return        : none
 */
void * udp_packet_recv(void *args)
{
    struct msghdr msg;
    struct sockaddr_in dest;
    struct cmsghdr *cmptr; /* pointer to ancillary data structure. */
    struct iovec iov;
    int32_t size, sockfd = -1;
    uint32_t ifinput = -1;
    char ifName[IFNAME_LEN];

    /* union to store ancillary data */
    union {
        struct cmsghdr align; /* This ensures alignment. */
        char control[CMSG_SPACE(sizeof(struct in_pktinfo))];
    } control_u;

    /* union to store udp payload */
    union {
        unsigned char *c;
        struct in_pktinfo *pktInfo;
    } p;

    VLOG_INFO("UDP Broadcast packet receiver thread started");

    /* Create UDP broadcast/unicast receiver socket */
    sockfd = create_udp_bcast_socket();
    if (-1 == sockfd) {
        VLOG_ERR("Failed to create udp bcast receiver");
        return NULL;
    }

    p.c = NULL; /*To Supress cmake error */
    iov.iov_base = (void *) udpfwd_ctrl_cb_p->rcvbuff; /* buffer to store udp packet payload */
    iov.iov_len = RECV_BUFFER_SIZE - 1; /* length of buffer */
    msg.msg_controllen = sizeof(control_u); /* ancillary data buffer len */
    msg.msg_control = control_u.control;  /* ancillary data length */
    msg.msg_name = &dest;
    msg.msg_namelen = sizeof(dest);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    VLOG_INFO("\nListening for udp packets");
    while (true) {
        size = recvmsg(sockfd, &msg, 0);

        VLOG_INFO("\n recvmsg returend :: %d\n", size);
        if (size < 0) {
            VLOG_ERR("Failed to recvmsg :%d, errno:%d", size, errno);
            return NULL;
        }

        if (msg.msg_controllen < sizeof(struct cmsghdr)) {
            continue;
        }

        ifinput = -1;
        /*
         * Iterate throught the control msg header
         * and extract UDP packets.
         */
        for (cmptr = CMSG_FIRSTHDR(&msg); cmptr;
            cmptr = CMSG_NXTHDR(&msg, cmptr)) {
            if (cmptr->cmsg_level == IPPROTO_IP
                && cmptr->cmsg_type == IP_PKTINFO)
            {
              p.c = CMSG_DATA(cmptr);
              ifinput = p.pktInfo->ipi_ifindex;
              break;
            }
        }
        if ((-1 == ifinput) ||
            (NULL == if_indextoname(ifinput, ifName))) {
            VLOG_ERR("Failed to read input interface : %d", ifinput);
            continue;
        }
        /* process the udp packets */
        udpfwd_ctrl((void*)msg.msg_iov->iov_base, size,
                    ntohs(dest.sin_port), p.pktInfo);
    }
    return NULL;
}
