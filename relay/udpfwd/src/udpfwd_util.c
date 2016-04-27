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
 * File: udpfwd_util.c
 *
 */

/*
 * This file has all the utility functions
 * related to DHCP relay functionality and also functions
 * to get ifIndex from IP address, getIpAddress from ifName.
 */

#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/if_packet.h>
#include "udpfwd_util.h"

/* Macros to iterate through ifaddrs structure */

#define GET_NEXT_IP_ON_INTERFACE(IFA_ITER, IFADDR, IFNAME) \
    for ((IFA_ITER) = IFADDR; \
        (IFA_ITER); \
        (IFA_ITER) = IFA_ITER->ifa_next) \
            if (IFA_ITER->ifa_addr \
                && IFA_ITER->ifa_addr->sa_family == AF_INET) \
                if (strncmp(IFNAME, IFA_ITER->ifa_name, IF_NAMESIZE) == 0)

/* Feature to name mapping. There should be exact one-to-one mapping
 * between UDPFWD_FEATURE enum and feature_name array */
char *feature_name[] =
       {
#ifdef FTR_UDP_BCAST_FWD
        "UDP-Bcast-Forwarder",            /* UDP_BCAST_FORWARDER */
#endif /* FTR_UDP_BCAST_FWD */
#ifdef FTR_DHCP_RELAY
        "DHCP-Relay",                     /* DHCP_RELAY */
        "DHCP-Relay hop-count increment", /* DHCP_RELAY_HOP_COUNT_INCREMENT */
        "DHCP-Relay Option 82",           /* DHCP_RELAY_OPTION82 */
        "DHCP-Relay Option 82 validation" /* DHCP_RELAY_OPTION82_VALIDATE */
#endif /* FTR_DHCP_RELAY */
       };

#ifdef FTR_DHCP_RELAY
/* Policy to name mapping. There should be strict one-to-one mapping
 * between DHCP_RELAY_OPTION82_POLICY and policy_name array */
char *policy_name[] =
     {"keep",    /* KEEP */
      "drop",    /* DROP */
      "replace"  /* REPLACE */
     };

/* remote-id to name mapping. There should be strict one-to-one mapping
 * between DHCP_RELAY_OPTION82_REMOTE_ID and policy_name array */
char *remote_id_name[] =
     {"ip",    /* REMOTE_ID_IP */
      "mac"    /* REMOTE_ID_MAC */
     };
#endif /* FTR_DHCP_RELAY */

/*
 * Function      : get_feature_status
 * Responsiblity : Get the configuration status of a feature
 * Parameters    : value - configuration bit map
 *                 feature - feature enum value
 * Return        : ENABLE - If the feature is enabled
 *                 DISABLE - otherwise
 */
FEATURE_STATUS get_feature_status(feature_bmap value, UDPFWD_FEATURE feature)
{
    FEATURE_STATUS status = DISABLE;

    switch(feature)
    {
#ifdef FTR_UDP_BCAST_FWD
    case UDP_BCAST_FORWARDER:
        if (value & UDP_BCAST_FORWARDER_BIT)
            status = ENABLE;
    break;
#endif /* FTR_UDP_BCAST_FWD */

#ifdef FTR_DHCP_RELAY
    case DHCP_RELAY:
        if (value & DHCP_RELAY_BIT)
            status = ENABLE;
    break;

    case DHCP_RELAY_HOP_COUNT_INCREMENT:
        if (value & DHCP_RELAY_HOP_COUNT_INCR_BIT)
            status = ENABLE;
    break;

    case DHCP_RELAY_OPTION82:
        if (value & DHCP_RELAY_OPTION82_BIT)
            status = ENABLE;
    break;

    case DHCP_RELAY_OPTION82_VALIDATE:
        if (value & DHCP_RELAY_OPTION82_VALIDATE_BIT)
            status = ENABLE;
    break;
#endif /* FTR_DHCP_RELAY */

    default:
    break;
    }

    return status;
}

/*
 * Function      : set_feature_status
 * Responsiblity : Set the configuration status of a feature
 * Parameters    : value - pointer to configuration bit map
 *                 feature - feature enum value
 *                 status - config status of the feature
 * Return        : none
 */
void set_feature_status(feature_bmap *value, UDPFWD_FEATURE feature,
                        FEATURE_STATUS status)
{
    switch(feature)
    {
#ifdef FTR_UDP_BCAST_FWD
    case UDP_BCAST_FORWARDER:
        if (ENABLE == status)
            *value |= UDP_BCAST_FORWARDER_BIT;
        else
            *value &= ~UDP_BCAST_FORWARDER_BIT;
    break;
#endif /* FTR_UDP_BCAST_FWD */

#ifdef FTR_DHCP_RELAY
    case DHCP_RELAY:
        if (ENABLE == status)
            *value |= DHCP_RELAY_BIT;
        else
            *value &= ~DHCP_RELAY_BIT;
    break;

    case DHCP_RELAY_HOP_COUNT_INCREMENT:
        if (ENABLE == status)
            *value |= DHCP_RELAY_HOP_COUNT_INCR_BIT;
        else
            *value &= ~DHCP_RELAY_HOP_COUNT_INCR_BIT;
    break;

    case DHCP_RELAY_OPTION82:
        if (ENABLE == status)
            *value |= DHCP_RELAY_OPTION82_BIT;
        else
            *value &= ~DHCP_RELAY_OPTION82_BIT;
    break;

    case DHCP_RELAY_OPTION82_VALIDATE:
        if (ENABLE == status)
            *value |= DHCP_RELAY_OPTION82_VALIDATE_BIT;
        else
            *value &= ~DHCP_RELAY_OPTION82_VALIDATE_BIT;
    break;
#endif /* FTR_DHCP_RELAY */

    default:
    break;
    }

    return;
}

/*
 * Function      : getIfIndexfromIpAddress
 * Responsiblity : This function is used to get ifIndex associated
 *                 with a IP address.
 * Parameters    : ip - IP Address
 * Return        : ifindex if found otherwise -1
 */
uint32_t getIfIndexfromIpAddress(IP_ADDRESS ip)
{
    struct ifaddrs *ifaddr, *ifaddr_iter;
    struct sockaddr_in *res;
    IP_ADDRESS ipaddr;
    struct if_nameindex *if_ni, *i;
    uint32_t ifIndex = -1;

    if_ni = if_nameindex();

    if (if_ni == NULL)
        return -1;

    if (getifaddrs(&ifaddr) == -1)
        return -1;

    /* Iterate through the linked list. */
    for (ifaddr_iter = ifaddr; ifaddr_iter;
        (ifaddr_iter = ifaddr_iter->ifa_next))
    {
        if (ifaddr_iter->ifa_addr
        && ifaddr_iter->ifa_addr->sa_family == AF_INET)
        {
            res = (struct sockaddr_in *)ifaddr_iter->ifa_addr;
            ipaddr = res->sin_addr.s_addr;
            /* For matching ip address, extract ifname and then ifindex. */
            if( ipaddr == ip )
            {
                for (i = if_ni;
                    !(i->if_index == 0 && i->if_name == NULL); i++)
                {
                    if (strncmp(i->if_name, ifaddr_iter->ifa_name,
                                IF_NAMESIZE) == 0)
                    {
                        ifIndex = i->if_index;
                        if_freenameindex(if_ni);
                        freeifaddrs(ifaddr);
                        return ifIndex;
                    }
                }
            }
        }
    }

    freeifaddrs(ifaddr);
    if_freenameindex(if_ni);
    return -1; /* Failure case. */
}

/*
 * Function      : getMacfromIfname
 * Responsiblity : This function is used to get MAC address associated
 *                 with a interface.
 * Parameters    : mac - variable to store mac address.
 *                 ifName - interface name
 * Return        : void
 */
void getMacfromIfname(MAC_ADDRESS mac, char * ifName)
{
    struct ifaddrs *ifaddr = NULL, *ifa = NULL;
    int32_t i = 0;
    struct sockaddr_ll *s = NULL;

    if (getifaddrs(&ifaddr) == -1)
        return;

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if ((ifa->ifa_addr) && (ifa->ifa_addr->sa_family == AF_PACKET))
        {
            s = (struct sockaddr_ll*)ifa->ifa_addr;
            if (strncmp(ifName, ifa->ifa_name, IF_NAMESIZE) == 0)
            {
                for (i = 0; i < s->sll_halen; i++)
                {
                  mac[i] = s->sll_addr[i];
                }
                break;
            }
        }
    }
    freeifaddrs(ifaddr);
}

/*
 * Function      : getLowestIpOnInterface
 * Responsiblity : This function is used to get lowest IP address
 *                 associated with a interface.
 * Parameters    : ifName - interface name
 * Return        : ip address if found otherwise 0
 */
IP_ADDRESS getLowestIpOnInterface(char *ifName)
{
    struct ifaddrs *ifaddr_iter = NULL, *ifaddr = NULL;
    struct sockaddr_in *res;
    IP_ADDRESS ip;
    IP_ADDRESS lowest_ip = MAX_UINT32; /*255.255.255.255.255 */

    if (getifaddrs(&ifaddr) == -1)
        return 0;

    GET_NEXT_IP_ON_INTERFACE(ifaddr_iter, ifaddr, ifName)
    {
        res = (struct sockaddr_in *)ifaddr_iter->ifa_addr;
        ip = res->sin_addr.s_addr;
        if (lowest_ip > ip)
            lowest_ip = ip;
    }

    freeifaddrs(ifaddr);

    if (lowest_ip != MAX_UINT32)
        return lowest_ip;
    else
        return 0; /* Failure case. */
}

/*
 * Function      : ipExistsOnInterface
 * Responsiblity : This function is used to check if IP address
 *                 exists on an interface.
 * Parameters    : ifName - interface name
 *                 address - ip address
 * Return        : true if ip address exists else false.
 */
bool ipExistsOnInterface(char *ifName, IP_ADDRESS address)
{
    struct ifaddrs *ifaddr_iter = NULL, *ifaddr = NULL;
    struct sockaddr_in *res;
    IP_ADDRESS ip;

    if (getifaddrs(&ifaddr) == -1)
        return false;

    GET_NEXT_IP_ON_INTERFACE(ifaddr_iter, ifaddr, ifName)
    {
        res = (struct sockaddr_in *)ifaddr_iter->ifa_addr;
        ip = res->sin_addr.s_addr;
        if (ip == address)
        {
            freeifaddrs(ifaddr);
            return true;
        }
    }

    freeifaddrs(ifaddr);
    return false;
}

/*
 * Function      : in_cksum
 * Responsiblity : Checksum computation function
 * Parameters    : addr - data pointer
 *                 len - Data length
 *                 csum - previous checksum if any
 * Return        : checksum value
 */
uint16_t
in_cksum(const uint16_t *addr, register int32_t len, uint16_t csum)
{
    register int32_t nleft = len;
    const uint16_t *w = addr;
    register uint16_t answer;
    register int32_t sum = csum;

    /*
     *  Our algorithm is simple, using a 32 bit accumulator (sum),
     *  we add sequential 16 bit words to it, and at the end, fold
     *  back all the carry bits from the top 16 bits into the lower
     *  16 bits.
     */
    while (nleft > 1)  {
        sum += *w++;
        nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if (nleft == 1)
        sum += (*(uint8_t *)w); /* le16toh() may be unavailable on old systems */

    /*
     * add back carry outs from top 16 bits to low 16 bits
     */
    sum = (sum >> 16) + (sum & 0xffff);    /* add hi 16 to low 16 */
    sum += (sum >> 16);            /* add carry */
    answer = ~sum;                /* truncate to 16 bits */
    return (answer);
}
