/*
 * Copyright (C) 2016 Hewlett Packard Enterprise Development LP
 *
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
*/

/****************************************************************************
 * @ingroup cli
 *
 * @file vtysh_ovsdb_dhcp_relay_context.h
 * Source for registering client callback with dhcp-relay context.
 *
 ***************************************************************************/

#ifndef _VTYSH_OVSDB_DHCP_RELAY_CONTEXT_H
#define _VTYSH_OVSDB_DHCP_RELAY_CONTEXT_H

#include "dhcp_relay_vty.h"
#include "vtysh_ovsdb_dhcp_relay_context.h"


int vtysh_init_dhcp_relay_context_clients(void);
vtysh_ret_val vtysh_dhcp_relay_context_clientcallback (void *p_private);

#endif /* VTYSH_OVSDB_DHCP_RELAY_CONTEXT_H */
