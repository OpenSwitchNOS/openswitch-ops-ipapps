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
#include "udpfwd_util.h"

VLOG_DEFINE_THIS_MODULE(udpfwd_recv);

/*
 * Function      : udpfwd_ctrl
 * Responsiblity : Depending on type of request(BOOTP REQUEST/BOOTP REPLY),
 *                 this function relays packet to client/server.
 * Parameters    : pkt  - raw ip packet
 *                 size - size of payload
 *                 pktInfo - pktInfo
 * Return        : none
 */
void udpfwd_ctrl(void *pkt, int32_t size,
                 struct in_pktinfo *pktInfo)
{
    struct ip *iph;              /* ip header */
    struct udphdr *udph;            /* udp header */
    struct dhcp_packet *dhcp;       /* dhcp header */
    uint32_t ifIndex = -1;
    unsigned char *option = NULL; /* dhcp packet type options */
    int dhcp_msg_type = 0;


    /* Input parameter validation */
    if ((NULL == pkt) || (NULL == pktInfo))
    {
        VLOG_ERR("\n Invalid input parameters. pkt : %p, pktInfo : %p",
                  pkt, pktInfo);
        return;
    }

    iph  = (struct ip *) pkt;
    udph = (struct udphdr *) ((char *)iph + (iph->ip_hl * 4));
    ifIndex = pktInfo->ipi_ifindex;

    switch (ntohs(udph->dest)) {
    case DHCPS_PORT:
    case DHCPC_PORT:
        {
            if (ENABLE != get_feature_status(udpfwd_ctrl_cb_p->feature_config.config,
                          DHCP_RELAY)) {
                VLOG_ERR("DHCP relay is disabled. Discard the packet\n");
                return;
            }

            dhcp = (struct dhcp_packet *)
                        ((char *)iph + (iph->ip_hl * 4) + UDPHDR_LENGTH);


            /* Check what type of packet this is :
            DHCPDICOVER/DHCPREQUEST/DHCPINFORM/DHCPRELEASE/DHCPDECLINE/
            BOOTPREQUEST/DHCPOFFER/DHCPACK/DHCPNAK/BOOTPREPLY
            */

            option = dhcpPickupOpt(dhcp, DHCP_PKTLEN(udph), DHCP_MSGTYPE);
            if(dhcp->op == BOOTREQUEST)
            {
                /* Packet must be relayed to DHCP servers. */
                if (option != NULL)
                {
                    dhcp_msg_type = (*OPTBODY(option));
                    /* increment the counter for type of client packet received */
                    udpfwd_client_packet_type_received(ifIndex, dhcp_msg_type);
                }
                /* Increment the counter for the packets  received from client */
                INC_UDPF_DHCPRECV_CLIENT_PKTS(ifIndex);

                if (udpfwd_relay_to_dhcp_server(pkt, size, pktInfo) == false)
                {
                    /* increment the DHCP client request Drop counter    */
                    INC_UDPF_DHCPR_CLIENT_DROPS(ifIndex);
                    /* Increment the counter for the bad packets received */
                    INC_UDPF_DHCPRECV_BAD_PKTS(ifIndex);
                }
            }
            else if(dhcp->op == BOOTREPLY)
            {
                /* temporary fix */
                if ( iph->ip_dst.s_addr != IP_ADDRESS_BCAST) {
                    /* Packet must be relayed to DHCP client. */
                    if (option != NULL)
                    {
                        dhcp_msg_type = (*OPTBODY(option));
                        /* Increment the counter for the packets received from server */
                        udpfwd_server_packet_type_received(ifIndex, dhcp_msg_type);
                    }
                    /* Increment the counter for the packets received from the server */
                    INC_UDPF_DHCPRECV_SERVER_PKTS(ifIndex);
                    if (udpfwd_relay_to_dhcp_client(pkt, size, pktInfo) == false)
                    {
                        INC_UDPF_DHCPR_SERVER_DROPS(ifIndex);
                        /* Increment the counter for bad packets received */
                        INC_UDPF_DHCPRECV_BAD_PKTS(ifIndex);
                    }
                }
            }
            else
            {
                VLOG_ERR("\n udpf_ctrl: Invalid DHCP operation type : %p", dhcp);
            }
            break;
        }
    default:
        {
            /* UDP Broadcast Forwarding Case. */
            udpfwd_forward_packet(pkt, ntohs(udph->dest), size, pktInfo);
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
    int32_t size;
    uint32_t ifinput = -1;
    union packet_info pinfo;
    union control_u ctrl;
    char ifName[IF_NAMESIZE];

    VLOG_INFO("UDP Broadcast packet receiver thread started");

    pinfo.c = NULL; /*To Supress cmake error */
    iov.iov_base = (void *) udpfwd_ctrl_cb_p->rcvbuff; /* buffer to store udp packet payload */
    iov.iov_len = RECV_BUFFER_SIZE - 1; /* length of buffer */
    msg.msg_controllen = sizeof(union control_u); /* ancillary data buffer len */
    msg.msg_control = ctrl.control;  /* ancillary data length */
    msg.msg_name = &dest;
    msg.msg_namelen = sizeof(dest);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    assert(udpfwd_ctrl_cb_p->udpSockFd);

    VLOG_INFO("\nListening for udp packets");
    while (true)
    {
        size = recvmsg(udpfwd_ctrl_cb_p->udpSockFd, &msg, 0);
        if (size < 0) {
            VLOG_FATAL("Failed to recvmsg :%d, errno:%d", size, errno);
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
              pinfo.c = CMSG_DATA(cmptr);
              ifinput = pinfo.pktInfo->ipi_ifindex;
              break;
            }
        }
        if (-1 == ifinput)
        {
           VLOG_ERR("Received packet input interface is invalid");
           continue;
        }
        else if (NULL == if_indextoname(ifinput, ifName)) {
            VLOG_ERR("Failed to convert ifindex to ifname : %d", ifinput);
            continue;
        }

        /* process the udp packets */
        udpfwd_ctrl((void*)msg.msg_iov->iov_base, size, pinfo.pktInfo);
    }
    return NULL;
}
