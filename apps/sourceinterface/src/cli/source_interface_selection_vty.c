/* Source Interface Selection CLI commands
 *
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
 *
 * File: source_interface_selection_vty.c
 *
 * Purpose: To add source interface CLI commands.
 */

#include <sys/un.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <pwd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <lib/version.h>
#include "getopt.h"
#include "command.h"
#include <stdbool.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "memory.h"
#include "vtysh/vtysh.h"
#include "vswitch-idl.h"
#include "ovsdb-idl.h"
#include "openvswitch/vlog.h"
#include "openswitch-idl.h"
#include "prefix.h"
#include "vtysh/vtysh_ovsdb_if.h"
#include "vtysh/vtysh_ovsdb_config.h"
#include "source_interface_selection_vty.h"
#include "vrf-utils.h"
#include "vtysh_ovsdb_source_interface_context.h"

VLOG_DEFINE_THIS_MODULE (source_interface_selection_vty);

extern struct ovsdb_idl *idl;

/*-----------------------------------------------------------------------------
| Function         : get_configured_protocol_source_local
| Responsibility   : To get the source interface details
|                  for the configured protocol
| Parameters       :
|    proto_source  : Structure to store the source-interface and type
|        type      : To specify the type of the protocol
|        vrf_name  : Name of the vrf
| Return           : On success returns true,
|                    On failure returns false
-----------------------------------------------------------------------------*/
bool
get_configured_protocol_source_local(PROTO_SOURCE *proto_source,
                           SOURCE_INTERFACE_PROTOCOL type,
                           const char *vrf_name)
{
    const struct ovsrec_vrf *vrf_row = NULL;
    int i = 0;
    char *source_intf_key = NULL, *source_ip_key = NULL;
    memset(proto_source, 0, sizeof(PROTO_SOURCE));

    if (vrf_name && strncmp(vrf_name, DEFAULT_VRF_NAME, strlen(vrf_name)) != 0)
    {
        VLOG_ERR("VRF table did not have default VRF data.");
        return false;
    }

    vrf_row = get_default_vrf(idl);
    if (vrf_row == NULL)
    {
        vty_out(vty, "Error: Could not fetch "
                "default VRF data.%s", VTY_NEWLINE);
        VLOG_ERR("VRF table did not have default VRF data.");
        return false;
    }
    switch (type) {

    case TFTP_PROTOCOL:

        source_intf_key = VRF_SOURCE_INTERFACE_MAP_TFTP;
        source_ip_key = VRF_SOURCE_IP_MAP_TFTP;

        break;

    default :
        vty_out(vty, "Spefied type is unknown protocol.%s", VTY_NEWLINE);
        return false;
    }
    /* To display the source interface details for TFTP protocol */
    proto_source->source = (char *)smap_get(&vrf_row->source_ip,
                            source_ip_key);
    if (proto_source->source == NULL) {

        for (i = 0; i < vrf_row->n_source_interface; i++) {
            if(!strcmp(source_intf_key,
                       vrf_row->key_source_interface[i]))
            {
                proto_source->source =
                    vrf_row->value_source_interface[i]->name;
                proto_source->isIp = 0;
                return true;
            }
        }
    }
    else
    {
        proto_source->isIp = 1;
        return true;
    }
    return false;
}

/*-----------------------------------------------------------------------------
| Function         : get_common_protocol_source_local
| Responsibility   : To get the common source interface details
| Parameters       :
|    proto_source  : Structure to store the source-interface and type
|        vrf_name  : Name of the vrf
| Return           : On success returns true,
|                    On failure returns false
-----------------------------------------------------------------------------*/
bool
get_common_protocol_source_local(PROTO_SOURCE *proto_source,
                           const char *vrf_name)
{
    const struct ovsrec_vrf *vrf_row = NULL;
    int i = 0;
    memset(proto_source, 0, sizeof(PROTO_SOURCE));

    if (vrf_name && strncmp(vrf_name, DEFAULT_VRF_NAME, strlen(vrf_name)) != 0)
    {
        VLOG_ERR("VRF table did not have default VRF data.");
        return false;
    }

    vrf_row = get_default_vrf(idl);
    if (vrf_row == NULL)
    {
        vty_out(vty, "Error: Could not fetch "
                "default VRF data.%s", VTY_NEWLINE);
        VLOG_ERR("VRF table did not have default VRF data.");
        return false;
    }

    proto_source->source = (char *)smap_get(&vrf_row->source_ip,
                        VRF_SOURCE_IP_MAP_ALL);
    if (proto_source->source == NULL) {

        for (i = 0; i < vrf_row->n_source_interface; i++) {
            if(!strcmp(VRF_SOURCE_INTERFACE_MAP_ALL,
               vrf_row->key_source_interface[i]))
            {
                proto_source->source =
                        vrf_row->value_source_interface[i]->name;
                proto_source->isIp = 0;
                return true;
            }
        }
    }
    else
    {
        proto_source->isIp = 1;
        return true;
    }
    return false;
}

/*-----------------------------------------------------------------------------
| Function         : get_protocol_source_local
| Responsibility   : To get the source interface details
|                    for the specified protocol
| Parameters       :
|    proto_source  : Structure to store the source-interface and type
|        type      : To specify the type of the protocol
|        vrf_name  : Name of the vrf
| Return           : On success returns true,
|                    On failure returns false
-----------------------------------------------------------------------------*/
bool
get_protocol_source_local(PROTO_SOURCE *proto_source,
                           SOURCE_INTERFACE_PROTOCOL type,
                           const char *vrf_name)
{
    bool is_configured_protocol_source = false;

    is_configured_protocol_source =
            get_configured_protocol_source_local(proto_source,
                             type, DEFAULT_VRF_NAME);

    if (is_configured_protocol_source)
        return true;

    is_configured_protocol_source =
            get_common_protocol_source_local(proto_source, DEFAULT_VRF_NAME);

    if (is_configured_protocol_source)
        return true;
    return false;
}

/*----------------------------------------------------------------------------
| Name : show_source_interface_selection
| Responsibility : To display the source interface details
| Return : CMD_SUCCESS
-----------------------------------------------------------------------------*/
static int
show_source_interface_selection(SOURCE_INTERFACE_PROTOCOL type)
{
    PROTO_SOURCE *proto_source;
    bool is_proto_source = false;

    proto_source = (PROTO_SOURCE *) malloc(sizeof(PROTO_SOURCE));
    vty_out(vty, "%sSource-interface Configuration Information %s",
                  VTY_NEWLINE, VTY_NEWLINE);
    vty_out(vty, "---------------------------------------- %s", VTY_NEWLINE);
    vty_out(vty, "Protocol        Source Interface %s", VTY_NEWLINE);
    vty_out(vty, "--------        ---------------- %s", VTY_NEWLINE);

    if (type != ALL_PROTOCOL)
        is_proto_source = get_protocol_source_local(proto_source,
                                type, DEFAULT_VRF_NAME);
    switch (type) {

    case TFTP_PROTOCOL:
        /*
         * To display the source interface details for
         * all the specified protocols
         */
        if (is_proto_source)
        {
            vty_out(vty, " %-15s %-46s ", "tftp", proto_source->source);
        }
        else
        {
            vty_out(vty, " %-15s %-6s ", "tftp", "");
        }

        vty_out(vty, "%s", VTY_NEWLINE);
        break;

    case ALL_PROTOCOL:
        /*
         * To display the source interface details for
         * all the specified protocols
         */

        is_proto_source = get_protocol_source_local(proto_source,
                             TFTP_PROTOCOL, DEFAULT_VRF_NAME);
        if (is_proto_source)
        {
            vty_out(vty, " %-15s %-46s ", "tftp", proto_source->source);
        }
        else
        {
            vty_out(vty, " %-15s %-6s ", "tftp", "");
        }

        vty_out(vty, "%s", VTY_NEWLINE);
        break;
    }
    return CMD_SUCCESS;
}

/*-----------------------------------------------------------------------------
| Function         : source_interface_lookup
| Responsibility   : To lookup for the record with source interface
|                    in the VRF table.
| Parameters       :
|        vrf_name  : Name of the VRF
|        key       : Name of the protocol
| Return           : On success returns true,
|                    On failure returns false
-----------------------------------------------------------------------------*/
bool source_interface_lookup(const char *vrf_name, char *key)
{
    const struct ovsrec_vrf *vrf_row = NULL;
    int i = 0;

    bool source_interface_match = false;

    vrf_row = get_default_vrf(idl);
    if (vrf_row == NULL)
    {
        vty_out(vty, "Error: Could not fetch "
                     "default VRF data.%s", VTY_NEWLINE);
        VLOG_ERR("VRF table did not have default VRF data.");
        return false;
    }

    for (i = 0; i < vrf_row->n_source_interface; i++) {
        if(!strcmp(key,
           vrf_row->key_source_interface[i]))
        {
            source_interface_match = true;
            break;
        }
    }

    if (source_interface_match)
        return true;
    else
        return false;
}

/*----------------------------------------------------------------------------
| Name : update_protocol_source_ip
| Responsibility : To update the source interface IP details
|                  for the specified protocol
| Parameters     :
|      vrf_row   : This will point to the specific vrf row
|      source    : Stores the source-ip
|      key       : Name of the protocol
|      add       : If value is true, set source-ip
|                  else unset source-ip
-----------------------------------------------------------------------------*/
void
update_protocol_source_ip(const struct ovsrec_vrf *vrf_row, const char *source,
                       const char *key, bool add)
{
    char *buff = NULL;
    struct smap smap = SMAP_INITIALIZER(&smap);
    smap_clone(&smap, &vrf_row->source_ip);

    if (add)
        smap_replace(&smap, key, source);
    else
    {
        buff = (char *)smap_get(&vrf_row->source_ip, key);

        if (buff != NULL)
            smap_remove(&smap, key);
        else
        {
            if (!strcmp(key, VRF_SOURCE_IP_MAP_ALL))
            {
                vty_out(vty, "No source interface was configured for all the "
                        "defined protocols. %s", VTY_NEWLINE);
                return;
            }
            else if (!strcmp(key, VRF_SOURCE_IP_MAP_TFTP))
            {
                vty_out(vty, "No source interface was configured for the "
                        "TFTP protocol. %s", VTY_NEWLINE);
                return;
            }
        }
    }
    ovsrec_vrf_set_source_ip(vrf_row, &smap);
    smap_destroy(&smap);
    return;
}

/*----------------------------------------------------------------------------
| Name : update_protocol_source_interface
| Responsibility : To update the source interface details
|                  for the specified protocol
| Parameters     :
|      vrf_row   : This will point to the specific vrf row
|      port_row  : This will point to the specific port row
|      source    : Stores the source-interface
|      key       : Name of the protocol
|      add       : If value is true, set source-interface
|                  else unset source-interface
-----------------------------------------------------------------------------*/
void
update_protocol_source_interface(const struct ovsrec_vrf *vrf_row,
            const struct ovsrec_port *port_row, const char *source,
                       const char *key, bool add)
{
    int i = 0, n = 0;
    struct ovsrec_port **port_lists;
    char **key_lists;

    if (add)
    {
        key_lists = xmalloc((vrf_row->n_source_interface + 1) *
                        sizeof(char *));
        port_lists = xmalloc((vrf_row->n_source_interface + 1) *
                        sizeof( * vrf_row->value_source_interface ));

        for (i = 0; i < vrf_row->n_source_interface; i++) {
            key_lists[i]  = vrf_row->key_source_interface[i];
            port_lists[i] = vrf_row->value_source_interface[i];
        }

        key_lists[vrf_row->n_source_interface] = (char *)key;
        port_lists[vrf_row->n_source_interface] =
            CONST_CAST(struct ovsrec_port *, port_row);

        ovsrec_vrf_set_source_interface(vrf_row, key_lists, port_lists,
                (vrf_row->n_source_interface + 1));
        free(key_lists);
        free(port_lists);
    }

    else
    {
        /* Checking if the entry is the last entry.
        * If true clear it , else continue */
        if (vrf_row->n_source_interface ==  1) {
            if(!strcmp(vrf_row->key_source_interface[0],key))
                ovsrec_vrf_set_source_interface(vrf_row, NULL, NULL, 0);
        }
        else {
            key_lists = xmalloc((vrf_row->n_source_interface - 1) *
                            sizeof(char *));
            port_lists = xmalloc((vrf_row->n_source_interface - 1) *
                            sizeof( * vrf_row->value_source_interface ));
            for (i = n = 0; i < vrf_row->n_source_interface; i++) {

                if (strcmp(vrf_row->key_source_interface[i], key) != 0)
                {
                    key_lists[n] = vrf_row->key_source_interface[i];
                    port_lists[n] = vrf_row->value_source_interface[i];
                    n++;
                }
            }

            ovsrec_vrf_set_source_interface(vrf_row, key_lists, port_lists,
                    (vrf_row->n_source_interface - 1));
            free(key_lists);
            free(port_lists);
        }
    }
    return;
}

/*-----------------------------------------------------------------------------
| Function         : isIpConfigured
| Responsibility   : To check specified source IP addres configuration exist
|                    on any interface.
| Parameters       :
|        *source   : Stores the source-ip
| Return           : On success returns true,
|                    On failure returns false
-----------------------------------------------------------------------------*/
bool isIpConfigured(const char *source)
{
    const struct ovsrec_port *port_row = NULL;
    size_t maskPosition, iter;
    char *mask = NULL;

    OVSREC_PORT_FOR_EACH (port_row, idl)
    {
        /* To check IP addres configuration on the interface. */
        if (port_row->ip4_address)
        {
            mask = strchr(port_row->ip4_address, '/');
            maskPosition = mask - (port_row->ip4_address);
            if (!strncmp(source, port_row->ip4_address,
                maskPosition))
            {
                return true;
            }
        }
        else
        {
            /* To check secondary IP address configuration
               on the interface. */
            for (iter = 0; iter < port_row->n_ip4_address_secondary; iter++)
            {
                mask = strchr(port_row->ip4_address_secondary[iter], '/');
                maskPosition = mask - (port_row->ip4_address_secondary[iter]);
                if (!strncmp(source,
                    port_row->ip4_address_secondary[iter], maskPosition))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

/*-----------------------------------------------------------------------------
| Function         : is_PortMatch
| Responsibility   : To check specified source interface is configured with
|                    any IP ADDRESS
| Parameters       :
|        *source   : Stores the source-interface
| Return           : On success returns the port row,
|                    On failure returns NULL
-----------------------------------------------------------------------------*/
const struct
ovsrec_port* getPort(const char *source)
{
    const struct ovsrec_port *port_row = NULL;
    OVSREC_PORT_FOR_EACH (port_row, idl)
    {
        if (strcmp(port_row->name, source) != 0)
            continue;

        /* To check IP address configuration on the interface. */
        if (port_row->ip4_address)
        {
             return port_row;
        }
        else if (port_row->n_ip4_address_secondary > 1)
        {
            return port_row;
        }
    }
    return NULL;
}

/*-----------------------------------------------------------------------------
| Name : set_source_ip
| Responsibility : To set source IP address to TFTP server and all
|                  the specified protocols in ovsdb VRF
|                  table source_ip columns
| Parameters : const char* source : Stores the source-interface
|              type : To specify the type of the protocol
| Return : CMD_SUCCESS for success , CMD_OVSDB_FAILURE for failure
-----------------------------------------------------------------------------*/
static int
set_source_ip(const char *source,
              SOURCE_INTERFACE_PROTOCOL type)
{
    const struct ovsrec_vrf *vrf_row = NULL;
    struct smap smap = SMAP_INITIALIZER(&smap);
    struct ovsdb_idl_txn *status_txn = NULL;
    enum ovsdb_idl_txn_status status;
    bool isAddrMatch = false;
    bool source_interface_match = false;
    char *source_intf_key = NULL, *source_ip_key = NULL;

    status_txn = cli_do_config_start();

    if (status_txn == NULL) {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    /* lookup for the Default VRF record in vrf table */
    vrf_row = get_default_vrf(idl);
    if (vrf_row == NULL)
    {
        vty_out(vty, "Error: Could not fetch "
                     "default VRF data.%s", VTY_NEWLINE);
        VLOG_ERR("VRF table did not have default VRF data.");
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    isAddrMatch = isIpConfigured(source);

    if (!isAddrMatch)
    {
        /* Unconfigured IP address on any interface. */

        vty_out(vty, "Specified IP address is not configured on " \
                "any interface.%s", VTY_NEWLINE);
        cli_do_config_abort(status_txn);
        return CMD_SUCCESS;
    }

    switch (type) {
    case ALL_PROTOCOL:
        /*
         * To set the source interface to all the specified
         * protocols in ovsdb vrf table source_ip/source_interface column.
         */
        source_intf_key = VRF_SOURCE_INTERFACE_MAP_ALL;
        source_ip_key = VRF_SOURCE_IP_MAP_ALL;
        break;

    case TFTP_PROTOCOL:
        /*
        * To set the source interface to TFTP server
        * in ovsdb vrf table source_ip/source_interface column.
        */
        source_intf_key = VRF_SOURCE_INTERFACE_MAP_TFTP;
        source_ip_key = VRF_SOURCE_IP_MAP_TFTP;
        break;

    default :
        vty_out(vty, "Spefied type is unknown protocol.%s", VTY_NEWLINE);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    source_interface_match = source_interface_lookup(DEFAULT_VRF_NAME,
                                    source_intf_key);
    if (source_interface_match)
        update_protocol_source_interface(vrf_row, NULL, NULL,
                    source_intf_key, false);

    update_protocol_source_ip(vrf_row, source, source_ip_key, true);

    status = cli_do_config_finish(status_txn);

    if (status == TXN_SUCCESS || status == TXN_UNCHANGED) {
        return CMD_SUCCESS;
    }
    else {
        return CMD_OVSDB_FAILURE;
    }

}

/*-----------------------------------------------------------------------------
| Name : set_source_interface
| Responsibility : To set the source interface to TFTP server and all
|                  the specified protocols in ovsdb VRF
|                  table source_interface
| Parameters : const char* source : Stores the source-interface
|              type : To specify the type of the protocol
| Return : CMD_SUCCESS for success , CMD_OVSDB_FAILURE for failure
-----------------------------------------------------------------------------*/
static int
set_source_interface(const char *source,
                     SOURCE_INTERFACE_PROTOCOL type)
{
    const struct ovsrec_vrf *vrf_row = NULL;
    const struct ovsrec_port *port_row = NULL;
    struct smap smap = SMAP_INITIALIZER(&smap);
    struct ovsdb_idl_txn *status_txn = NULL;
    enum ovsdb_idl_txn_status status;
    char *buff = NULL;
    char *source_ip_key = NULL, *source_intf_key = NULL;

    status_txn = cli_do_config_start();

    if (status_txn == NULL) {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    /* lookup for the Default VRF record in vrf table */
    vrf_row = get_default_vrf(idl);
    if (vrf_row == NULL)
    {
        vty_out(vty, "Error: Could not fetch "
                     "default VRF data.%s", VTY_NEWLINE);
        VLOG_ERR("VRF table did not have default VRF data.");
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    port_row = getPort(source);

    if (port_row == NULL)
    {
        /* Unconfigured interface. */

        vty_out(vty, "IP address is not yet configured on " \
                "the specified interface.%s", VTY_NEWLINE);
        cli_do_config_abort(status_txn);
        return CMD_SUCCESS;
    }

    switch (type) {
    case ALL_PROTOCOL:
        /*
         * To set the source interface to all the specified
         * protocols in ovsdb vrf table source_ip/source_interface column.
         */
        source_ip_key = VRF_SOURCE_IP_MAP_ALL;
        source_intf_key = VRF_SOURCE_INTERFACE_MAP_ALL;
        break;

    case TFTP_PROTOCOL:
        /*
        * To set the source interface to TFTP server
        * in ovsdb vrf table source_ip/source_interface column.
        */
        source_ip_key = VRF_SOURCE_IP_MAP_TFTP;
        source_intf_key = VRF_SOURCE_INTERFACE_MAP_TFTP;
        break;

    default :
        cli_do_config_abort(status_txn);
        vty_out(vty, "Spefied type is unknown protocol.%s", VTY_NEWLINE);
        return CMD_OVSDB_FAILURE;
    }

    buff = (char *)smap_get(&vrf_row->source_ip, source_ip_key);
    if (buff != NULL)
    {
        update_protocol_source_ip(vrf_row, source,
                               source_ip_key, false);
    }
    update_protocol_source_interface(vrf_row, port_row, source,
                    source_intf_key, true);
    status = cli_do_config_finish(status_txn);

    if (status == TXN_SUCCESS || status == TXN_UNCHANGED) {
        return CMD_SUCCESS;
    }
    else {
        return CMD_OVSDB_FAILURE;
    }

}

/*-----------------------------------------------------------------------------
| Name : reset_source_interface
| Responsibility : To reset the source interface to TFTP server and all
|                  the specified protocols in ovsdb VRF
|                  table source_interface and source_ip columns
| Parameters : const char* source : Stores the source-interface
|              type : To specify the type of the protocol
| Return : CMD_SUCCESS for success , CMD_OVSDB_FAILURE for failure
-----------------------------------------------------------------------------*/
static int
reset_source_interface(const char *source,
                       SOURCE_INTERFACE_PROTOCOL type)
{
    const struct ovsrec_vrf *vrf_row = NULL;
    struct smap smap = SMAP_INITIALIZER(&smap);
    struct ovsdb_idl_txn *status_txn = NULL;
    enum ovsdb_idl_txn_status status;
    char *buff = NULL;
    bool source_interface_match = false;
    char *source_ip_key = NULL, *source_intf_key = NULL;
    char *error_message = NULL;

    status_txn = cli_do_config_start();

    if (status_txn == NULL) {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    /* lookup for the Default VRF record in vrf table */
    vrf_row = get_default_vrf(idl);
    if (vrf_row == NULL)
    {
        vty_out(vty, "Error: Could not fetch "
                     "default VRF data.%s", VTY_NEWLINE);
        VLOG_ERR("VRF table did not have default VRF data.");
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    switch (type) {
    case ALL_PROTOCOL:
        source_ip_key = VRF_SOURCE_IP_MAP_ALL;
        source_intf_key = VRF_SOURCE_INTERFACE_MAP_ALL;
        error_message = "Common source interface is not configured.";
        break;

    case TFTP_PROTOCOL:
        source_ip_key = VRF_SOURCE_IP_MAP_TFTP;
        source_intf_key = VRF_SOURCE_INTERFACE_MAP_TFTP;
        error_message = "No source interface was configured for the TACACS protocol.";
        break;

    default :
        cli_do_config_abort(status_txn);
        vty_out(vty, "Spefied type is unknown protocol.%s", VTY_NEWLINE);
        return CMD_OVSDB_FAILURE;
    }

    /*
     * To set the source interface to all the specified
     * protocols in ovsdb vrf table source_ip/source_interface column.
     */

    buff = (char *)smap_get(&vrf_row->source_ip, source_ip_key);
    if (buff != NULL)
    {
        update_protocol_source_ip(vrf_row, NULL,
                                  source_ip_key, false);
    }
    else
    {
        source_interface_match = source_interface_lookup(DEFAULT_VRF_NAME,
                                        source_intf_key);
        if (!source_interface_match)
            vty_out(vty, "%s%s", error_message, VTY_NEWLINE);
        else
        {
            update_protocol_source_interface(vrf_row, NULL, NULL,
                source_intf_key, false);
        }
    }
    status = cli_do_config_finish(status_txn);

    if (status == TXN_SUCCESS || status == TXN_UNCHANGED) {
        return CMD_SUCCESS;
    }
    else {
        return CMD_OVSDB_FAILURE;
    }

}

/*-----------------------------------------------------------------------------
| Defun for source IP interface
| Responsibility : Configure source IP interface
-----------------------------------------------------------------------------*/
DEFUN(ip_source_interface,
      ip_source_interface_cmd,
      "ip source-interface (tftp | all) interface IFNAME ",
      IP_STR
      SOURCE_STRING
      TFTP_STRING
      ALL_STRING
      INTERFACE_STR
      IFNAME_STR)
{
    if (strcmp(TFTP, (char*)argv[0]) == 0) {
        return set_source_interface(argv[1], TFTP_PROTOCOL);
    } else if (strcmp(ALL, (char*)argv[0]) == 0){
        return set_source_interface(argv[1], ALL_PROTOCOL);
    }
    return CMD_SUCCESS;
}

/*-----------------------------------------------------------------------------
| Defun for source IP address
| Responsibility : Configure source IP address
-----------------------------------------------------------------------------*/
DEFUN(ip_source_address,
      ip_source_address_cmd,
      "ip source-interface (tftp | all) A.B.C.D",
      IP_STR
      SOURCE_STRING
      TFTP_STRING
      ALL_STRING
      ADDRESS_STRING)
{
    struct in_addr addr;
    memset (&addr, 0, sizeof (struct in_addr));
    inet_pton (AF_INET, (char*)argv[1], &addr);
    if (!IS_VALID_IPV4(htonl(addr.s_addr)))
    {
        vty_out(vty,"Broadcast, multicast and "
                "loopback addresses are not allowed.%s", VTY_NEWLINE);
        return CMD_SUCCESS;
    }

    if (strcmp(TFTP, (char*)argv[0]) == 0) {
        return set_source_ip(argv[1], TFTP_PROTOCOL);
    } else if (strcmp(ALL, (char*)argv[0]) == 0) {
        return set_source_ip(argv[1], ALL_PROTOCOL);
    }
    return CMD_SUCCESS;
}

/*-----------------------------------------------------------------------------
| Defun for no source interface
| Responsibility : Unset source interface
-----------------------------------------------------------------------------*/
DEFUN(no_ip_source_interface,
      no_ip_source_interface_cmd,
      "no ip source-interface (tftp | all) ",
      NO_STR
      IP_STR
      SOURCE_STRING
      TFTP_STRING
      ALL_STRING)
{
    if (strcmp(TFTP, (char*)argv[0]) == 0) {
        return reset_source_interface(NULL, TFTP_PROTOCOL);
    } else if (strcmp(ALL, (char*)argv[0]) == 0) {
        return reset_source_interface(NULL, ALL_PROTOCOL);
    }
    return CMD_SUCCESS;
}
/*-----------------------------------------------------------------------------
| Defun for show source interface
| Responsibility :Displays the source interface configuration
-----------------------------------------------------------------------------*/
DEFUN(show_source_interface,
      show_source_interface_cmd,
      "show ip source-interface {tftp} ",
      SHOW_STR
      IP_STR
      SOURCE_STRING
      TFTP_STRING)
{
    if (argv[0])
    {
        if (strcmp(TFTP, (char*)argv[0]) == 0)
        {
            return show_source_interface_selection(TFTP_PROTOCOL);
        }
    }
    return show_source_interface_selection(ALL_PROTOCOL);
}

/*-----------------------------------------------------------------------------
| Function       : sftp_ovsdb_init
| Responsibility : Initialise source interface details.
| Parameters     :
|      idl       : Pointer to idl structure
-----------------------------------------------------------------------------*/
static void
source_interface_ovsdb_init(void)
{
    ovsdb_idl_add_table(idl, &ovsrec_table_vrf);
    ovsdb_idl_add_column(idl, &ovsrec_vrf_col_source_ip);
    ovsdb_idl_add_column(idl, &ovsrec_vrf_col_source_interface);

    return;
}

/* Install source interface related vty commands */
void
cli_pre_init(void)
{
    vtysh_ret_val source_interface_retval = e_vtysh_error;

    source_interface_ovsdb_init();

    source_interface_retval = install_show_run_config_context(
                            e_vtysh_source_interface_context,
                            &vtysh_source_interface_context_clientcallback,
                            NULL, NULL);
    if(e_vtysh_ok != source_interface_retval)
    {
       vtysh_ovsdb_config_logmsg(VTYSH_OVSDB_CONFIG_ERR,
                           "source_interface context unable "\
                           "to add config callback");
        assert(0);
    }
    return;
}

/* Install source interface related vty commands */
void
cli_post_init(void)
{
    install_element (CONFIG_NODE, &ip_source_interface_cmd);
    install_element (CONFIG_NODE, &ip_source_address_cmd);
    install_element (ENABLE_NODE, &show_source_interface_cmd);
    install_element (CONFIG_NODE, &no_ip_source_interface_cmd);

    return;
}