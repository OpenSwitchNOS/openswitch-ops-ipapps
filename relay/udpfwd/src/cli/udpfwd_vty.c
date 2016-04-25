/* UDP Broadcast Forwarder CLI commands
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
 * File: udpfwd_vty.c
 *
 * Purpose:  To add UDP Broadcast Forwarder CLI configuration and
 *           display commands.
 */
#include "udpfwd_vty_utils.h"

extern struct ovsdb_idl *idl;

#ifdef FTR_UDP_BCAST_FWD

VLOG_DEFINE_THIS_MODULE(udpfwd_vty);

/*-----------------------------------------------------------------------------
| Function       : udpfwd_globalconfig
| Responsibility : To enable/disable udp broadcast forwarding and dhcp-relay.
| Parameters     :
|        status  : If true, enable UDP broadcast forwarding/dhcp-relay and
|                  if false disable the UDP broadcast forwarding/dhcp-relay
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t udpfwd_globalconfig(const char *status)
{
    const struct ovsrec_system *ovs_row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    struct smap smap_status_value;
    enum ovsdb_idl_txn_status txn_status;
    char *key = NULL;

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

    key = SYSTEM_OTHER_CONFIG_MAP_UDP_BCAST_FWD_ENABLED;

    smap_clone(&smap_status_value, &ovs_row->other_config);

    /* Update the latest config status. */
    smap_replace(&smap_status_value, key, status);

    ovsrec_system_set_other_config(ovs_row, &smap_status_value);
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
| Function         : udp_bcast_server_row_lookup
| Responsibility   : To lookup for the record with port and
|                    default VRF in the UDP broadcast server table.
| Parameters       :
|        portname  : Name of the port
|        vrf_name  : Name of the VRF
| Return           : On success returns the UDP broadcast server row,
|                    On failure returns NULL
-----------------------------------------------------------------------------*/
const struct
ovsrec_udp_bcast_forwarder_server *udp_bcast_server_row_lookup
        (const char *portname, const char *vrf_name, udpfwd_server *udpfwdServ)
{
    const struct ovsrec_udp_bcast_forwarder_server *row_serv = NULL;

    OVSREC_UDP_BCAST_FORWARDER_SERVER_FOR_EACH (row_serv, idl)
    {
        if ((row_serv->src_port) &&
            (!strcmp(row_serv->src_port->name, portname)))
        {
            if ((row_serv->dest_vrf) &&
                (!strcmp(row_serv->dest_vrf->name, vrf_name)))
            {
                if (row_serv->udp_dport == udpfwdServ->udpPort)
                {
                    return row_serv;
                }
            }
        }
    }

    return row_serv;
}

/*-----------------------------------------------------------------------------
| Function         : udpfwd_serverconfig
| Responsibility   : set/unset UDP forward-protocol.
| Parameters       :
|      *udpfwdServ : Pointer containing user input details
|      set         : Flag to set or unset
| Return           : On success returns CMD_SUCCESS,
|                    On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
udpfwd_serverconfig(udpfwd_server *udpfwdServ, bool set)
{
    const struct ovsrec_udp_bcast_forwarder_server *row_serv = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status txn_status;
    bool isAddrMatch = false, isMaxEntries = false;
    UDPFWD_FEATURE type = UDP_BCAST_FORWARDER;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    /* lookup for the record in the UDP broadcast server table */
    row_serv = udp_bcast_server_row_lookup((char*)vty->index,
                                           DEFAULT_VRF_NAME, udpfwdServ);

    if (row_serv)
    {
        isAddrMatch = find_udpfwd_server_ip(row_serv->ipv4_ucast_server,
                                            row_serv->n_ipv4_ucast_server,
                                            udpfwdServ);
    }

    if (set)
    {
        isMaxEntries = server_address_maxcount_reached
                            ((char*)vty->index, type);
        if (isMaxEntries)
        {
            vty_out(vty, "%s: Entry not allowed as maximum "
                         "entries per interface exceeded.%s",
                         udpfwdServ->ipAddr, VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }

        if (NULL == row_serv)
        {
            /*
             * First set of UDP forward-protocol
             */

            row_serv = ovsrec_udp_bcast_forwarder_server_insert(status_txn);

            if (!row_serv)
            {
                VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }

            /* Update the UDP broadcast forwarder server table. */
            if (!udpfwd_setcommoncolumn((void *)row_serv, type))
            {
                cli_do_config_abort(status_txn);
                return CMD_OVSDB_FAILURE;
            }

            /* Update the dst udp port */
            ovsrec_udp_bcast_forwarder_server_set_udp_dport
                        (row_serv, udpfwdServ->udpPort);

            /* Update the protocol server IP */
            udpfwd_serverupdate((void *)row_serv, true, udpfwdServ, type);
        }
        else
        {
            if (!isAddrMatch)
            {
                /* set action */
                udpfwd_serverupdate((void *)row_serv, true, udpfwdServ, type);
            }
            else
            {
                /* Existing entry */
                vty_out(vty, "This entry already exists.%s", VTY_NEWLINE);
                cli_do_config_abort(status_txn);
                return CMD_SUCCESS;
            }
        }
    }
    else
    {
        if (NULL == row_serv)
        {
            vty_out(vty, "UDP forward-protocol is not present. %s", VTY_NEWLINE);
            cli_do_config_abort(status_txn);
            return CMD_SUCCESS;
        }
        else
        {
            if (!isAddrMatch)
            {
                vty_out(vty, "No such entries are present. %s", VTY_NEWLINE);
                cli_do_config_abort(status_txn);
                return CMD_SUCCESS;
            }
            else
            {
                /*
                 * If this is the last entry then after unset remove
                 * the complete row.
                 */
                if (row_serv->n_ipv4_ucast_server == 1)
                {
                    ovsrec_udp_bcast_forwarder_server_delete(row_serv);
                }
                else
                {
                    /* No set action */
                    udpfwd_serverupdate((void *)row_serv, false, udpfwdServ, type);
                }
            }
        }
    }

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
| Responsibility : To show the UDP Broadcast Forwarder configurations.
| Parameters     :
|       *ifname  : Interface name
| Return         : On success returns CMD_SUCCESS,
|                  On failure returns CMD_OVSDB_FAILURE
-----------------------------------------------------------------------------*/
int8_t
show_udp_forwarder_configuration (const char *ifname)
{
    const struct ovsrec_udp_bcast_forwarder_server *row_serv = NULL;
    const struct ovsrec_system *ovs_row = NULL;
    const struct ovsdb_datum *datum = NULL;
    char *udp_status = NULL ;
    int index = 0;
    size_t i = 0;
    bool entry_found = false;

    ovs_row = ovsrec_system_first(idl);
    if (ovs_row == NULL)
    {
        VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
        return CMD_OVSDB_FAILURE;
    }

    udp_status = (char *)smap_get(&ovs_row->other_config,
                         SYSTEM_OTHER_CONFIG_MAP_UDP_BCAST_FWD_ENABLED);
    if (!udp_status)
    {
        udp_status = "disabled";
    }
    else
    {
        if (!strcmp(udp_status, "true"))
            udp_status = "enabled";
        else
            udp_status = "disabled";
    }

    vty_out(vty, "%sUDP Broadcast Forwarder : %s%s", VTY_NEWLINE,
            udp_status, VTY_NEWLINE);
    vty_out(vty, "%s", VTY_NEWLINE);

    if (ifname)
    {
        vty_out(vty, "Interface: %s%s", ifname, VTY_NEWLINE);
        vty_out(vty, "%2sIP Forward Addresses%4sUDP Port %s", "", "",
                     VTY_NEWLINE);
        vty_out(vty, "%2s------------------------------- %s", "",
                     VTY_NEWLINE);
    }

    row_serv = ovsrec_udp_bcast_forwarder_server_first(idl);

    OVSREC_UDP_BCAST_FORWARDER_SERVER_FOR_EACH (row_serv, idl)
    {
        /* get the interface details. */
        if(row_serv->src_port)
        {
            if (ifname)
            {
                if(!strcmp(row_serv->src_port->name, ifname))
                {
                    if(row_serv->n_ipv4_ucast_server)
                    {
                        for (i = 0; i < row_serv->n_ipv4_ucast_server; i++)
                        {
                            /* get the UDP port number. */
                            datum =
                            ovsrec_udp_bcast_forwarder_server_get_udp_dport
                                    (row_serv, OVSDB_TYPE_INTEGER);
                            if ((NULL!=datum) && (datum->n >0))
                            {
                                index = datum->keys[0].integer;
                            }
                            vty_out(vty, "%2s%s%16s%d%s", "",
                                    row_serv->ipv4_ucast_server[i], "",
                                    index, VTY_NEWLINE);
                        }
                        entry_found = true;
                    }
                }
            }
            else
            {
                vty_out(vty, "Interface: %s%s",
                        row_serv->src_port->name, VTY_NEWLINE);
                vty_out(vty, "%2sIP Forward Addresses%4sUDP Port%s",
                        "", "", VTY_NEWLINE);
                vty_out(vty, "%2s-----------------------------%s",
                        "", VTY_NEWLINE);

                for (i = 0; i < row_serv->n_ipv4_ucast_server; i++)
                {
                    /* get the UDP port number. */
                    datum = ovsrec_udp_bcast_forwarder_server_get_udp_dport
                                                (row_serv, OVSDB_TYPE_INTEGER);
                    if ((NULL!=datum) && (datum->n >0))
                    {
                        index = datum->keys[0].integer;
                    }
                    vty_out(vty, "%2s%s%16s%d%s", "",
                                 row_serv->ipv4_ucast_server[i], "",
                                 index, VTY_NEWLINE);
                }
                entry_found = true;
            }
        }
    }

    if (entry_found == false)
    {
        if (ifname)
            vty_out(vty, "No forward address configuration found on "
                    "the interface %s.%s", ifname, VTY_NEWLINE);
        else
            vty_out(vty, "No forward address configuration found.%s",
                    VTY_NEWLINE);
    }

    return CMD_SUCCESS;
}

/*-----------------------------------------------------------------------------
| Defun for UDP broadcast forwarding
| Responsibility: UDP broadcast forwarding enable
-----------------------------------------------------------------------------*/
DEFUN ( cli_udp_bcast_fwd_enable,
        cli_udp_bcast_fwd_enable_cmd,
        "ip udp-bcast-forward",
        IP_STR
        UDPFWD_BCAST_STR )
{
      return udpfwd_globalconfig("true");
}

/*-----------------------------------------------------------------------------
| Defun for UDP broadcast forwarding
| Responsibility: UDP broadcast forwarding disable
-----------------------------------------------------------------------------*/
DEFUN ( cli_udp_bcast_fwd_disable,
        cli_udp_bcast_fwd_disable_cmd,
        "no ip udp-bcast-forward",
        NO_STR
        IP_STR
        UDPFWD_BCAST_STR )
{
      return udpfwd_globalconfig("false");
}

/*-----------------------------------------------------------------------------
| Defun UDP forward-protocol configuration
| Responsibility: Set UDP forward-protocol config
-----------------------------------------------------------------------------*/
DEFUN ( cli_udpf_bcast_set_proto,
        intf_set_udpf_proto_cmd,
        "ip forward-protocol udp A.B.C.D (INTEGER | WORD)",
        IP_STR
        UDPFWD_PROTO_STR
        UDPFWD_PROTO_STR
        UDPFWD_SERV_ADDR_STR
        UDPFWD_PORT_NUM_STR
        UDPFWD_PROTO_NAME_STR )
{
    udpfwd_server udpfServer;
    memset(&udpfServer, 0, sizeof(udpfwd_server));

    /* Validate the input parameters. */
    if (decode_server_param(&udpfServer, argv, UDP_BCAST_FORWARDER))
    {
        return udpfwd_serverconfig(&udpfServer, SET);
    }
    else
    {
        return CMD_SUCCESS;
    }
}

/*-----------------------------------------------------------------------------
| Defun UDP forward-protocol configuration
| Responsibility: Unset UDP forward-protocol config
-----------------------------------------------------------------------------*/
DEFUN ( cli_udpf_bcast_no_set_proto,
        intf_no_set_udpf_proto_cmd,
        "no ip forward-protocol udp A.B.C.D (INTEGER | WORD)",
        NO_STR
        IP_STR
        UDPFWD_PROTO_STR
        UDPFWD_PROTO_STR
        UDPFWD_SERV_ADDR_STR
        UDPFWD_PORT_NUM_STR
        UDPFWD_PROTO_NAME_STR )
{
    udpfwd_server udpfServer;
    memset(&udpfServer, 0, sizeof(udpfwd_server));

    /* Validate the input parameters. */
    if (decode_server_param(&udpfServer, argv, UDP_BCAST_FORWARDER))
    {
        return udpfwd_serverconfig(&udpfServer, UNSET);
    }
    else
    {
        return CMD_SUCCESS;
    }
}

/*-----------------------------------------------------------------------------
| Defun show ip forward-protocol
| Responsibility: Show the UDP broadcast forward configurations
-----------------------------------------------------------------------------*/
DEFUN ( cli_show_udpf_forward_protocol,
        cli_show_udpf_forward_protocol_cmd,
        "show ip forward-protocol {interface (IFNAME | A.B)}",
        SHOW_STR
        IP_STR
        UDPFWD_PROTO_STR
        UDPFWD_INT_STR
        UDPFWD_IFNAME_STR
        SUBIFNAME_STR )
{
    return show_udp_forwarder_configuration(argv[0]);
}
#endif /* FTR_UDP_BCAST_FWD */
