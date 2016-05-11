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

/* dhcp receiver thread handle */
pthread_t dhcpRecv_thread;


/* FIXME: Temporary addition. Once macro is merged following shall be removed */
#define SYSTEM_DHCP_CONFIG_MAP_V6RELAY_ENABLED    "v6relay_enabled"
#define SYSTEM_DHCP_CONFIG_MAP_V6RELAY_OPTION79_ENABLED "dhcpv6_relay_option79_enabled"

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
int create_socket(void)
{
    VLOG_INFO("Creating send/receive socket");
    int on = 1, dhcpv6Sock = -1;
    struct sockaddr_in6 sock6Addr;
    memset(&sock6Addr, 0, sizeof(sock6Addr));
    dhcpv6Sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

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
        close(dhcpv6Sock);
        return -1;
    }

    VLOG_INFO("UDP send/receive socket created successfully");
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

    /* Create UDP socket */
    if (-1 == (sock = create_socket()))
    {
        VLOG_FATAL("Failed to create socket");
        return false;
    }

    dhcpv6_relay_ctrl_cb_p->dhcpv6_relaySockFd = sock;

    /* Allocate memory for packet recieve buffer */
    dhcpv6_relay_ctrl_cb_p->rcvbuff = (char *)
                calloc(DHCPV6_RELAY_MSG_SIZE, sizeof(char));

    if (NULL == dhcpv6_relay_ctrl_cb_p->rcvbuff)
    {
        close(dhcpv6_relay_ctrl_cb_p->dhcpv6_relaySockFd);
        VLOG_FATAL(" Memory allocation for receive buffer failed\n");
        return false;
    }

     /* Create DHCPv6 relay receiver thread */
    retVal = pthread_create(&dhcpRecv_thread, (pthread_attr_t *)NULL,
                            dhcpv6r_recv, NULL);
    if (0 != retVal)
    {
        free(dhcpv6_relay_ctrl_cb_p->rcvbuff);
        close(dhcpv6_relay_ctrl_cb_p->dhcpv6_relaySockFd);
        cmap_destroy(&dhcpv6_relay_ctrl_cb_p->serverHashMap);
        VLOG_FATAL("Failed to create DHCPv6 packet receiver thread : %d",
                 retVal);
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
        /* FIXME: Log to be removed after initial round of testing */
        VLOG_DBG("No table changes updates");
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
    VLOG_INFO(" dhcpv6_relay_reconfigure");

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
    if (0 < dhcpv6_relay_ctrl_cb_p->dhcpv6_relaySockFd)
        close(dhcpv6_relay_ctrl_cb_p->dhcpv6_relaySockFd);

    if (NULL != dhcpv6_relay_ctrl_cb_p->rcvbuff)
        free(dhcpv6_relay_ctrl_cb_p->rcvbuff);

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
    return;
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
 * Function      : dhcpv6r_init
 * Responsiblity : idl create/registration, module initialization and
 *                 unixctl registrations
 * Parameters    : none
 * Return        : true - on success
 *                 false - on failure
 */
bool dhcpv6r_init()
{
    VLOG_INFO("DHCPV6 relay init");

    ovsdb_idl_add_column(idl, &ovsrec_system_col_dhcp_config);

    /* Register for DHCP_Relay table updates */
    ovsdb_idl_add_table(idl, &ovsrec_table_dhcp_relay);
    ovsdb_idl_add_column(idl, &ovsrec_dhcp_relay_col_port);
    ovsdb_idl_add_column(idl,
                        &ovsrec_dhcp_relay_col_vrf);
    ovsdb_idl_add_column(idl,
                        &ovsrec_dhcp_relay_col_ipv6_ucast_server);

    #if 0 /* Code changes to be enabled once schema is pushed */
    ovsdb_idl_add_column(idl,
                        &ovsrec_dhcp_relay_col_ipv6_mcast_server);
    #endif

    /* Register for port table for dhcp_relay_statistics update */
    ovsdb_idl_add_table(idl, &ovsrec_table_port);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_name);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_dhcp_relay_statistics);

    /* register for ipv6 address */
    ovsdb_idl_add_column(idl, &ovsrec_port_col_ip6_address);

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
