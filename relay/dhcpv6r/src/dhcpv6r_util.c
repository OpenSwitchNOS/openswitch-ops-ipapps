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
 * File: dhcpv6r_util.c
 *
 */

/*
 * This file has all the utility functions
 * related to dhcpv6_relay.
 */

#include "dhcpv6_relay.h"

#ifdef FTR_DHCPV6_RELAY
/*
 * Function      : dhcpv6r_is_valid_msg
 * Responsiblity : This function determines if the message type passed in is valid.
 * Parameters    : msgType - Type of DHCPv6 message to be validated.
 * Return        : true - if the message is a valid type.
 *                 false - failure
 */
bool dhcpv6r_is_valid_msg(uint32_t msgType)
{
    switch (msgType)
    {
    case SOLICIT_MSG:
    case REQUEST_MSG:
    case CONFIRM_MSG:
    case RENEW_MSG:
    case REBIND_MSG:
    case RELEASE_MSG:
    case DECLINE_MSG:
    case INFOREQ_MSG:
    case RELAY_FORW_MSG:
    case RELAY_REPLY_MSG:
    case REPLY_MSG:
    case ADVERTISE_MSG:
    case RECONFIGURE_MSG:
    {
        return true;
    }
    default:
    {
        break;
    }
    }
    return false;
}

/*
 * Function      : dhcpv6r_is_msg_from_client
 * Responsiblity : This function determines if the message was sent by the client.
 * Parameters    : msgType - Type of DHCPv6 message to be checked.
 * Return        : true - if the message was sent from a client.
 *                 false - failure
 */
bool dhcpv6r_is_msg_from_client(uint32_t msgType)
{
    switch (msgType)
    {
    case SOLICIT_MSG:
    case REQUEST_MSG:
    case CONFIRM_MSG:
    case RENEW_MSG:
    case REBIND_MSG:
    case RELEASE_MSG:
    case DECLINE_MSG:
    case INFOREQ_MSG:
    case RELAY_FORW_MSG:
        return true;
    case ADVERTISE_MSG:
    case REPLY_MSG:
    case RELAY_REPLY_MSG:
    case RECONFIGURE_MSG:
        return false;
    default:
        break;
    }
    return true;
}

/*
 * Function      : dhcpv6r_is_msg_from_server
 * Responsiblity : This function determines if a valid message was
 *                 sent by the server.
 * Parameters    : msgType - Type of DHCPv6 message to be checked.
 * Return        : true - if a valid message was sent from a server.
 *                 false - failure
 */
bool dhcpv6r_is_msg_from_server(uint32_t msgType)
{
    switch (msgType)
    {
    case RELAY_REPLY_MSG:
        return true;
    default:
        return false;
   }
}

#endif /*FTR_DHCPV6_RELAY*/
