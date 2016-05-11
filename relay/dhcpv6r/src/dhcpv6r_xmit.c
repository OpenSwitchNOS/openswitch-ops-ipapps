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
 * File: dhcpv6r_xmit.c
 *
 */

/*
 * This file handles the following functionality:
 * - Relay Packet to client/server.
 */


#include "dhcpv6_relay.h"

VLOG_DEFINE_THIS_MODULE(dhcpv6r_xmit);

#ifdef FTR_DHCPV6_RELAY

/* FIXME : This file entire code needs to be tested */

/*FIXME : need to add functionality */
bool dhcpv6_discoverNeighbor(void)
{
    return true;
}

bool getIpv6Address(uint32_t ifIndex, struct sockaddr_in6*ss)
{
    return true;

}

/*
 * Function      : dhcpv6r_relayToServer
 * Responsiblity : This function relays a packet from a DHCPv6 client
 *                 or relay to server.
 * Parameters    : pktPtr - Ptr to the packet to be relayed.
 *                 len - length of the pkt to be relayed
 *                 srcAddr - Source IP address of the received packet.
 *                 recv_ifindex -  interface ifIndex on which the packet was received.
 *                 srcRecv_ifindex -interface ifIndex on which the packet was received.
 * Return        : true - if the packet is relayed successfully.
 *                 false - failure
 */
bool dhcpv6r_relayToServer (dhcpv6r_basePkt_t *rxPktPtr, int rxPktLen,
                           struct sockaddr_in6 srcAddr, uint32_t recv_ifindex,
                           uint32_t srcRecv_ifindex)
{
    char sbuf[DHCPV6_RELAY_MSG_SIZE]; /* Send buffer for the transmitted pkt. */
    uint32_t pktLen = 0;
    uint8_t msgType = (RELAY_FORW_MSG >> 24), hopCount = 0;
    bool pktFromClient = true;
    struct shash_node *node;
    dhcpv6r_basePkt_t pkt;
    dhcpv6r_basePkt_t *pktPtr = NULL, *optPtr = NULL;
    struct sockaddr_in6 ss;
    char ifName[IF_NAMESIZE + 1];

    DHCPV6_RELAY_INTERFACE_NODE_T *intfNode = NULL;
    dhcpv6_clientmac_Opt_t clientMacOpt = {0};
    bool includeMac = false;
    char ipv6Str[INET6_ADDRSTRLEN] = {0};

    VLOG_INFO("in dhcpv6 relay to server function");
    if (rxPktPtr == NULL)
    {
        assert(0);
        return false;
    }
    if ((-1 == recv_ifindex) ||
            (NULL == if_indextoname(recv_ifindex, ifName))) {
            printf("Failed to read input interface : %d", recv_ifindex);
            return 0;
    }

    /*FIXME : add code to check if ifindex is of loopback interface */

    /* Check to see that the DHCPv6 relay must be enabled on the Rx Vlan. */
    /* Acquire db lock */
    sem_wait(&dhcpv6_relay_ctrl_cb_p->waitSem);
    node = shash_find(&dhcpv6_relay_ctrl_cb_p->intfHashTable, ifName);
    if (NULL == node) {
        /* Release db lock */
        sem_post(&dhcpv6_relay_ctrl_cb_p->waitSem);
        return false;
    }

    intfNode = (DHCPV6_RELAY_INTERFACE_NODE_T *)node->data;
    /* Check if the message has been received from another relay. */
    if (rxPktPtr->msgType == (RELAY_FORW_MSG >> 24))
    {
        pktFromClient = false;
        /* Check if the hop count limit has been reached. */
        if (rxPktPtr->hopCount >= DHCPV6R_MAX_HOPCOUNT)
        {
            /* FIXME: increment client drop packet counter */
            VLOG_ERR(" Drop the packet as Hop count exceeds %d",DHCPV6R_MAX_HOPCOUNT);
            sem_post(&dhcpv6_relay_ctrl_cb_p->waitSem);
            return false;
        }
    hopCount = rxPktPtr->hopCount + 1;
    }

    memset(sbuf, 0, sizeof(sbuf));
    memset(&pkt, 0, sizeof(pkt));

    pktPtr = (dhcpv6r_basePkt_t*)sbuf;
    /* Assign the packet parameters. */
    pkt.msgType = msgType;
    pkt.hopCount = hopCount;
    if (pktFromClient)
    {
       /* Link-address must be set to the IPv6 address of the receiving
        * interface. In case, a global or site-local IPv6 address does not
        * exist on the receiving interface, the link-local IPv6 address
        * will be set as the link address and the interface-id option will
        * be included.
        */
        memset(&ss, 0, sizeof(ss));
        if (getIpv6Address(srcRecv_ifindex, &ss))
        {
            memcpy(&pkt.linkAddress, &(ss.sin6_addr), sizeof(struct in6_addr));
        }
    }
    else
    {
       /* If the packet is received from a relay, first the source IPv6
        * address of the received packet needs to be checked. If it is a global
        * or site-local IPv6 address, the link-address is set to 0. If not i.e.
        * the source IPv6 address is a link-local IPv6 address, then the
        * link address needs to be set to the IPv6 address of the receiving interface.
        */
        if (IN6_IS_ADDR_LINKLOCAL(&(srcAddr.sin6_addr)))
        {
            memset(&ss, 0, sizeof(ss));
            if (getIpv6Address(srcRecv_ifindex, &ss))
            {
                memcpy(&pkt.linkAddress, &(ss.sin6_addr), sizeof(struct in6_addr));
            }
        }
    }

    /* Assign the source IPv6 address of the incoming packet to the
     * peer address field of the DHCPv6 relay packet. */
    memcpy(&pkt.peerAddress,  &(srcAddr.sin6_addr), sizeof(struct in6_addr));

    memcpy(pktPtr, &pkt, sizeof(pkt));
    pktLen = sizeof(pkt);
    optPtr = (pktPtr + 1);

    VLOG_INFO("do make compiler happy for now %d %p", pktLen, optPtr);

    if (dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_option79_enable)
    {
        if (pktFromClient)
        {
            includeMac = dhcpv6_discoverNeighbor();
            if(!includeMac)
            {
                INC_DHCPR_CLIENT_DROPS(intfNode);
                inet_ntop(AF_INET6, (void *)&srcAddr.sin6_addr, ipv6Str, INET6_ADDRSTRLEN);
                VLOG_ERR("option 79 failed %s", ipv6Str);
                sem_post(&dhcpv6_relay_ctrl_cb_p->waitSem);
                return true;
            }
        }
        /* Set up the relay options. */
        if (!(dhcpv6r_setOptions((dhcpv6_opt_t *) optPtr, &pktLen, (dhcpv6_basepkt *) rxPktPtr,
                                       rxPktLen, srcRecv_ifindex,(includeMac)? (&clientMacOpt):NULL)))
        {
            VLOG_ERR("setting option 79 failed %s", ifName);
            INC_DHCPR_CLIENT_DROPS(intfNode);
            sem_post(&dhcpv6_relay_ctrl_cb_p->waitSem);
            return false;
        }
    }
    else if (!(dhcpv6r_setOptions((dhcpv6_opt_t *) optPtr, &pktLen, (dhcpv6_basepkt *) rxPktPtr,
                                       rxPktLen, srcRecv_ifindex, NULL))) {
        VLOG_ERR("setting option 79 failed %s", ifName);
        INC_DHCPR_CLIENT_DROPS(intfNode);
        sem_post(&dhcpv6_relay_ctrl_cb_p->waitSem);
        return false;
    }

    /* Transmit the DHCPv6 relay packet. */
    if (!(dhcpv6r_xmit((char *) pktPtr, pktLen, true, srcRecv_ifindex, NULL, true, intfNode)))
    {
        VLOG_ERR(" packet relay to server failed ");
        sem_post(&dhcpv6_relay_ctrl_cb_p->waitSem);
        return false;
    }

    sem_post(&dhcpv6_relay_ctrl_cb_p->waitSem);
    return true;
}

/*
 * Function      : dhcpv6r_xmit
 * Responsiblity : Transmits a message for the DHCPv6 relay.
 * Parameters    : sBuf - Buffer with data to be relayed
 *                 len - length of the pkt to be relayed
 *                 ifIndex - Ingress ifIndex if relayToServer is true
 *                 Egress ifIndex if relayToServer is false.
 *                 relayToServer ->Flag to indicate where the pkt is to be relayed
 *                 true implies send to server, false implies send to client.
 *                 data -> Ptr to additional data.
 *                 sendToRelay -> Flag to indicate whether this packet needs to be
 *                 sent to a relay/server port.
 * Return        : true - if the packet is sent successfully.
 *                 false - failure
 */
bool dhcpv6r_xmit (char *sBuf, uint32_t pktLen, bool relayToServer,
                  uint32_t ifIndex, void *data, bool sendToRelay,
                  DHCPV6_RELAY_INTERFACE_NODE_T *intfNode)
{
    struct sockaddr_in6 dst;
    int iter = 0, hopLimit = DHCPV6R_MULTICAST_HOPLIMIT;
    DHCPV6_RELAY_SERVER_T *server = NULL;
    DHCPV6_RELAY_SERVER_T **serverArray = NULL;

    if (sBuf == NULL)
    {
        assert(0);
        return false;
    }
    memset(&dst, 0, sizeof(dst));

    assert(dhcpv6_relay_ctrl_cb_p->dhcpv6_relaySockFd);

    if (!relayToServer)
    {
        #if 0
        dst.sin6_len = sizeof(dst);
        #endif
        dst.sin6_family = PF_INET6;
        if (sendToRelay)
        {
            dst.sin6_port = htons((uint16_t)DHCPV6_RELAY_PORT);
        }
        else
        {
            dst.sin6_port = htons((uint16_t)DHCPV6_CL_PORT);
        }
        memcpy(&(dst.sin6_addr), (struct in6_addr*)data, sizeof(struct in6_addr));
        if (IN6_IS_ADDR_LINKLOCAL(&(dst.sin6_addr)))
        {
            dst.sin6_scope_id = ifIndex;
        }
        if (sendto(dhcpv6_relay_ctrl_cb_p->dhcpv6_relaySockFd, sBuf, pktLen,
                 0, (struct sockaddr *)&dst,
                 sizeof(dst)) < 0)
        {
            VLOG_ERR("sendto failed =%d", errno);
            return false;
        }
    }
    else
    {
        serverArray = intfNode->serverArray;
        /* Relay DHCP-Request to each of the configured server. */
        for(iter = 0; iter < intfNode->addrCount; iter++) {
            server = serverArray[iter];
            memset(&dst, 0, sizeof(dst));
            #if 0
            dst.sin6_len = sizeof(dst);
            #endif
            dst.sin6_family = PF_INET6;
            dst.sin6_port = htons((uint16_t)DHCPV6_RELAY_PORT);

            inet_pton(PF_INET6, server->ipv6_address, &(dst.sin6_addr));

            if (IN6_IS_ADDR_MULTICAST(&(dst.sin6_addr)))
            {
               /* Note: If IPv6 multicast routing is available on the box, we may
                * need to re-visit this code and determine what would be the right
                * approach in that case. For now, we set the egress interface for the
                * IPv6 multicast packet by using the IPV6_MULTICAST_IF socket option.
                */

                if (0 != if_nametoindex(server->egressIfName))
                    ifIndex = if_nametoindex(server->egressIfName);
                else
                {
                    VLOG_ERR("Failed to read ifIndex, errno = %d", errno);
                    return false;
                }
                if (setsockopt(dhcpv6_relay_ctrl_cb_p->dhcpv6_relaySockFd, IPPROTO_IPV6,
                              IPV6_MULTICAST_IF, (char*)&ifIndex, sizeof(ifIndex)) < 0)
                {
                    VLOG_ERR("setsockopt ipv6_multicast ifndex failed =%d", errno);
                    continue;
                }
               /* RFC 3315, Section 20 states that the hop limit must be set
                * to the max. value of 32 for the All_DHCP_Servers IPv6 address
                * as well as for any other multicast IPv6 helper address.
                */
                if (setsockopt(dhcpv6_relay_ctrl_cb_p->dhcpv6_relaySockFd, IPPROTO_IPV6,
                              IPV6_MULTICAST_HOPS, (char*)&hopLimit,
                              sizeof(hopLimit)) < 0)
                {
                    VLOG_ERR("setsockopt ipv6_multicast hoplimit ifndex failed =%d", errno);
                    continue;
                }
            }
            else if (IN6_IS_ADDR_LINKLOCAL(&(dst.sin6_addr)))
            {
               /*FIXME */
            }
            if (sendto(dhcpv6_relay_ctrl_cb_p->dhcpv6_relaySockFd, sBuf, pktLen,
                       0, (struct sockaddr *)&dst, sizeof(dst)) < 0)
            {
               VLOG_ERR("relay to server sendto failed =%d", errno);
               continue;
            }
        }
    }
    return true;
}

/*
 * Function      : dhcpv6r_getOptions
 * Responsiblity : This function gets the options information in the packet.
 * Parameters    : bufPtr - Buffer into which the pkt to be relayed
 *                 to client is to be copied into.
 *                 bufLenPtr - Ptr to the length of the buffer copied
 *                 rxPktLen - Length of packet received that is to be relayed.
 *                 clientDataPtr - Ptr to the client data parsed out of the packet.
 *                 relayFlagPtr - Indicates whether the packet is to be sent to a
 *                 downstream relay or to a client.
 * Return        : true - if options are successfully retrieved.
 *                 false - failure
 */
bool dhcpv6r_getOptions (char *bufPtr, uint32_t *bufLenPtr,
                        dhcpv6r_basePkt_t *pktPtr, int pktLen,
                        dhcpv6r_clientInfo_t *clientDataPtr, bool *relayFlagPtr)
{
    dhcpv6_opt_t thisOpt;
    dhcpv6_opt_t *optStartPtr = NULL, *optEndPtr = NULL;
    dhcpv6_opt_t *optCurPtr = NULL, *optNextPtr = NULL;
    dhcpv6r_basePkt_t pkt, relayPkt;
    bool relayMsgOptFound = false;

    if (bufPtr == NULL || bufLenPtr == NULL || pktPtr == NULL ||
       clientDataPtr == NULL || relayFlagPtr == NULL)
    {
        assert(0);
        return false;
    }

    /* Copy the relay reply message header into a structure. */
    memcpy(&pkt, pktPtr, sizeof(pkt));
    memcpy(&(clientDataPtr->clientAddress), &(pkt.peerAddress),
          sizeof(struct in6_addr));

    optStartPtr = (dhcpv6_opt_t*)((char*)pktPtr + sizeof(dhcpv6r_basePkt_t));
    optEndPtr = (dhcpv6_opt_t*)((char*)pktPtr + pktLen);

    for (; (optStartPtr < optEndPtr); (optStartPtr = optNextPtr))
    {
        memcpy(&thisOpt, optStartPtr, 4);
        thisOpt.opt_type = ntohs(thisOpt.opt_type);
        thisOpt.opt_len  = ntohs(thisOpt.opt_len);
        optCurPtr  = (dhcpv6_opt_t*)((char*)optStartPtr + 4);
        optNextPtr = (dhcpv6_opt_t*)((char*)optCurPtr + thisOpt.opt_len);

        switch (thisOpt.opt_type)
        {
        case OPT_RELAY_MSG:
        {
            if (thisOpt.opt_len <= DHCPV6_RELAY_MSG_SIZE)
            {
                memcpy(bufPtr, optCurPtr, thisOpt.opt_len);
                *bufLenPtr = thisOpt.opt_len;
                relayMsgOptFound = true;
                memcpy(&relayPkt, optCurPtr, sizeof(relayPkt));
               /* The message types defined have the MSB populated with
                * the appropriate number. The right shift is to get that MSB
                * to the LSB.
                */
                if (relayPkt.msgType == (RELAY_REPLY_MSG >> 24))
                    *relayFlagPtr = true;
                else
                    *relayFlagPtr = false;
            }
            break;
        }
        case OPT_INTF_ID:
        {
            if (thisOpt.opt_len <= sizeof(uint32_t))
            {
                memcpy(&(clientDataPtr->clientIntf), optCurPtr, thisOpt.opt_len);
                clientDataPtr->clientIntf = ntohl(clientDataPtr->clientIntf);
            }
            break;
        }
        default:
            break;
      } /* End of switch */
    } /* End of for */
    if (!relayMsgOptFound)
    {
      return false;
    }
    if (IN6_IS_ADDR_LINKLOCAL(&(clientDataPtr->clientAddress)))
    {
        if (clientDataPtr->clientIntf == 0)
            return false;
    }
    return true;
}


/*
 * Function      : dhcpv6r_setOptions
 * Responsiblity : This function sets the options information in the packet.
 * Parameters    : optStartPtr - Ptr to the start of the DHCPv6 options in the pkt.
 *                 pktLenPtr - Ptr to the constructed DHCPv6 packet length.
 *                 rxPktPtr - Ptr to the pkt to be relayed.
 *                 rxPktLen - Length of packet received that is to be relayed.
 *                 data - Ptr to additional data.
 * Return        : true - if options are successfully set.
 *                 false - failure
 */
bool dhcpv6r_setOptions (dhcpv6_opt_t *optStartPtr, uint32_t *pktLenPtr,
                        dhcpv6_basepkt *rxPktPtr, int rxPktLen,
                        uint32_t ifIndex, void *data)
{
    uint32_t tempIf = 0;
    if (optStartPtr == NULL || rxPktPtr == NULL || pktLenPtr == NULL)
    {
        assert(0);
        return false;
    }

    /* Set the relay message option. */
    optStartPtr->opt_type = htons(OPT_RELAY_MSG);
    optStartPtr->opt_len = htons(rxPktLen);
    /* Update pointers. */
    (*pktLenPtr) = (*pktLenPtr) + sizeof(dhcpv6_opt_t);
    optStartPtr = (dhcpv6_opt_t*)((char*)optStartPtr + sizeof(dhcpv6_opt_t));
    /* Copy the original packet. */
    memcpy(optStartPtr, rxPktPtr, rxPktLen);
    /* Update pointer. */
    optStartPtr = (dhcpv6_opt_t*)((char*)optStartPtr + rxPktLen);
    (*pktLenPtr) = (*pktLenPtr) + rxPktLen;

    /* Set the interface id option. */
    optStartPtr->opt_type = htons(OPT_INTF_ID);
    optStartPtr->opt_len = htons(sizeof(ifIndex));
    /* Update pointers. */
    (*pktLenPtr) = (*pktLenPtr) + sizeof(dhcpv6_opt_t);
    optStartPtr = (dhcpv6_opt_t*)((char*)optStartPtr + sizeof(dhcpv6_opt_t));

    tempIf = htonl(ifIndex);
    memcpy(optStartPtr, &tempIf, sizeof(ifIndex));

    optStartPtr = (dhcpv6_opt_t*)((char*)optStartPtr + sizeof(ifIndex));
    (*pktLenPtr) = (*pktLenPtr) + sizeof(ifIndex);

    /* Set the client MAC option */
    if( dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_option79_enable && (data !=0)) {
        /*set the option-79 option */
        optStartPtr->opt_type = htons(OPT_CLIENT_LINKLAYER_ADDR);
        optStartPtr->opt_len =htons(sizeof(dhcpv6_clientmac_Opt_t));
        /* Update pointers. */
        (*pktLenPtr) = (*pktLenPtr) + sizeof(dhcpv6_opt_t);
        optStartPtr = (dhcpv6_opt_t*)((char*)optStartPtr + sizeof(dhcpv6_opt_t));
        /*add the client MAC option */
        memcpy(optStartPtr, data, sizeof(dhcpv6_clientmac_Opt_t));
        /* Update pointers. */
        optStartPtr = (dhcpv6_opt_t*)((char*)optStartPtr + sizeof(dhcpv6_clientmac_Opt_t));
        (*pktLenPtr) = (*pktLenPtr) + sizeof(dhcpv6_clientmac_Opt_t);
    }
    return 1;
}

#endif
