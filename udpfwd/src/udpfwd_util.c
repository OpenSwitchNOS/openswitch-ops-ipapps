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

#include "udpfwd_util.h"

/* Feature to name mapping. There should be exact one-to-one mapping
 * between UDPFWD_FEATURE enum and feature_name array */
char *feature_name[] =
       {"UDP-Bcast-Forwarder",            /* UDP_BCAST_FORWARDER */
        "DHCP-Relay",                     /* DHCP_RELAY */
        "DHCP-Relay hop-count increment", /* DHCP_RELAY_HOP_COUNT_INCREMENT */
        "DHCP-Relay Option 82",           /* DHCP_RELAY_OPTION82 */
        "DHCP-Relay Option 82 validation" /* DHCP_RELAY_OPTION82_VALIDATE */
       };

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
    case UDP_BCAST_FORWARDER:
        if (value & UDP_BCAST_FORWARDER_BIT)
            status = ENABLE;
    break;

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
    case UDP_BCAST_FORWARDER:
        if (ENABLE == status)
            *value |= UDP_BCAST_FORWARDER_BIT;
        else
            *value &= ~UDP_BCAST_FORWARDER_BIT;
    break;

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

    default:
    break;
    }

    return;
}

/*
 * Function      : dhcpScanOpt
 * Responsiblity : This function is used to search for a specified option tag
 *                 in the options field that starts at opt and ends at optend.
 *                 If the ovld_opt argument is non-null, and an overload
 *                 option is encountered, its value
 *                 will be returned in *opt_ovld.
 * Parameters    : opt - option field starting point
 *                 optend - option field ending point
 *                 tag - option tag to search
 *                 ovld_opt - overload option
 * Return        : If the option tag is found, a pointer to it is returned.
 *                 Otherwise, this function returns NULL.
 */
uint8_t * dhcpScanOpt(uint8_t *opt, uint8_t *optend,
                            uint8_t tag, uint8_t *ovld_opt)
{
    while (opt < optend) {
        if (*opt == tag) {
            return(opt);
        }
        else if (*opt == END) {
            break;
        }
        else if (*opt == PAD) {
            opt++;
        }
        else {
            if (*opt == OPT_OVERLOAD) {
                if (ovld_opt != NULL) {
                    *ovld_opt = *OPTBODY(opt);
                }
            }
            opt += 2 + DHCPOPTLEN(opt);  /* + 2 for tag and length.*/
        }
    }
    return NULL;
}

/*
 * Function      : dhcpPickupOpt
 * Responsiblity : This function is used to search for a specified option tag in the
 *                 dhcp packet pointed to by dhcp of length len. It first searches the
 *                 options field. If an overload option is encountered in the options field,
 *                 it will also search the sname and/or file fields, as indicated by the
 *                 value of the overload option.
 * Parameters    : dhcp - dhcp packet
 *                 len - length of dhcp packet
 *                 tag - option tag
 * Return        : If the option tag is found, a pointer to it is returned.
 *                 Otherwise, this function returns NULL.
 */
uint8_t * dhcpPickupOpt(struct dhcp_packet *dhcp, int32_t len, uint8_t tag)
{
    bool useSname = false;
    bool useFile = false;
    uint8_t *opt;
    uint8_t *optend;
    uint8_t *retval;
    uint8_t overload = 0;

    /* First, try searching the options field. */
    opt = (uint8_t *)&(dhcp->options[MAGIC_LEN]);
    optend = (uint8_t *)&(dhcp->options[len - DFLTDHCPLEN + DFLTOPTLEN]);
    if ((retval = dhcpScanOpt(opt, optend, tag, &overload)) != NULL) {
        if (overload == 0)
            return retval;
    }

    /*
     * If we encountered an overload option in the options field, decode
     * its value to determine if we should look at the file/sname fields.
     */
    switch (overload) {
    case FILE_ISOPT:
        useFile = true;
        break;
    case SNAME_ISOPT:
        useSname = true;
        break;
    case BOTH_AREOPT:
        useFile = useSname = true;
        break;
    default:
        break;
    }

    /* Search the file field if the overload option said we should. */
    if (useFile) {
        opt = (uint8_t *)(dhcp->file);
        optend = (uint8_t *)&(dhcp->file[DHCP_BOOT_FILENAME_LEN]);
        if ((retval = dhcpScanOpt(opt, optend, tag, NULL)) != NULL) {
            return retval;
        }
    }
    /* Search the sname field if the overload option said we should. */
    else if (useSname) {
        opt = (uint8_t *)(dhcp->sname);
        optend = (uint8_t *)&(dhcp->sname[DHCP_SERVER_HOSTNAME_LEN]);
        if ((retval = dhcpScanOpt(opt, optend, tag, NULL)) != NULL) {
            return retval;
        }
    }

    /* Didn't find the tag, return NULL. */
    return (NULL);
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
 * Function      : getIpAddressfromIfname
 * Responsiblity : This function is used to get IP address associated
 *                 with a interface.
 * Parameters    : ifName - interface name
 * Return        : ip address if found otherwise 0
 */
IP_ADDRESS getIpAddressfromIfname(char *ifName)
{
    struct ifaddrs *ifaddr, *ifaddr_iter;
    struct sockaddr_in *res;
    IP_ADDRESS ip;

    if (getifaddrs(&ifaddr) == -1)
        return 0;

    for (ifaddr_iter = ifaddr; ifaddr_iter;
        (ifaddr_iter = ifaddr_iter->ifa_next))
    {
        if (ifaddr_iter->ifa_addr
            && ifaddr_iter->ifa_addr->sa_family == AF_INET)
        {
            if (strncmp(ifName, ifaddr_iter->ifa_name, IF_NAMESIZE) == 0)
            {
                res = (struct sockaddr_in *)ifaddr_iter->ifa_addr;
                ip = res->sin_addr.s_addr;
                freeifaddrs(ifaddr);
                return ip;
            }
        }
    }

    freeifaddrs(ifaddr);
    return 0; /* Failure case. */
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
