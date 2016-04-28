#!/usr/bin/env python
# Copyright (C) 2015 Hewlett Packard Enterprise Development LP
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.

import os
import sys
import time
from time import sleep

import ovs.dirs
from ovs.db import error
from ovs.db import types
import ovs.daemon
import ovs.db.idl
import ovs.unixctl
import ovs.unixctl.server
import argparse
import ovs.vlog
import ops_diagdump

# Local variables to check if system is configured
system_initialized = 0

# Schema path
ovs_schema = '/usr/share/openvswitch/vswitch.ovsschema'

# Program control
exiting = False
seqno = 0

# OVS Defination
idl = None

vlog = ovs.vlog.Vlog("ops_appsdnsclient")
def_db = 'unix:/var/run/openvswitch/db.sock'

SYSTEM_TABLE = "System"
DNS_CLIENT_TABLE = "DNS_Client"
DNS_CLIENT_VRF = "vrf"
DNS_CLIENT_DOMAIN_LIST = "domain_list"
DNS_CLIENT_HOST_V4_ADDRESS_MAPPING = "host_v4_address_mapping"
DNS_CLIENT_HOST_V6_ADDRESS_MAPPING = "host_v6_address_mapping"
DNS_CLIENT_NAME_SERVERS = "name_servers"
DNS_CLIENT_OTHER_CONFIG = "other_config"

DOMAIN_NAME = "domain_name"
DNS_CLIENT_CONFIG = "/etc/resolv.conf"

#---------------- unixctl_exit --------------------------
def unixctl_exit(conn, unused_argv, unused_aux):
    global exiting

    exiting = True
    conn.reply(None)

#------daemon handler function for diagnostic dump--------
def diag_basic_handler(argv):
    # argv[0] is basic
    # argv[1] is feature name
    feature = argv.pop()
    buff = 'Diagnostic dump response for feature ' + feature + '.\n'
    buff = buff + 'diag-dump feature for DNS Client is not implemented'
    return buff

#------------------ db_get_system_status() ----------------
def db_get_system_status(data):
    '''
    Check the configuration initialization completed
    (System:cur_cfg == true)
    '''
    for ovs_rec in data[SYSTEM_TABLE].rows.itervalues():
        if ovs_rec.cur_cfg:
            if ovs_rec.cur_cfg == 0:
                return False
            else:
                return True

    return False

#------------------ system_is_configured() ----------------
def system_is_configured():
    '''
    Check the OVS_DB if system initialization has completed.
    Initialization completed: return True
    else: return False
    '''
    global idl
    global system_initialized

    # Check the OVS-DB/File status to see if initialization has completed.
    if not db_get_system_status(idl.tables):
        # Delay a little before trying again
        sleep(1)
        return False

    system_initialized = 1
    return True

#---------------------- update_domain_name --------------------------------
def update_domain_name():
    '''
    Update the domain name in resolv.conf as per the user input.
    '''

    dname = False

    for ovs_rec in idl.tables[DNS_CLIENT_TABLE].rows.itervalues():
        if ovs_rec.other_config and ovs_rec.other_config is not None:
            for key, value in ovs_rec.other_config.iteritems():
                if key == DOMAIN_NAME:
                    dname = True
                    dom_name = value

    with open(DNS_CLIENT_CONFIG, "r") as f:
        contents = f.readlines()

    if not contents :
        if dname == True:
            contents.insert(1, "domain " + dom_name + "\n")
    else:
        if dname == True:
            for index, line in enumerate(contents):
                if "domain" in line:
                    del contents[index]
                    break
            contents.insert(index, "domain " + dom_name + "\n")

    with open(DNS_CLIENT_CONFIG, "w") as f:
        contents = "".join(contents)
        f.write(contents)

    return

#---------------------- update_domain_list --------------------------------
def update_domain_list():
    '''
    Update the domain list entries in resolv.conf as per the user input.
    '''

    dlist = False

    for ovs_rec in idl.tables[DNS_CLIENT_TABLE].rows.itervalues():
        if ovs_rec.domain_list:
            dlist = True
            dom_list = ovs_rec.domain_list

    with open(DNS_CLIENT_CONFIG, "r") as f:
        contents = f.readlines()

    if not contents :
        if dlist == True:
            contents.insert(1, "search " + str(dom_list) + "\n")
        else:
            if dlist == True:
                for index, line in enumerate(contents):
                    if "search" in line:
                        del contents[index]
                        break
                entry_list = " ".join(str(k) for k in dom_list)
                contents.insert(index, "search " + entry_list + "\n")

    with open(DNS_CLIENT_CONFIG, "w") as f:
        contents = "".join(contents)
        f.write(contents)

    return

#---------------------- update_name_servers --------------------------------
def update_name_servers():
    '''
    Update the name servers in resolv.conf as per the user input
    '''

    nserv = False
    indices = []

    for ovs_rec in idl.tables[DNS_CLIENT_TABLE].rows.itervalues():
        if ovs_rec.name_servers:
            nserv = True
            name_serv = ovs_rec.name_servers

    with open(DNS_CLIENT_CONFIG, "r") as f:
        contents = f.readlines()

    if not contents :
        if nserv == True:
            contents.insert(1, "nameserver " + str(name_serv) + "\n")
    else:
        if nserv == True:
            for index, line in enumerate(contents):
                if "nameserver" in line:
                    indices.append(index)

            i = 0
            for index in indices:
                del contents[index - i]
                i += 1

            for index, line in enumerate(contents):
                pass

            i = 0
            for k in name_serv:
                contents.insert(index+i, "nameserver " + str(k) + "\n")
                i += 1

    with open(DNS_CLIENT_CONFIG, "w") as f:
        contents = "".join(contents)
        f.write(contents)

    return

#-------------------------- update_hosts ----------------------------------
def update_hosts():
    '''
    Update the host entries in resolv.conf as per teh user input
    '''

    host = False
    hosts = []
    indices = []

    for ovs_rec in idl.tables[DNS_CLIENT_TABLE].rows.itervalues():
        if ovs_rec.host_v4_address_mapping and \
           ovs_rec.host_v4_address_mapping is not None:
            for key, value in ovs_rec.host_v4_address_mapping.iteritems():
                host = True
                hosts.append(key)
                hosts.append(value)

        if ovs_rec.host_v6_address_mapping and \
           ovs_rec.host_v6_address_mapping is not None:
            for key, value in ovs_rec.host_v6_address_mapping.iteritems():
                host = True
                hosts.append(key)
                hosts.append(value)

    with open(DNS_CLIENT_CONFIG, "r") as f:
        contents = f.readlines()

    if not contents :
        if host == True:
            i = 0
            while (i < len(hosts)):
                contents.insert(index+2, str(hosts[i]) + " " + str(hosts[i+1]) + "\n")
                i += 2
    else:
        if host == True:
            for index, line in enumerate(contents):
                if "nameserver" not in line \
                    and "search" not in line \
                    and "domain" not in line:
                        indices.append(index)

            j = 0
            for index in indices:
                del contents[index-j]
                j += 1

            i = 0
            j = 0
            while (i < len(hosts)):
                contents.insert(index+j, str(hosts[i]) + " " + str(hosts[i+1]) + "\n")
                i += 2
                j += 1

    with open(DNS_CLIENT_CONFIG, "w") as f:
        contents = "".join(contents)
        f.write(contents)

    return

#---------------------- update_dns_client_config_file ---------------------
def update_dns_client_config_file():
    '''
    modify resolv.conf file, based on the dns client details
    configured in DNS Client table
    '''

    update_domain_name()
    update_domain_list()
    update_name_servers()
    update_hosts()

    return

#---------------- dns_client_reconfigure() ----------------
def dns_client_reconfigure():
    '''
    Check system initialization, add default rows and update files accordingly
    based on the values in DB
    '''

    global system_initialized

    if system_initialized == 0:
        rc = system_is_configured()
        if rc is False:
            return
    # Any default initialization do it here

    update_dns_client_config_file()

    return

#----------------- dns_client_run() -----------------------
def dns_client_run():
    '''
    Run idl, and call reconfigure function when there is a change in DB \
    sequence number. \
    '''

    global idl
    global seqno

    idl.run()

    if seqno != idl.change_seqno:
        dns_client_reconfigure()
        seqno = idl.change_seqno

    return

#----------------- main() -------------------
def main():

    global exiting
    global idl
    global seqno

    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--database', metavar="DATABASE",
                        help="A socket on which ovsdb-server is listening.",
                        dest='database')

    ovs.vlog.add_args(parser)
    ovs.daemon.add_args(parser)
    args = parser.parse_args()
    ovs.vlog.handle_args(args)
    ovs.daemon.handle_args(args)

    if args.database is None:
        remote = def_db
    else:
        remote = args.database

    schema_helper = ovs.db.idl.SchemaHelper(location=ovs_schema)
    schema_helper.register_columns(SYSTEM_TABLE, ["cur_cfg"])

    schema_helper.register_table(DNS_CLIENT_TABLE)
    schema_helper.register_columns(DNS_CLIENT_TABLE,
                                   [DNS_CLIENT_VRF,
                                    DNS_CLIENT_DOMAIN_LIST,
                                    DNS_CLIENT_HOST_V4_ADDRESS_MAPPING,
                                    DNS_CLIENT_HOST_V6_ADDRESS_MAPPING,
                                    DNS_CLIENT_NAME_SERVERS,
                                    DNS_CLIENT_OTHER_CONFIG])


    idl = ovs.db.idl.Idl(remote, schema_helper)

    ovs.daemon.daemonize()

    ops_diagdump.init_diag_dump_basic(diag_basic_handler)

    ovs.unixctl.command_register("exit", "", 0, 0, unixctl_exit, None)
    error, unixctl_server = ovs.unixctl.server.UnixctlServer.create(None)
    if error:
        ovs.util.ovs_fatal(error, "could not create unixctl server", vlog)

    seqno = idl.change_seqno  # Sequence number when last processed the db

    while not exiting:
        unixctl_server.run()

        dns_client_run()

        if exiting:
            break

        poller = ovs.poller.Poller()
        unixctl_server.wait(poller)
        idl.wait(poller)
        poller.block()

    #Daemon Exit
    unixctl_server.close()
    idl.close()

    return

if __name__ == '__main__':
    try:
        main()
    except SystemExit:
        # Let system.exit() calls complete normally
        raise
    except:
        vlog.exception("traceback")
        sys.exit(ovs.daemon.RESTART_EXIT_CODE)
