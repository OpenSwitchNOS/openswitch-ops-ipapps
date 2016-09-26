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
| Responsibility   : Read source-interface configuration of a protocol
|                    on a specific VRF.
| Parameters       :
|        type      : Specify the type of the protocol
|        vrf_name  : VRF name to use to locate the record
|    proto_source  : Structure to store the source-interface and type
| Return           : On success returns true,
|                    On failure returns false
-----------------------------------------------------------------------------*/
bool
get_configured_protocol_source_local(SOURCE_INTERFACE_PROTOCOL type,
                               const char *vrf_name,
                               protoSource *proto_source)
{
    const struct ovsrec_vrf *vrf_row = NULL;
    int i = 0;

    if (proto_source == NULL)
    {
        return false;
    }

    /* Set default values. */
    proto_source->source = NULL;
    proto_source->config_type = SOURCE_MAX;

    if (type >= PROTOCOL_MAX)
    {
        return false;
    }

    vrf_row = vrf_lookup(idl, vrf_name);
    if (vrf_row == NULL)
    {
        VLOG_ERR("Unable to find %s entry in the VRF table.", vrf_name );
        return false;
    }

    /* source-ip configuration is given priority over source-interface */
    proto_source->source = (char *)smap_get(&vrf_row->source_ip,
                            protocol_keys[type]);
    if (proto_source->source == NULL) {

        for (i = 0; i < vrf_row->n_source_interface; i++) {
            if(!strncmp(protocol_keys[type], vrf_row->key_source_interface[i],
                        strlen(protocol_keys[type])))
            {
                proto_source->source =
                    vrf_row->value_source_interface[i]->name;
                proto_source->config_type = SOURCE_INTERFACE;
                return true;
            }
        }
    }
    else
    {
        proto_source->config_type = SOURCE_IP;
        return true;
    }
    return false;
}

/*-----------------------------------------------------------------------------
| Function         : get_protocol_source_local
| Responsibility   : Reads source-interface configuration to be used for a
|                    protocol on a given VRF. Following sequence is used in
|                    deciding the configuration to be returned (from a specific VRF)
|                    1. Protocol source-interface configuration
|                    2. NULL configuration
| Parameters       :
|        type      : To specify the type of the protocol
|        vrf_name  : VRF name to use to locate the record
|    proto_source  : Structure to store the source-interface and type
| Return           : On success returns true,
|                    On failure returns false
-----------------------------------------------------------------------------*/
bool
get_protocol_source_local(SOURCE_INTERFACE_PROTOCOL type,
                    const char *vrf_name,
                    protoSource *proto_source)
{
    if (get_configured_protocol_source_local(type, DEFAULT_VRF_NAME,
                                       proto_source))
    {
        return true;
    }
    if ((type != ALL_PROTOCOL) && (((get_configured_protocol_source_local(
        ALL_PROTOCOL, DEFAULT_VRF_NAME, proto_source)))))
    {
        return true;
    }
    return false;
}

/*----------------------------------------------------------------------------
| Name : print_proto_source
| Responsibility       : Print the source interface details
| Parameters           :
|        type          : To specify the type of the protocol
|        vrf_name      : VRF name to use to locate the record
| common_proto_source  : Structure to store the source-interface and type
-----------------------------------------------------------------------------*/

void print_proto_source(SOURCE_INTERFACE_PROTOCOL type, const char *vrf_name,
                        protoSource *common_proto_source)
{
    protoSource proto_source;

    if (get_configured_protocol_source_local(type, vrf_name,
                                  &proto_source))
    {
        vty_out(vty, " %-15s %-46s ", protocol_keys[type],
                    proto_source.source);
    }
    else if (common_proto_source->source != NULL)
    {
        vty_out(vty, " %-15s %-46s ", protocol_keys[type],
                    common_proto_source->source);
    }
    else
    {
        vty_out(vty, " %-15s %-6s ", protocol_keys[type], "");
    }
    vty_out(vty, "%s", VTY_NEWLINE);
    return;
}

/*----------------------------------------------------------------------------
| Name : show_source_interface_selection
| Responsibility : To display the source interface details
| Return : CMD_SUCCESS
-----------------------------------------------------------------------------*/
static int
show_source_interface_selection(SOURCE_INTERFACE_PROTOCOL type)
{
    protoSource common_proto_source;

    vty_out(vty, "%sSource-interface Configuration Information %s",
                  VTY_NEWLINE, VTY_NEWLINE);
    vty_out(vty, "---------------------------------------- %s", VTY_NEWLINE);
    vty_out(vty, "Protocol        Source Interface %s", VTY_NEWLINE);
    vty_out(vty, "--------        ---------------- %s", VTY_NEWLINE);

    get_configured_protocol_source_local(ALL_PROTOCOL,
                                DEFAULT_VRF_NAME, &common_proto_source);
    if (type != ALL_PROTOCOL)
    {
        print_proto_source(type, DEFAULT_VRF_NAME,
                           &common_proto_source);
    }
    else
    {
        /* To display the source interface details for all the protocols */
        print_proto_source(TFTP_PROTOCOL, DEFAULT_VRF_NAME,
                       &common_proto_source);
        print_proto_source(TACACS_PROTOCOL, DEFAULT_VRF_NAME,
                       &common_proto_source);
        print_proto_source(RADIUS_PROTOCOL, DEFAULT_VRF_NAME,
                       &common_proto_source);
    }

    return CMD_SUCCESS;
}

/*-----------------------------------------------------------------------------
| Function         : source_interface_lookup
| Responsibility   : Lookup for the record with source interface
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
        if(!strncmp(key, vrf_row->key_source_interface[i], strlen(key)))
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
| Responsibility : Update the source interface IP details
|                  for the specified protocol
| Parameters     :
|      vrf_row   : Points to the specific vrf row
|      source    : Stores the source-ip
|      key       : Name of the protocol
|      add       : If value is true, set source-ip
|                  else unset source-ip
-----------------------------------------------------------------------------*/
void
update_protocol_source_ip(const struct ovsrec_vrf *vrf_row, const char *source,
                       const char *key, bool add)
{
    struct smap smap = SMAP_INITIALIZER(&smap);
    smap_clone(&smap, &vrf_row->source_ip);

    if (add)
        smap_replace(&smap, key, source);
    else
    {
        smap_remove(&smap, key);
    }
    ovsrec_vrf_set_source_ip(vrf_row, &smap);
    smap_destroy(&smap);
    return;
}

/*----------------------------------------------------------------------------
| Name : update_protocol_source_interface
| Responsibility : Updates the source interface details
|                  for the specified protocol
| Parameters     :
|      vrf_row   : Points to the specific vrf row
|      port_row  : Points to the specific port row
|      key       : Name of the protocol
|      add       : If value is true, set source-interface
|                  else unset source-interface
-----------------------------------------------------------------------------*/
void
update_protocol_source_interface(const struct ovsrec_vrf *vrf_row,
            const struct ovsrec_port *port_row, const char *key, bool add)
{
    int i = 0, n = 0;
    struct ovsrec_port **port_lists = NULL;
    char **key_lists = NULL;
    size_t n_sources= 0;

    if (add)
    {
        n_sources = vrf_row->n_source_interface + 1;
        key_lists = xmalloc(n_sources *
                        sizeof(char *));
        port_lists = xmalloc(n_sources *
                        sizeof( * vrf_row->value_source_interface ));

        for (i = 0; i < vrf_row->n_source_interface; i++) {
            key_lists[i]  = vrf_row->key_source_interface[i];
            port_lists[i] = vrf_row->value_source_interface[i];
        }

        key_lists[vrf_row->n_source_interface] = (char *)key;
        port_lists[vrf_row->n_source_interface] =
            CONST_CAST(struct ovsrec_port *, port_row);
    }

    else
    {
        /*
         * Checking if the entry is the last entry.
         * If true clear it , else continue.
         */
        if (vrf_row->n_source_interface ==  1) {
            if(!strncmp(key, vrf_row->key_source_interface[0], strlen(key)))
                n_sources= 0;
        }
        else {
            n_sources = vrf_row->n_source_interface - 1;
            key_lists = xmalloc(n_sources *
                            sizeof(char *));
            port_lists = xmalloc(n_sources *
                            sizeof( * vrf_row->value_source_interface ));
            for (i = n = 0; i < vrf_row->n_source_interface; i++) {

                if (strncmp(key, vrf_row->key_source_interface[i],
                    strlen(key)) != 0)
                {
                    key_lists[n] = vrf_row->key_source_interface[i];
                    port_lists[n] = vrf_row->value_source_interface[i];
                    n++;
                }
            }
        }
    }

    ovsrec_vrf_set_source_interface(vrf_row, key_lists,
                                    port_lists, n_sources);
    if (key_lists != NULL)
        free(key_lists);
    if (port_lists!= NULL)
        free(port_lists);
    return;
}

/*-----------------------------------------------------------------------------
| Function         : isIpConfigured
| Responsibility   : checks the IP addres configuration exist
|                    on any interface
| Parameters       :
|        *source   : Reference to IP address
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
| Function         : getPort
| Responsibility   : Reads the Port if any IP address is configured
| Parameters       :
|    *if_name      : Name of interface
| Return           : On success returns the port row,
|                    On failure returns NULL
-----------------------------------------------------------------------------*/
const struct
ovsrec_port* getPort(const char *if_name)
{
    const struct ovsrec_port *port_row = NULL;
    OVSREC_PORT_FOR_EACH (port_row, idl)
    {
        if (strncmp(if_name, port_row->name, strlen(if_name)) != 0)
            continue;

        /* To check IP address configuration on the interface. */
        if ((port_row->ip4_address) ||
            (port_row->n_ip4_address_secondary >= 1))
            return port_row;
    }
    return NULL;
}

/*-----------------------------------------------------------------------------
| Name : set_source_ip
| Responsibility : sets source IP address to TFTP server and all
|                  the specified protocols in ovsdb VRF
|                  table source_ip columns
| Parameters :
|         source : source IP address
|         type   : To specify the type of the protocol
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

    source_interface_match = source_interface_lookup(DEFAULT_VRF_NAME,
                                    protocol_keys[type]);

    /* Removing the source-interface configuration. */
    if (source_interface_match)
        update_protocol_source_interface(vrf_row, NULL,
                     protocol_keys[type], false);

    /* Adding the source IP address configuration. */
    update_protocol_source_ip(vrf_row, source, protocol_keys[type], true);

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
| Responsibility : Sets the source interface to TFTP server and all
|                  the specified protocols in ovsdb VRF
|                  table source_interface
| Parameters : const char* source : source-interface
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

        vty_out(vty, "No IP address is configured on " \
                "the specified interface.%s", VTY_NEWLINE);
        cli_do_config_abort(status_txn);
        return CMD_SUCCESS;
    }

    buff = (char *)smap_get(&vrf_row->source_ip, protocol_keys[type]);
    if (buff != NULL)
    {
        /* Removing the source IP address configuration. */
        update_protocol_source_ip(vrf_row, source,
                               protocol_keys[type], false);
    }

    /* Adding the source-interface configuration. */
    update_protocol_source_interface(vrf_row, port_row,
                    protocol_keys[type], true);
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
| Responsibility : Resets the source interface to TFTP server and all
|                  the specified protocols in ovsdb VRF
|                  table source_interface and source_ip columns
| Parameters : const char* source : source-interface
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
        error_message = "Common source interface is not configured.";
        break;

    case TFTP_PROTOCOL:
        error_message = "TFTP Source interface is not configured.";
        break;

    case TACACS_PROTOCOL:
        error_message = "TACACS Source interface is not configured.";
        break;

    case RADIUS_PROTOCOL:
        error_message = "RADIUS Source interface is not configured.";
        break;

    default :
        cli_do_config_abort(status_txn);
        vty_out(vty, "Spefied type is unknown protocol.%s", VTY_NEWLINE);
        return CMD_OVSDB_FAILURE;
    }

    /*
     * To unset the source interface to all the specified
     * protocols in ovsdb vrf table source_ip/source_interface column.
     */

    buff = (char *)smap_get(&vrf_row->source_ip, protocol_keys[type]);
    if (buff != NULL)
    {
        /* Unset the source IP address configuration. */
        update_protocol_source_ip(vrf_row, NULL,
                                  protocol_keys[type], false);
    }
    /* lookup for the source-interface record in vrf table */
    source_interface_match = source_interface_lookup(DEFAULT_VRF_NAME,
                                                     protocol_keys[type]);
    if (source_interface_match)
    {
        /* Unset the source-interface configuration. */
        update_protocol_source_interface(vrf_row, NULL,
                                         protocol_keys[type], false);
    }

    if ((buff == NULL) && (!source_interface_match))
            vty_out(vty, "%s%s", error_message, VTY_NEWLINE);

    status = cli_do_config_finish(status_txn);

    if (status == TXN_SUCCESS || status == TXN_UNCHANGED) {
        return CMD_SUCCESS;
    }
    else {
        return CMD_OVSDB_FAILURE;
    }

}

/*----------------------------------------------------------------------------
| Name : get_protocol_by_name
| Responsibility : Gets the protocol type
| Return : SOURCE_INTERFACE_PROTOCOL type
-----------------------------------------------------------------------------*/
SOURCE_INTERFACE_PROTOCOL
get_protocol_by_name(const char *protocol_name)
{
    if (strncmp(protocol_name, protocol_keys[TFTP_PROTOCOL],
        strlen (protocol_name)) == 0) {
        return TFTP_PROTOCOL;
    } else if (strncmp(protocol_name, protocol_keys[TACACS_PROTOCOL],
        strlen (protocol_name)) == 0) {
        return TACACS_PROTOCOL;
    } else if (strncmp(protocol_name, protocol_keys[RADIUS_PROTOCOL],
        strlen (protocol_name)) == 0) {
        return RADIUS_PROTOCOL;
    }

    return ALL_PROTOCOL;
}

/*-----------------------------------------------------------------------------
| Defun for source IP interface
| Responsibility : Configure source IP interface
-----------------------------------------------------------------------------*/
DEFUN(ip_source_interface,
      ip_source_interface_cmd,
      "ip source-interface (tftp | tacacs | radius | all) interface IFNAME ",
      IP_STR
      SOURCE_INTERFACE_STRING
      TFTP_STRING
      TACACS_STRING
      RADIUS_STRING
      ALL_STRING
      INTERFACE_STR
      IFNAME_STR)
{
    return set_source_interface(argv[1], get_protocol_by_name((char*)argv[0]));
}

/*-----------------------------------------------------------------------------
| Defun for source IP address
| Responsibility : Configure source IP address
-----------------------------------------------------------------------------*/
DEFUN(ip_source_address,
      ip_source_address_cmd,
      "ip source-interface (tftp | tacacs | radius | all) A.B.C.D",
      IP_STR
      SOURCE_INTERFACE_STRING
      TFTP_STRING
      TACACS_STRING
      RADIUS_STRING
      ALL_STRING
      ADDRESS_STRING)
{
    struct in_addr addr;
    memset (&addr, 0, sizeof (struct in_addr));

    if (inet_pton (AF_INET, (char*)argv[1], &addr)<= 0)
    {
        vty_out(vty, "Invalid IPv4 address.%s", VTY_NEWLINE);
        return CMD_SUCCESS;
    }
    if (!IS_VALID_IPV4(htonl(addr.s_addr)))
    {
        vty_out(vty,"Broadcast, multicast and "
                "loopback addresses are not allowed.%s", VTY_NEWLINE);
        return CMD_SUCCESS;
    }

    return set_source_ip(argv[1], get_protocol_by_name((char*)argv[0]));
}

/*-----------------------------------------------------------------------------
| Defun for no source interface
| Responsibility : Unset source interface
-----------------------------------------------------------------------------*/
DEFUN(no_ip_source_interface,
      no_ip_source_interface_cmd,
      "no ip source-interface (tftp | tacacs | radius | all) ",
      NO_STR
      IP_STR
      SOURCE_INTERFACE_STRING
      TFTP_STRING
      TACACS_STRING
      RADIUS_STRING
      ALL_STRING)
{
    return reset_source_interface(NULL, get_protocol_by_name((char*)argv[0]));

}

/*-----------------------------------------------------------------------------
| Defun for show source interface for all the protocols
| Responsibility : Displays all the source interface configuration
-----------------------------------------------------------------------------*/
DEFUN(show_source_interface_all,
      show_source_interface_all_cmd,
      "show ip source-interface",
      SHOW_STR
      IP_STR
      SOURCE_INTERFACE_STRING)
{
    return show_source_interface_selection(ALL_PROTOCOL);
}

/*-----------------------------------------------------------------------------
| Defun for show source interface for specified protocol
| Responsibility : Displays the specified protocol
|                  source interface configuration
-----------------------------------------------------------------------------*/
DEFUN(show_source_interface_protocol,
      show_source_interface_protocol_cmd,
      "show ip source-interface (tftp | tacacs | radius) ",
      SHOW_STR
      IP_STR
      SOURCE_INTERFACE_STRING
      TFTP_STRING
      TACACS_STRING
      RADIUS_STRING)
{
    return show_source_interface_selection(
                            get_protocol_by_name((char*)argv[0]));
}

/*-----------------------------------------------------------------------------
| Function       : source_interface_ovsdb_init
| Responsibility : Initialise source interface details.
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
    install_element (CONFIG_NODE, &no_ip_source_interface_cmd);
    install_element (ENABLE_NODE, &show_source_interface_protocol_cmd);
    install_element (ENABLE_NODE, &show_source_interface_all_cmd);

    return;
}
