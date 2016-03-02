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
 * @ingroup cli/vtysh
 *
 * @file dhcp_relay_vty.h
 * purpose: To add declarations required for
 * dhcp_relay_vty.c
 ***************************************************************************/

#ifndef _DHCP_RELAY_VTY_H
#define _DHCP_RELAY_VTY_H

#define DHCP_RELAY_STRING \
"Configure DHCP-Relay\n"
#define HELPER_ADDRESS_STRING \
"Configure the helper address for dhcp-relay\n"
#define SHOW_DHCP_RELAY_STRING \
"Show DHCP-Relay configuration\n"
#define SHOW_HELPER_ADDRESS_STRING \
"Show the helper address for dhcp-relay configuration\n"
#define SUBIFNAME_STR \
"Subinterface name as physical_interface.subinterface name\n"
#define IP_ADDRESS_LENGTH 18

void cli_pri_init(void);
void cli_post_init(void);

#endif /*_DHCP_RELAY_VTY_H*/
