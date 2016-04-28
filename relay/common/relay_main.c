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
 * File: relay_main.c
 *
 */

/*
 * This daemon handles the following functionality:
 * - Dhcp-relay agent for IPv4 and IPv6.
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

#include "udpfwd.h"
#include "dhcpv6_relay.h"

/*
 * Global variable declarations.
 */
/* Cached idl sequence number */
uint32_t idl_seqno;

/* idl pointer */
struct ovsdb_idl *idl;

VLOG_DEFINE_THIS_MODULE(relay_main);

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
 * Function      : relay_unixctl_run_and_wait
 * Responsiblity : run unixctl and wait for next idl run
 * Parameters    : none
 * Return        : none
 */
static void relay_unixctl_run_and_wait(struct unixctl_server *unixctl,
                                        bool exiting)
{
    unixctl_server_run(unixctl);

    ovsdb_idl_wait(idl);
    poll_timer_wait(IDL_POLL_INTERVAL * 1000);

    unixctl_server_wait(unixctl);
    if (exiting) {
        poll_immediate_wake();
    }
    poll_block();
}

/*
 * Function      : relay_exit_cb
 * Responsiblity : Daemon exit callback invoked from unixctl
 * Parameters    : -
 * Return        : none
 */
static void relay_exit_cb(struct unixctl_conn *conn, int argc OVS_UNUSED,
                           const char *argv[] OVS_UNUSED, void *exiting_)
{
    bool *exiting = exiting_;
    *exiting = true;
    unixctl_command_reply(conn, NULL);
}

/*
 * Function      : relay_exit
 * Responsiblity : Daemon cleanup before exit
 * Parameters    : -
 * Return        : none
 */
static void relay_exit(void)
{
    udpfwd_exit();
#ifdef FTR_DHCPV6_RELAY
    dhcpv6r_exit();
#endif /* FTR_DHCPV6_RELAY */

    ovsdb_idl_destroy(idl);
}

/*
 * Function      : idl_init
 * Responsiblity : Initialize idl
 * Parameters    : remote - daemon launch command
 * Return        : void
 */
void idl_init(const char *remote)
{
    idl = ovsdb_idl_create(remote, &ovsrec_idl_class, false, true);
    idl_seqno = ovsdb_idl_get_seqno(idl);
    ovsdb_idl_set_lock(idl, "ops_relay");

    /* Register for System table updates */
    ovsdb_idl_add_table(idl, &ovsrec_table_system);
    ovsdb_idl_add_column(idl, &ovsrec_system_col_cur_cfg);
    ovsdb_idl_add_column(idl, &ovsrec_system_col_other_config);

    return;
}

/**
 * Function      : relay_chk_for_system_configured
 * Responsiblity : Check if system is fully configured by looking into
 *                 system table
 * Parameters    : none
 * Return        : true  - if system configured
 *                 false - otherwise
 */
bool relay_chk_for_system_configured(void)
{
    const struct ovsrec_system *ovs_vsw = NULL;

    ovs_vsw = ovsrec_system_first(idl);

    if (ovs_vsw && (ovs_vsw->cur_cfg > (int64_t) 0)) {
        VLOG_INFO("System is now configured (cur_cfg=%d).",
                  (int)ovs_vsw->cur_cfg);
        return true;
    }

    return false;
}

/*
 * Function      : relay_idl_run_and_lockcheck
 * Responsiblity : trigger idl run and check if we have the lock
 * Parameters    : none
 * Return        : true  - on success
 *                 false - otherwise
 */
bool relay_idl_run_and_lockcheck(void)
{
    ovsdb_idl_run(idl);

    if (ovsdb_idl_is_lock_contended(idl)) {
        static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(1, 1);

        VLOG_ERR_RL(&rl, "another udpfwd process is running, "
                    "disabling this process until it goes away");
        return false;
    } else if (!ovsdb_idl_has_lock(idl)) {
        return false;
    }

    return true;
}

/*
 * Function      : relay_run
 * Responsiblity : idl refresh routine. Trigger update processing if need be.
 * Parameters    : none
 * Return        : none
 */
void relay_run(void)
{
    uint32_t new_idl_seqno;
    if (!relay_idl_run_and_lockcheck())
        return;

    new_idl_seqno = ovsdb_idl_get_seqno(idl);

    /* Do NOOP if there is not change in idl sequence number */
    if (new_idl_seqno == idl_seqno){
        VLOG_DBG("No config change in ovs");
        return;
    }

    udpfwd_reconfigure();

#ifdef FTR_DHCPV6_RELAY
    dhcpv6r_reconfigure();
#endif /* FTR_DHCPV6_RELAY */

    /* Cache the lated idl sequence number */
    idl_seqno = new_idl_seqno;

}

/*
 * Function      : main
 * Responsiblity : Daemon main function
 * Parameters    : argc, argv - daemon arguments
 * Return        : none
 */
int main(int argc, char *argv[])
{
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
    unixctl_command_register("exit", "", 0, 0, relay_exit_cb, &exiting);

    idl_init(remote);

    if (false == udpfwd_init())
    {
        free(remote);
        VLOG_ERR("Failed to initialize UDP Broadcast Forwarder");
        return -1;
    }

#ifdef FTR_DHCPV6_RELAY
    if (false == dhcpv6r_init())
    {
        free(remote);
        VLOG_ERR("Failed to initialize DHCPv6_Relay");
        return -1;
    }
#endif /* FTR_DHCPV6_RELAY */

    free(remote);
    daemonize_complete();
    vlog_enable_async();

    /* Wait until system is completely initialized */
    while (!exiting)
    {
        relay_idl_run_and_lockcheck();

        /* Check if system initialization is done */
        if (true == relay_chk_for_system_configured())
            break;

        relay_unixctl_run_and_wait(unixctl, exiting);
    }

    /* Daemon Task loop */
    while (!exiting)
    {
        relay_run();
        relay_unixctl_run_and_wait(unixctl, exiting);
    }

    relay_exit();
    unixctl_server_destroy(unixctl);

    return 0;
}
