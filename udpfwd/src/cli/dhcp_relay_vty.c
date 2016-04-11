/* dhcp-relay CLI commands
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
 * along with GNU Zebra; see the file COPYING.  If not, write to the FreeAC
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * File: dhcp_relay_vty.c
 *
 * Purpose: To add dhcp-relay configuration CLI commands.
 */

#include <sys/un.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <pwd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "vtysh/lib/version.h"
#include "getopt.h"
#include "vtysh/command.h"
#include "vtysh/memory.h"
#include <stdbool.h>
#include <stdlib.h>
#include "vtysh/vtysh.h"
#include "vswitch-idl.h"
#include "ovsdb-idl.h"
#include "openvswitch/vlog.h"
#include "openswitch-idl.h"
#include "vtysh/prefix.h"
#include "vtysh/vtysh_ovsdb_if.h"
#include "vtysh/vtysh_ovsdb_config.h"
#include "dhcp_relay_vty.h"
#include "vtysh_ovsdb_dhcp_relay_context.h"
#include "udpfwd_vty_utils.h"
#include "udpfwd_common.h"
#include "udpfwd_util.h"
#include "udpfwd.h"

extern struct ovsdb_idl *idl;
VLOG_DEFINE_THIS_MODULE(dhcp_relay_vty);

/*-----------------------------------------------------------------------------
| Function       : print_relay_statistics
| Responsibility : To print the dhcp-relay statistics.
| Return         : Returns CMD_SUCCESS
-----------------------------------------------------------------------------*/
int32_t
print_relay_statistics(void)
{
    const struct ovsrec_dhcp_relay *row = NULL;
    const struct ovsdb_datum *datum = NULL;
    union ovsdb_atom atom;
    uint32_t index;
    DHCP_RELAY_PKT_COUNTER dhcp_relay_pkt_counters;
    memset(&dhcp_relay_pkt_counters, 0, sizeof(DHCP_RELAY_PKT_COUNTER));

    row = ovsrec_dhcp_relay_first(idl);

    OVSREC_DHCP_RELAY_FOR_EACH (row, idl)
    {
        datum = ovsrec_port_get_dhcp_relay_statistics(row->port,
                                OVSDB_TYPE_STRING, OVSDB_TYPE_INTEGER);
        if (NULL == datum)
            continue;

        atom.string =
            PORT_DHCP_RELAY_STATISTICS_MAP_VALID_V4CLIENT_REQUESTS;
        index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
        dhcp_relay_pkt_counters.client_valids +=
            ((index == UINT_MAX)? 0 : datum->values[index].integer);

        atom.string =
            PORT_DHCP_RELAY_STATISTICS_MAP_DROPPED_V4CLIENT_REQUESTS;
        index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
        dhcp_relay_pkt_counters.client_drops +=
            ((index == UINT_MAX)? 0 : datum->values[index].integer);

        atom.string =
            PORT_DHCP_RELAY_STATISTICS_MAP_VALID_V4SERVER_RESPONSES;
        index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
        dhcp_relay_pkt_counters.serv_valids +=
            ((index == UINT_MAX)? 0 : datum->values[index].integer);

        atom.string =
            PORT_DHCP_RELAY_STATISTICS_MAP_DROPPED_V4SERVER_RESPONSES;
        index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
        dhcp_relay_pkt_counters.serv_drops +=
            ((index == UINT_MAX)? 0 : datum->values[index].integer);

        atom.string =
            PORT_DHCP_RELAY_STATISTICS_MAP_VALID_V4CLIENT_REQUESTS_WITH_OPTION82;
        index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
        dhcp_relay_pkt_counters.client_valids_with_option82 +=
            ((index == UINT_MAX)? 0 : datum->values[index].integer);

        atom.string =
            PORT_DHCP_RELAY_STATISTICS_MAP_DROPPED_V4CLIENT_REQUESTS_WITH_OPTION82;
        index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
        dhcp_relay_pkt_counters.client_drops_with_option82 +=
            ((index == UINT_MAX)? 0 : datum->values[index].integer);

        atom.string =
            PORT_DHCP_RELAY_STATISTICS_MAP_VALID_V4SERVER_RESPONSES_WITH_OPTION82;
        index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
        dhcp_relay_pkt_counters.serv_valids_with_option82 +=
            ((index == UINT_MAX)? 0 : datum->values[index].integer);

        atom.string =
            PORT_DHCP_RELAY_STATISTICS_MAP_DROPPED_V4SERVER_RESPONSES_WITH_OPTION82;
        index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
        dhcp_relay_pkt_counters.serv_drops_with_option82 +=
            ((index == UINT_MAX)? 0 : datum->values[index].integer);
    }

    vty_out(vty, "%s DHCP Relay Statistics:%s",
                VTY_NEWLINE, VTY_NEWLINE);
    vty_out(vty, "%s  Client Requests       Server Responses%s",
                VTY_NEWLINE, VTY_NEWLINE);
    vty_out(vty, "%s  Valid      Dropped    Valid      Dropped%s",
                VTY_NEWLINE, VTY_NEWLINE);
    vty_out(vty, "  ---------- ---------- ---------- ----------%s",
            VTY_NEWLINE);
    vty_out(vty, "  %-10d %-10d %-10d %-10d%s",
            dhcp_relay_pkt_counters.client_valids,
            dhcp_relay_pkt_counters.client_drops,
            dhcp_relay_pkt_counters.serv_valids,
            dhcp_relay_pkt_counters.serv_drops, VTY_NEWLINE);

    vty_out(vty, "%s DHCP Relay Option 82 Statistics:%s",
                VTY_NEWLINE, VTY_NEWLINE);
    vty_out(vty, "%s  Client Requests       Server Responses%s",
                VTY_NEWLINE, VTY_NEWLINE);
    vty_out(vty, "%s  Valid      Dropped    Valid      Dropped%s",
                VTY_NEWLINE, VTY_NEWLINE);
    vty_out(vty, "  ---------- ---------- ---------- ----------%s",
            VTY_NEWLINE);
    vty_out(vty, "  %-10d %-10d %-10d %-10d%s",
            dhcp_relay_pkt_counters.client_valids_with_option82,
            dhcp_relay_pkt_counters.client_drops_with_option82,
            dhcp_relay_pkt_counters.serv_valids_with_option82,
            dhcp_relay_pkt_counters.serv_drops_with_option82, VTY_NEWLINE);

    return CMD_SUCCESS;
}
/*-----------------------------------------------------------------------------
| Function       : show_dhcp_relay_config
| Responsibility : To show the dhcp-relay configuration.
| Return         : Returns CMD_SUCCESS
-----------------------------------------------------------------------------*/
int32_t
show_dhcp_relay_config (void)
{
    const struct ovsrec_system *ovs_row = NULL;
    char *status = NULL;

    ovs_row = ovsrec_system_first(idl);
    if (ovs_row == NULL)
    {
        VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
        return CMD_OVSDB_FAILURE;
    }

    /* Display the dhcp-relay global configuration */
    status = (char *)smap_get(&ovs_row->dhcp_config,
                             SYSTEM_DHCP_CONFIG_MAP_V4RELAY_DISABLED);

    if (status && !strcmp(status, "true"))
        vty_out(vty, "\n DHCP Relay Agent                 : %s%s",
                "disabled", VTY_NEWLINE);
    else
        vty_out(vty, "\n DHCP Relay Agent                 : %s%s",
                "enabled", VTY_NEWLINE);

    /* Display the dhcp-relay hop-count increment configuration */
    status = NULL;
    status = (char *)smap_get(&ovs_row->dhcp_config,
                SYSTEM_DHCP_CONFIG_MAP_V4RELAY_HOP_COUNT_INCREMENT_DISABLED);

    if (status && !strcmp(status, "true"))
        vty_out(vty, " DHCP Request Hop Count Increment : %s%s",
                "disabled", VTY_NEWLINE);
    else
        vty_out(vty, " DHCP Request Hop Count Increment : %s%s",
                "enabled", VTY_NEWLINE);

    /* Display the dhcp-relay option 82 configuration */
    status = NULL;
    status = (char *)smap_get(&ovs_row->dhcp_config,
                SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_ENABLED);

    if (status && !strcmp(status, "true"))
        vty_out(vty, " Option 82                        : %s%s",
                "enabled", VTY_NEWLINE);
    else
        vty_out(vty, " Option 82                        : %s%s",
                "disabled", VTY_NEWLINE);

    /* Display the dhcp-relay option 82 response validation configuration */
    status = NULL;
    status = (char *)smap_get(&ovs_row->dhcp_config,
                SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_VALIDATION_ENABLED);

    if (status && !strcmp(status, "true"))
        vty_out(vty, " Response Validation              : %s%s",
                "enabled", VTY_NEWLINE);
    else
        vty_out(vty, " Response Validation              : %s%s",
                "disabled", VTY_NEWLINE);

    /* Display the dhcp-relay option 82 policy configuration */
    status = NULL;
    status = (char *)smap_get(&ovs_row->dhcp_config,
                SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_POLICY);
    if (!status)
    {
        status = policy_name[REPLACE];
    }

    if (status)
    {
        vty_out(vty, " Option 82 Handle Policy          : %s%s",
                status, VTY_NEWLINE);
    }

    /* Display the dhcp-relay option 82 remote ID configuration */
    status = NULL;
    status = (char *)smap_get(&ovs_row->dhcp_config,
                SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_REMOTE_ID);
    if (!status)
    {
        status = remote_id_name[REMOTE_ID_MAC];
    }

    if (status)
    {
        vty_out(vty, " Remote ID                        : %s%s",
                status, VTY_NEWLINE);
    }
    print_relay_statistics();
    return CMD_SUCCESS;
}

/*-----------------------------------------------------------------------------
| Function          : update_feature_key
| Responsibility    : Update the configuration status of a feature
| Parameters        :
|      value        : Pointer to configuration bit map
|      status       : Config status of the feature
| smap_status_value : Pointer to smap structure
|      feature      : feature enum value
| Return            : None
-----------------------------------------------------------------------------*/
void update_feature_key(FEATURE_STATUS status,
                        struct smap *smap_status_value,
                        UDPFWD_FEATURE feature)
{
    char *state = NULL, *key = NULL;

    switch(feature)
    {
    case DHCP_RELAY:
        key = SYSTEM_DHCP_CONFIG_MAP_V4RELAY_DISABLED;
    break;

    case DHCP_RELAY_HOP_COUNT_INCREMENT:
        key = SYSTEM_DHCP_CONFIG_MAP_V4RELAY_HOP_COUNT_INCREMENT_DISABLED;
    break;

    case DHCP_RELAY_OPTION82:
        key = SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_ENABLED;
    break;

    case DHCP_RELAY_OPTION82_VALIDATE:
        key = SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_VALIDATION_ENABLED;
    break;

    default:
    break;
    }

    state = (ENABLE == status) ? "true" : "false";
    smap_replace(smap_status_value, key, state);

    return;
}

/*-----------------------------------------------------------------------------
| Function          : update_option_82_policy
| Responsibility    : Update the forward policy of option 82
| Parameters        :
|      value        : Pointer to option 82 forward policy
|      rid          : Policy enum value
| smap_status_value : Pointer to smap structure
| Return            : None
-----------------------------------------------------------------------------*/
void update_option_82_policy(DHCP_RELAY_OPTION82_POLICY policy,
                             struct smap *smap_status_value)
{
    char *policy_type = NULL;

    switch(policy)
    {
    case KEEP:
        policy_type = policy_name[KEEP];
    break;

    case DROP:
        policy_type = policy_name[DROP];
    break;

    case REPLACE:
        policy_type = policy_name[REPLACE];
    break;

    default:
    break;
    }

    smap_replace(smap_status_value,
                 SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_POLICY, policy_type);
    return;
}

/*-----------------------------------------------------------------------------
| Function          : update_option_82_remote_id
| Responsibility    : Update the remote ID of option 82
| Parameters        :
|      value        : Pointer to option 82 remote ID key
|      rid          : remote id enum value
| smap_status_value : Pointer to smap structure
| Return            : None
-----------------------------------------------------------------------------*/
void update_option_82_remote_id(DHCP_RELAY_OPTION82_REMOTE_ID rid,
                                struct smap *smap_status_value)
{
    char *remote_id_type = NULL;

    switch(rid)
    {
    case REMOTE_ID_IP:
        remote_id_type = remote_id_name[REMOTE_ID_IP];
    break;

    case REMOTE_ID_MAC:
        remote_id_type = remote_id_name[REMOTE_ID_MAC];
    break;

    default:
    break;
    }

    smap_replace(smap_status_value,
                 SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_REMOTE_ID,
                 remote_id_type);
    return;
}
/*-----------------------------------------------------------------------------
| Function       : dhcp_relay_config
| Responsibility : To configure dhcp-relay and option 82.
| Parameters     :
|     *dhcpRelay : Pointer containing user inputdhcp-relay details
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
dhcp_relay_config (uint16_t update_config, FEATURE_CONFIG *config_value)
{
    const struct ovsrec_system *ovs_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    struct smap smap_status_value;
    enum ovsdb_idl_txn_status txn_status;
    FEATURE_STATUS state;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    ovs_row = ovsrec_system_first(idl);
    if (!ovs_row) {
        VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    smap_clone(&smap_status_value, &ovs_row->dhcp_config);

     /* Check if dhcp-relay configuration needs to be udpated. */
    if (ENABLE == get_feature_status(update_config, DHCP_RELAY)) {
        state = get_feature_status(config_value->config, DHCP_RELAY);

        /*
         * Pass the state as DISABLE for setting the key value to false to
         * enable the dhcp-relay and pass ENABLE otherwise.
         */
        update_feature_key((state == ENABLE) ? DISABLE : ENABLE,
                           &smap_status_value, DHCP_RELAY);
    }

    /*
     * Check if dhcp-relay hop-count increment configuration
     * needs to be udpated.
     */
    if (ENABLE == get_feature_status(update_config,
                      DHCP_RELAY_HOP_COUNT_INCREMENT)) {
        state = get_feature_status(config_value->config,
                                   DHCP_RELAY_HOP_COUNT_INCREMENT);
        /*
         * Pass the state as DISABLE for setting the key value to false to
         * enable the dhcp-relay hop-count increment and pass ENABLE
         * otherwise.
         */
        update_feature_key((state == ENABLE) ? DISABLE : ENABLE,
                           &smap_status_value,
                           DHCP_RELAY_HOP_COUNT_INCREMENT);
    }

    /* Check if dhcp-relay option 82 configuration needs to be udpated. */
    if (ENABLE == get_feature_status(update_config, DHCP_RELAY_OPTION82)) {
        state = get_feature_status(config_value->config, DHCP_RELAY_OPTION82);
        update_feature_key(state, &smap_status_value,
                           DHCP_RELAY_OPTION82);
    }

    /*
     * Check if dhcp-relay option 82 response validation configuration
     * needs to be udpated.
     */
    if (ENABLE == get_feature_status(update_config,
                      DHCP_RELAY_OPTION82_VALIDATE)) {
        state = get_feature_status(config_value->config,
                                   DHCP_RELAY_OPTION82_VALIDATE);
        update_feature_key(state, &smap_status_value,
                           DHCP_RELAY_OPTION82_VALIDATE);
    }

    /*
     * Check if dhcp-relay option 82 forward policy configuration
     * needs to be udpated.
     */
    if (config_value->policy < INVALID) {
        update_option_82_policy(config_value->policy, &smap_status_value);
    }
    /*
     * Check if dhcp-relay option 82 remote ID configuration
     * needs to be udpated.
     */
    if (config_value->r_id < REMOTE_ID_INVALID) {
        update_option_82_remote_id(config_value->r_id, &smap_status_value);
    }

    ovsrec_system_set_dhcp_config(ovs_row, &smap_status_value);
    smap_destroy(&smap_status_value);
    txn_status = cli_do_config_finish(status_txn);

    if (txn_status == TXN_SUCCESS || txn_status == TXN_UNCHANGED)
    {
        return CMD_SUCCESS;
    }
    else
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
        return CMD_OVSDB_FAILURE;
    }
}

/*-----------------------------------------------------------------------------
| Function         : dhcp_relay_bootp_gateway_config
| Responsibility   : Set/unset dhcp-relay bootp-gateway.
| Parameters       :
|  *gatewayAddress : Pointer containing bootp-gateway
|      set         : Flag to set or unset
| Return           : On success returns CMD_SUCCESS,
|                    On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
dhcp_relay_bootp_gateway_config(const char *gatewayAddress, bool set)
{
    const struct ovsrec_dhcp_relay *row = NULL;
    const struct ovsrec_port *port_row = NULL;
    const struct ovsrec_vrf *vrf_row = NULL;
    struct smap smap_status_value;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status txn_status;
    bool isAddrMatch = false, deleteRow = false;
    char *mask = NULL, *buff = NULL;
    size_t maskPosition, iter;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    /* Lookup for the record in the dhcp-relay table */
    row = dhcp_relay_row_lookup((char*)vty->index, DEFAULT_VRF_NAME);
    if (set)
    {

        OVSREC_PORT_FOR_EACH (port_row, idl)
        {
            if (strcmp(port_row->name, (char *)vty->index) != 0)
                continue;

            /* To check IP address configuration on the interface. */
            if (port_row->ip4_address)
            {
                mask = strchr(port_row->ip4_address, '/');
                maskPosition = mask - (port_row->ip4_address);
                if (!strncmp(gatewayAddress, port_row->ip4_address,
                   maskPosition))
                {
                    isAddrMatch = true;
                    break;
                }
            }

            /* To check secondary IP address configuration on the interface. */
            for (iter = 0; iter < port_row->n_ip4_address_secondary; iter++)
            {
                mask = strchr(port_row->ip4_address_secondary[iter], '/');
                maskPosition = mask - (port_row->ip4_address_secondary[iter]);
                if (!strncmp(gatewayAddress,
                   port_row->ip4_address_secondary[iter], maskPosition))
                {
                    isAddrMatch = true;
                    break;
                }
            }
            break;
        }

        if (!isAddrMatch)
        {
            /* Unconfigured IP address on the interface. */

            vty_out(vty, "The IP address %s is not yet configured on " \
                    "this interface.%s", gatewayAddress, VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }

        if (NULL == row)
        {
            row = ovsrec_dhcp_relay_insert(status_txn);
            /* Update the dhcp-relay table. */
            ovsrec_dhcp_relay_set_port(row, port_row);

            /* Update the vrf name of the port */
            vrf_row = udp_bcast_config_vrf_lookup(DEFAULT_VRF_NAME);
            if (!vrf_row)
            {
                vty_out(vty, "Error: Could not fetch "
                    "default VRF data.%s", VTY_NEWLINE);
                VLOG_ERR("%s VRF table did not have any rows. "
                    "Ideally it should have just one entry.", __func__);
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }

            ovsrec_dhcp_relay_set_vrf(row, vrf_row);
        }

        /* Update the bootp-gateway IP. */
        smap_clone(&smap_status_value, &row->other_config);
        smap_replace(&smap_status_value,
            DHCP_RELAY_OTHER_CONFIG_MAP_BOOTP_GATEWAY, gatewayAddress);
    }
    else
    {
        if (NULL == row)
        {
            /* Unconfigured bootp-gateway on the interface. */
            vty_out(vty, "The BOOTP Gateway %s is not configured on " \
                    "this interface.%s", gatewayAddress, VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }
        else
        {
            smap_clone(&smap_status_value, &row->other_config);
            buff = (char *)smap_get(&row->other_config,
                                DHCP_RELAY_OTHER_CONFIG_MAP_BOOTP_GATEWAY);

            if (buff && strcmp(buff, gatewayAddress) == 0)
            {
                if (row->n_ipv4_ucast_server)
                {
                    smap_remove(&smap_status_value,
                        DHCP_RELAY_OTHER_CONFIG_MAP_BOOTP_GATEWAY);
                }
                else
                {
                    /*
                     * Delete the row if the helper address configuration
                     * is not present on the interface.
                     */
                    deleteRow = true;
                }
            }
            else
            {
                /* Unconfigured IP address on the interface. */

                vty_out(vty, "The BOOTP Gateway %s is not configured on " \
                        "this interface.%s", gatewayAddress, VTY_NEWLINE);
                cli_do_config_abort(status_txn);
                return CMD_SUCCESS;
            }
        }
    }

    if (deleteRow)
    {
        ovsrec_dhcp_relay_delete(row);
    }
    else
    {
        ovsrec_dhcp_relay_set_other_config(row, &smap_status_value);
    }

    smap_destroy(&smap_status_value);
    txn_status = cli_do_config_finish(status_txn);

    if (txn_status == TXN_SUCCESS || txn_status == TXN_UNCHANGED)
    {
        return CMD_SUCCESS;
    }
    else
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
        return CMD_OVSDB_FAILURE;
    }
}

/*-----------------------------------------------------------------------------
| Responsibility : To show the dhcp-relay bootp-gateway configurations.
| Parameters     :
|      *portname : Name of the Port
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
show_dhcp_relay_bootp_gateway_config(const char *portname)
{
    const struct ovsrec_dhcp_relay *row = NULL;
    char *buff = NULL;
    bool entry_found = false;

    row = ovsrec_dhcp_relay_first(idl);

    OVSREC_DHCP_RELAY_FOR_EACH (row, idl)
    {
        /* Get the interface details. */
        if (row->port)
        {
            if (portname)
            {
                if (strcmp(row->port->name, portname) == 0)
                {
                    buff = (char *)smap_get(&row->other_config,
                             DHCP_RELAY_OTHER_CONFIG_MAP_BOOTP_GATEWAY);
                    if (buff)
                    {
                        if (entry_found == false)
                        {
                            entry_found = true;
                            vty_out(vty, "%s BOOTP Gateway Entries%s",
                                    VTY_NEWLINE, VTY_NEWLINE);
                            vty_out(vty, "%s Interface            BOOTP "
                                    "Gateway%s", VTY_NEWLINE, VTY_NEWLINE);
                            vty_out(vty, " -------------------- ---------"
                                    "------%s", VTY_NEWLINE);
                        }

                        vty_out(vty, " %-20s %s%s", row->port->name,
                                buff, VTY_NEWLINE);
                    }

                    return CMD_SUCCESS;
                }
            }
            else
            {
                buff = (char *)smap_get(&row->other_config,
                             DHCP_RELAY_OTHER_CONFIG_MAP_BOOTP_GATEWAY);
                if (buff)
                {
                    if (entry_found == false)
                    {
                        entry_found = true;
                        vty_out(vty, "%s BOOTP Gateway Entries%s",
                                    VTY_NEWLINE, VTY_NEWLINE);
                        vty_out(vty, "%s Interface            BOOTP "
                                "Gateway%s", VTY_NEWLINE, VTY_NEWLINE);
                        vty_out(vty, " -------------------- ---------"
                                "------%s", VTY_NEWLINE);
                    }

                    vty_out(vty, " %-20s %s%s", row->port->name,
                            buff, VTY_NEWLINE);
                }

            }
        }
    }

    if (entry_found == false)
    {
        if (portname)
            vty_out(vty, "No bootp-gateway configuration found on "
                    "the interface : %s.%s", portname, VTY_NEWLINE);
        else
            vty_out(vty, "No bootp-gateway configuration found.%s",
                    VTY_NEWLINE);
    }

    return CMD_SUCCESS;
}

/*-----------------------------------------------------------------------------
| Defun for dhcp-relay and hop count increment configuration
| Responsibility: Enable dhcp-relay or hop count increment
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_configuration,
      dhcp_relay_configuration_cmd,
      "dhcp-relay {hop-count-increment}",
      DHCP_RELAY_STR
      HOP_COUNT_INCREMENT_STR)
{
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;
    config_value.policy = INVALID;
    config_value.r_id = REMOTE_ID_INVALID;

    if (argv[0])
    {
        /* Mark dhcp-relay hop count increment feature to be udpated */
        set_feature_status(&update_features,
                           DHCP_RELAY_HOP_COUNT_INCREMENT, ENABLE);

        /* Set new configuration value */
        set_feature_status(&config_value.config,
                           DHCP_RELAY_HOP_COUNT_INCREMENT, ENABLE);
    }
    else
    {
        /* Mark dhcp-relay feature to be udpated */
        set_feature_status(&update_features, DHCP_RELAY, ENABLE);

        /* Set new configuration value */
        set_feature_status(&config_value.config, DHCP_RELAY, ENABLE);
    }

    return dhcp_relay_config(update_features, &config_value);
}

/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 keep or drop or replace policy or
| response validation configuration
| Responsibility: Enable dhcp-relay option 82 keep or drop or replace policy or
|                 response validation configuration
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_options_configuration,
      dhcp_relay_options_configuration_cmd,
      "dhcp-relay option 82 (keep|validate|drop|replace)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      KEEP_STR
      VALIDATE_STR
      DROP_STR
      REPLACE_STR)
{
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;
    config_value.r_id = REMOTE_ID_INVALID;

    /* Mark dhcp-relay and option 82 features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY, ENABLE);
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY, ENABLE);
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);

    if (!strcmp((char*)argv[0], "validate"))
    {
        const struct ovsrec_system *ovs_row = NULL;
        char *policy = NULL;
        ovs_row = ovsrec_system_first(idl);
        if (ovs_row == NULL)
        {
            VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
            return CMD_OVSDB_FAILURE;
        }
        policy = (char *)smap_get(&ovs_row->dhcp_config,
                         SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_POLICY);
        if (policy && strcmp(policy, policy_name[KEEP]) == 0)
        {
            vty_out(vty, "Keep and validate options cannot be configured "
                    "together.%s", VTY_NEWLINE);
            return CMD_SUCCESS;
        }
        else
        {
            /* Mark dhcp-relay option 82 validation to be udpated */
            set_feature_status(&update_features,
                          DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

            /* Set new configuration values */
            set_feature_status(&config_value.config,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
            config_value.policy = INVALID;
        }
    }
    else
    {
        /* Set new configuration value */
        if (!strcmp((char*)argv[0], policy_name[KEEP]))
        {
            /* Mark dhcp-relay option 82 validation to be udpated */
            set_feature_status(&update_features,
                          DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
            /* Set new configuration values */
            set_feature_status(&config_value.config,
                          DHCP_RELAY_OPTION82_VALIDATE, DISABLE);
            config_value.policy = KEEP;
        }
        else if(!strcmp((char*)argv[0], policy_name[REPLACE]))
            config_value.policy = REPLACE;
        else
            config_value.policy = DROP;
    }

    return dhcp_relay_config(update_features, &config_value);
}

/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 keep policy with ip or mac
| remote id configuration
| Responsibility: Enable dhcp-relay option 82 keep policy with ip or mac
|                 remote id configuration
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_keep_option_configuration,
      dhcp_relay_keep_option_configuration_cmd,
      "dhcp-relay option 82 keep (mac|ip)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      KEEP_STR
      MAC_STR
      OPTION_IP_STR)
{
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;
    /* Mark dhcp-relay, option 82 and validation features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY, ENABLE);
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&update_features,
                          DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY, ENABLE);
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&config_value.config,
                          DHCP_RELAY_OPTION82_VALIDATE, DISABLE);
    config_value.policy = KEEP;
    if(argv[0])
    {
        if (!strcmp((char*)argv[0], remote_id_name[REMOTE_ID_IP]))
            config_value.r_id = REMOTE_ID_IP;
        else
            config_value.r_id = REMOTE_ID_MAC;
    }
    else
    {
        config_value.r_id = REMOTE_ID_INVALID;
    }

    return dhcp_relay_config(update_features, &config_value);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 drop policy with response validation
| or ip or mac remote id configuration
| Responsibility: Enable dhcp-relay option 82 drop policy with response
|                 validation or ip or mac remote id
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_drop_option_configuration,
      dhcp_relay_drop_option_configuration_cmd,
      "dhcp-relay option 82 drop (validate|mac|ip)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      DROP_STR
      VALIDATE_STR
      MAC_STR
      OPTION_IP_STR)
{
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;
    /* Mark dhcp-relay and option 82 features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&config_value.config, DHCP_RELAY, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY, ENABLE);
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    config_value.policy = DROP;

    if(argv[0])
    {
        if (!strcmp((char*)argv[0], "validate"))
        {
            /* Mark dhcp-relay option 82 feature to be udpated */
            set_feature_status(&update_features,
                               DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

            /* Set new configuration values */
            set_feature_status(&config_value.config,
                               DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
            config_value.r_id = REMOTE_ID_INVALID;
        }
        else
        {
            /* Set new configuration values */
            if (!strcmp((char*)argv[0], remote_id_name[REMOTE_ID_IP]))
                config_value.r_id = REMOTE_ID_IP;
            else
                config_value.r_id = REMOTE_ID_MAC;
        }
    }

    return dhcp_relay_config(update_features, &config_value);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 drop policy with response validation
| mac or ip remote id configuration
| Responsibility: Enable dhcp-relay option 82 with drop policy,
|                 response validation and mac or ip remote id
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_drop_validate_option_configuration,
      dhcp_relay_drop_validate_option_configuration_cmd,
      "dhcp-relay option 82 drop validate (mac|ip)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      DROP_STR
      VALIDATE_STR
      MAC_STR
      OPTION_IP_STR)
{
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */

    FEATURE_CONFIG config_value;
    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    /* Mark dhcp-relay, option 82 and validation features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY, ENABLE);
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&update_features,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY, ENABLE);
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&config_value.config,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    config_value.policy = DROP;

    if (argv[0])
    {
        if (!strcmp((char*)argv[0], remote_id_name[REMOTE_ID_IP]))
            config_value.r_id = REMOTE_ID_IP;
        else
            config_value.r_id = REMOTE_ID_MAC;
    }
    else
    {
        config_value.r_id = REMOTE_ID_INVALID;
    }

    return dhcp_relay_config(update_features, &config_value);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 drop policy with mac remote id and
| response validation configuration
| Responsibility: Enable dhcp-relay option 82 with replace policy,
|                 mac remote id and response validation
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_drop_remote_id_mac_option_configuration,
      dhcp_relay_drop_remote_id_mac_option_configuration_cmd,
      "dhcp-relay option 82 drop mac (validate)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      DROP_STR
      MAC_STR
      VALIDATE_STR)
{
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    /* Mark dhcp-relay and option 82 features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY, ENABLE);
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);


    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY, ENABLE);
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    config_value.policy = DROP;
    config_value.r_id = REMOTE_ID_MAC;

    if (argv[0])
    {
        /* Mark dhcp-relay option 82 feature to be udpated */
        set_feature_status(&update_features,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

        /* Set new configuration value */
        set_feature_status(&config_value.config,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    }

    return dhcp_relay_config(update_features, &config_value);
}

/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 drop policy with ip remote id and
| response validation configuration
| Responsibility: Enable dhcp-relay option 82 with drop policy,
|                 ip remote id and response validation
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_drop_remote_id_ip_option_configuration,
      dhcp_relay_drop_remote_id_ip_option_configuration_cmd,
      "dhcp-relay option 82 drop ip (validate)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      DROP_STR
      OPTION_IP_STR
      VALIDATE_STR)
{
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    /* Mark dhcp-relay and option 82 features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY, ENABLE);
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY, ENABLE);
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    config_value.policy = DROP;
    config_value.r_id = REMOTE_ID_IP;

    if (argv[0])
    {
        /* Mark dhcp-relay option 82 feature to be udpated */
        set_feature_status(&update_features,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

        /* Set new configuration value */
        set_feature_status(&config_value.config,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    }

    return dhcp_relay_config(update_features, &config_value);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 replace policy with response validation
| or ip or mac remote id configuration
| Responsibility: Enable dhcp-relay option 82 replace policy with response
|                 validation or ip or mac remote id
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_replace_option_configuration,
      dhcp_relay_replace_option_configuration_cmd,
      "dhcp-relay option 82 replace (validate|mac|ip)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      REPLACE_STR
      VALIDATE_STR
      MAC_STR
      OPTION_IP_STR)
{
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;
    /* Mark dhcp-relay and option 82 features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY, ENABLE);
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY, ENABLE);
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    config_value.policy = REPLACE;

    if(argv[0])
    {
        if (!strcmp((char*)argv[0], "validate"))
        {
            /* Mark dhcp-relay option 82 validation feature to be udpated */
            set_feature_status(&update_features,
                               DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

            /* Set new configuration values */
            set_feature_status(&config_value.config,
                               DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
            config_value.r_id = REMOTE_ID_INVALID;
        }
        else
        {
            /* Set new configuration values */
            if (!strcmp((char*)argv[0], remote_id_name[REMOTE_ID_IP]))
                config_value.r_id = REMOTE_ID_IP;
            else
                config_value.r_id = REMOTE_ID_MAC;
        }
    }

    return dhcp_relay_config(update_features, &config_value);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 replace policy with response validation
| mac or ip remote id configuration
| Responsibility: Enable dhcp-relay option 82 with replace policy,
|                 response validation and mac or ip remote id
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_replace_validate_option_configuration,
      dhcp_relay_replace_validate_option_configuration_cmd,
      "dhcp-relay option 82 replace validate (mac|ip)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      REPLACE_STR
      VALIDATE_STR
      MAC_STR
      OPTION_IP_STR)
{
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;
    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    /* Mark dhcp-relay, option 82 and validation features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY, ENABLE);
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&update_features,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY, ENABLE);
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&config_value.config,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    config_value.policy = REPLACE;

    if (argv[0])
    {
        if (!strcmp((char*)argv[0], remote_id_name[REMOTE_ID_IP]))
            config_value.r_id = REMOTE_ID_IP;
        else
            config_value.r_id = REMOTE_ID_MAC;
    }
    else
    {
        config_value.r_id = REMOTE_ID_INVALID;
    }

    return dhcp_relay_config(update_features, &config_value);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 replace policy with ip remote id and
| response validation configuration
| Responsibility: Enable dhcp-relay option 82 with replace policy,
|                 ip remote id and response validation
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_replace_remote_id_mac_option_configuration,
      dhcp_relay_replace_remote_id_mac_option_configuration_cmd,
      "dhcp-relay option 82 replace mac (validate)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      REPLACE_STR
      MAC_STR
      VALIDATE_STR)
{
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    /* Mark dhcp-relay and option 82 features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY, ENABLE);
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY, ENABLE);
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    config_value.policy = REPLACE;
    config_value.r_id = REMOTE_ID_MAC;

    if (argv[0])
    {
        /* Mark dhcp-relay option 82 feature to be udpated */
        set_feature_status(&update_features,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

        /* Set new configuration value */
        set_feature_status(&config_value.config,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    }

    return dhcp_relay_config(update_features, &config_value);
}

/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 replace policy with ip remote id and
| response validation configuration
| Responsibility: Enable dhcp-relay option 82 with replace policy,
|                 ip remote id and response validation
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_replace_remote_id_ip_option_configuration,
      dhcp_relay_replace_remote_id_ip_option_configuration_cmd,
      "dhcp-relay option 82 replace ip (validate)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      REPLACE_STR
      OPTION_IP_STR
      VALIDATE_STR)
{
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    /* Mark dhcp-relay and option 82 features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY, ENABLE);
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY, ENABLE);
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    config_value.policy = REPLACE;
    config_value.r_id = REMOTE_ID_IP;

    if (argv[0])
    {
        /* Mark dhcp-relay option 82 validation feature to be udpated */
        set_feature_status(&update_features,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

        /* Set new configuration value */
        set_feature_status(&config_value.config,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    }

    return dhcp_relay_config(update_features, &config_value);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 response validation with drop
| or replace policy configuration
| Responsibility: Enable dhcp-relay option 82 validation with
|                 drop or replace policy
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_validate_policy_option_configuration,
      dhcp_relay_validate_policy_option_configuration_cmd,
      "dhcp-relay option 82 validate (drop|replace)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      VALIDATE_STR
      DROP_STR
      REPLACE_STR)
{
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;
    config_value.r_id = REMOTE_ID_INVALID;

    /* Mark dhcp-relay, option 82 and validation features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY, ENABLE);
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&update_features,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY, ENABLE);
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&config_value.config,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    if (argv[0])
    {
        if(!strcmp((char*)argv[0], policy_name[REPLACE]))
            config_value.policy = REPLACE;
        else
            config_value.policy = DROP;
    }

    return dhcp_relay_config(update_features, &config_value);
}

/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 response validation with replace policy
| and mac or ip remote ID configuration
| Responsibility: Enable dhcp-relay option 82 validation with
|                 replace policy and mac or ip remote ID
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_validate_drop_option_configuration,
      dhcp_relay_validate_drop_option_configuration_cmd,
      "dhcp-relay option 82 validate drop (mac|ip)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      VALIDATE_STR
      DROP_STR
      MAC_STR
      OPTION_IP_STR)
{
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    /* Mark dhcp-relay, option 82 and validation features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY, ENABLE);
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&update_features,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY, ENABLE);
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&config_value.config,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    config_value.policy = DROP;
    if (argv[0])
    {
        if (!strcmp((char*)argv[0], remote_id_name[REMOTE_ID_IP]))
            config_value.r_id = REMOTE_ID_IP;
        else
            config_value.r_id = REMOTE_ID_MAC;
    }
    else
    {
        config_value.r_id = REMOTE_ID_INVALID;
    }

    return dhcp_relay_config(update_features, &config_value);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 response validation with replace policy
| and mac or ip remote ID configuration
| Responsibility: Enable dhcp-relay option 82 validation with
|                 replace policy and mac or ip remote ID
-----------------------------------------------------------------------------*/
DEFUN(dhcp_relay_validate_replace_option_configuration,
      dhcp_relay_validate_replace_option_configuration_cmd,
      "dhcp-relay option 82 validate replace (mac|ip)",
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      VALIDATE_STR
      REPLACE_STR
      MAC_STR
      OPTION_IP_STR)
{
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;

    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    /* Mark dhcp-relay, option 82 and validation features to be udpated */
    set_feature_status(&update_features, DHCP_RELAY, ENABLE);
    set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&update_features,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

    /* Set new configuration values */
    set_feature_status(&config_value.config, DHCP_RELAY, ENABLE);
    set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, ENABLE);
    set_feature_status(&config_value.config,
                       DHCP_RELAY_OPTION82_VALIDATE, ENABLE);
    config_value.policy = DROP;
    if (argv[0])
    {
        if (!strcmp((char*)argv[0], remote_id_name[REMOTE_ID_IP]))
            config_value.r_id = REMOTE_ID_IP;
        else
            config_value.r_id = REMOTE_ID_MAC;
    }
    else
    {
        config_value.r_id = REMOTE_ID_INVALID;
    }

    return dhcp_relay_config(update_features, &config_value);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay option 82 and dhcp-relay option 82 validation
| unconfiguration
| Responsibility: Disable dhcp-relay or hop-count-increment
-----------------------------------------------------------------------------*/
DEFUN(no_dhcp_relay_option_configuration,
      no_dhcp_relay_option_configuration_cmd,
      "no dhcp-relay option 82 {validate}",
      NO_STR
      DHCP_RELAY_STR
      OPTION_STR
      OPTION_82_STR
      VALIDATE_STR)
{
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;
    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    if (argv[0])
    {
        /* Mark dhcp-relay option 82 feature to be udpated */
        set_feature_status(&update_features,
                           DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

        /* Set new configuration values */
        set_feature_status(&config_value.config,
                           DHCP_RELAY_OPTION82_VALIDATE, DISABLE);
        config_value.r_id = REMOTE_ID_INVALID;
        config_value.policy = INVALID;
    }
    else
    {
        /* Mark dhcp-relay option 82 and validation features to be udpated */
        set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);
        set_feature_status(&update_features, DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

        /* Set new configuration values */
        set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, DISABLE);
        set_feature_status(&config_value.config, DHCP_RELAY_OPTION82_VALIDATE, DISABLE);
        config_value.r_id = REMOTE_ID_MAC;
        config_value.policy = REPLACE;
    }

    return dhcp_relay_config(update_features, &config_value);
}
/*-----------------------------------------------------------------------------
| Defun for dhcp-relay and hop-count-increment unconfiguration
| Responsibility: Disable dhcp-relay or hop-count-increment
-----------------------------------------------------------------------------*/
DEFUN(no_dhcp_relay_configuration,
      no_dhcp_relay_configuration_cmd,
      "no dhcp-relay {hop-count-increment}",
      NO_STR
      DHCP_RELAY_STR
      HOP_COUNT_INCREMENT_STR)
{
    /* Features for which config update is required */
    uint16_t update_features = 0;
    /* New configuration data */
    FEATURE_CONFIG config_value;
    memset(&config_value, 0, sizeof(FEATURE_CONFIG));
    config_value.config = 0;

    if (argv[0])
    {
        /* Mark dhcp-relay hop count increment feature to be udpated */
        set_feature_status(&update_features,
                           DHCP_RELAY_HOP_COUNT_INCREMENT, ENABLE);

        /* Set new configuration value */
        set_feature_status(&config_value.config,
                           DHCP_RELAY_HOP_COUNT_INCREMENT, DISABLE);
        config_value.r_id = REMOTE_ID_INVALID;
        config_value.policy = INVALID;
    }
    else
    {
        /* Mark dhcp-relay, option 82 and validation features to be udpated */
        set_feature_status(&update_features, DHCP_RELAY, ENABLE);
        set_feature_status(&update_features, DHCP_RELAY_OPTION82, ENABLE);
        set_feature_status(&update_features, DHCP_RELAY_OPTION82_VALIDATE, ENABLE);

        /* Set new configuration values */
        set_feature_status(&config_value.config, DHCP_RELAY, DISABLE);
        set_feature_status(&config_value.config, DHCP_RELAY_OPTION82, DISABLE);
        set_feature_status(&config_value.config, DHCP_RELAY_OPTION82_VALIDATE, DISABLE);
        config_value.r_id = REMOTE_ID_MAC;
        config_value.policy = REPLACE;
    }

    return dhcp_relay_config(update_features, &config_value);
}


/*-----------------------------------------------------------------------------
| Defun for show dhcp-relay
| Responsibility: Displays the dhcp-relay configurations
-----------------------------------------------------------------------------*/
DEFUN(show_dhcp_relay_configuration,
      show_dhcp_relay_configuration_cmd,
      "show dhcp-relay",
      SHOW_STR
      SHOW_DHCP_RELAY_STR)
{
     return show_dhcp_relay_config();
}

/*-----------------------------------------------------------------------------
| Defun ip helper-address configuration
| Responsibility: Set a helper-addresses for a dhcp-relay
-----------------------------------------------------------------------------*/
DEFUN(ip_helper_address_configuration,
      ip_helper_address_configuration_cmd,
      "ip helper-address A.B.C.D ",
      IP_STR
      HELPER_ADDRESS_STR
      HELPER_ADDRESS_INPUT_STR)
{
    udpfwd_server udpfwdServer;
    memset(&udpfwdServer, 0, sizeof(udpfwd_server));

    /* Validate the input parameters. */
    if (decode_server_param(&udpfwdServer, argv, DHCP_RELAY))
    {
        return udpfwd_helperaddressconfig(&udpfwdServer, SET);
    }
    else
    {
        return CMD_SUCCESS;
    }

}

/*-----------------------------------------------------------------------------
| Defun ip bootp-gateway configuration
| Responsibility: Set a bootp-gateway for a dhcp-relay
-----------------------------------------------------------------------------*/
DEFUN(ip_bootp_gateway_configuration,
      ip_bootp_gateway_configuration_cmd,
      "ip bootp-gateway A.B.C.D ",
      IP_STR
      BOOTP_GATEWAY_STR
      BOOTP_GATEWAY_INPUT_STR)
{
    struct in_addr addr;
    memset (&addr, 0, sizeof (struct in_addr));

    /* Validate bootp-gateway address. */
    if (inet_pton (AF_INET, (char*)argv[0], &addr)<= 0)
    {
        vty_out(vty, "Invalid IPv4 address.%s", VTY_NEWLINE);
        return CMD_SUCCESS;
    }

    if (!IS_VALID_IPV4(htonl(addr.s_addr)))
    {
        vty_out(vty,
                "Broadcast, multicast and loopback addresses "
                "are not allowed.%s",
                VTY_NEWLINE);
        return CMD_SUCCESS;
    }

    return dhcp_relay_bootp_gateway_config(argv[0], SET);

}
/*-----------------------------------------------------------------------------
| Defun ip helper-address unconfiguration
| Responsibility: Unset a helper-addresses for a dhcp-relay
-----------------------------------------------------------------------------*/
DEFUN(no_ip_helper_address_configuration,
      no_ip_helper_address_configuration_cmd,
      "no ip helper-address A.B.C.D ",
      NO_STR
      IP_STR
      HELPER_ADDRESS_STR
      HELPER_ADDRESS_INPUT_STR)
{
    udpfwd_server udpfwdServer;
    memset(&udpfwdServer, 0, sizeof(udpfwd_server));

    /* Validate the input parameters. */
    if (decode_server_param(&udpfwdServer, argv, DHCP_RELAY))
    {
        return udpfwd_helperaddressconfig(&udpfwdServer, UNSET);
    }
    else
    {
        return CMD_SUCCESS;
    }
}

/*-----------------------------------------------------------------------------
| Defun ip bootp-gateway unconfiguration
| Responsibility: Unset a bootp-gateway for a dhcp-relay
-----------------------------------------------------------------------------*/
DEFUN(no_ip_bootp_gateway_configuration,
      no_ip_bootp_gateway_configuration_cmd,
      "no ip bootp-gateway A.B.C.D ",
      NO_STR
      IP_STR
      BOOTP_GATEWAY_STR
      BOOTP_GATEWAY_INPUT_STR)
{
    struct in_addr addr;
    memset (&addr, 0, sizeof (struct in_addr));

    /* Validate bootp-gateway address. */
    if (inet_pton (AF_INET, (char*)argv[0], &addr)<= 0)
    {
        vty_out(vty, "Invalid IPv4 address.%s", VTY_NEWLINE);
        return CMD_SUCCESS;
    }

    if (!IS_VALID_IPV4(htonl(addr.s_addr)))
    {
        vty_out(vty,
                "Broadcast, multicast and loopback addresses "
                "are not allowed.%s",
                VTY_NEWLINE);
        return CMD_SUCCESS;
    }

    return dhcp_relay_bootp_gateway_config(argv[0], UNSET);

}
/*-----------------------------------------------------------------------------
| Defun for show ip helper-address
| Responsibility: Displays the helper-addresses of a dhcp-relay
-----------------------------------------------------------------------------*/
DEFUN(show_ip_helper_address_configuration,
      show_ip_helper_address_configuration_cmd,
      "show ip helper-address {interface (IFNAME | A.B )} ",
      SHOW_STR
      IP_STR
      SHOW_HELPER_ADDRESS_STR
      INTERFACE_STR
      IFNAME_STR
      SUBIFNAME_STR)
{
    return show_ip_helper_address_config(argv[0]);
}

/*-----------------------------------------------------------------------------
| Defun for show dhcp-relay bootp-gateway
| Responsibility: Displays the gateways of a dhcp-relay
-----------------------------------------------------------------------------*/
DEFUN(show_dhcp_relay_bootp_gateway_configuration,
      show_dhcp_relay_bootp_gateway_configuration_cmd,
      "show dhcp-relay bootp-gateway {interface (IFNAME | A.B )} ",
      SHOW_STR
      SHOW_DHCP_RELAY_STR
      BOOTP_GATEWAY_STR
      INTERFACE_STR
      IFNAME_STR
      SUBIFNAME_STR)
{
    return show_dhcp_relay_bootp_gateway_config(argv[0]);
}
