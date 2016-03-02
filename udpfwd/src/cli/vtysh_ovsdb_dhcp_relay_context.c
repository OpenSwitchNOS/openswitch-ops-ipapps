/* dhcp-relay functionality client callback resigitration
 * source files.
 *
 * Copyright (C) 2016 Hewlett Packard Enterprise Development LP
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 * File: vtysh_ovsdb_dhcp_relay_context.c
 *
 * Purpose: Source for registering client callback with dhcp-relay
 *          context.
 */
/****************************************************************************
 * @ingroup cli
 *
 * @file vtysh_ovsdb_dhcp_relay_context.c
 * Source for registering dhcp-relay functionality client callback with
 * openvswitch table.
 *
 ***************************************************************************/

#include "vtysh/zebra.h"
#include "vtysh/vty.h"
#include "vtysh/vector.h"
#include "vswitch-idl.h"
#include "openswitch-idl.h"
#include "vtysh/command.h"
#include "vtysh/vtysh_ovsdb_if.h"
#include "vtysh/vtysh_ovsdb_config.h"
#include "vtysh_ovsdb_dhcp_relay_context.h"

/*-----------------------------------------------------------------------------
| Function : vtysh_dhcp_relay_context_clientcallback
| Responsibility : client callback routine for dhcp-relay
| Parameters :
|     void *p_private: void type object typecast to required
| Return : On success, returns e_vtysh_ok. On failure, returns e_vtysh_error.
-----------------------------------------------------------------------------*/
vtysh_ret_val
vtysh_dhcp_relay_context_clientcallback (void *p_private)
{
    vtysh_ovsdb_cbmsg_ptr p_msg = (vtysh_ovsdb_cbmsg *)p_private;
    const struct ovsrec_system *ovs_row = NULL;
    char *dhcp_relay_status = NULL;

    ovs_row = ovsrec_system_first (p_msg->idl);
    if (!ovs_row) {
        return e_vtysh_ok;
    }

    dhcp_relay_status = (char *)smap_get(&ovs_row->other_config,
                                  SYSTEM_OTHER_CONFIG_MAP_DHCP_RELAY_DISABLED);
    if (dhcp_relay_status && !strcmp(dhcp_relay_status, "true")) {
        vtysh_ovsdb_cli_print(p_msg, "%s", "no dhcp-relay");
    }

    return e_vtysh_ok;
}