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

/* Defuns for dhcp-relay */
extern struct cmd_element dhcp_relay_configuration_cmd;
extern struct cmd_element no_dhcp_relay_configuration_cmd;
extern struct cmd_element show_dhcp_relay_configuration_cmd;
extern struct cmd_element ip_helper_address_configuration_cmd;
extern struct cmd_element no_ip_helper_address_configuration_cmd;
extern struct cmd_element show_ip_helper_address_configuration_cmd;

#endif /* dhcp_relay_vty.h */
