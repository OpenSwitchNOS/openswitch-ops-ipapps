#!/usr/bin/env python

# Copyright (C) 2015 Hewlett Packard Enterprise Development LP
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.

import pytest
import re
from opstestfw import *
from opstestfw.switch.CLI import *
from opstestfw.switch import *

# Topology definition
topoDict = {"topoExecution": 1000,
            "topoTarget": "dut01",
            "topoDevices": "dut01",
            "topoFilters": "dut01:system-category:switch"}


def enterVtyshShell(dut01):
    retStruct = dut01.VtyshShell(enter=True)
    retCode = retStruct.returnCode()
    assert retCode == 0, "Failed to enter vtysh prompt"

    return True


def enterConfigShell(dut01):

    retStruct = dut01.DeviceInteract(command="configure terminal")
    retCode = retStruct.get('returnCode')
    assert retCode == 0, "Failed to enter config terminal"

    return True


def enterInterfaceContext(dut01, interface):

    cmd = "interface " + str(interface)
    devIntReturn = dut01.DeviceInteract(command=cmd)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    return True


def udp_bcast_forwarder_enable(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip udp-bcast-forward")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable UDP Bcast Forwarding failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip forward-protocol")
    assert 'UDP Broadcast Forwarder : enabled' in cmdOut,\
           "Test to enable UDP Bcast" \
           " Forwarding failed"

    return True


def udp_bcast_forwarder_disable(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="no ip udp-bcast-forward")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to disable UDP Bcast Forwarding failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip forward-protocol")
    assert 'UDP Broadcast Forwarder : disabled' in cmdOut, \
           "Test to disable UDP Bcast " \
           "Forwarding failed"

    return True


def show_udp_bcast_forwarder(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    #Enable the UDP Bcast Forwarder and then verify the show command.
    devIntReturn = dut01.DeviceInteract(command="ip udp-bcast-forward")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable UDP Bcast Forwarder failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip forward-protocol")
    assert 'enabled' in cmdOut, "Test to show UDP Bcast Forwarder " \
                                " configuration failed"

    return True


def udp_bcast_forwarder_runningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip udp-bcast-forward")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Enabling UDP Bcast Forwading failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'ip udp-bcast-forward' in cmdOut, \
           "Test to show UDP Bcast Forwarding " \
           "in running config failed"

    return True


def forward_protocol_invalidParams(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    interface = 17
    if (enterInterfaceContext(dut01, interface) is False):
        return False

    serverIp = "15.15.15.15"
    # Invalid UDP Port number
    uPort = 81
    cmd = "ip forward-protocol udp "+serverIp+" "+str(uPort)

    devIntReturn = dut01.DeviceInteract(command=cmd)
    cmdOut = devIntReturn.get('buffer')
    assert 'Invalid UDP portname/portnumber entered.' \
           in cmdOut, "Test to configure forward-protocol " \
                      "with invalid UDP port failed"

    # Invalid UDP Protocol name
    uProtocol = "invalidUDP"
    cmd = "ip forward-protocol udp "+serverIp+" "+uProtocol

    devIntReturn = dut01.DeviceInteract(command=cmd)
    cmdOut = devIntReturn.get('buffer')
    assert 'Invalid UDP portname/portnumber entered.' \
           in cmdOut, "Test to configure forward-protocol " \
                      "with invalid UDP Protocol name failed"
    dut01.DeviceInteract(command="end")

    return True


def forward_protocol_unicastServerIp(dut01):

    # Enter the Configure terminal and configure the forward-protocol
    # on an interface.
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1) is False):
        return False

    cmd = "ip forward-protocol udp 1.1.1.1 53"
    devIntReturn = dut01.DeviceInteract(command=cmd)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to set forward-protocol " \
                         "configuration for unicast " \
                         "server IP is failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip forward-protocol")
    assert '  1.1.1.1                53' in cmdOut, \
           "Test to set forward-protocol configuration" \
           "for unicast server IP failed"

    return True


def forward_protocol_unconfigure_unicastServerIp(dut01):

    # Enter the Configure terminal and unconfigure the forward-protocol
    # on an interface.
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1) is False):
        return False

    cmd = "no ip forward-protocol udp 1.1.1.1 53"
    devIntReturn = dut01.DeviceInteract(command=cmd)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to unset forward-protocol " \
                         "configuration for unicast " \
                         "server IP failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip forward-protocol")
    assert '  1.1.1.1                53' not in cmdOut, \
           "Test to unset forward-protocol configuration" \
           "for unicast server IP failed"

    return True


def forward_protocol_configure_bcast_fwd_disable(dut01):

    # Disable the UDP Bcast Forwarder
    udp_bcast_forwarder_disable(dut01)

    cmd = "ip forward-protocol udp 1.1.1.5 69"
    if (enterConfigShell(dut01) is False):
        return False
    if (enterInterfaceContext(dut01, 1) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command=cmd)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to set forward-protocol " \
                         "when UDP Bcast Forwarder is disabled"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip forward-protocol")
    assert '  1.1.1.5                69' in cmdOut, \
           "Test to set forward-protocol configuration" \
           "when UDP Bcast Forwader is disabled - failed"

    # Re-enable the UDP Bcast Forwarder
    udp_bcast_forwarder_enable(dut01)

    return True


def forward_protocol_subnetBroadcastServerIp(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    interface = 2
    if (enterInterfaceContext(dut01, interface) is False):
        return False

    cmd = "ip forward-protocol udp 10.0.0.255 69"
    devIntReturn = dut01.DeviceInteract(command=cmd)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute the " \
                         "forward-protocol for subnet "\
                         "broadcast server IP"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip forward-protocol")
    assert '  10.0.0.255                69' in cmdOut, \
           "Test to set forward-protocol " \
           "for subnet broadcast server IP - failed"

    return True


def forward_protocol_configure_sub_interface(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    interface = 1.1  # sub-interface
    if (enterInterfaceContext(dut01, interface) is False):
        return False

    cmd = "ip forward-protocol udp 10.0.1.1 123"
    devIntReturn = dut01.DeviceInteract(command=cmd)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute the " \
                         "forward-protocol on sub-interface"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip forward-protocol")
    assert '  10.0.1.1                123' in cmdOut, \
           "Test to set forward-protocol " \
           "on sub-interface - failed"

    return True


def forward_protocol_configure_split_interface(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    interface = 54-4  # split-interface
    if (enterInterfaceContext(dut01, interface) is False):
        return False

    cmd = "ip forward-protocol udp 10.0.1.11 137"
    devIntReturn = dut01.DeviceInteract(command=cmd)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute the " \
                         "forward-protocol on split-interface"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip forward-protocol")
    assert '  10.0.1.11                137' in cmdOut, \
           "Test to set forward-protocol " \
           "on split-interface - failed"

    return True


def forward_protocol_configure_multipleTest01(dut01):
    # Test to configure multiple server IPs for multiple UDP
    # ports but on single interface

    if (enterConfigShell(dut01) is False):
        return False

    interface = 2
    if (enterInterfaceContext(dut01, interface) is False):
        return False

    cmd1 = "ip forward-protocol udp 4.4.4.1 53"
    cmd2 = "ip forward-protocol udp 4.4.4.2 69"
    cmd3 = "ip forward-protocol udp 4.4.4.4 161"
    cmd4 = "ip forward-protocol udp 4.4.4.9 123"

    devIntReturn = dut01.DeviceInteract(command=cmd1)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP"

    devIntReturn = dut01.DeviceInteract(command=cmd2)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP"

    devIntReturn = dut01.DeviceInteract(command=cmd3)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP"

    devIntReturn = dut01.DeviceInteract(command=cmd4)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP"
    dut01.DeviceInteract(command="end")

    showCmd = "show ip forward-protocol interface "+str(interface)
    devIntReturn = dut01.DeviceInteract(command=showCmd)
    retCode = devIntReturn.get('returnCode')
    showCmdOut = devIntReturn.get('buffer')
    assert retCode == 0, "Unable to execute show forward-protocol"

    assert '4.4.4.9                123' and \
           '4.4.4.4                161' and \
           '4.4.4.1                53' and \
           '4.4.4.2                69'  \
           in showCmdOut, "Test to configure multiple server " \
                          "IP for multiple UDP ports on " \
                          "a single interface failed"

    return True


def forward_protocol_configure_multipleTest02(dut01):
    # Test to configure same server IP and same UDP port
    # but on multiple interfaces

    if (enterConfigShell(dut01) is False):
        return False

    interface = 5
    if (enterInterfaceContext(dut01, interface) is False):
        return False

    cmd1 = "ip forward-protocol udp 11.11.11.11 69"
    devIntReturn = dut01.DeviceInteract(command=cmd1)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP on interface "+str(interface)
    dut01.DeviceInteract(command="end")

    cmd2 = "show ip forward-protocol interface "+str(interface)
    cmdOut = dut01.cmdVtysh(command=cmd2)
    assert 'Interface: 5' and \
           '11.11.11.11                69' in cmdOut, \
           "Test to configure same server " \
           "IP for same UDP port but on " \
           "a multiple interfaces failed"

    if (enterConfigShell(dut01) is False):
        return False

    interface = 6
    if (enterInterfaceContext(dut01, interface) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command=cmd1)
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP on interface "+str(interface)
    dut01.DeviceInteract(command="end")

    cmd2 = "show ip forward-protocol interface "+str(interface)
    cmdOut = dut01.cmdVtysh(command=cmd2)
    assert 'Interface: 6' and \
           '11.11.11.11                69' in cmdOut, \
           "Test to configure same server " \
           "IP for same UDP port but on " \
           "a multiple interfaces failed"

    if (enterConfigShell(dut01) is False):
        return False

    interface = 7
    if (enterInterfaceContext(dut01, interface) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command=cmd1)
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP on interface "+str(interface)
    dut01.DeviceInteract(command="end")

    cmd2 = "show ip forward-protocol interface "+str(interface)
    cmdOut = dut01.cmdVtysh(command=cmd2)
    assert 'Interface: 7' and \
           '11.11.11.11                69' in cmdOut, \
           "Test to configure same server " \
           "IP for same UDP port but on " \
           "a multiple interfaces failed"

    return True


def forward_protocol_configure_multipleTest03(dut01):
    # Test to configure multiple server IP but on same UDP port
    # and on single interface

    if (enterConfigShell(dut01) is False):
        return False

    interface = 8
    if (enterInterfaceContext(dut01, interface) is False):
        return False

    cmd1 = "ip forward-protocol udp 11.11.11.11 161"
    cmd2 = "ip forward-protocol udp 11.11.11.22 161"
    cmd3 = "ip forward-protocol udp 11.11.11.33 161"

    devIntReturn = dut01.DeviceInteract(command=cmd1)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP on interface "+str(interface)

    devIntReturn = dut01.DeviceInteract(command=cmd2)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP on interface "+str(interface)

    devIntReturn = dut01.DeviceInteract(command=cmd3)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP on interface "+str(interface)
    dut01.DeviceInteract(command="end")

    cmd4 = "show ip forward-protocol interface "+str(interface)
    cmdOut = dut01.cmdVtysh(command=cmd4)
    assert 'Interface: 8' and \
           '11.11.11.11                161' and \
           '11.11.11.22                161' and \
           '11.11.11.33                161' \
           in cmdOut, "Test to configure multiple server " \
                      "IP for same UDP port but on " \
                      "a single interface failed"

    return True


def forward_protocol_configure_multipleTest04(dut01):
    # Test to configure same server IP but multiple UDP ports
    # on single interface

    if (enterConfigShell(dut01) is False):
        return False

    interface = 9
    if (enterInterfaceContext(dut01, interface) is False):
        return False

    cmd1 = "ip forward-protocol udp 11.11.0.11 123"
    cmd2 = "ip forward-protocol udp 11.11.0.11 161"
    cmd3 = "ip forward-protocol udp 11.11.0.11 137"

    devIntReturn = dut01.DeviceInteract(command=cmd1)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP on interface "+str(interface)

    devIntReturn = dut01.DeviceInteract(command=cmd2)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP on interface "+str(interface)

    devIntReturn = dut01.DeviceInteract(command=cmd3)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP on interface "+str(interface)
    dut01.DeviceInteract(command="end")

    cmd4 = "show ip forward-protocol interface "+str(interface)
    cmdOut = dut01.cmdVtysh(command=cmd4)
    assert 'Interface: 9' and \
           '11.11.0.11                123' and \
           '11.11.0.11                161' and \
           '11.11.0.11                137' \
           in cmdOut, "Test to configure same server " \
                      "IP but multiple UDP port and on " \
                      "a single interface failed"

    return True


def forward_protocol_configure_multipleTest05(dut01):
    # Test to configure same server IP but multiple UDP ports
    # on multiple interfaces

    if (enterConfigShell(dut01) is False):
        return False

    interface = 10
    if (enterInterfaceContext(dut01, interface) is False):
        return False

    cmd1 = "ip forward-protocol udp 8.8.8.8 69"
    cmd2 = "ip forward-protocol udp 8.8.8.8 123"
    cmd3 = "ip forward-protocol udp 8.8.8.8 137"

    devIntReturn = dut01.DeviceInteract(command=cmd1)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP on interface "+str(interface)
    dut01.DeviceInteract(command="end")

    if (enterConfigShell(dut01) is False):
        return False

    interface = 11
    if (enterInterfaceContext(dut01, interface) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command=cmd2)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP on interface "+str(interface)
    dut01.DeviceInteract(command="end")

    if (enterConfigShell(dut01) is False):
        return False

    interface = 12
    if (enterInterfaceContext(dut01, interface) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command=cmd3)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP on interface "+str(interface)
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip forward-protocol")
    assert '  8.8.8.8                69' and \
           '  8.8.8.8                123' and \
           '  8.8.8.8                137' \
           in cmdOut, "Test to configure same server " \
                      "IP but with multiple UDP port and on " \
                      "multiple interfaces"

    return True


def forward_protocol_unconfigure_non_exist(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1) is False):
        return False

    cmd = "no ip forward-protocol udp 1.1.1.1 53"
    devIntReturn = dut01.DeviceInteract(command=cmd)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to unset forward-protocol " \
                         "configuration of a non-existing " \
                         "unicast server IP"

    cmdOut = devIntReturn.get('buffer')
    assert 'UDP forward-protocol is not present.' in cmdOut, \
           "Test to unset forward-protocol " \
           "of a non-existing server IP failed"
    dut01.DeviceInteract(command="end")

    return True


def forward_protocol_configure_multicastServerIp(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1) is False):
        return False

    cmd = "ip forward-protocol udp 244.0.0.0 123"
    devIntReturn = dut01.DeviceInteract(command=cmd)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a multicast " \
                         "server IP"

    cmdOut = devIntReturn.get('buffer')
    assert 'Broadcast, multicast and loopback addresses are not allowed.' \
           in cmdOut, "Test to configure forward-protocol " \
                      "for multicast server IP failed"
    dut01.DeviceInteract(command="end")

    return True


def forward_protocol_configure_broadcastServerIp(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1) is False):
        return False

    cmd = "ip forward-protocol udp 255.255.255.255 dns"
    devIntReturn = dut01.DeviceInteract(command=cmd)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for an anynet broadcast " \
                         "server IP"

    cmdOut = devIntReturn.get('buffer')
    assert 'Broadcast, multicast and loopback addresses are not allowed.' \
           in cmdOut, "Test to configure forward-protocol " \
                      "for an anynet broadcast server IP failed"
    dut01.DeviceInteract(command="end")

    return True


def forward_protocol_configure_loopbackServerIp(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1) is False):
        return False

    cmd = "ip forward-protocol udp 127.0.0.0 dns"
    devIntReturn = dut01.DeviceInteract(command=cmd)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a loopback " \
                         "server IP"

    cmdOut = devIntReturn.get('buffer')
    assert 'Broadcast, multicast and loopback addresses are not allowed.' \
           in cmdOut, "Test to configure forward-protocol " \
                      "for loopback server IP failed"
    dut01.DeviceInteract(command="end")

    return True


def forward_protocol_configure_multicastServerIp(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1) is False):
        return False

    cmd = "ip forward-protocol udp 244.0.0.0 dns"
    devIntReturn = dut01.DeviceInteract(command=cmd)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a loopback " \
                         "server IP"

    cmdOut = devIntReturn.get('buffer')
    assert 'Broadcast, multicast and loopback addresses are not allowed.' \
           in cmdOut, "Test to configure forward-protocol " \
                      "for loopback server IP failed"
    dut01.DeviceInteract(command="end")

    return True


def forward_protocol_configure_invalidServerIp(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1) is False):
        return False

    cmd = "ip forward-protocol udp 0.0.0.0 snmp"
    devIntReturn = dut01.DeviceInteract(command=cmd)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for an invalid " \
                         "server IP"

    cmdOut = devIntReturn.get('buffer')
    assert 'Broadcast, multicast and loopback addresses are not allowed.' \
           in cmdOut, "Test to configure forward-protocol " \
                      "for invalid server IP failed"
    dut01.DeviceInteract(command="end")

    return True


def forward_protocol_configure_duplication(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1) is False):
        return False

    cmd1 = "ip forward-protocol udp 4.4.4.1 53"
    cmd2 = "ip forward-protocol udp 4.4.4.1 53"
    devIntReturn = dut01.DeviceInteract(command=cmd1)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP"

    devIntReturn = dut01.DeviceInteract(command=cmd2)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to execute forward-protocol " \
                         "configuration for a " \
                         "server IP"

    cmdOut = devIntReturn.get('buffer')
    assert 'This entry already exists.' in cmdOut, \
           "Test to verify forward-protocol " \
           "for duplicate entries failed"
    dut01.DeviceInteract(command="end")

    return True


def forward_protocol_maximumEntry_perPort(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    interface = 3
    if (enterInterfaceContext(dut01, interface) is False):
        return False

    cmd1 = "ip forward-protocol udp 5.5.5.1 123"
    cmd2 = "ip forward-protocol udp 5.5.5.2 123"
    cmd3 = "ip forward-protocol udp 5.5.5.3 123"
    cmd4 = "ip forward-protocol udp 5.5.5.4 123"
    cmd5 = "ip forward-protocol udp 5.5.5.5 123"
    cmd6 = "ip forward-protocol udp 5.5.5.6 123"
    cmd7 = "ip forward-protocol udp 5.5.5.7 123"
    cmd8 = "ip forward-protocol udp 5.5.5.8 123"
    cmd9 = "ip forward-protocol udp 5.5.5.9 123"
    cmd10 = "ip forward-protocol udp 5.5.5.10 123"
    cmd11 = "ip forward-protocol udp 5.5.5.11 123"
    cmd12 = "ip forward-protocol udp 5.5.5.12 123"
    cmd13 = "ip forward-protocol udp 5.5.5.13 123"
    cmd14 = "ip forward-protocol udp 5.5.5.14 123"
    cmd15 = "ip forward-protocol udp 5.5.5.15 123"
    cmd16 = "ip forward-protocol udp 5.5.5.16 123"
    cmd17 = "ip forward-protocol udp 5.5.5.17 123"

    devIntReturn = dut01.DeviceInteract(command=cmd1)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd1 forward-protocol"
    devIntReturn = dut01.DeviceInteract(command=cmd2)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd2 forward-protocol"
    devIntReturn = dut01.DeviceInteract(command=cmd3)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd3 forward-protocol"
    devIntReturn = dut01.DeviceInteract(command=cmd4)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd4 forward-protocol"
    devIntReturn = dut01.DeviceInteract(command=cmd5)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd5 forward-protocol"
    devIntReturn = dut01.DeviceInteract(command=cmd6)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd6 forward-protocol"
    devIntReturn = dut01.DeviceInteract(command=cmd7)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd7 forward-protocol"
    devIntReturn = dut01.DeviceInteract(command=cmd8)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd8 forward-protocol"
    devIntReturn = dut01.DeviceInteract(command=cmd9)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd9 forward-protocol"
    devIntReturn = dut01.DeviceInteract(command=cmd10)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd10 forward-protocol"
    devIntReturn = dut01.DeviceInteract(command=cmd11)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd11 forward-protocol"
    devIntReturn = dut01.DeviceInteract(command=cmd12)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd12 forward-protocol"
    devIntReturn = dut01.DeviceInteract(command=cmd13)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd13 forward-protocol"
    devIntReturn = dut01.DeviceInteract(command=cmd14)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd14 forward-protocol"
    devIntReturn = dut01.DeviceInteract(command=cmd15)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd15 forward-protocol"
    devIntReturn = dut01.DeviceInteract(command=cmd16)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd16 forward-protocol"
    devIntReturn = dut01.DeviceInteract(command=cmd17)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd17 forward-protocol"

    cmdOut = devIntReturn.get('buffer')
#    assert 'Entry not allowed as maximum entries per interface exceeded.' \
    assert 'Command failed' \
           in cmdOut, \
           "Test to validate maximum forward-protocol "\
           "entries per UDP port per interface failed"
    dut01.DeviceInteract(command="end")

    return True


def forward_protocol_inRunningConfig(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    interface = 19
    if (enterInterfaceContext(dut01, interface) is False):
        return False

    cmd1 = "ip forward-protocol udp 3.3.3.1 53"
    cmd2 = "ip forward-protocol udp 3.3.3.2 69"
    devIntReturn = dut01.DeviceInteract(command=cmd1)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd1 forward-protocol"
    devIntReturn = dut01.DeviceInteract(command=cmd2)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue cmd2 forward-protocol"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'ip forward-protocol udp 3.3.3.1 53' and \
           'ip forward-protocol udp 3.3.3.2 69' in cmdOut, \
           "Test to verify forward-protocol " \
           "configuration in running config failed"

    return True


# Support function to reboot the switch
def switch_reboot(deviceObj):
    LogOutput('info', "Reboot switch " + deviceObj.device)
    deviceObj.Reboot()
    rebootRetStruct = returnStruct(returnCode=0)
    return rebootRetStruct


def udp_forwarder_complete_post_reboot(dut01):

    # Enable UDP Bcast Forwarding
    udp_bcast_forwarder_enable(dut01)
    # Set a forward-protocol
    forward_protocol_unicastServerIp(dut01)

    # Save the running to start-up config
    runCfg = "copy running-config startup-config"
    devIntReturn = dut01.DeviceInteract(command=runCfg)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to save the running configuration"

    # Verify the configurations are saved in startup configs
    startCfg = "show startup-config"
    devIntReturn = dut01.DeviceInteract(command=startCfg)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to execute show startup-config"

    startCmdOut = devIntReturn.get('buffer')
    assert 'ip forward-protocol udp 1.1.1.1 53' and \
           'ip udp-bcast-forward' in startCmdOut, \
           "Configurations not saved, hence it is not " \
           "present in startup-config"

    # Perform reboot
    devRebootRetStruct = switch_reboot(dut01)
    if devRebootRetStruct.returnCode() != 0:
        LogOutput('error', "dut01 reboot - FAILED")
        assert(devRebootRetStruct.returnCode() == 0)
    else:
        LogOutput('info', "dut01 reboot - SUCCESS")

    # Check for the configs post reboot
    time.sleep(30)

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'ip forward-protocol udp 1.1.1.1 53' and \
           'ip udp-bcast-forward' in cmdOut, \
           "Test to verify UDP Bcast Forwarder configs " \
           "post reboot - failed"

    return True


@pytest.mark.skipif(True, reason="Disabling old tests")
class Test_udp_bcast_forwarder_configuration:
    def setup_class(cls):
        # Test object will parse command line and formulate the env.
        Test_udp_bcast_forwarder_configuration.testObj =\
            testEnviron(topoDict=topoDict, defSwitchContext="vtyShell")
        #    Get topology object.
        Test_udp_bcast_forwarder_configuration.topoObj = \
            Test_udp_bcast_forwarder_configuration.testObj.topoObjGet()

    def teardown_class(cls):
        Test_udp_bcast_forwarder_configuration.topoObj.terminate_nodes()

    def test_udp_bcast_forwarder_enable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to enable UDP Bcast Forwarding
        retValue = udp_bcast_forwarder_enable(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable UDP Bcast Forwarding - passed")
        else:
            LogOutput('error', "Enable UDP Bcast Forwarding - failed")

    def test_udp_bcast_forwarder_disable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to disable UDP Bcast Forwarding
        retValue = udp_bcast_forwarder_disable(dut01Obj)
        if(retValue):
            LogOutput('info', "Disable UDP Bcast Forwarding - passed")
        else:
            LogOutput('error', "Disable UDP Bcast Forwarding - failed")

    def test_show_udp_bcast_forwarder(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to show UDP Bcast Forwarding status
        retValue = show_udp_bcast_forwarder(dut01Obj)
        if(retValue):
            LogOutput('info', "Show UDP Bcast Forwarding "
                              "configuration - passed")
        else:
            LogOutput('error', "Show UDP Bcast Forwarding "
                               "configuration - failed")

    def test_udp_bcast_forwarder_runningconfig(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to show running config for UDP Bcast Forwarding
        retValue = udp_bcast_forwarder_runningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Show UDP Bcast Forwarding "
                              "running configuration - passed")
        else:
            LogOutput('error', "Show UDP Bcast Forwarding "
                               "running configuration - failed")

    def test_udp_forward_protocol_invalidparams(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to configure forward-protocol with invalid params
        retValue = forward_protocol_invalidParams(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to configure forward-protocol "
                              "with invalid params - passed")
        else:
            LogOutput('error', "Test to configure forward-protocol "
                              "with invalid params - passed")

    def test_udp_forward_protocol_unicastserver(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to configure forward-protocol Unicast Server IP
        retValue = forward_protocol_unicastServerIp(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set forward-protocol "
                              "configuration for unicast "
                              "server IP - passed")
        else:
            LogOutput('error', "Test to set forward-protocol "
                               "configuration for unicast "
                               "server IP - failed")

    def test_udp_forward_protocol_unicastserver_unconfigure(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to unconfigure forward-protocol Unicast Server IP
        retValue = forward_protocol_unconfigure_unicastServerIp(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to unset forward-protocol "
                              "configuration for unicast "
                              "server IP - passed")
        else:
            LogOutput('error', "Test to unset forward-protocol "
                               "configuration for unicast "
                               "server IP - failed")

    def test_udp_forward_protocol_configure_bcast_fwd_disable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to configure forward-protocol unicast server IP
        # when UDP Broadcast Forwarding is disabled
        retValue = forward_protocol_configure_bcast_fwd_disable(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set forward-protocol "
                              "configuration when UDP Broadcast "
                              "Forwarding is disabled - passed")
        else:
            LogOutput('error', "Test to set forward-protocol "
                               "configuration when UDP Broadcast "
                               "Forwarding is disabled - failed")

    def test_udp_forward_protocol_subinterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to configure forward-protocol on a sub-interface
        retValue = forward_protocol_configure_sub_interface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set forward-protocol "
                              "configuration on sub-interface - passed")
        else:
            LogOutput('error', "Test to set forward-protocol "
                               "configuration on sub-interface - failed")

    def test_udp_forward_protocol_splitinterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to configure forward-protocol on a split-interface
        retValue = forward_protocol_configure_split_interface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set forward-protocol "
                              "configuration on split-interface - passed")
        else:
            LogOutput('error', "Test to set forward-protocol "
                               "configuration on split-interface - failed")

    def test_udp_forward_protocol_subnetbroadcast(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to configure forward-protocol subnet broadcast server IP
        retValue = forward_protocol_subnetBroadcastServerIp(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set forward-protocol "
                              "configuration for subnet broadcast "
                              "server IP - passed")
        else:
            LogOutput('error', "Test to set forward-protocol "
                               "configuration for subnet broadcast "
                               "server IP - failed")

    def test_udp_forward_protocol_multipleTest01(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to configure multiple server IPs for multiple UDP
        # ports but on single interface
        retValue = forward_protocol_configure_multipleTest01(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set forward-protocol "
                              "configuration for multiple "
                              "server IP for multiple UDP ports "
                              "but on single interface - passed")
        else:
            LogOutput('error', "Test to set forward-protocol "
                               "configuration for multiple "
                               "server IP for multiple UDP ports "
                               "but on single interface - failed")

    def test_udp_forward_protocol_multipleTest02(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to configure same server IP and same UDP port
        # but on multiple interfaces
        retValue = forward_protocol_configure_multipleTest02(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set forward-protocol "
                              "configuration with same "
                              "server IP and same UDP port "
                              "but on multiple interfaces - passed")
        else:
            LogOutput('error', "Test to set forward-protocol "
                               "configuration with same "
                               "server IP and same UDP port "
                               "but on multiple interfaces - failed")

    def test_udp_forward_protocol_multipleTest03(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to configure multiple server IP but on same UDP port
        # and on single interface
        retValue = forward_protocol_configure_multipleTest03(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set forward-protocol "
                              "configuration with multiple "
                              "server IP but on same UDP port "
                              "and on single interface - passed")
        else:
            LogOutput('error', "Test to set forward-protocol "
                               "configuration with multiple "
                               "server IP but on same UDP port "
                               "and on single interface - failed")

    def test_udp_forward_protocol_multipleTest04(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to configure same server IP but multiple UDP ports
        # on single interface
        retValue = forward_protocol_configure_multipleTest04(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set forward-protocol "
                              "configuration with same "
                              "server IP but multiple UDP ports "
                              "on single interface - passed")
        else:
            LogOutput('error', "Test to set forward-protocol "
                               "configuration with same "
                               "server IP but multiple UDP ports "
                               "on single interface - failed")

    def test_udp_forward_protocol_multipleTest05(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to configure same server IP but multiple UDP ports
        # on multiple interfaces
        retValue = forward_protocol_configure_multipleTest05(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set forward-protocol "
                              "configuration with same "
                              "server IP but multiple UDP ports "
                              "on multiple interfaces - passed")
        else:
            LogOutput('error', "Test to set forward-protocol "
                               "configuration with same "
                               "server IP but multiple UDP ports "
                               "on multiple interfaces - failed")

#    def test_udp_forward_protocol_maxentry(self):
 #       dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to verify the maximum entries allowed per UDP
        # port per interface
#        retValue = forward_protocol_maximumEntry_perPort(dut01Obj)
 #       if(retValue):
  #          LogOutput('info', "Test to set maximum forward-protocol "
   #                           "entries per port on an interface - passed")
    #    else:
     #       LogOutput('error', "Test to set maximum forward-protocol "
      #                         "entries per port on an interface - failed")

    def test_udp_forward_protocol_non_exist(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to unconfigure a non-existing forward-protocol server IP
        retValue = forward_protocol_unconfigure_non_exist(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to unset forward-protocol "
                              "configuration for a non-existing "
                              "server IP - passed")
        else:
            LogOutput('error', "Test to unset forward-protocol "
                               "configuration for a non-existing "
                               "server IP - failed")

    def test_udp_forward_protocol_multicastserver(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to configure a Multicast server IP
        retValue = forward_protocol_configure_multicastServerIp(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set forward-protocol "
                              "configuration for multicast "
                              "server IP - passed")
        else:
            LogOutput('error', "Test to set forward-protocol "
                               "configuration for multicast "
                               "server IP - failed")

    def test_udp_forward_protocol_broadcastserver(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to configure an anynet Broadcast server IP
        retValue = forward_protocol_configure_broadcastServerIp(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set forward-protocol "
                              "configuration for broadcast "
                              "server IP - passed")
        else:
            LogOutput('error', "Test to set forward-protocol "
                               "configuration for broadcast "
                               "server IP - failed")

    def test_udp_forward_protocol_loopbackserver(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to configure a Loopback server IP
        retValue = forward_protocol_configure_loopbackServerIp(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set forward-protocol "
                              "configuration for loopback "
                              "server IP - passed")
        else:
            LogOutput('error', "Test to set forward-protocol "
                               "configuration for loopback "
                               "server IP - failed")

    def test_udp_forward_protocol_multicastserver(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to configure a Multicast server IP
        retValue = forward_protocol_configure_multicastServerIp(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set forward-protocol "
                              "configuration for multicast "
                              "server IP - passed")
        else:
            LogOutput('error', "Test to set forward-protocol "
                               "configuration for multicast "
                               "server IP - failed")

    def test_udp_forward_protocol_invalidserver(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to configure an invalid server IP
        retValue = forward_protocol_configure_invalidServerIp(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set forward-protocol "
                              "configuration for invalid "
                              "server IP - passed")
        else:
            LogOutput('error', "Test to set forward-protocol "
                               "configuration for invalid "
                               "server IP - failed")

    def test_udp_forward_protocol_duplicastes(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to configure duplicate server IP
        retValue = forward_protocol_configure_duplication(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set forward-protocol "
                              "configuration for duplicate "
                              "server IP - passed")
        else:
            LogOutput('error', "Test to set forward-protocol "
                               "configuration for duplicate "
                               "server IP - failed")

    def test_udp_forward_protocol_runnconfig(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to verify the UDP Broadcast Forwarder details in
        # show running
        retValue = forward_protocol_inRunningConfig(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to verify UDP Broadcast "
                              "Forwarder details in show "
                              "running - passed")
        else:
            LogOutput('error', "Test to verify UDP Broadcast "
                              "Forwarder details in show "
                              "running - failed")

#   def test_udp_forward_protocol_post_reboot(self):
 #      dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        # Test to verify configurations post reboot
        #retValue = udp_forwarder_complete_post_reboot(dut01Obj)
#        if(retValue):
 #           LogOutput('info', "UDP Bcast Forwarder " \
  #                            "post Reboot - passed")
   #     else:
    #        LogOutput('info', "UDP Bcast Forwarder " \
     #                         "post Reboot - failed")
