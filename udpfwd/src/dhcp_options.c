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
 * File: dhcp_options.c
 *
 */

/*
 *
 * Purpose: DHCP relay agent Information Option (82) implementation
 */

#include "dhcp_relay.h"
#include "udpfwd_util.h"
#include "udpfwd.h"
#include <stdlib.h>

VLOG_DEFINE_THIS_MODULE(dhcp_options);

unsigned char dhcpCookie[] = RFC1048_MAGIC;

/*
 * Function: dhcp_relay_get_option82_len
 * Responsibility: Calculates the number of bytes required to fill the
 *                 option 82 field in the DHCP message. The option 82 field will
 *                 look as follows:
 *
 *     | Relay agent   |   payload                                       |
 *     | info header   |                                                 |
 *     ------------------------------------------------------------------
 *          |                     |
 *          v                     v
 *     +-----------------------------------------------------------------+
 *     |Option 82 | len| circuit id| len|circuit  | remote | len| remote |
 *     | id       |    |           |    | data    | id     |    |  data  |
 *     +-----------------------------------------------------------------+
 *
 * Parameterss: remote_id - remote_id
 * Returns: option length in bytes
 */
int32_t dhcp_relay_get_option82_len(DHCP_RELAY_OPTION82_REMOTE_ID remote_id)
{
    /* Base length */
    int length = DHCP_OPTION_HEADER_LENGTH;

    /* Add circuit id sub-option length */
    length += DHCP_OPTION_HEADER_LENGTH + sizeof(CIRCUIT_ID_t);

    /* Add remote id sub-option length */
    switch (remote_id) {
    case REMOTE_ID_MAC:
        length += DHCP_OPTION_HEADER_LENGTH + MAC_HEADER_LENGTH ;
        break;
    case REMOTE_ID_IP:
        length += DHCP_OPTION_HEADER_LENGTH + sizeof(IP_ADDRESS);
        break;
    default: break;
    }

    /* We should never exceed 255 octets */
    assert(length <= 255);

    return length;
}

/*
 * Function: dhcp_relay_validate_agent_option
 *
 * Responsibility: Find an interface that matches the circuit ID specified in the
 *                 Relay Agent Information option and also validate the remote id
 *                 specified in the relay agent option with the switch mac or ip address.
 * Parameters: buf - buffer which has Relay agent infortion
 *             len - length of the buffer
 *             pkt_info - stores the interface info only if the received packet
 *             contains valid Relay info.
 *             remote_id - remote_id
 *             ifName - interface name
 * Returns: status of the validation
 *          DHCP_RELAY_OPTION_82_OK - If it matches the option choosen in the switch.
 *          DHCP_RELAY_INVALID_OPTION_82 - If the option is corrupted, or
 *          DHCP_RELAY_INVALID_OPTION_82 - If the relay agent option
 *                                    doesn't match with the switch option.
 */
int32_t dhcp_relay_validate_agent_option(const uint8_t *buf, int32_t buflen,
                            char *ifName, DHCP_OPTION_82_OPTIONS *pkt_info,
                            DHCP_RELAY_OPTION82_REMOTE_ID remote_id)
{
    int32_t iter, circuit_id_len = 0, remote_id_len = 0, opttype = 0, optlen =0;
    const uint8_t *circuit_id_ptr = NULL, *remote_id_ptr = NULL, *optvalue;
    CIRCUIT_ID_t circuit_id = 0;
    struct in_addr intf_ip_address;
    MAC_ADDRESS mac;

    assert(buf);

    /* Each sub-option has a one octet type, one octet length, and variable body */
    for (iter = 0;  iter < buflen - 1; )
    {
        opttype = buf[iter++];
        optlen = buf[iter++];
        optvalue = buf + iter;
        iter += optlen;

        /* If this sub-option goes past the end of the buffer, the option 82 info
         * is invalid.
         */
        if (iter > buflen)
            return DHCP_RELAY_INVALID_OPTION_82;

      /* Parse sub-option */
        switch (opttype)
        {
        case DHCP_RAI_CIRCUIT_ID:
            circuit_id_len = optlen;
            circuit_id_ptr = optvalue;
            break;

        case DHCP_RAI_REMOTE_ID:
            remote_id_len = optlen;
            remote_id_ptr = optvalue;
            break;

        default:
            return DHCP_RELAY_INVALID_OPTION_82;
      }
   }

    /* If there is no circuit ID and remote ID, the option is invalid */
    if (!circuit_id_ptr || !remote_id_ptr)
        return DHCP_RELAY_OPTION_82_MISMATCH;

    /* Validate circuit_id value length */
    if (circuit_id_len != sizeof circuit_id)
        return DHCP_RELAY_INVALID_OPTION_82;

    /* Convert circuit id in string format back to unsigned int */
    circuit_id = ((circuit_id_ptr[0] << 24) + (circuit_id_ptr[1] << 16) +
                  (circuit_id_ptr[2] << 8) + circuit_id_ptr[3]);

    /* Check that the remote_id is correct based on our current config */
    switch (remote_id)
    {
    case REMOTE_ID_MAC:
        /* Remote ID should be a MAC address */
        if (remote_id_len != MAC_HEADER_LENGTH )
            return DHCP_RELAY_OPTION_82_MISMATCH;

        getMacfromIfname(mac, ifName);
        if (memcmp(remote_id_ptr, mac, MAC_HEADER_LENGTH ) != 0)
            return DHCP_RELAY_OPTION_82_MISMATCH;
        break;

    case REMOTE_ID_IP:
        /* Remote ID should be an IP address */
        if (remote_id_len != sizeof intf_ip_address.s_addr)
            return DHCP_RELAY_OPTION_82_MISMATCH;
        memcpy(&intf_ip_address.s_addr, remote_id_ptr, sizeof intf_ip_address.s_addr);
        /* check interface with this ip exists or not */
        if (!ipExistsOnInterface(ifName, intf_ip_address.s_addr))
            return DHCP_RELAY_OPTION_82_MISMATCH;
        pkt_info->ip_addr = intf_ip_address.s_addr;
        break;

    default: break;
    }

    /* If we get here, everything is valid so save the circuit_id value */
    pkt_info->circuit_id = circuit_id;

    return DHCP_RELAY_OPTION_82_OK;
}

/*
 * Function: process_dhcp_relay_option82_message
 * Responsibility: Process the DHCP packet based on the relay_type param.
 *                 If the relay_type = DHCPR_HANDLE_RESPONSE then strip any
 *                 relay agent information present in the packet.
 *                 If the relay_type = DHCPR_HANDLE_REQUEST
 *                 then add relay agent information based on policy
 *
 *  ------------------------------------------------------------------
 * |ETHERNET HDR | IP HDR | DHCP HDR | MAGIC COOKIE |OPT1|OPT2|....|FF|
 *  ------------------------------------------------------------------
 * Parameters: pkt - received DHCP packet
 *             pkt_info - stores the interface info,
 *             if relay agent info option is valid.
 *             ifIndex - interface index
 *             ifName - interface name
 *             bootp_gw - bootp_gw address
 *
 * Returns:  true - if the packet is valid, else returns false
 *          false - any failures.
 */
bool process_dhcp_relay_option82_message(void *pkt, DHCP_OPTION_82_OPTIONS *pkt_info,
                               uint32_t ifIndex, char *ifName, IP_ADDRESS bootp_gw)
{
    struct ip *iph = NULL;       /* pointer to IP header */
    struct udphdr *udph = NULL;  /* pointer to UDP header */
    struct dhcp_packet* dhcp = NULL;    /* pointer to DHCP header */
    bool is_dhcp = false, opt82 = false;
    uint8_t *option_parser_ptr = NULL, *sp = NULL, *max = NULL, *end_pad = NULL;
    int32_t good_agent_option = 0, status =0, len = 0;
    uint32_t length = 0, packlen = 0;
    uint16_t max_msg_size = 0;
    DHCP_RELAY_OPTION82_REMOTE_ID remote_id;
    DHCP_RELAY_OPTION82_POLICY    policy;
    CIRCUIT_ID_t circuit_id = ifIndex;

    /* Don't process the packet if the admin status is not enabled */
    if (ENABLE != get_feature_status(udpfwd_ctrl_cb_p->feature_config.config,
                          DHCP_RELAY_OPTION82)) {
        VLOG_INFO("DHCP relay option 82 is disabled. dont process the packet");
        return true;
    }

    /* fill values of remote-id and policy from global config */
    policy = udpfwd_ctrl_cb_p->feature_config.policy;
    remote_id = udpfwd_ctrl_cb_p->feature_config.r_id;

    iph  = (struct ip *) pkt;
    udph = (struct udphdr *) ((char *)iph + (iph->ip_hl * 4));
    dhcp = (struct dhcp_packet *)
                       ((char *)iph + (iph->ip_hl * 4) + UDPHDR_LENGTH);


    length = DHCP_PKTLEN(udph);

    /* If there's no cookie, it's a bootp packet and we forward it unchanged */
    if (memcmp((char *)dhcp->options, (char *)dhcpCookie, MAGIC_LEN) != 0)
        return true;

    max = ((uint8_t *)dhcp) + length;
    option_parser_ptr = (uint8_t *)(dhcp->options + MAGIC_LEN);
    sp = option_parser_ptr;

    while (option_parser_ptr < max)
    {
        switch (*option_parser_ptr)
        {
        /* Skip padding... */
        case PAD:
            end_pad = sp;
            *sp++ = *option_parser_ptr++;
        continue;

        /* If we see a message type, it's a DHCP packet. */
        case DHCP_MSGTYPE:
            is_dhcp = 1;
        break;

        case DHCP_MAXMSGSIZE:
            if (dhcp->op == BOOTREQUEST)
            {
                len = option_parser_ptr[1];
                if (len == 2)
                {
                    max_msg_size = (option_parser_ptr[2] << 8) + option_parser_ptr[3];
                    if (max_msg_size < MAX_DHCP_MESSAGE_SIZE)
                        max_msg_size = 0;
               }
               else
               {
                  VLOG_ERR("Pkt dropped. dhcp_maxmsgsize option with invalid length");
                  return false;
               }
            }
        break;

        /* Quit immediately if we hit an End option. */
        case END:
            if (sp != option_parser_ptr)
            {
               *sp++ = *option_parser_ptr;
            } else
               sp++;
            option_parser_ptr = max;
            continue;

        case DHCP_AGENT_OPTIONS:
            /* We shouldn't see a relay agent option in a packet before we've seen
             * the DHCP packet type, but if we do, we have to leave it alone.  */
            if (!is_dhcp)
            {
               break;
            }
            if (dhcp->giaddr.s_addr == 0)
                /* drop packets with giaddr field set to NULL and option 82 !=NULL */
                return false;

            end_pad = 0;
            if (dhcp->op == BOOTREPLY)
            {
                /* We are validating the response received from the server for a
                * request we sent.  Verify if we receive the correct relay agent
                * information.  */
                status = dhcp_relay_validate_agent_option(option_parser_ptr + 2,
                                                   option_parser_ptr[1],
                                                   ifName,
                                                   pkt_info,
                                                   remote_id);
                if (status == DHCP_RELAY_INVALID_OPTION_82)
                {
                    /* The packet is corrupted so drop it */
                    return false;
                }
                if (status == DHCP_RELAY_OPTION_82_OK)
                {
                    /* We found the matching relay agent information.  Set the flag,
                    * so that we can process the message.  */
                    good_agent_option = true;
                }

                /* Advance to next option */
                option_parser_ptr += option_parser_ptr[1] + DHCP_OPTION_HEADER_LENGTH;
                continue;
            }
            else if (dhcp->op == BOOTREQUEST)
            {
                /* We received the DHCP request from the client and it contains
                * relay agent information within the packet.  The relay agent
                * will process the packet based on the policy defined in the
                * switch.  The policies are:
                *  keep    - keep the existing relay agent information
                *  drop    - drop the packet
                *  replace - replace the existing relay agent info with our info
                */
                switch (policy)
                {
                case KEEP:
                    return true;
                case DROP:
                    return false;
                case REPLACE:
                default:
                    /* Skip over the agent option */
                    option_parser_ptr += option_parser_ptr[1] + DHCP_OPTION_HEADER_LENGTH;
                    continue;
                }
            }
            break;

            /* Ignore all other options */
        default:
            break;
      } /* End of switch */

        end_pad = 0;
        if (sp != option_parser_ptr)
        {
            memmove(sp, option_parser_ptr,
                 (unsigned)(option_parser_ptr[1] + DHCP_OPTION_HEADER_LENGTH));
        }

       /* Here,
        * option_parser_ptr [0] = Option Code (Type).
        * option_parser_ptr [1] = Option Length (Length).
        * option_parser_ptr [2] = Value.
        * So adding option_parser_ptr [1] and DHCPR_DHCP_OPT_AND_LEN_FIELD_BYTES
        * to option_parser_ptr, places it to the beginning of the next DHCP option
        * or the end of the packet.
        */
        sp += option_parser_ptr[1] + DHCP_OPTION_HEADER_LENGTH;
        option_parser_ptr += option_parser_ptr[1] + DHCP_OPTION_HEADER_LENGTH;

    } /* while more options to process */

    /* If it's not a DHCP packet, we don't modify it */
    if (!is_dhcp)
        return true;

    if (dhcp->op == BOOTREPLY)
    {
        /* If none of the agent options we found matched, or if we didn't find
        * any agent options, count this packet as not having any matching agent
        * options, and if we're relying on agent options to determine the
        * outgoing interface, drop the packet.
        */
        if ((good_agent_option == false) &&
            (ENABLE == get_feature_status
            (udpfwd_ctrl_cb_p->feature_config.config, DHCP_RELAY_OPTION82_VALIDATE)))
        {
            VLOG_ERR("DHCP relay option 82 validate is enabled. drop the packet");
            return false;
        }
        else
            return true;
   }
    else if (dhcp->op == BOOTREQUEST)
    {
        /* If the packet had padding, we can store the agent option at the
        * beginning of the pad.  */
        if (end_pad)
            sp = end_pad;


        /* Remember where the end of the packet was after parsing it */
        option_parser_ptr = sp;

        length = sp - ((uint8_t *)dhcp);
        packlen = (length + dhcp_relay_get_option82_len(remote_id));

        /* Check if there enough space to add new option */
        opt82 = (RECV_BUFFER_SIZE > packlen);

        /* If there is max dhcp size option in the pkt, check if we will exceed
        * this size.  If so, do not add option82.  */
        if (opt82 && max_msg_size)
        {
            opt82 = (max_msg_size >= packlen);
            if (!opt82)
                VLOG_ERR("Option 82 not added. Pkt size > max msg size in pkt");
        }
        if (opt82)
        {
            /* We have space to add the option 82 field in the message.  If the sp
            * points past the END option, back up and overwrite it.  */

            if (sp[-1] == END)
                sp--;

            /* Add the option 82 header */
            *sp++ = DHCP_AGENT_OPTIONS;
            *sp++ = dhcp_relay_get_option82_len(remote_id) - DHCP_OPTION_HEADER_LENGTH;

            /* Copy in the circuit id */
            *sp++ = DHCP_RAI_CIRCUIT_ID;
            *sp++ = sizeof circuit_id;

            *sp++ = (circuit_id >> 24);
            *sp++ = ((circuit_id >> 16) & 0xff);
            *sp++ = ((circuit_id >> 8)& 0xff);
            *sp++ = (circuit_id & 0xff);

            /* Copy in the remote ID */
            if (remote_id == REMOTE_ID_MAC)
            {
                *sp++ = DHCP_RAI_REMOTE_ID;
                *sp++ = MAC_HEADER_LENGTH ;

                getMacfromIfname(sp, ifName);
                sp += MAC_HEADER_LENGTH ;
            }
            else if (remote_id == REMOTE_ID_IP)
            {
                struct in_addr addr;

                /* If a BOOTP gateway is configured, use that address */
                if (bootp_gw)
                    addr.s_addr = bootp_gw;
                else
                    /* Use IP address of the interface where the packet was received */
                    addr.s_addr = pkt_info->ip_addr;

                *sp++ = DHCP_RAI_REMOTE_ID;
                *sp++ = sizeof(addr.s_addr);
                memcpy(sp, &addr.s_addr, sizeof(addr.s_addr));
                sp += sizeof(addr.s_addr);
            }

            /* Add END option to packet */
            *sp++ = END;
        }
    }

    /* Recalculate the DHCP length */

    length = sp - ((uint8_t *)dhcp);

    /* If length is less than minimum, add padding */
    if (length < MINBOOTPLEN) {
        memset(sp, 0, MINBOOTPLEN - length);
        length = MINBOOTPLEN;
    }

   /* The packet is modified, fill the UDP length parameters */
   udph->uh_ulen = htons(length + UDPHDR_LENGTH);
   udph->uh_sum = 0;

   /* Update IP header.  The only parameter changed is length.  */
   iph->ip_len = htons(length + UDPHDR_LENGTH + (iph->ip_hl * 4));
   iph->ip_sum = 0;  /* actual checksum calculated before sending the packet */

   return true;
}
