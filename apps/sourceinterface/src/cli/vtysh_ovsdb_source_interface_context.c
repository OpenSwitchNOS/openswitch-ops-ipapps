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
#include "vtysh_ovsdb_source_interface_context.h"
#include "openswitch-dflt.h"

/*-----------------------------------------------------------------------------
| Function : vtysh_source_interface_context_clientcallback
| Responsibility : Client callback routine
| Parameters :
|     void *p_private: void type object typecast to required
| Return : Returns e_vtysh_ok
-----------------------------------------------------------------------------*/
vtysh_ret_val
vtysh_source_interface_context_clientcallback (void *p_private)
{
    PROTO_SOURCE *proto_source;
    memset (&proto_source, 0, sizeof (struct PROTO_SOURCE));
    struct in_addr addr;
    memset (&addr, 0, sizeof (struct in_addr));
    vtysh_ovsdb_cbmsg_ptr p_msg = (vtysh_ovsdb_cbmsg *)p_private;
    const struct ovsrec_vrf *vrf_row = NULL;
    char *source_interface_buff = NULL;

    vrf_row = ovsrec_vrf_first(p_msg->idl);
    if (!vrf_row) {
        return e_vtysh_ok;
    }

    proto_source = get_protocol_source_local(TFTP_PROTOCOL);
    source_interface_buff = proto_source->source;
    if (source_interface_buff != NULL )
    {
        /* Validate protocol server IP. */
        if (inet_pton (AF_INET, source_interface_buff, &addr) > 0) {
            vtysh_ovsdb_cli_print(p_msg, "%s %s",
                                  "ip source-interface tftp ",
                                  source_interface_buff);
        }
        else {
            vtysh_ovsdb_cli_print(p_msg, "%s %s",
                                  "ip source-interface tftp interface",
                                  source_interface_buff);
        }
    }
    proto_source = get_common_protocol_source_local();
    source_interface_buff = proto_source->source;
    if (source_interface_buff != NULL )
    {
        /* Validate protocol server IP. */
        if (inet_pton (AF_INET, source_interface_buff, &addr) > 0) {
            vtysh_ovsdb_cli_print(p_msg, "%s %s",
                                  "ip source-interface all ",
                                  source_interface_buff);
        }
        else {
            vtysh_ovsdb_cli_print(p_msg, "%s %s",
                                  "ip source-interface all interface",
                                  source_interface_buff);
        }
    }
    return e_vtysh_ok;

}
