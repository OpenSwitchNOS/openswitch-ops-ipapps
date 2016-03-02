/*
 * Copyright (C) 2016 Hewlett-Packard Development Company, L.P.
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

/**
 * This daemon handles the following functionality:
 * - Dhcp-relay agent for IPv4.
 */

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
#include "vswitch-idl.h"
#include "openswitch-idl.h"
#include "openswitch-dflt.h"
#include "coverage.h"
#include "hash.h"
#include "svec.h"

#include "udpfwd.h"


/**
 * Global variable declarations.
 */
unsigned int idl_seqno;
static struct ovsdb_idl *idl;
bool commit_txn = false;
pthread_t dhcp_bcast_recv_t;

static int system_configured = false;


VLOG_DEFINE_THIS_MODULE(udpfwd);
/**
 * Function prototypes
 */
void udpfwd_init(const char *remote);
void udpfwd_run(void);
void udpfwd_reconfigure();
static void usage(void);
static char *parse_options(int argc, char *argv[], char **unixctl_pathp);
static inline void udpfwd_chk_for_system_configured(void);
static void udpfwd_wait(void);
static void udpfwd_exit_cb(struct unixctl_conn *conn, int argc OVS_UNUSED,
                           const char *argv[] OVS_UNUSED, void *exiting_);
static void udpfwd_exit(void);

extern void * udp_bcast_recv(void *data);
/**
 * Daemon main function
 */
int main (int argc, char *argv[])
{
    char *unixctl_path = NULL;
    struct unixctl_server *unixctl;
    char *remote;
    bool exiting = false;
    int retVal = 0;

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

/**
 * Initialize UDP Forwarder module
 */
void udpfwd_init(const char *remote)
{
    return;
}

/**
 * Refresh IDL cache and process the updates
 */
void udpfwd_run(void)
{
    return;
}

/**
 * Process the configuration changes
 */
void udpfwd_reconfigure ()
{
    return;
}

/**
 * UDP forwarder daemon usage guidelines
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

/**
 * UDP forwarder daemon options validator
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

/**
 *  Check if system is configured completely
 */
static inline void udpfwd_chk_for_system_configured(void)
{
    const struct ovsrec_system *ovs_vsw = NULL;

    if (system_configured) {
        /* Nothing to do if we're already configured. */
        return;
    }

    ovs_vsw = ovsrec_system_first(idl);

    if (ovs_vsw && (ovs_vsw->cur_cfg > (int64_t) 0)) {
        system_configured = true;
        VLOG_DBG("System is now configured (cur_cfg=%d).",
                (int)ovs_vsw->cur_cfg);
    }
}

/**
 * Wait for next idl run
 */
static void udpfwd_wait(void)
{
    ovsdb_idl_wait(idl);
    poll_timer_wait(IDL_POLL_INTERVAL * 1000);
}

/**
 * Module exit callback from unixctl
 */
static void udpfwd_exit_cb(struct unixctl_conn *conn, int argc OVS_UNUSED,
                           const char *argv[] OVS_UNUSED, void *exiting_)
{
    bool *exiting = exiting_;
    *exiting = true;
    unixctl_command_reply(conn, NULL);
}

/**
 * Module cleanup
 */
static void udpfwd_exit(void)
{
    ovsdb_idl_destroy(idl);

    /* TODO: Clean up other resources */
}
