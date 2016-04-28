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
 * File: udpfwd.c
 *
 */

/*
 * This daemon handles the following functionality:
 * - Dhcp-relay agent for IPv4.
 * - UDP-Broadcast-Forwarder for IPv4.
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
#include <sys/socket.h>
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
#include "udpfwd_util.h"
#include "udpfwd.h"

/*
 * Global variable declarations.
 */

/* udp socket receiver thread handle */
pthread_t udpBcastRecv_thread;

/* Structure to store value of unixctl arguments */
struct dump_params {
    char *ifName; /* Name of the Interface */
    uint16_t port; /* udp destination port */
};

/* UDP forwarder global configuration context */
UDPFWD_CTRL_CB udpfwd_ctrl_cb;
UDPFWD_CTRL_CB *udpfwd_ctrl_cb_p = &udpfwd_ctrl_cb;

VLOG_DEFINE_THIS_MODULE(udpfwd);

/**
 * External Function Declarations
 */
extern void *udp_packet_recv(void *args);

/*
 * Function      : udpfwd_module_init
 * Responsiblity : Initialization routine for udp broadcast forwarder module
 * Parameters    : none
 * Return        : sockfd, on success
 *                 -1, on failure
 */
/* FIXME: Socket needs to be created per VRF basis based on the
 * configuration
 */
int create_udp_socket(void)
{
    int32_t sock = -1;
    int32_t val = 1;
    int retVal = -1;

    VLOG_INFO("Creating UDP send socket");

    /* Create socket to send UDP packets to client or server */
    sock = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
    if (-1 == sock) {
        VLOG_ERR("Failed to create UDP socket");
        return -1;
    }

    retVal = setsockopt(sock, IPPROTO_IP, IP_PKTINFO,
                        (char *)&val, sizeof(val));
    if (0 != retVal) {
        VLOG_ERR("Failed to set IP_PKTINFO socket option : %d", retVal);
        close(sock);
        return -1;
    }

    retVal = setsockopt(sock, IPPROTO_IP, IP_HDRINCL,
                        (char *)&val, sizeof(val));
    if (0 != retVal) {
        VLOG_ERR("Failed to set IP_HDRINCL socket option : %d", retVal);
        close(sock);
        return -1;
    }

    retVal = setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
                       (char *)&val, sizeof (val));
    if (0 != retVal) {
        VLOG_ERR("Failed to set broadcast socket option : %d", retVal);
        close(sock);
        return -1;
    }

    VLOG_INFO("UDP send socket created successfully");
    return sock;
}

/*
 * Function      : udpfwd_set_default_config
 * Responsiblity : Set default configuration for the features
 * Parameters    : none
 * Return        : none
 */
void udpfwd_set_default_config(void)
{
#ifdef FTR_UDP_BCAST_FWD
    /* Set UDP-broadcast-forwarding enabled */
    set_feature_status(&(udpfwd_ctrl_cb_p->feature_config.config),
                       UDP_BCAST_FORWARDER, DISABLE);
#endif /* FTR_UDP_BCAST_FWD */


#ifdef FTR_DHCP_RELAY
    /* Set DHCP-Relay enabled */
    set_feature_status(&(udpfwd_ctrl_cb_p->feature_config.config),
                       DHCP_RELAY, ENABLE);

    /* Set DHCP hop-count-increment enabled */
    set_feature_status(&(udpfwd_ctrl_cb_p->feature_config.config),
                       DHCP_RELAY_HOP_COUNT_INCREMENT, ENABLE);

    /* Set DHCP-Relay Option82 disabled  */
    set_feature_status(&(udpfwd_ctrl_cb_p->feature_config.config),
                       DHCP_RELAY_OPTION82, DISABLE);

    /* Set DHCP-Relay validation disabled */
    set_feature_status(&(udpfwd_ctrl_cb_p->feature_config.config),
                       DHCP_RELAY_OPTION82_VALIDATE, DISABLE);

    /* Set DHCP-Relay option82 policy keep */
    udpfwd_ctrl_cb_p->feature_config.policy = REPLACE;

    /* Set DHCP-Relay option82 remote-id to mac */
    udpfwd_ctrl_cb_p->feature_config.r_id = REMOTE_ID_MAC;

    /* Set statistics refresh interval */
    udpfwd_ctrl_cb_p->stats_interval = STATS_UPDATE_DEFAULT_INTERVAL;
#endif /* FTR_DHCP_RELAY */

    return;
}

/*
 * Function      : udpfwd_module_init
 * Responsiblity : Initialization routine for udp broadcast forwarder module
 * Parameters    : none
 * Return        : true, on success
 *                 false, on failure
 */
bool udpfwd_module_init(void)
{
    int32_t retVal;
    int32_t sock;

    memset(udpfwd_ctrl_cb_p, 0, sizeof(UDPFWD_CTRL_CB));

    /* Set feature default configuration status */
    udpfwd_set_default_config();

    /* DB access semaphore initialization */
    retVal = sem_init(&udpfwd_ctrl_cb_p->waitSem, 0, 1);
    if (0 != retVal) {
        VLOG_FATAL("Failed to initilize the semaphore, retval : %d (errno : %d)",
                   retVal, errno);
        return false;
    }

    /* Create UDP socket */
    if (-1 == (sock = create_udp_socket()))
    {
        VLOG_FATAL("Failed to create broadcast socket");
        return false;
    }

    udpfwd_ctrl_cb_p->udpSockFd = sock;

    /* Allocate memory for packet recieve buffer */
    udpfwd_ctrl_cb_p->rcvbuff = (char *) calloc(RECV_BUFFER_SIZE, sizeof(char));

    if (NULL == udpfwd_ctrl_cb_p->rcvbuff)
    {
        close(udpfwd_ctrl_cb_p->udpSockFd);
        VLOG_FATAL(" Memory allocation for receive buffer failed\n");
        return false;
    }

    /* Initialize server hash table */
    shash_init(&udpfwd_ctrl_cb_p->intfHashTable);

    /* Initialize server hash map */
    cmap_init(&udpfwd_ctrl_cb_p->serverHashMap);

    /* Create UDP broadcast receiver thread */
    retVal = pthread_create(&udpBcastRecv_thread, (pthread_attr_t *)NULL,
                            udp_packet_recv, NULL);
    if (0 != retVal)
    {
        free(udpfwd_ctrl_cb_p->rcvbuff);
        close(udpfwd_ctrl_cb_p->udpSockFd);
        cmap_destroy(&udpfwd_ctrl_cb_p->serverHashMap);
        VLOG_FATAL("Failed to create UDP broadcast packet receiver thread : %d",
                 retVal);
        return false;
    }

    return true;
}

/*
 * Function      : update_feature_state
 * Responsiblity : Update config state of a feature.
 * Parameters    : feature - feature enum value
 *                 state - latest config state of the feature
 * Return        : none
 */
void update_feature_state(UDPFWD_FEATURE feature, FEATURE_STATUS state)
{
    FEATURE_STATUS prev_state;
    feature_bmap *config = &(udpfwd_ctrl_cb_p->feature_config.config);

    /* Cache previous config state of the feature */
    prev_state = get_feature_status(*config, feature);

    /* If there is a change in config state, update the flag */
    if (state != prev_state) {
        VLOG_INFO("%s config change. old : %d, new : %d",
                  feature_name[feature], prev_state, state);
        set_feature_status(config, feature, state);
    }

    return;
}

#ifdef FTR_DHCP_RELAY
/*
 * Function      : update_option82_policy
 * Responsiblity : Check for dhcp relay option 82 policy update.
 * Parameters    : value - policy type
 * Return        : none
 */
void update_option82_policy(char *value)
{
    DHCP_RELAY_OPTION82_POLICY policy = KEEP;

    if (value && !strncmp(value, "replace", strlen(value)))
        policy = REPLACE;
    else if (value && !strncmp(value, "drop", strlen(value)))
        policy = DROP;

    if (policy != udpfwd_ctrl_cb_p->feature_config.policy) {
        VLOG_INFO("Option 82 policy config changed. old : %s, new : %s",
                  policy_name[udpfwd_ctrl_cb_p->feature_config.policy],
                  policy_name[policy]);

        /* update global structure with the new policy value. */
        udpfwd_ctrl_cb_p->feature_config.policy = policy;
    }

    return;
}

/*
 * Function      : update_option82_remote_id
 * Responsiblity : Check for dhcp relay option 82 remote-id update.
 * Parameters    : value - remote-id type
 * Return        : none
 */
void update_option82_remote_id(char *value)
{
    DHCP_RELAY_OPTION82_REMOTE_ID r_id = REMOTE_ID_MAC;

    if (value && !strncmp(value, "ip", strlen(value)))
        r_id = REMOTE_ID_IP;

    if (r_id != udpfwd_ctrl_cb_p->feature_config.r_id) {
        VLOG_INFO("Option 82 policy remote_id config changed. old : %s, new : %s",
                  remote_id_name[udpfwd_ctrl_cb_p->feature_config.r_id],
                  remote_id_name[r_id]);

        /* update global structure with the new remote_id value. */
        udpfwd_ctrl_cb_p->feature_config.r_id = r_id;
    }

    return;
}

/*
 * Function      : update_stats_refresh_interval
 * Responsiblity : Check for statistics refresh interval update.
 * Parameters    : value - statistics refresh interval
 * Return        : none
 */
void update_stats_refresh_interval(const char *value)
{
    if (atoi(value) != udpfwd_ctrl_cb_p->stats_interval) {
        VLOG_INFO("statistics refresh interva changed. old : %d, new : %d",
                  udpfwd_ctrl_cb_p->stats_interval,
                  atoi(value));

        /* update global structure with the new statistics refresh interval. */
        udpfwd_ctrl_cb_p->stats_interval = atoi(value);
    }

    return;
}
#endif /* FTR_DHCP_RELAY */

/*
 * Function      : udpfwd_process_globalconfig_update
 * Responsiblity : Process system table update notifications related to udp
 *                 forwarder from OVSDB.
 * Parameters    : none
 * Return        : none
 */
void udpfwd_process_globalconfig_update(void)
{
    const struct ovsrec_system *system_row = NULL;
#if defined(FTR_DHCP_RELAY) || defined(FTR_UDP_BCAST_FWD)
    FEATURE_STATUS state;
    char *value;
#endif /* (FTR_DHCP_RELAY | FTR_UDP_BCAST_FWD) */

    system_row = ovsrec_system_first(idl);
    if (NULL == system_row) {
        VLOG_ERR("Unable to find system record in idl cache");
        return;
    }

    /* If system table row is not modified do no-op */
    if (!OVSREC_IDL_IS_ROW_MODIFIED(system_row, idl_seqno))
    {
        return;
    }

#ifdef FTR_DHCP_RELAY
    /* Check if dhcp-relay global configuration is changed */
    if (OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_system_col_dhcp_config,
                                   idl_seqno)) {
        /* Check for dhcp-relay configuration update */
        state = ENABLE;
        value = (char *)smap_get(&system_row->dhcp_config,
                                 SYSTEM_DHCP_CONFIG_MAP_V4RELAY_DISABLED);

        if (value && (!strncmp(value, "true", strlen(value)))) {
            state = DISABLE;
        }
        update_feature_state(DHCP_RELAY, state);

        /* Check for dhcp-relay hop count increment configuration update */
        state = ENABLE;
        value = (char *)smap_get(&system_row->dhcp_config,
                  SYSTEM_DHCP_CONFIG_MAP_V4RELAY_HOP_COUNT_INCREMENT_DISABLED);
        if (value && (!strncmp(value, "true", strlen(value)))) {
            state = DISABLE;
        }
        update_feature_state(DHCP_RELAY_HOP_COUNT_INCREMENT, state);

        /* Check for dhcp-relay option 82 configuration update */
        state = DISABLE;
        value = (char *)smap_get(&system_row->dhcp_config,
                  SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_ENABLED);
        if (value && (!strncmp(value, "true", strlen(value)))) {
            state = ENABLE;
        }
        update_feature_state(DHCP_RELAY_OPTION82, state);

        /* Check for dhcp-relay option 82 validate configuration update */
        state = DISABLE;
        value = (char *)smap_get(&system_row->dhcp_config,
                  SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_VALIDATION_ENABLED);
        if (value && (!strncmp(value, "true", strlen(value)))) {
            state = ENABLE;
        }
        update_feature_state(DHCP_RELAY_OPTION82_VALIDATE, state);

        /* Check for dhcp-relay option 82 policy configuration update */
        value = (char *)smap_get(&system_row->dhcp_config,
                  SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_POLICY);
        update_option82_policy(value);

        /* Check for dhcp-relay option 82 remote-id configuration update */
        value = (char *)smap_get(&system_row->dhcp_config,
                  SYSTEM_DHCP_CONFIG_MAP_V4RELAY_OPTION82_REMOTE_ID);
        update_option82_remote_id(value);
    }
#endif /* FTR_DHCP_RELAY */

    /* Check if there is a change in UDP Broadcast Forwarder global config */
    if (OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_system_col_other_config,
                                   idl_seqno)) {
#ifdef FTR_UDP_BCAST_FWD
        state = DISABLE;
        value = (char *)smap_get(&system_row->other_config,
                                 SYSTEM_OTHER_CONFIG_MAP_UDP_BCAST_FWD_ENABLED);
        if (value && (!strncmp(value, "true", strlen(value)))) {
            state = ENABLE;
        }
        update_feature_state(UDP_BCAST_FORWARDER, state);
#endif /* FTR_UDP_BCAST_FWD */

#ifdef FTR_DHCP_RELAY
        /* Check if there is a change in statistics refresh interval */
        value = (char *)smap_get(&system_row->other_config,
                                 SYSTEM_OTHER_CONFIG_MAP_STATS_UPDATE_INTERVAL);
        if (value)
            update_stats_refresh_interval(value);
#endif /* FTR_DHCP_RELAY */
    }

    return;
}

#ifdef FTR_DHCP_RELAY
/*
 * Function      : dhcp_relay_server_config_update
 * Responsiblity : Process dhcp_relay table update notifications from OVSDB for the
 *                 configuration changes.
 * Parameters    : none
 * Return        : none
 */
void dhcp_relay_server_config_update(void)
{
    const struct ovsrec_dhcp_relay *rec_first = NULL;
    const struct ovsrec_dhcp_relay *rec = NULL;

    /* Check for server configuration changes */
    rec_first = ovsrec_dhcp_relay_first(idl);
    if (NULL == rec_first) {
        /* Check if last entry from the table is deleted */
        udpfwd_handle_dhcp_relay_row_delete(idl);
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
        udpfwd_handle_dhcp_relay_row_delete(idl);
    }

    /* Process row insert/modify notifications */
    OVSREC_DHCP_RELAY_FOR_EACH (rec, idl) {
        if (OVSREC_IDL_IS_ROW_INSERTED(rec, idl_seqno)
            || OVSREC_IDL_IS_ROW_MODIFIED(rec, idl_seqno)) {
            udpfwd_handle_dhcp_relay_config_change(rec, idl_seqno);
        }
    }

    /* FIXME: Handle the case of NULL port column value. Currently
     * "no interface" command doesn't seem to be working */

    return;
}
#endif /* FTR_DHCP_RELAY */

#ifdef FTR_UDP_BCAST_FWD
/*
 * Function      : udp_bcast_forwarder_server_config_update
 * Responsiblity : Process udp_bcast_forwarder table update notifications from
 *                 OVSDB for the configuration changes.
 * Parameters    : none
 * Return        : none
 */
void udp_bcast_forwarder_server_config_update(void)
{
    const struct ovsrec_udp_bcast_forwarder_server *rec = NULL;
    const struct ovsrec_udp_bcast_forwarder_server *rec_first = NULL;

    /* Check for server configuration changes */
    rec_first = ovsrec_udp_bcast_forwarder_server_first(idl);
    if (NULL == rec_first) {
        /* Check if last entry from the table is deleted */
        udpfwd_handle_udp_bcast_forwarder_row_delete(idl);
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
        udpfwd_handle_udp_bcast_forwarder_row_delete(idl);
    }

    /* Process row insert/modify notifications */
    OVSREC_UDP_BCAST_FORWARDER_SERVER_FOR_EACH (rec, idl) {
        if (OVSREC_IDL_IS_ROW_INSERTED(rec, idl_seqno)
            || OVSREC_IDL_IS_ROW_MODIFIED(rec, idl_seqno)) {
            udpfwd_handle_udp_bcast_forwarder_config_change(rec);
        }
    }

    /* FIXME: Handle the case of NULL port column value. Currently
     * "no interface" command doesn't seem to be working */

    return;
}
#endif /* FTR_UDP_BCAST_FWD */

#ifdef FTR_DHCP_RELAY
/*
 * Function      : run_stats_update
 * Responsiblity : To update interface dhcp-relay statistics if necessary.
 * Parameters    : none
 * Return        : none
 */
static void
run_stats_update(void)
{
    int stats_interval;
    static int stats_timer_interval;
    static long long int stats_timer = LLONG_MIN;

    /* Statistics update interval should always be greater than or equal to
     * 5000 ms. */
    stats_interval = udpfwd_ctrl_cb_p->stats_interval;

    if (stats_timer_interval != stats_interval) {
        stats_timer_interval = stats_interval;
        stats_timer = LLONG_MIN;
    }

    /* Rate limit the update. */
    if (time_msec() >= stats_timer) {

        refresh_dhcp_relay_stats();
        stats_timer = time_msec() + stats_timer_interval;
    }
}
#endif /* FTR_DHCP_RELAY */

/*
 * Function      : udpfwd_reconfigure
 * Responsiblity : Process the table update notifications from OVSDB for the
 *                 configuration changes.
 * Parameters    : none
 * Return        : none
 */
void udpfwd_reconfigure(void)
{

#ifdef FTR_DHCP_RELAY
    /* Update statistics */
    run_stats_update();
#endif /* FTR_DHCP_RELAY */

    /* Check for global configuration changes in system table */
    udpfwd_process_globalconfig_update();

#ifdef FTR_DHCP_RELAY
    /* Process dhcp_relay table updates */
    dhcp_relay_server_config_update();
#endif /* FTR_DHCP_RELAY */

#ifdef FTR_UDP_BCAST_FWD
    /* Process udp_bcast_forwarder_server table updates */
    udp_bcast_forwarder_server_config_update();
#endif /* FTR_UDP_BCAST_FWD */

    return;
}

/*
 * Function      : udpfwd_interface_dump
 * Responsiblity : Function dumps information about server IP address,
 *                 udp destination port Number , ref count for each
 *                 interface into dynamic string ds.
 * Parameters    : ds - output buffer
 *                 params - structure which has interface name
 *                 and port number.
 * Return        : none
 */
static void udpfwd_interface_dump(struct shash_node *node,
                                  struct ds *ds, uint16_t udp_port)
{
    UDPFWD_SERVER_T *server = NULL;
    UDPFWD_SERVER_T **serverArray = NULL;
    UDPFWD_INTERFACE_NODE_T *intfNode = NULL;
    int32_t iter = 0;
    bool found = false;
    struct in_addr ip_addr;

    intfNode = (UDPFWD_INTERFACE_NODE_T *)node->data;
    serverArray = intfNode->serverArray;

    /* Print all configured server IP addresses along with port number */
    ds_put_format(ds, "Interface %s: %d\n", intfNode->portName,
                      intfNode->addrCount);

#ifdef FTR_DHCP_RELAY
    /* Print dhcp-relay statistics */
    ds_put_format(ds, "client request dropped packets = %d\n",
                  UDPF_DHCPR_CLIENT_DROPS(intfNode));
    ds_put_format(ds, "client request valid packets = %d\n",
                  UDPF_DHCPR_CLIENT_SENT(intfNode));
    ds_put_format(ds, "server request dropped packets = %d\n",
                  UDPF_DHCPR_SERVER_DROPS(intfNode));
    ds_put_format(ds, "server request valid packets = %d\n",
                  UDPF_DHCPR_SERVER_SENT(intfNode));

    ds_put_format(ds, "client request dropped packets with option 82 = %d\n",
                  UDPF_DHCPR_CLIENT_DROPS_WITH_OPTION82(intfNode));
    ds_put_format(ds, "client request valid packets with option 82 = %d\n",
                  UDPF_DHCPR_CLIENT_SENT_WITH_OPTION82(intfNode));
    ds_put_format(ds, "server request dropped packets with option 82 = %d\n",
                  UDPF_DHCPR_SERVER_DROPS_WITH_OPTION82(intfNode));
    ds_put_format(ds, "server request valid packets with option 82 = %d\n",
                  UDPF_DHCPR_SERVER_SENT_WITH_OPTION82(intfNode));

    /* Print bootp gateway */
    ip_addr.s_addr = intfNode->bootp_gw;
    ds_put_format(ds, "%s\n", inet_ntoa(ip_addr));
#endif /* FTR_DHCP_RELAY */

    for(iter = 0; iter < intfNode->addrCount; iter++)
    {
        server = serverArray[iter];
        if(udp_port && (udp_port != server->udp_port))
            continue;
        found = true;
        ds_put_format(ds, "Port %d - ", server->udp_port);
        ip_addr.s_addr = server->ip_address;
        ds_put_format(ds, "%s,%d\n", inet_ntoa(ip_addr), server->ref_count);
    }

    if(!found && udp_port)
        ds_put_format(ds, "No IP address associated with this port: %d\n",
                      udp_port);

    return;
}

/*
 * Function      : udpfwd_interfaces_dump
 * Responsiblity : Function dumps information about interfaces
 *                 into dynamic string ds.
 * Parameters    : ds - output buffer
 *                 params - structure which has interface name
 *                 and udp destination port number
 * Return        : none
 */
static void udpfwd_interfaces_dump(struct ds *ds, struct dump_params *params)
{
    struct shash_node *node, *temp;
#if defined(FTR_DHCP_RELAY) || defined(FTR_UDP_BCAST_FWD)
    feature_bmap config = udpfwd_ctrl_cb_p->feature_config.config;
#endif /* (FTR_DHCP_RELAY | FTR_UDP_BCAST_FWD) */

#ifdef FTR_UDP_BCAST_FWD
    ds_put_format(ds, "UDP Bcast Forwarder : %d\n",
                      get_feature_status(config, UDP_BCAST_FORWARDER));
#endif /* FTR_UDP_BCAST_FWD */

#ifdef FTR_DHCP_RELAY
    ds_put_format(ds, "DHCP Relay : %d\n", get_feature_status(config, DHCP_RELAY));
    ds_put_format(ds, "DHCP Relay hop-count-increment : %d\n",
                      get_feature_status(config, DHCP_RELAY_HOP_COUNT_INCREMENT));
    ds_put_format(ds, "DHCP Relay Option82 : %d\n",
                      get_feature_status(config, DHCP_RELAY_OPTION82));
    ds_put_format(ds, "DHCP Relay Option82 validate : %d\n",
                      get_feature_status(config, DHCP_RELAY_OPTION82_VALIDATE));
    ds_put_format(ds, "DHCP Relay Option82 policy : %s\n",
                      policy_name[udpfwd_ctrl_cb_p->feature_config.policy]);
    ds_put_format(ds, "DHCP Relay Option82 remote-id : %s\n",
                      remote_id_name[udpfwd_ctrl_cb_p->feature_config.r_id]);
#endif /* FTR_DHCP_RELAY */

    if (!params->ifName) {
        /* dump all interfaces */
        SHASH_FOR_EACH(temp, &udpfwd_ctrl_cb_p->intfHashTable)
            udpfwd_interface_dump(temp, ds, params->port);
    }
    else {
        node = shash_find(&udpfwd_ctrl_cb_p->intfHashTable, params->ifName);
        if (NULL == node) {
            ds_put_format(ds, "No servers are configured on"
            " this interface :%s\n", params->ifName);
            return;
        }
        udpfwd_interface_dump(node, ds, params->port);
    }
}

/*
 * Function      : udpfwd_unixctl_dump
 * Responsiblity : UDP fowarder module core dump callback
 * Parameters    : conn - unixctl socket connection
 *                 argc, argv - function parameters
 *                 aux - aux connection data
 * Return        : none
 */
static void udpfwd_unixctl_dump(struct unixctl_conn *conn, int argc OVS_UNUSED,
                   const char *argv[] OVS_UNUSED, void *aux OVS_UNUSED)
{
    struct ds ds = DS_EMPTY_INITIALIZER;
    struct dump_params params;
    memset(&params, 0, sizeof(struct dump_params));
    /*
     * Parse unxictl arguments.
     * second argument is always interface name.
     * fourth argument is udp destination port number.
     * ex : ovs-appctl -t ops-udpfwd udpfwd/dump int 1 port 67.
     */
    if (argc > 3) {
        params.ifName = (char*)argv[2];
        params.port = atoi(argv[4]);
    }
    else if (argc > 1)
        params.ifName = (char*)argv[2];

    udpfwd_interfaces_dump(&ds, &params);
    unixctl_command_reply(conn, ds_cstr(&ds));
    ds_destroy(&ds);
}

/*
 * Function      : udpfwd_exit
 * Responsiblity : Daemon cleanup before exit
 * Parameters    : -
 * Return        : none
 */
void udpfwd_exit(void)
{
    /* free memory for packet receive buffer */
    if (0 < udpfwd_ctrl_cb_p->udpSockFd)
        close(udpfwd_ctrl_cb_p->udpSockFd);

    if (NULL != udpfwd_ctrl_cb_p->rcvbuff)
        free(udpfwd_ctrl_cb_p->rcvbuff);
}

/*
 * Function      : udpfwd_init
 * Responsiblity : idl create/registration, module initialization and
 *                 unixctl registrations
 * Parameters    : none
 * Return        : void
 */
bool udpfwd_init(void)
{
#ifdef FTR_DHCP_RELAY
    ovsdb_idl_add_column(idl, &ovsrec_system_col_dhcp_config);
#endif /* FTR_DHCP_RELAY */

    /* Register for DHCP_Relay table updates */
#ifdef FTR_DHCP_RELAY
    ovsdb_idl_add_table(idl, &ovsrec_table_dhcp_relay);
    ovsdb_idl_add_column(idl, &ovsrec_dhcp_relay_col_port);
    ovsdb_idl_add_column(idl,
                        &ovsrec_dhcp_relay_col_vrf);
    ovsdb_idl_add_column(idl,
                        &ovsrec_dhcp_relay_col_ipv4_ucast_server);

    ovsdb_idl_add_column(idl,
                           &ovsrec_dhcp_relay_col_other_config);
#endif /* FTR_DHCP_RELAY */

    /* Register for UDP_Bcast_Forwarder table updates */
#ifdef FTR_UDP_BCAST_FWD
    ovsdb_idl_add_table(idl, &ovsrec_table_udp_bcast_forwarder_server);
    ovsdb_idl_add_column(idl, &ovsrec_udp_bcast_forwarder_server_col_src_port);
    ovsdb_idl_add_column(idl, &ovsrec_udp_bcast_forwarder_server_col_dest_vrf);
    ovsdb_idl_add_column(idl, &ovsrec_udp_bcast_forwarder_server_col_udp_dport);
    ovsdb_idl_add_column(idl,
                         &ovsrec_udp_bcast_forwarder_server_col_ipv4_ucast_server);
#endif /* FTR_UDP_BCAST_FWD */

    /* Register for port table for dhcp_relay_statistics update */
    ovsdb_idl_add_table(idl, &ovsrec_table_port);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_name);
#ifdef FTR_DHCP_RELAY
    ovsdb_idl_add_column(idl, &ovsrec_port_col_dhcp_relay_statistics);
#endif /* FTR_DHCP_RELAY */

    /* Initialize module data structures */
    if (true != udpfwd_module_init())
    {
        VLOG_FATAL("Module failed to initialize");
        return false;
    }

    unixctl_command_register("udpfwd/dump", "", 0, 4,
                             udpfwd_unixctl_dump, NULL);

    return true;
}
