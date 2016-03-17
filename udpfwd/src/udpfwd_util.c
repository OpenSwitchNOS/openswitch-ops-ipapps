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
                        IFNAME_LEN) == 0)
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
            if (strncmp(ifName, ifaddr_iter->ifa_name, IFNAME_LEN) == 0)
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
 * Function      : get_ipsum
 * Responsiblity : IP checksum calculation function
 * Parameters    : data - pointer to the data
 *                 len - data length
 *                 initialValue - prior checksum value
 * Return        : checksum value
 */
uint32_t get_ipsum(uint16_t *data, int32_t len, uint32_t initialValue)
{
    uint16_t tmp;

    /*
     * The data is in network order and may not be non-aligned so
     * we use getShortFromPacket to get data from the pkt which
     * handles both of these issues.
     */
    for ( ; len > 0; len--)
    {
       initialValue += getShortFromPacket(data);
       data++;
    }

    /* Add the two partial sums (upper and lower 16 bits) together */
    initialValue = (initialValue & 0xffff) + (initialValue >> 16);
    tmp  = (initialValue & 0xffff) + (initialValue >> 16);

    tmp = (uint16_t) ~tmp;
    return tmp;
}

/*
 * Function      : checksumUdpHeader
 * Responsiblity : Sums the stuff at 'ptr' in preparation for use in the 1's
 *                 complement checksum.  It is used during udp psuedo header
 *                 calculation.
 * Parameters    : data - pointer to the data
 *                 len - number of words to be added
 * Return        : 16 bit checksum value
 */
uint16_t checksumUdpHeader(uint16_t *data, uint32_t len)
{
   register uint32_t total = 0;

   while (len--)
   {
      total += getShortFromPacket(data);
      data++;
      if (total >> 16 )
      {
         total = (total & 0xFFFF);
         total++;
      }
   }

   /* Make sure we are only returning 16-bit number */

   return (total & 0x0000ffff);
}

/*
 * Function      : get_udpsum
 * Responsiblity : Sums the stuff at 'ptr' in preparation for use in the 1's
 *                 complement checksum.  It is used during udp psuedo header
 *                 calculation.
 * Parameters    : data - pointer to the data
 *                 len - number of words to be added
 * Return        : 16 bit checksum value
 */
uint16_t get_udpsum(struct ip *iph, struct udphdr *udph)
{

    struct ps_udph ps_udph;       /* UDP pseudo-header */
    uint32_t  rounded_length = 0;
    u_int32_t xsum = 0;

    memset((char *)&ps_udph, 0, sizeof(ps_udph));
    ps_udph.srcip.s_addr = iph->ip_src.s_addr;
    ps_udph.dstip.s_addr = iph->ip_dst.s_addr;
    ps_udph.zero = 0;
    ps_udph.proto = iph->ip_p;
    ps_udph.ulen = udph->uh_ulen;
    udph->uh_sum = 0;

   if (0 == (ntohs(udph->uh_ulen) % 2))
      rounded_length = (uint32_t)(getShortFromPacket(&udph->uh_ulen));
   else
   {
      rounded_length = (uint32_t)((getShortFromPacket(&udph->uh_ulen)+1));
          /* pad the data with a zero byte */
      *((uint8_t *) udph + rounded_length-1) = 0;
   }
   xsum =  checksumUdpHeader((uint16_t *)&udph, sizeof(struct ps_udph)/2);
   xsum =  get_ipsum((uint16_t *)udph, rounded_length/2, xsum);
   return (htons((uint16_t)xsum));
}
