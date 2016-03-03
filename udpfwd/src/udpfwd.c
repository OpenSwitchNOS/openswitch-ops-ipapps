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
#include "openvswitch/types.h"
#include "openswitch-dflt.h"
#include "coverage.h"
#include "svec.h"

#include "udpfwd.h"

/*
 * Global variable declarations.
 */

/* Cached idl sequence number */
uint32_t idl_seqno;
/* idl pointer */
struct ovsdb_idl *idl;
/* udp socket receiver thread handle */
pthread_t udpBcastRecv_thread;

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
 * Return        : true, on success
 *                 false, on failure
 */
bool udpfwd_module_init(void)
{
    int32_t val = 1;
    int32_t retVal;

    VLOG_INFO("Creating UDP send socket");

    /* Create socket to send UDP packets to client or server */
    udpfwd_ctrl_cb_p->send_sockFd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (-1 == udpfwd_ctrl_cb_p->send_sockFd) {
        VLOG_ERR("Failed to create UDP socket");
        return false;
    }

    retVal = setsockopt(udpfwd_ctrl_cb_p->send_sockFd, IPPROTO_IP, IP_PKTINFO,
                        (char *)&val, sizeof(val));
    if (0 != retVal) {
        VLOG_ERR("Failed to set IP_PKTINFO socket option : %d", retVal);
        close(udpfwd_ctrl_cb_p->send_sockFd);
        return false;
    }

    retVal = setsockopt(udpfwd_ctrl_cb_p->send_sockFd, SOL_SOCKET, SO_BROADCAST,
                       (char *)&val, sizeof (val));
    if (0 != retVal) {
        VLOG_ERR("Failed to set broadcast socket option : %d", retVal);
        return false;
    }

    VLOG_INFO("UDP send socket created successfully");

    /* Initialize server hash table */
    shash_init(&udpfwd_ctrl_cb_p->intfHashTable);

    /* Initialize server hash map */
    cmap_init(&udpfwd_ctrl_cb_p->serverHashMap);

    /* dhcp-relay is enabled by default */
    udpfwd_ctrl_cb_p->dhcp_relay_enable = true;

    /* DB access semaphore initialization */
    retVal = sem_init(&udpfwd_ctrl_cb_p->waitSem, 0, 1);
    if (0 != retVal) {
        VLOG_ERR("Failed to initilize the semaphore, retval : %d (errno : %d)",
                 retVal, errno);
        close(udpfwd_ctrl_cb_p->send_sockFd);
        return false;
    }

#if 0 /* Shall be enabled thorugh another  review request */
    /* Create UDP broadcast receiver thread */
    /* FIXME: Add logic to create recv thread only when there is a valid config */
    retVal = pthread_create(&udpBcastRecv_thread, (pthread_attr_t *)NULL,
                            udp_packet_recv, NULL);
    if (0 != retVal)
    {
        VLOG_ERR("Failed to create UDP broadcast packet receiver thread : %d",
                 retVal);
        return false;
    }
#endif

    return true;
}

/*
 * Function      : udpfwd_process_globalconfig_update
 * Responsiblity : Process system table update notifications related to udp
 *                 forwarder from OVSDB.
 * Parameters    : idl_seqno - Cached idl sequence number
 * Return        : none
 */
void udpfwd_process_globalconfig_update(int32_t idl_seqno)
{
    const struct ovsrec_system *system_row = NULL;
    bool dhcp_relay_enabled = false;
    char *disabled;

    system_row = ovsrec_system_first(idl);
    if (NULL == system_row) {
        VLOG_ERR("Unable to find system record in idl cache");
        return;
    }

    if (OVSREC_IDL_IS_ROW_MODIFIED(system_row, idl_seqno) &&
        (OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_system_col_other_config,
                                       idl_seqno))) {
        disabled = (char *)smap_get(&system_row->other_config,
                                 SYSTEM_OTHER_CONFIG_MAP_DHCP_RELAY_DISABLED);

        /* Check if dhcp-relay is enabled globally */
        dhcp_relay_enabled = (NULL == disabled) ? true : false;

        /* Check if dhcp-relay global configuration is changed */
        if (dhcp_relay_enabled != udpfwd_ctrl_cb_p->dhcp_relay_enable) {
            VLOG_INFO("DHCP-Relay global config change. old : %d, new : %d",
                     udpfwd_ctrl_cb_p->dhcp_relay_enable, dhcp_relay_enabled);
            udpfwd_ctrl_cb_p->dhcp_relay_enable = dhcp_relay_enabled;
        }
    }

    return;
}

/*
 * Function      : udpfwd_dhcp_relay_config_update
 * Responsiblity : Process dhcp_relay table update notifications from OVSDB for the
 *                 configuration changes.
 * Parameters    : idl_seqno - Cached idl sequence number
 * Return        : none
 */
void udpfwd_dhcp_relay_config_update(int32_t idl_seqno)
{
    const struct ovsrec_dhcp_relay *rec_first = NULL;
    const struct ovsrec_dhcp_relay *rec = NULL;

    /* Check for server configuration changes */
    rec_first = ovsrec_dhcp_relay_first(idl);
    if (NULL == rec_first) {
        return;
    }

    if (!OVSREC_IDL_ANY_TABLE_ROWS_INSERTED(rec_first, idl_seqno)
        && !OVSREC_IDL_ANY_TABLE_ROWS_DELETED(rec_first, idl_seqno)
        && !OVSREC_IDL_ANY_TABLE_ROWS_MODIFIED(rec_first, idl_seqno)) {
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
            udpfwd_handle_dhcp_relay_config_change(rec);
        }
    }

    /* FIXME: Handle the case of NULL port column value. Currently
     * "no interface" command doesn't seem to be working */

    return;
}

/*
 * Function      : udpfwd_reconfigure
 * Responsiblity : Process the table update notifications from OVSDB for the
 *                 configuration changes.
 * Parameters    : none
 * Return        : none
 */
void udpfwd_reconfigure(void)
{
    uint32_t new_idl_seqno = ovsdb_idl_get_seqno(idl);

    if (new_idl_seqno == idl_seqno){
        VLOG_DBG("No config change for udpfwd in ovs");
        return;
    }

    /* Check for global configuration changes in system table */
    udpfwd_process_globalconfig_update(idl_seqno);

    /* Process dhcp_relay table updates */
    udpfwd_dhcp_relay_config_update(idl_seqno);

    /* Cache the lated idl sequence number */
    idl_seqno = new_idl_seqno;
    return;
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
    unixctl_command_reply_error(conn, "NA");
}

/*
 * Function      : usage
 * Responsiblity : Daemon usage help display
 * Parameters    : none
 * Return        : none
 */
static void usage(void)
{
    printf("%s: OPS udpfwd daemon\n"
            "usage: %s [OPTIONS] [DATABASE]\n"
            "where DATABASE is a socket on which ovsdb-server is listening\n"
            "      (default: \"unix:%s/db.sock\").\n",
            program_name, program_name, ovs_rundir());
    stream_usage("DATABASE", true, false, true);
    daemon_usage();
    vlog_usage();
    printf("\nOther options:\n"
            "  --unixctl=SOCKET        override default control socket name\n"
            "  -h, --help              display this help message\n"
            "  -V, --version           display version information\n");
    exit(EXIT_SUCCESS);
}

/*
 * Function      : parse_options
 * Responsiblity : Daemon options validator
 * Parameters    : argc, argv[] - Arguments
 *               : unixctl_pathp - unixctl path
 * Return        : char* - daemon launch command
 */
static char *parse_options(int argc, char *argv[], char **unixctl_pathp)
{
    enum {
        OPT_UNIXCTL = UCHAR_MAX + 1,
        VLOG_OPTION_ENUMS,
        DAEMON_OPTION_ENUMS,
    };

    static const struct option long_options[] = {
            {"help",        no_argument, NULL, 'h'},
            {"version",     no_argument, NULL, 'V'},
            {"unixctl",     required_argument, NULL, OPT_UNIXCTL},
            DAEMON_LONG_OPTIONS,
            VLOG_LONG_OPTIONS,
            {NULL, 0, NULL, 0},
    };

    char *short_options = long_options_to_short_options(long_options);

    for (;;) {
        int c;

        c = getopt_long(argc, argv, short_options, long_options, NULL);

        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            usage();

        case 'V':
            ovs_print_version(OFP10_VERSION, OFP10_VERSION);
            exit(EXIT_SUCCESS);

        case OPT_UNIXCTL:
            *unixctl_pathp = optarg;
            break;

            VLOG_OPTION_HANDLERS
            DAEMON_OPTION_HANDLERS

        case '?':
            exit(EXIT_FAILURE);

        default:
            abort();
        }
    }
    free(short_options);

    argc -= optind;
    argv += optind;

    switch (argc) {
    case 0:
        return xasprintf("unix:%s/db.sock", ovs_rundir());

    case 1:
        return xstrdup(argv[0]);

    default:
        VLOG_FATAL("at most one non-option argument accepted; "
                "use --help for usage");
    }
}

/*
 * Function      : udpfwd_wait
 * Responsiblity : Wait for next idl run
 * Parameters    : none
 * Return        : none
 */
static void udpfwd_wait(void)
{
    ovsdb_idl_wait(idl);
    poll_timer_wait(IDL_POLL_INTERVAL * 1000);
}

/*
 * Function      : udpfwd_exit_cb
 * Responsiblity : Daemon exit callback invoked from unixctl
 * Parameters    : -
 * Return        : none
 */
static void udpfwd_exit_cb(struct unixctl_conn *conn, int argc OVS_UNUSED,
                           const char *argv[] OVS_UNUSED, void *exiting_)
{
    bool *exiting = exiting_;
    *exiting = true;
    unixctl_command_reply(conn, NULL);
}

/*
 * Function      : udpfwd_exit
 * Responsiblity : Daemon cleanup before exit
 * Parameters    : -
 * Return        : none
 */
static void udpfwd_exit(void)
{
    ovsdb_idl_destroy(idl);
}

/*
 * Function      : udpfwd_init
 * Responsiblity : idl create/registration, module initialization and
 *                 unixctl registrations
 * Parameters    : remote - daemon launch command
 * Return        : none
 */
void udpfwd_init(const char *remote)
{
    idl = ovsdb_idl_create(remote, &ovsrec_idl_class, false, true);
    idl_seqno = ovsdb_idl_get_seqno(idl);
    ovsdb_idl_set_lock(idl, "ops_udpfwd");

    /* Register for System table updates */
    ovsdb_idl_add_table(idl, &ovsrec_table_system);
    ovsdb_idl_add_column(idl, &ovsrec_system_col_cur_cfg);
    ovsdb_idl_add_column(idl, &ovsrec_system_col_other_config);

    /* Register for DHCP_Relay table updates */
    ovsdb_idl_add_table(idl, &ovsrec_table_dhcp_relay);
    ovsdb_idl_add_column(idl, &ovsrec_dhcp_relay_col_port);
    ovsdb_idl_add_column(idl,
                        &ovsrec_dhcp_relay_col_vrf);
    ovsdb_idl_add_column(idl,
                        &ovsrec_dhcp_relay_col_ipv4_ucast_server);

    /* Register for port table for dhcp_relay_statistics update */
    ovsdb_idl_add_table(idl, &ovsrec_table_port);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_name);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_dhcp_relay_statistics);

    /* Initialize module data structures */
    udpfwd_module_init();

    unixctl_command_register("udpfwd/dump", "", 0, 0,
                             udpfwd_unixctl_dump, NULL);
}

/*
 * Function      : udpfwd_run
 * Responsiblity : idl refresh routine. Trigger update processing if need be.
 * Parameters    : none
 * Return        : none
 */
void udpfwd_run(void)
{
    ovsdb_idl_run(idl);

    if (ovsdb_idl_is_lock_contended(idl)) {
        static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(1, 1);

        VLOG_ERR_RL(&rl, "another udpfwd process is running, "
                    "disabling this process until it goes away");
        return;
    } else if (!ovsdb_idl_has_lock(idl)) {
        return;
    }

    udpfwd_reconfigure();
}

/*
 * Function      : main
 * Responsiblity : Daemon main function
 * Parameters    : argc, argv - daemon arguments
 * Return        : none
 */
int main(int argc, char *argv[])
{
    const struct ovsrec_system *ovs_vsw = NULL;
    char *unixctl_path = NULL;
    struct unixctl_server *unixctl;
    char *remote;
    bool exiting = false;
    int32_t retVal = 0;

    set_program_name(argv[0]);
    proctitle_init(argc, argv);
    remote = parse_options(argc, argv, &unixctl_path);

    ovsrec_init();
    daemonize_start();

    retVal = unixctl_server_create(unixctl_path, &unixctl);
    if (retVal) {
        exit(EXIT_FAILURE);
    }
    unixctl_command_register("exit", "", 0, 0, udpfwd_exit_cb, &exiting);

    udpfwd_init(remote);
    free(remote);
    daemonize_complete();
    vlog_enable_async();

    /* Wait for system ready state */
    while (!exiting) {
        ovs_vsw = ovsrec_system_first(idl);

        if (ovs_vsw && (ovs_vsw->cur_cfg > (int64_t) 0)) {
            VLOG_INFO("System is now configured (cur_cfg=%d).",
                      (int)ovs_vsw->cur_cfg);
            break;
        }
        udpfwd_wait();
    }

    /* Daemon Task loop */
    while (!exiting)
    {
        udpfwd_run();
        unixctl_server_run(unixctl);

        udpfwd_wait();
        unixctl_server_wait(unixctl);
        if (exiting) {
            poll_immediate_wake();
        }
        poll_block();
    }

    udpfwd_exit();
    unixctl_server_destroy(unixctl);

    return 0;
}
