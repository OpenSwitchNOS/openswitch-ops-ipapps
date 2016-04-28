/* DNS client functionality client callback resigitration
 * source files.
 *
 * Copyright (C) 2016 Hewlett Packard Enterprise Development LP.
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
 * File: vtysh_ovsdb_dnsclient_context.c
 *
 * Purpose: Source for registering client callback with DNS client
 *          context.
 */
/****************************************************************************
 * @ingroup ops-ipapps
 *
 * @file vtysh_ovsdb_dnsclient_context.c
 * Source for registering DNS client functionality client
 * callback with openvswitch table.
 *
 ***************************************************************************/
#if 0
/* Commenting out the current code as there is a change in priorities,
   will come back to this and continue the development */
#include "vtysh/vty.h"
#include "vtysh/vector.h"
#include "vswitch-idl.h"
#include "openswitch-idl.h"
#include "vtysh/vtysh_ovsdb_config.h"
#include "vtysh_ovsdb_dnsclient_context.h"

extern struct ovsdb_idl *idl;
char dns_client_context_client_name[] = "vtysh_dns_client_context_\
                                                        clientcallback";

/*-----------------------------------------------------------------------------
| Function : vtysh_dns_client_context_clientcallback
| Responsibility : client callback routine
| Parameters :
|     void *p_private: void type object typecast to required
| Return : On success, returns e_vtysh_ok. On failure, returns e_vtysh_error.
-----------------------------------------------------------------------------*/
vtysh_ret_val
vtysh_dns_client_context_clientcallback (void *p_private)
{
    vtysh_ovsdb_cbmsg_ptr p_msg = (vtysh_ovsdb_cbmsg *)p_private;
    const struct ovsrec_system *ovs_row = NULL;
    const struct ovsrec_dns_client *dns_client_row = NULL;
    struct smap smap;
    char *dns_client_status = NULL;
    int8_t i = 0;

    ovs_row = ovsrec_system_first (p_msg->idl);
    if (!ovs_row) {
        return e_vtysh_ok;
    }

    dns_client_status = (char *)smap_get(&ovs_row->other_config,
                              SYSTEM_OTHER_CONFIG_MAP_DNS_CLIENT_DISABLED);
    if (dns_client_status && !strcmp(dns_client_status, "true")) {
        vtysh_ovsdb_cli_print(p_msg, "%s", "no ip dns");
    }

    /* Display the dns client parameters */
    OVSREC_DNS_CLIENT_FOR_EACH (dns_client_row, idl)
    {
        smap_clone(&smap, &dns_client_row->other_config);
        dns_client_status = (char *)smap_get(&smap, DOMAIN_NAME);

        /* Fetch the DNS Client domain name */
        if (dns_client_status)
            vtysh_ovsdb_cli_print(p_msg, "ip dns domain-name %s",
                                         dns_client_status);

        /* Fetch the DNS Client domain list */
        if (dns_client_row->n_domain_list > 0)
        {
            for(i = 0; i < dns_client_row->n_domain_list; i++)
            {
                vtysh_ovsdb_cli_print(p_msg, "ip dns domain-list %s",
                                             dns_client_row->domain_list[i]);
            }
        }

        /* Fetch the DNS Client name servers */
        if (dns_client_row->n_name_servers > 0)
        {
            for(i = 0; i < dns_client_row->n_name_servers; i++)
            {
                vtysh_ovsdb_cli_print(p_msg, "ip dns server-address %s",
                                             dns_client_row->name_servers[i]);
            }
        }
    }

    smap_destroy(&smap);
    return e_vtysh_ok;
}
#endif
