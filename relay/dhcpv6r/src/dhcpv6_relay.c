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
 * File: dhcpv6_relay.c
 *
 */

/*
 * This daemon handles the following functionality:
 * - Dhcpv6-relay agent for IPv6.
 */

/* Linux includes */
#include <errno.h>
#include <timeval.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <semaphore.h>

/* Dynamic string */
#include <dynamic-string.h>

/* OVSDB Includes */
#include "config.h"
#include "command-line.h"
#include "stream.h"
#include "daemon.h"
#include "fatal-signal.h"
#include "dirs.h"
#include "poll-loop.h"
#include "unixctl.h"
#include "openvswitch/vconn.h"
#include "openvswitch/vlog.h"
#include "openswitch-dflt.h"
#include "coverage.h"
#include "svec.h"

#include "relay_common.h"
#include "dhcpv6_relay.h"

#ifdef FTR_DHCPV6_RELAY

struct ovsdb_idl_index_cursor ipv6_ucast_cursor;

/* dhcp receiver thread handle */
pthread_t dhcp6_recv_thread;

/* FIXME: Temporary addition. Once macro is merged following shall be removed */
#define SYSTEM_DHCP_CONFIG_MAP_V6RELAY_ENABLED    "v6relay_enabled"
#define SYSTEM_DHCP_CONFIG_MAP_V6RELAY_OPTION79_ENABLED "v6relay_option79_enabled"

/*
 * Global variable declarations.
 */

/* DHCPv6 relay global configuration context */
DHCPV6_RELAY_CTRL_CB dhcpv6_relay_ctrl_cb;
DHCPV6_RELAY_CTRL_CB *dhcpv6_relay_ctrl_cb_p = &dhcpv6_relay_ctrl_cb;

VLOG_DEFINE_THIS_MODULE(dhcpv6_relay);

/* FIXME: Socket needs to be created per VRF basis based on the
 * configuration
 */

/*
 * Function      : create_recv_socket
 * Responsiblity : receive socket creation for dhcpv6 relay feature
 * Parameters    : none
 * Return        : socket descriptor on success
 *                 -1 on failure
 */
int create_recv_socket(void)
{
    VLOG_INFO("Creating send/receive socket");
    int on = 1, dhcpv6Sock = -1;
    struct sockaddr_in6 sock6Addr;
    memset(&sock6Addr, 0, sizeof(sock6Addr));
    dhcpv6Sock = socket(AF_INET6, SOCK_RAW, IPPROTO_UDP);

    if (-1 == dhcpv6Sock) {
        VLOG_ERR("Failed to create UDP socket");
        return -1;
    }

    /* Set the requisite socket options. */
    /* IPV6_RERCVPKTINFO */
    if (setsockopt(dhcpv6Sock, IPPROTO_IPV6, IPV6_RECVPKTINFO,
                    (char*)&on, sizeof(on)) < 0)
    {
        VLOG_ERR("Failed to set IPV6_RECVPKTINFO socket option");
        close(dhcpv6Sock);
        return -1;
    }

    sock6Addr.sin6_family = PF_INET6;
    sock6Addr.sin6_port = htons((uint16_t)DHCPV6_RELAY_PORT);
    sock6Addr.sin6_addr = in6addr_any;

    /* Bind to the address. */
    if (bind(dhcpv6Sock, (struct sockaddr*)&sock6Addr, sizeof(sock6Addr)) < 0)
    {
        VLOG_ERR("Failed to bind to ipv6 address");
        close(dhcpv6Sock);
        return -1;
    }

    VLOG_INFO("Raw receive socket created successfully");
    return dhcpv6Sock;
}

/*
 * Function      : create_send_socket
 * Responsiblity : send socket creation for dhcpv6 relay feature
 * Parameters    : none
 * Return        : socket descriptor on success
 *                 -1 on failure
 */
int create_send_socket()
{
    VLOG_INFO("Creating send socket");
    int dhcpv6Sock = -1;
    struct sockaddr_in6 sock6Addr;
    memset(&sock6Addr, 0, sizeof(sock6Addr));
    dhcpv6Sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

    if (-1 == dhcpv6Sock) {
        VLOG_ERR("Failed to create UDP socket");
        return -1;
    }

    sock6Addr.sin6_family = PF_INET6;
    sock6Addr.sin6_port = 0;
    sock6Addr.sin6_addr = in6addr_any;

    /* Bind to the address. */
    if (bind(dhcpv6Sock, (struct sockaddr*)&sock6Addr, sizeof(sock6Addr)) < 0)
    {
        VLOG_ERR("Failed to bind to ipv6 address");
        close(dhcpv6Sock);
        return -1;
    }

    VLOG_INFO("send socket created successfully");
    return dhcpv6Sock;
}

/*
 * Function      : dhcpv6r_module_init
 * Responsiblity : Initialization routine for dhcpv6 relay feature
 * Parameters    : none
 * Return        : true, on success
 *                 false, on failure
 */
bool dhcpv6r_module_init(void)
{
    int32_t retVal, sock;
    memset(dhcpv6_relay_ctrl_cb_p, 0, sizeof(DHCPV6_RELAY_CTRL_CB));

    /* DB access semaphore initialization */
    retVal = sem_init(&dhcpv6_relay_ctrl_cb_p->waitSem, 0, 1);
    if (0 != retVal) {
        VLOG_FATAL("Failed to initilize the semaphore, retval : %d (errno : %d)",
                   retVal, errno);
        return false;
    }

    /* Initialize server hash table */
    shash_init(&dhcpv6_relay_ctrl_cb_p->intfHashTable);

    /* Initialize server hash map */
    cmap_init(&dhcpv6_relay_ctrl_cb_p->serverHashMap);

    /* default values */
    dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_enable = false;
    dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_option79_enable = false;

    /* Store the DHCPv6 Relay Agents and Servers IPv6 address. */
    if (inet_pton(PF_INET6, DHCPV6_ALLAGENTS,
                 (void *)&dhcpv6_relay_ctrl_cb_p->agentIpv6Address) <= 0)
    {
        VLOG_FATAL("Could not register DHCPv6 All Relay Agents and "
                "Servers address");
        return false;
    }

    /* Create Raw receive socket */
    if (-1 == (sock = create_recv_socket()))
    {
        VLOG_FATAL("Failed to create receive socket");
        return false;
    }

    dhcpv6_relay_ctrl_cb_p->dhcpv6r_recvSock = sock;

    /* Create UDP send socket */
    if (-1 == (sock = create_send_socket()))
    {
        VLOG_FATAL("Failed to create  send socket");
        return false;
    }

    dhcpv6_relay_ctrl_cb_p->dhcpv6r_sendSock = sock;

    /* Allocate memory for packet recieve buffer */
    dhcpv6_relay_ctrl_cb_p->rcvbuff = (char *)
                calloc(DHCPV6_RELAY_MSG_SIZE, sizeof(char));

    if (NULL == dhcpv6_relay_ctrl_cb_p->rcvbuff)
    {
        close(dhcpv6_relay_ctrl_cb_p->dhcpv6r_recvSock);
        close(dhcpv6_relay_ctrl_cb_p->dhcpv6r_sendSock);
        VLOG_FATAL("Memory allocation for receive buffer failed");
        return false;
    }

     /* Create DHCPv6 relay receiver thread */
    retVal = pthread_create(&dhcp6_recv_thread, (pthread_attr_t *)NULL,
                            dhcpv6r_recv, NULL);
    if (0 != retVal)
    {
        free(dhcpv6_relay_ctrl_cb_p->rcvbuff);
        close(dhcpv6_relay_ctrl_cb_p->dhcpv6r_recvSock);
        close(dhcpv6_relay_ctrl_cb_p->dhcpv6r_sendSock);
        shash_destroy(&dhcpv6_relay_ctrl_cb_p->intfHashTable);
        cmap_destroy(&dhcpv6_relay_ctrl_cb_p->serverHashMap);
        VLOG_FATAL("Failed to create DHCPv6 packet receiver thread : %d",
                 retVal);
        return false;
    }

    return true;
}

/*
 * Function      : dhcpv6r_get_client_mac
 *
 * Responsibilty : Checks if the Client is found in the Neighbor cache. If present
 *                 copy the mac address from neighbor table.
 * Parameters    : in6Addr - ipv6 address of the client for which MAC is needed.
 *                 clientMacOpt - pointer to structure containing MAC hwtype and MAC
 * Returns       : true if the client is found in local cache
 *                 false otherwise.
 */
bool dhcpv6r_get_client_mac(struct in6_addr *in6Addr, dhcpv6_clientmac_Opt_t *clientMacOpt)
{
    /*FIXME : need to explicitly issue ping so that neighbor table has value */
    const struct ovsrec_neighbor *ovs_nbr = NULL;
    char ipv6Str[INET6_ADDRSTRLEN] = {0}, nbr_mac[19];
    char *mac = NULL;
    int i = 0;

    memset(&nbr_mac, 0, sizeof(nbr_mac));
    ovs_nbr = ovsrec_neighbor_first(idl);

    inet_ntop(AF_INET6, (void *)in6Addr, ipv6Str, INET6_ADDRSTRLEN);

    OVSREC_NEIGHBOR_FOR_EACH (ovs_nbr, idl) {
        if (strncmp((ovs_nbr)->ip_address, ipv6Str, strlen(ipv6Str)) == 0)
        {
            /* Split based on : */
            strncpy(nbr_mac, ovs_nbr->mac, strlen(ovs_nbr->mac));
            mac = strtok(ovs_nbr->mac, ":");
            clientMacOpt->hwtype  = htons(ETHER_ADDR_TYPE);
            while (mac != NULL)
            {
                clientMacOpt->macaddr[i] = (uint8_t) strtol (mac, (char **) NULL, 16);
                i++;
                mac = strtok(NULL, ":");
            }
        }
    }

    if (strlen(nbr_mac) == 0)
    {
        VLOG_ERR(" Entry is not present in neighbor table");
        return false;
    }
    return true;
}

/*
 * Function      : dhcpv6r_process_globalconfig_update
 * Responsiblity : Process system table update notifications related to
 *                 dhcpv6 relay from OVSDB.
 * Parameters    : none
 * Return        : none
 */
void dhcpv6r_process_globalconfig_update(void)
{
    const struct ovsrec_system *system_row = NULL;

    bool dhcpv6_relay_enabled = false, dhcpv6_relay_option79 = false;
    char *value;

    system_row = ovsrec_system_first(idl);
    if (NULL == system_row) {
        VLOG_ERR("Unable to find system record in idl cache");
        return;
    }

    /* If system table row is not modified do nothing */
    if (!OVSREC_IDL_IS_ROW_MODIFIED(system_row, idl_seqno))
    {
        return;
    }

    /* Check if dhcpv6-relay global configuration is changed */
    if (OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_system_col_dhcp_config,
                                   idl_seqno)) {
        value = (char *)smap_get(&system_row->dhcp_config,
                                 SYSTEM_DHCP_CONFIG_MAP_V6RELAY_ENABLED);

        /* DHCPv6-Relay is disabled by default.*/
        if (value && (!strncmp(value, "true", strlen(value)))) {
            dhcpv6_relay_enabled = true;
        }

        /* Check if dhcpv6-relay global configuration is changed */
        if (dhcpv6_relay_enabled != dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_enable) {
            VLOG_INFO("DHCPv6-Relay global config change. old : %d, new : %d",
                     dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_enable, dhcpv6_relay_enabled);
            dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_enable = dhcpv6_relay_enabled;
        }

        /* Check if dhcpv6-relay option 79 value is changed */
        value = (char *)smap_get(&system_row->dhcp_config,
                                 SYSTEM_DHCP_CONFIG_MAP_V6RELAY_OPTION79_ENABLED);
        if (value && (!strncmp(value, "true", strlen(value)))) {
            dhcpv6_relay_option79 = true;
        }

        if (dhcpv6_relay_option79 != dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_option79_enable) {
            VLOG_INFO("DHCPv6-Relay option 79 global config change. old : %d, new : %d",
                     dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_option79_enable, dhcpv6_relay_option79);
            dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_option79_enable = dhcpv6_relay_option79;
        }
    }
    return;
}

/*
 * Function      : dhcpv6r_server_config_update
 * Responsiblity : Process dhcp_relay table update notifications from OVSDB for the
 *                 dhcpv6 relay configuration changes.
 * Parameters    : none
 * Return        : none
 */
void dhcpv6r_server_config_update(void)
{
    const struct ovsrec_dhcp_relay *rec_first = NULL;
    const struct ovsrec_dhcp_relay *rec = NULL;
    const struct ovsrec_port *port_row = NULL;
    DHCPV6_RELAY_INTERFACE_NODE_T *intfNode = NULL;
    struct shash_node *node;

    /* Check for server configuration changes */
    rec_first = ovsrec_dhcp_relay_first(idl);
    if (NULL == rec_first) {
        /* Check if last entry from the table is deleted */
        dhcpv6r_handle_row_delete(idl);
        return;
    }

    if (!OVSREC_IDL_ANY_TABLE_ROWS_INSERTED(rec_first, idl_seqno)
        && !OVSREC_IDL_ANY_TABLE_ROWS_DELETED(rec_first, idl_seqno)
        && !OVSREC_IDL_ANY_TABLE_ROWS_MODIFIED(rec_first, idl_seqno)) {
        return;
    }

    /* Process row delete notification */
    if (OVSREC_IDL_ANY_TABLE_ROWS_DELETED(rec_first, idl_seqno)) {
      dhcpv6r_handle_row_delete(idl);
    }

    /* Process row insert/modify notifications */
    OVSREC_DHCP_RELAY_FOR_EACH (rec, idl) {
        if (OVSREC_IDL_IS_ROW_INSERTED(rec, idl_seqno)
            || OVSREC_IDL_IS_ROW_MODIFIED(rec, idl_seqno)) {
            dhcpv6r_handle_config_change(rec, idl_seqno);
        }
    }
    /*FIXME: handle port deletion */

    /* FIXME : For now, primary ipv6 address is being used as source interface address */
    port_row = ovsrec_port_first(idl);
    if (OVSREC_IDL_IS_ROW_MODIFIED(port_row, idl_seqno))
    {
        if(OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_port_col_ip6_address,
                                      idl_seqno)) {
            node = shash_find(&dhcpv6_relay_ctrl_cb_p->intfHashTable, port_row->name);
            if (NULL != node) {
                intfNode = (DHCPV6_RELAY_INTERFACE_NODE_T *)node->data;
                strncpy(intfNode->ip6_address, port_row->ip6_address,
                        strlen(port_row->ip6_address));
            }
        }
    }
    return;
}

/*
 * Function      : dhcpv6_relay_reconfigure
 * Responsiblity : Process the table update notifications from OVSDB for the
 *                 configuration changes.
 * Parameters    : none
 * Return        : none
 */
void dhcpv6r_reconfigure()
{
    /* Check for global configuration changes in system table */
    dhcpv6r_process_globalconfig_update();

    /* Process dhcp_relay table updates */
    dhcpv6r_server_config_update();

    return;
}

/*
 * Function      : dhcpv6r_exit
 * Responsiblity : dhcpv6_relay module cleanup before exit
 * Parameters    : -
 * Return        : none
 */
void dhcpv6r_exit(void)
{
    /* free memory for packet receive buffer */
    if (0 < dhcpv6_relay_ctrl_cb_p->dhcpv6r_recvSock)
        close(dhcpv6_relay_ctrl_cb_p->dhcpv6r_recvSock);

    if (0 < dhcpv6_relay_ctrl_cb_p->dhcpv6r_sendSock)
        close(dhcpv6_relay_ctrl_cb_p->dhcpv6r_sendSock);

    if (NULL != dhcpv6_relay_ctrl_cb_p->rcvbuff)
        free(dhcpv6_relay_ctrl_cb_p->rcvbuff);

    shash_destroy(&dhcpv6_relay_ctrl_cb_p->intfHashTable);
    cmap_destroy(&dhcpv6_relay_ctrl_cb_p->serverHashMap);

    return;
}

/*
 * Function      : dhcpv6r_interface_dump
 * Responsiblity : Function dumps information about server IPv6 address,
 *                 outgoing interface name for each
 *                 interface into dynamic string ds.
 * Parameters    : ds - output buffer
 *                 node - interface node.
 * Return        : none
 */
static void dhcpv6r_interface_dump(struct shash_node *node,
                                  struct ds *ds)
{
    DHCPV6_RELAY_SERVER_T *server = NULL, **serverArray = NULL;
    DHCPV6_RELAY_INTERFACE_NODE_T *intfNode = NULL;
    int32_t iter = 0;

    intfNode = (DHCPV6_RELAY_INTERFACE_NODE_T *)node->data;
    serverArray = intfNode->serverArray;

    /* Print all configured server IPv6 addresses */
    ds_put_format(ds, "Interface %s: %d\n", intfNode->portName,
                      intfNode->addrCount);

    for(iter = 0; iter < intfNode->addrCount; iter++)
    {
        server = serverArray[iter];
        ds_put_format(ds, "%s,%d,egress %s ", server->ipv6_address,
            server->ref_count, server->egressIfName);
    }

    /* Print dhcpv6-relay statistics */
    ds_put_format(ds, "client request dropped packets = %d\n",
                  DHCPV6R_CLIENT_DROPS(intfNode));
    ds_put_format(ds, "client request valid packets = %d\n",
                  DHCPV6R_CLIENT_SENT(intfNode));
    ds_put_format(ds, "server request dropped packets = %d\n",
                  DHCPV6R_SERVER_DROPS(intfNode));
    ds_put_format(ds, "server request valid packets = %d\n",
                  DHCPV6R_SERVER_SENT(intfNode));

}

/*
 * Function      : dhcpv6r_unixctl_dump
 * Responsiblity : DHCPv6 Relay module core dump callback
 * Parameters    : conn - unixctl socket connection
 *                 argc, argv - function parameters
 *                 aux - aux connection data
 * Return        : none
 */
void dhcpv6r_unixctl_dump(struct unixctl_conn *conn, int argc OVS_UNUSED,
                   const char *argv[] OVS_UNUSED, void *aux OVS_UNUSED)
{
    struct ds ds = DS_EMPTY_INITIALIZER;
    struct shash_node *node, *temp;

    /*
     * Parse unxictl arguments.
     * second argument is always interface name.
     * ex : ovs-appctl -t ops-relay dhcpv6r/dump int 1
     */

    ds_put_format(&ds, "DHCPv6 Relay : %d\n", dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_enable);
    ds_put_format(&ds, "DHCPv6 Relay Option79 : %d\n",
        dhcpv6_relay_ctrl_cb_p->dhcpv6_relay_option79_enable);

    if (!argv[2]) {
        /* dump all interfaces */
        SHASH_FOR_EACH(temp, &dhcpv6_relay_ctrl_cb_p->intfHashTable)
            dhcpv6r_interface_dump(temp, &ds);
    }
    else {
        node = shash_find(&dhcpv6_relay_ctrl_cb_p->intfHashTable, argv[2]);
        if (NULL == node) {
            ds_put_format(&ds, "No servers are configured on"
            " this interface :%s\n", argv[2]);
            return;
        }
        dhcpv6r_interface_dump(node, &ds);
    }

    unixctl_command_reply(conn, ds_cstr(&ds));
    ds_destroy(&ds);
}

/*
 * Function      : ipv6_ucast_addr_cmp
 * Responsiblity : comparator function for ipv6_ucast_address
 * Parameters    : none
 * Return        : ovsdb_idl_index_strcmp result
 */
int ipv6_ucast_addr_cmp(const void *entry1, const void *entry2)
{
    char *ip1 = *(((struct ovsrec_dhcp_relay *)entry1)->ipv6_ucast_server);
    char *ip2 = *(((struct ovsrec_dhcp_relay *)entry2)->ipv6_ucast_server);

    return (ovsdb_idl_index_strcmp(ip1, ip2));
}

/*
 * Function      : dhcpv6r_init
 * Responsiblity : idl create/registration, module initialization and
 *                 unixctl registrations
 * Parameters    : none
 * Return        : true - on success
 *                 false - on failure
 */
bool dhcpv6r_init()
{
    struct ovsdb_idl_index *ipv6_ucast_index;

    ovsdb_idl_add_column(idl, &ovsrec_system_col_dhcp_config);

    /* Register for DHCP_Relay table updates */
    ovsdb_idl_add_table(idl, &ovsrec_table_dhcp_relay);
    ovsdb_idl_add_column(idl, &ovsrec_dhcp_relay_col_port);
    ovsdb_idl_add_column(idl,
                        &ovsrec_dhcp_relay_col_vrf);
    ovsdb_idl_add_column(idl,
                        &ovsrec_dhcp_relay_col_ipv6_ucast_server);
    /* FIXME: This will be removed after schema changes got merged. */
#if 0
    ovsdb_idl_add_column(idl,
                        &ovsrec_dhcp_relay_col_ipv6_mcast_server);
    #endif

    ovsdb_idl_add_table(idl, &ovsrec_table_port);
    /* register for ipv6 address primary and secondary */
    ovsdb_idl_add_column(idl, &ovsrec_port_col_ip6_address);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_ip6_address_secondary);

    /* Register for mac address of client, neighbor table */
    ovsdb_idl_add_table(idl, &ovsrec_table_neighbor);
    ovsdb_idl_add_column(idl, &ovsrec_neighbor_col_mac);
    ovsdb_idl_add_column(idl, &ovsrec_neighbor_col_ip_address);

    /* Create index */
    ipv6_ucast_index = ovsdb_idl_create_index(idl, &ovsrec_table_dhcp_relay,
                                         "ipv6_ucast_index");

    if (ipv6_ucast_index) {
        ovsdb_idl_index_add_column
            (ipv6_ucast_index, &ovsrec_dhcp_relay_col_ipv6_ucast_server,
            OVSDB_INDEX_ASC, ipv6_ucast_addr_cmp);

        /* Create a cursor to perform queries to the index defined */
        if (!ovsdb_idl_initialize_cursor(idl, &ovsrec_table_dhcp_relay,
                                  "ipv6_ucast_index",
                                  &ipv6_ucast_cursor)) {
            VLOG_ERR("Failed to initialize the cursor used to query the ipv6 "
               "ucast column");
        }
    } else {
        VLOG_ERR("Failed to create an index for the dhcp relay table");
    }

    /* Initialize module data structures */
    if (true != dhcpv6r_module_init())
    {
        VLOG_FATAL("DHCPv6 relay module failed to initialize");
        return false;
    }

    unixctl_command_register("dhcpv6r/dump", "", 0, 2,
                             dhcpv6r_unixctl_dump, NULL);
    return true;
}
#endif /* FTR_DHCPV6_RELAY */
