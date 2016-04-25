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
 * File: dhcpv6r_config.c
 *
 */

/*
 * Configuration maintainer for dhcpv6-relay feature.
 *
 */

#include "dhcpv6r.h"
#include "hash.h"


VLOG_DEFINE_THIS_MODULE(dhcpv6r_config);

/*
 * Function      : dhcpv6r_handle_row_delete
 * Responsiblity : Process delete event for one or more ports records from
 *                 DHCP-Relay table
 * Parameters    : idl - idl reference
 * Return        : none
 */
void dhcpv6r_handle_row_delete(struct ovsdb_idl *idl)
{
    const struct ovsrec_dhcp_relay *rec = NULL;
    DHCPV6R_INTERFACE_NODE_T *intf = NULL;
    struct shash_node *node = NULL, *next = NULL;
    int iter = 0;
    DHCPV6R_SERVER_T servers[MAX_SERVERS_PER_INTERFACE];
    bool found = false;

    /* Walk the server configuration hash table per "port" to
     * see if the corresponding record is deleted */
    SHASH_FOR_EACH_SAFE(node, next, &dhcpv6r_ctrl_cb_p->intfHashTable) {
        found = false;
        /* Iterate through dhcp relay table to find a match */
        OVSREC_DHCP_RELAY_FOR_EACH(rec, idl) {
            if ((NULL != rec->port) &&
                !strncmp(rec->port->name, node->name,
                         strlen(rec->port->name))) {
                found = true;
                break;
            }
        }

        if (false == found) {
            intf = (DHCPV6R_INTERFACE_NODE_T *)node->data;
            memset(servers, 0, sizeof(servers));
            /* Delete the interface entry from hash table */
            for (iter = 0; iter < intf->addrCount; iter++) {
                /*FIXME: dhcpv6r_remove_address(intf, servers[iter].ipv6_address,
                                      servers[iter].egressIfName); */
            }
        }
    }
    return;
}

/*
 * Function      : dhcpv6r_handle_config_change
 * Responsiblity : Handle a record change in DHCP-Relay table
 * Parameters    : rec - DHCP-Relay OVSDB table record
 *                 idl_seqno - idl change identifier
 * Return        : none
 */
void dhcpv6r_handle_config_change(
              const struct ovsrec_dhcp_relay *rec, uint32_t idl_seqno)
{
}
