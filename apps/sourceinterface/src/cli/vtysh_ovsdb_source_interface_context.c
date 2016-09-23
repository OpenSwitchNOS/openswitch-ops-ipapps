/*
 * Copyright (C) 2016 Hewlett Packard Enterprise Development LP
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
*/
/****************************************************************************
 * @ingroup cli
 *
 * @file vtysh_ovsdb_source_interface_context.c
 * Source for registering source interface client
 * callback with openvswitch table.
 *
 ***************************************************************************/

#include "vty.h"
#include <stdlib.h>
#include <vector.h>
#include "vswitch-idl.h"
#include "openswitch-idl.h"
#include "vtysh_ovsdb_if.h"
#include "vtysh_ovsdb_config.h"
#include "source_interface_selection_vty.h"
#include "vtysh_ovsdb_source_interface_context.h"
#include "openswitch-dflt.h"
#include "vrf-utils.h"

/*-----------------------------------------------------------------------------
| Function       : display_source_interface_config
| Responsibility : To display the source-interface running-config
| Parameters     :
|     type       : To specify the type of the protocol
|     p_msg      : Used for idl operations
-----------------------------------------------------------------------------*/
static void
display_source_interface_config(SOURCE_INTERFACE_PROTOCOL type,
                                vtysh_ovsdb_cbmsg_ptr p_msg)
{
    protoSource proto_source;
    char *source_interface_buff = NULL;
    bool is_proto_source = false;

    is_proto_source = get_configured_protocol_source_local(type,
                            DEFAULT_VRF_NAME, &proto_source);
    if (is_proto_source)
    {
        source_interface_buff = proto_source.source;

        /* Validate source-interface config type*/

        if (proto_source.config_type == SOURCE_IP) {
            vtysh_ovsdb_cli_print(p_msg, "%s %s %s",
                                  "ip source-interface",
                                  protocol_keys[type],
                                  source_interface_buff);
        }
        else if (proto_source.config_type == SOURCE_INTERFACE) {
            vtysh_ovsdb_cli_print(p_msg, "%s %s %s %s",
                                  "ip source-interface",
                                  protocol_keys[type],
                                  "interface",
                                  source_interface_buff);
        }
    }
    return;
}
/*-----------------------------------------------------------------------------
| Function          : vtysh_source_interface_context_clientcallback
| Responsibility    : Client callback routine
| Parameters        :
|    void *p_private: void type object typecast to required
| Return            : Returns e_vtysh_ok
-----------------------------------------------------------------------------*/
vtysh_ret_val
vtysh_source_interface_context_clientcallback (void *p_private)
{
    vtysh_ovsdb_cbmsg_ptr p_msg = (vtysh_ovsdb_cbmsg *)p_private;
    const struct ovsrec_vrf *vrf_row = NULL;

    vrf_row = ovsrec_vrf_first(p_msg->idl);
    if (!vrf_row) {
        return e_vtysh_ok;
    }

    display_source_interface_config(TFTP_PROTOCOL, p_msg);
    display_source_interface_config(TACACS_PROTOCOL, p_msg);
    display_source_interface_config(RADIUS_PROTOCOL, p_msg);
    display_source_interface_config(ALL_PROTOCOL, p_msg);

    return e_vtysh_ok;
}
