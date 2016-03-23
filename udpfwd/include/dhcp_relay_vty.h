/* DHCP-Relay CLI commands header file
 *
 * Copyright (C) 2016 Hewlett Packard Enterprise Development LP
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/****************************************************************************
 * @file dhcp_relay_vty.h
 * purpose: To add declarations required for
 * dhcp_relay_vty.c
 ***************************************************************************/

#ifndef DHCP_RELAY_VTY_H
#define DHCP_RELAY_VTY_H

#include <stdbool.h>

#define DHCP_RELAY_STR \
"Configure dhcp-relay\n"
#define HELPER_ADDRESS_STR \
"Configure the helper-address for dhcp-relay\n"
#define SHOW_DHCP_RELAY_STR \
"Show dhcp-relay configuration\n"
#define SHOW_HELPER_ADDRESS_STR \
"Show the helper-address for dhcp-relay configuration\n"
#define HELPER_ADDRESS_INPUT_STR \
"Enter helper address\n"

#define SUBIFNAME_STR \
"Subinterface name\n"
#define HOP_COUNT_INCREMENT_STR \
"Increment hop count in relayed DHCP packets\n"
#define OPTION_STR \
"Configure DHCP option processing\n"
#define OPTION_82_STR \
"Configure DHCP option 82 processing\n"
#define DROP_STR \
"Drop DHCP packets that contain option 82 information\n"
#define KEEP_STR \
"Retain existing option 82 information in DHCP packets\n"
#define REPLACE_STR \
"Replace existing option 82 information in DHCP packets with information from this switch\n"
#define VALIDATE_STR \
"Validate option 82 information in DHCP server responses and drop invalid responses\n"
#define OPTION_IP_STR \
"Use the IP address of the client's interface as the option 82 remote ID\n"
#define MAC_STR \
"Use the MAC address of the router as the option 82 remote ID (this is the default)\n"

/* FIXME: Temporary addition. Once macro definitions are merged in idl, this shall shall be removed */
#define SYSTEM_DHCP_CONFIG_MAP_V4RELAY_DISABLED    "v4relay_disabled"
#define SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_ENABLED "v4relay_option82_enabled"
#define SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_POLICY "v4relay_option82_policy"
#define SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_VALIDATION_ENABLED "v4relay_option82_validation_enabled"
#define SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_REMOTE_ID "v4relay_option82_remote_id"
#define SYSTEM_DHCP_CONFIG_MAP_V4RELAY_HOP_COUNT_INCREMENT_DISABLED "v4relay_hop_count_increment_disabled"
#define DHCP_RELAY_OTHER_CONFIG_MAP_BOOTP_GATEWAY    "bootp_gateway"

/* Defuns for dhcp-relay */
extern struct cmd_element dhcp_relay_configuration_cmd;
extern struct cmd_element dhcp_relay_options_configuration_cmd;
extern struct cmd_element dhcp_relay_keep_option_configuration_cmd;
extern struct cmd_element dhcp_relay_drop_option_configuration_cmd;
extern struct cmd_element dhcp_relay_drop_validate_option_configuration_cmd;
extern struct cmd_element dhcp_relay_drop_remote_id_mac_option_configuration_cmd;
extern struct cmd_element dhcp_relay_drop_remote_id_ip_option_configuration_cmd;
extern struct cmd_element dhcp_relay_replace_option_configuration_cmd;
extern struct cmd_element dhcp_relay_replace_validate_option_configuration_cmd;
extern struct cmd_element dhcp_relay_replace_remote_id_mac_option_configuration_cmd;
extern struct cmd_element dhcp_relay_replace_remote_id_ip_option_configuration_cmd;
extern struct cmd_element dhcp_relay_validate_policy_option_configuration_cmd;
extern struct cmd_element dhcp_relay_validate_drop_option_configuration_cmd;
extern struct cmd_element dhcp_relay_validate_replace_option_configuration_cmd;
extern struct cmd_element no_dhcp_relay_configuration_cmd;
extern struct cmd_element no_dhcp_relay_option_configuration_cmd;
extern struct cmd_element show_dhcp_relay_configuration_cmd;
extern struct cmd_element ip_helper_address_configuration_cmd;
extern struct cmd_element no_ip_helper_address_configuration_cmd;
extern struct cmd_element show_ip_helper_address_configuration_cmd;


#endif /* dhcp_relay_vty.h */
