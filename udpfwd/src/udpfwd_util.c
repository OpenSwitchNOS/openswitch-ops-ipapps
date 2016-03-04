/*
 * (c) Copyright 2016 Hewlett Packard Enterprise Development LP
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
#include "udpfwd.h"
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

unsigned char * dhcpScanOpt(unsigned char *opt, unsigned char *optend,
                            unsigned char tag, unsigned char *ovld_opt)
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

unsigned char * dhcpPickupOpt(struct dhcp_packet *dhcp, int len,
                                unsigned char tag)
{
    bool useSname = FALSE;
    bool useFile = FALSE;
    unsigned char *opt;
    unsigned char *optend;
    unsigned char *retval;
    unsigned char overload = 0;

    /* First, try searching the options field. */
    opt = (unsigned char *)&(dhcp->options[MAGIC_LEN]);
    optend = (unsigned char *)&(dhcp->options[len - DFLTDHCPLEN + DFLTOPTLEN]);
    if ((retval = dhcpScanOpt(opt, optend, tag, &overload)) != NULL) {
       return retval;
    }

    /* If we encountered an overload option in the options field, decode
     * its value to determine if we should look at the file/sname fields.
     */
    switch (overload) {
        case FILE_ISOPT:
            useFile = TRUE;
            break;
        case SNAME_ISOPT:
            useSname = TRUE;
            break;
        case BOTH_AREOPT:
            useFile = useSname = TRUE;
            break;
        default:
            break;
    }

    /* Search the file field if the overload option said we should. */
    if (useFile) {
        opt = (unsigned char *)(dhcp->file);
        optend = (unsigned char *)&(dhcp->file[DHCP_BOOT_FILENAME_LEN]);
        if ((retval = dhcpScanOpt(opt, optend, tag, NULL)) != NULL) {
            return retval;
        }
    }

    /* Search the sname field if the overload option said we should. */
    if (useSname) {
        opt = (unsigned char *)(dhcp->sname);
        optend = (unsigned char *)&(dhcp->sname[DHCP_SERVER_HOSTNAME_LEN]);
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
                    if (strcmp(i->if_name, ifaddr_iter->ifa_name) == 0)
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
 * Function      : getIfIndexfromIpAddress
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
            if (strcmp (ifName, ifaddr_iter->ifa_name) == 0)
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
