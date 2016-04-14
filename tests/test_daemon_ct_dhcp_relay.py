#!/usr/bin/env python

# Copyright (C) 2016 Hewlett Packard Enterprise Development LP
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


def enterConfigShell(dut01):
    retStruct = dut01.VtyshShell(enter=True)
    retCode = retStruct.returnCode()
    assert retCode == 0, "Failed to enter vtysh prompt"

    retStruct = dut01.ConfigVtyShell(enter=True)
    retCode = retStruct.returnCode()
    assert retCode == 0, "Failed to enter config terminal"

    return True


def exitContext(dut01):
    retStruct = dut01.ConfigVtyShell(enter=False)
    retCode = retStruct.returnCode()
    assert retCode == 0, "Failed to exit config terminal"

    retStruct = dut01.VtyshShell(enter=False)
    retCode = retStruct.returnCode()
    assert retCode == 0, "Failed to exit vtysh prompt"

    return True


def dhcp_relay_enable(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    LogOutput('info', "Test to enable DHCP relay")
    devIntReturn = dut01.DeviceInteract(command="dhcp-relay")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to execute no dhcp-relay command"

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    assert 'DHCP Relay : 1' in output, "Test to enable"
    " dhcp-relay failed"

    return True


def dhcp_relay_disable(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    LogOutput('info', "Test to disable DHCP relay")
    devIntReturn = dut01.DeviceInteract(command="no dhcp-relay")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to execute no dhcp-relay command"

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    assert 'DHCP Relay : 0' in output, "Test to disable"
    " dhcp-relay failed"

    return True


def dhcp_relay_hop_count_increment_enable(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    LogOutput('info', "Test to enable DHCP relay hop count increment")
    devIntReturn = dut01.DeviceInteract(command="dhcp-relay"
                                        " hop-count-increment")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to execute dhcp-relay hop-count"
    "increment command"

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    assert 'DHCP Relay hop-count-increment : 1' in output, "Test to enable"
    " dhcp-relay hop-count increment failed"

    return True


def dhcp_relay_hop_count_increment_disable(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    LogOutput('info', "Test to disable DHCP relay hop count increment")
    devIntReturn = dut01.DeviceInteract(command="no dhcp-relay"
                                        " hop-count-increment")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to execute dhcp-relay hop-count"
    "increment command"

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    assert 'DHCP Relay hop-count-increment : 0' in output, "Test to disable"
    " dhcp-relay hop-count increment failed"

    return True


def dhcp_relay_option_82_enable(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    LogOutput('info', "Test to enable DHCP relay option 82")
    devIntReturn = dut01.DeviceInteract(command="dhcp-relay"
                                        " option 82 replace")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to execute dhcp-relay option 82 command"

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    assert 'DHCP Relay Option82 : 1' in output, "Test to enable"
    " dhcp-relay option 82 failed"

    return True


def dhcp_relay_option_82_disable(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    LogOutput('info', "Test to disable DHCP relay option 82")
    devIntReturn = dut01.DeviceInteract(command="no dhcp-relay option 82")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to execute dhcp-relay option 82 command"

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    assert 'DHCP Relay Option82 : 0' in output, "Test to disable"
    " dhcp-relay option 82 failed"

    return True


def dhcp_relay_option_82_validation_enable(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    LogOutput('info', "Test to enable DHCP relay option 82"
              " response validation")
    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option"
                                        " 82 validate")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to execute dhcp-relay option 82 "
    "response validation command"

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    assert 'DHCP Relay Option82 validate : 1' in output, "Test to enable"
    " dhcp-relay option 82 response validation failed"

    return True


def dhcp_relay_option_82_validation_disable(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    LogOutput('info', "Test to disable DHCP relay option 82"
              " response validation")
    devIntReturn = dut01.DeviceInteract(command="no dhcp-relay option"
                                        " 82 validate")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to execute dhcp-relay option 82 "
    "response validation command"

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    assert 'DHCP Relay Option82 validate : 0' in output, "Test to disable"
    " dhcp-relay option 82 response validation failed"

    return True


def dhcp_relay_option_82_drop_policy_enable(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    LogOutput('info', "Test to enable DHCP relay option 82"
              " drop policy")
    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option"
                                        " 82 drop")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to execute dhcp-relay option 82 "
    "drop policy enable command"

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    assert 'DHCP Relay Option82 policy : drop' in output, "Test to enable"
    " dhcp-relay option 82 drop policy failed"

    return True


def dhcp_relay_option_82_drop_policy_disable(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    LogOutput('info', "Test to disable DHCP relay option 82"
              " drop policy")
    devIntReturn = dut01.DeviceInteract(command="no dhcp-relay option 82")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to execute dhcp-relay option 82 "
    "drop policy disable command"

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    assert 'DHCP Relay Option82 policy : drop' not in output, "Test to disable"
    " dhcp-relay option 82 drop policy failed"

    return True


def dhcp_relay_option_82_keep_policy_enable(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    LogOutput('info', "Test to enable DHCP relay option 82"
              " keep policy")
    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option"
                                        " 82 keep")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to execute dhcp-relay option 82 "
    "keep policy enable command"

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    assert 'DHCP Relay Option82 policy : keep' in output, "Test to enable"
    " dhcp-relay option 82 keep policy failed"

    return True


def dhcp_relay_option_82_replace_policy_enable(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    LogOutput('info', "Test to enable DHCP relay option 82"
              " replace policy")
    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option"
                                        " 82 replace")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to execute dhcp-relay option 82 "
    "replace policy enable command"

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    assert 'DHCP Relay Option82 policy : replace' in output, "Test to enable"
    " dhcp-relay option 82 replace policy failed"

    return True


def dhcp_relay_option_82_keep_policy_disable(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    LogOutput('info', "Test to disable DHCP relay option 82"
              " keep policy")
    devIntReturn = dut01.DeviceInteract(command="no dhcp-relay option 82")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to execute dhcp-relay option 82 "
    "keep policy disable command"

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    assert 'DHCP Relay Option82 policy : keep' not in output, "Test to"
    " disable dhcp-relay option 82 keep policy failed"

    return True


def dhcp_relay_option_82_status_when_relay_disabled(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    LogOutput('info', "Test to check status of DHCP relay option 82"
              " when DHCP-Relay is disabled")
    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 keep")
    devIntReturn = dut01.DeviceInteract(command="no dhcp-relay")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to execute no dhcp-relay command "

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    assert 'DHCP Relay Option82 : 0' in output, "Test to disable"
    " dhcp-relay option 82 failed"

    return True


def helper_address_configuration_per_interface(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="interface 1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    devIntReturn = dut01.DeviceInteract(command="ip helper-address "
                                        "192.168.10.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to execute helper-address"
    " configuration command"

    dut01.DeviceInteract(command="exit")

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 1")
    assert '192.168.10.1' and \
    'client request dropped packets = 0' and \
    'client request valid packets = 0' and \
    'server request dropped packets = 0' and \
    'server request valid packets = 0'
    'client request dropped packets with option 82 = 0' and \
    'client request valid packets with option 82 = 0' and \
    'server request dropped packets with option 82 = 0' and \
    'server request valid packets with option 82 = 0' in output, "Test to set "
    "helper-address configuration failed"

    # Remove configuration
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="interface 1")
    devIntReturn = dut01.DeviceInteract(command="no ip helper-address "
                                        "192.168.10.1")

    devIntReturn = dut01.DeviceInteract(command="exit")
    if (exitContext(dut01) is False):
        return False

    return True


def maximum_helper_address_configuration_per_interface(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="interface 2")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    devIntReturn = dut01.DeviceInteract(command="ip helper-address "
                                        "192.168.10.1")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address "
                                        "192.168.10.2")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address "
                                        "192.168.10.3")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address "
                                        "192.168.10.4")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address "
                                        "192.168.10.5")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address "
                                        "192.168.10.6")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address "
                                        "192.168.10.7")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address "
                                        "192.168.10.8")

    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to set maximum helper-address configuration"
    " per interface failed"

    dut01.DeviceInteract(command="exit")

    if (exitContext(dut01) is False):
        return False

    retBuffer = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 2")
    assert '192.168.10.1' in retBuffer \
        and '192.168.10.2' in retBuffer \
        and '192.168.10.3' in retBuffer \
        and '192.168.10.4' in retBuffer \
        and '192.168.10.5' in retBuffer \
        and '192.168.10.6' in retBuffer \
        and '192.168.10.7' in retBuffer \
        and '192.168.10.8' in retBuffer, "maximum helper-address"
    " configuration on interface 1 failed"

    # Remove configuration
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="interface 2")
    devIntReturn = dut01.DeviceInteract(command="no ip helper-address "
                                        "192.168.10.1")
    devIntReturn = dut01.DeviceInteract(command="no ip helper-address "
                                        "192.168.10.2")
    devIntReturn = dut01.DeviceInteract(command="no ip helper-address "
                                        "192.168.10.3")
    devIntReturn = dut01.DeviceInteract(command="no ip helper-address "
                                        "192.168.10.4")
    devIntReturn = dut01.DeviceInteract(command="no ip helper-address "
                                        "192.168.10.5")
    devIntReturn = dut01.DeviceInteract(command="no ip helper-address "
                                        "192.168.10.6")
    devIntReturn = dut01.DeviceInteract(command="no ip helper-address "
                                        "192.168.10.7")
    devIntReturn = dut01.DeviceInteract(command="no ip helper-address "
                                        "192.168.10.8")
    devIntReturn = dut01.DeviceInteract(command="exit")
    if (exitContext(dut01) is False):
        return False

    return True


def same_helper_address_on_multiple_interface(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="interface 1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    devIntReturn = dut01.DeviceInteract(command="ip helper-address 10.10.10.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, " Failed to execute helper-address"
    " configuration command"
    dut01.DeviceInteract(command="exit")

    devIntReturn = dut01.DeviceInteract(command="interface 3")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    devIntReturn = dut01.DeviceInteract(command="ip helper-address 10.10.10.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, " Failed to execute helper-address"
    " configuration command"
    dut01.DeviceInteract(command="exit")

    devIntReturn = dut01.DeviceInteract(command="interface 4")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    devIntReturn = dut01.DeviceInteract(command="ip helper-address 10.10.10.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, " Failed to execute helper-address"
    " configuration command"
    dut01.DeviceInteract(command="exit")

    devIntReturn = dut01.DeviceInteract(command="interface 5")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    devIntReturn = dut01.DeviceInteract(command="ip helper-address 10.10.10.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, " Failed to execute helper-address"
    " configuration command"
    dut01.DeviceInteract(command="exit")

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 5")
    lines = output.split('\n')
    for line in lines:
        if "10.10.10.1" in line:
            out = line.split(',')
            count = int(out[1])

    assert count == 4, "Test to set same"
    " helper-address configuration failed"

    #Remove configuration
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="interface 1")
    devIntReturn = dut01.DeviceInteract(command="no ip helper-address "
                                        "10.10.10.1")
    devIntReturn = dut01.DeviceInteract(command="exit")
    devIntReturn = dut01.DeviceInteract(command="interface 3")
    devIntReturn = dut01.DeviceInteract(command="no ip helper-address "
                                        "10.10.10.1")
    devIntReturn = dut01.DeviceInteract(command="exit")
    devIntReturn = dut01.DeviceInteract(command="interface 4")
    devIntReturn = dut01.DeviceInteract(command="no ip helper-address "
                                        "10.10.10.1")
    devIntReturn = dut01.DeviceInteract(command="exit")
    devIntReturn = dut01.DeviceInteract(command="interface 5")
    devIntReturn = dut01.DeviceInteract(command="no ip helper-address "
                                        "10.10.10.1")
    devIntReturn = dut01.DeviceInteract(command="exit")

    if (exitContext(dut01) is False):
        return False

    return True


def configure_bootp_gateway_address(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="interface 1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    devIntReturn = dut01.DeviceInteract(command="ip address 9.0.0.1/8")
    devIntReturn = dut01.DeviceInteract(command="ip bootp-gateway 9.0.0.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, " Failed to execute bootp gateway"
    " configuration command"

    dut01.DeviceInteract(command="exit")

    if (exitContext(dut01) is False):
        return False

    retBuffer = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 1")
    assert '9.0.0.1' in retBuffer, "Failed to set ip bootp gateway"

    # Remove configuration
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="interface 1")
    devIntReturn = dut01.DeviceInteract(command="no ip address "
                                        "9.0.0.1/8")
    devIntReturn = dut01.DeviceInteract(command="no ip bootp-gateway 9.0.0.1")
    devIntReturn = dut01.DeviceInteract(command="exit")

    if (exitContext(dut01) is False):
        return False

    # After unconfiguring also check value in daemon
    retBuffer = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 1")
    assert '9.0.0.1' not in retBuffer, "Failed to set ip bootp gateway"

    return True


def add_helper_addresses(dut01):

    # Create IP pool , adding 100 IP addresses
    global ipPool
    ipPool = []

    # 192.168.1.1 to 192.168.1.32
    for i in range(1, 33):
        ip = "192.168.1." + str(i)
        ipPool.append(ip)

    # 10.10.1.1 to 10.10.1.32
    for i in range(1, 33):
        ip = "10.10.1." + str(i)
        ipPool.append(ip)

    # 192.168.1.1 to 192.168.1.16
    for i in range(1, 17):
        ip = "192.168.1." + str(i)
        ipPool.append(ip)

    # 9.10.1.1 to 9.10.1.20
    for i in range(1, 21):
        ip = "9.10.1." + str(i)
        ipPool.append(ip)

    # 100 IP addresses
    print " number of ip address = %d" % len(ipPool)

    if (enterConfigShell(dut01) is False):
        return False

    #enter interface 11
    devIntReturn = dut01.DeviceInteract(command="interface 11")

    for k in range(0, 8):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 12
    devIntReturn = dut01.DeviceInteract(command="interface 12")

    for k in range(8, 16):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 13
    devIntReturn = dut01.DeviceInteract(command="interface 13")

    for k in range(16, 24):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 14
    devIntReturn = dut01.DeviceInteract(command="interface 14")

    for k in range(24, 32):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 15
    devIntReturn = dut01.DeviceInteract(command="interface 15")
    for k in range(32, 40):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 16
    devIntReturn = dut01.DeviceInteract(command="interface 16")

    for k in range(40, 48):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 17
    devIntReturn = dut01.DeviceInteract(command="interface 17")

    for k in range(48, 56):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 18
    devIntReturn = dut01.DeviceInteract(command="interface 18")
    for k in range(56, 64):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 19
    devIntReturn = dut01.DeviceInteract(command="interface 19")
    for k in range(64, 72):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 20
    devIntReturn = dut01.DeviceInteract(command="interface 20")
    for k in range(72, 80):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 21
    devIntReturn = dut01.DeviceInteract(command="interface 21")
    for k in range(80, 88):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 22
    devIntReturn = dut01.DeviceInteract(command="interface 22")
    for k in range(88, 96):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 23
    devIntReturn = dut01.DeviceInteract(command="interface 23")
    for k in range(96, 100):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    if (exitContext(dut01) is False):
        return False

    # Check output
    count = 0
    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    lines = output.split('\n')
    for line in lines:
        if 'Interface ' in line:
            out = line.split(':')
            count += int(out[1])

    #for debug
    print " length of ipPool = %d" % len(ipPool)
    print " value of count = %d" % count

    if len(ipPool) != count:
        print " Adding Helper Addresses failed"
        return False

    return True


def delete_helper_addresses(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    #enter interface 11
    devIntReturn = dut01.DeviceInteract(command="interface 11")

    for k in range(0, 8):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    #enter interface 12
    devIntReturn = dut01.DeviceInteract(command="interface 12")

    for k in range(8, 16):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    #enter interface 13
    devIntReturn = dut01.DeviceInteract(command="interface 13")

    for k in range(16, 24):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    #enter interface 14
    devIntReturn = dut01.DeviceInteract(command="interface 14")

    for k in range(24, 32):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    #enter interface 15
    devIntReturn = dut01.DeviceInteract(command="interface 15")

    for k in range(32, 40):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    #enter interface 16
    devIntReturn = dut01.DeviceInteract(command="interface 16")

    for k in range(40, 48):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    #enter interface 17
    devIntReturn = dut01.DeviceInteract(command="interface 17")

    for k in range(48, 56):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    #enter interface 18
    devIntReturn = dut01.DeviceInteract(command="interface 18")

    for k in range(56, 64):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    #enter interface 19
    devIntReturn = dut01.DeviceInteract(command="interface 19")

    for k in range(64, 72):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    #enter interface 20
    devIntReturn = dut01.DeviceInteract(command="interface 20")

    for k in range(72, 80):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    #enter interface 21
    devIntReturn = dut01.DeviceInteract(command="interface 21")

    for k in range(80, 88):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    #enter interface 22
    devIntReturn = dut01.DeviceInteract(command="interface 22")

    for k in range(88, 96):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    #enter interface 23
    devIntReturn = dut01.DeviceInteract(command="interface 23")

    for k in range(96, 100):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 11")
    assert 'No servers are configured on this interface :11' in output, "Helper"
    " address deletion on interface 11 failed"

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 12")
    assert 'No servers are configured on this interface :12' in output, "Helper"
    " address deletion on interface 12 failed"

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 13")
    assert 'No servers are configured on this interface :13' in output, "Helper"
    " address deletion on interface 13 failed"

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 14")
    assert 'No servers are configured on this interface :14' in output, "Helper"
    " address deletion on interface 14 failed"

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 15")
    assert 'No servers are configured on this interface :15' in output, "Helper"
    " address deletion on interface 15 failed"

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 16")
    assert 'No servers are configured on this interface :16' in output, "Helper"
    " address deletion on interface 16 failed"

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 17")
    assert 'No servers are configured on this interface :17' in output, "Helper"
    " address deletion on interface 17 failed"

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 18")
    assert 'No servers are configured on this interface :18' in output, "Helper"
    " address deletion on interface 18 failed"

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 19")
    assert 'No servers are configured on this interface :19' in output, "Helper"
    " address deletion on interface 19 failed"

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 20")
    assert 'No servers are configured on this interface :20' in output, "Helper"
    " address deletion on interface 20 failed"

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 21")
    assert 'No servers are configured on this interface :21' in output, "Helper"
    " address deletion on interface 21 failed"

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 22")
    assert 'No servers are configured on this interface :22' in output, "Helper"
    " address deletion on interface 22 failed"

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 23")
    assert 'No servers are configured on this interface :23' in output, "Helper"
    " address deletion on interface 22 failed"

    return True


class Test_dhcp_relay_configuration:

    def setup_class(cls):
        # Test object will parse command line and formulate the env
        Test_dhcp_relay_configuration.testObj = \
            testEnviron(topoDict=topoDict)
        #    Get topology object
        Test_dhcp_relay_configuration.topoObj = \
            Test_dhcp_relay_configuration.testObj.topoObjGet()

    def teardown_class(cls):
        Test_dhcp_relay_configuration.topoObj.terminate_nodes()

    def test_dhcp_relay_enable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_enable(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay - failed")

    def test_dhcp_relay_disable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_disable(dut01Obj)
        if(retValue):
            LogOutput('info', "Disable DHCP-Relay - passed")
        else:
            LogOutput('error', "Disable DHCP-Relay - failed")

    def test_dhcp_relay_hop_count_increment_enable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_hop_count_increment_enable(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay hop count "
                              "increment - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay hop count "
                               "increment - failed")

    def test_dhcp_relay_hop_count_increment_disable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_hop_count_increment_disable(dut01Obj)
        if(retValue):
            LogOutput('info', "Disable DHCP-Relay option 82 hop count "
                      "validation - passed")
        else:
            LogOutput('error', "Disable DHCP-Relay option 82 hop count "
                      "validation - failed")

    def test_dhcp_relay_option_82_enable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_option_82_enable(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 - passed")
        else:
            LogOutput('error', "Enable DDHCP-Relay option 82 - failed")

    def test_dhcp_relay_option_82_disable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_option_82_disable(dut01Obj)
        if(retValue):
            LogOutput('info', "Disable DHCP-Relay option 82 - passed")
        else:
            LogOutput('info', "Disable DHCP-Relay option 82 - failed")

    def test_dhcp_relay_option_82_validation_enable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_option_82_validation_enable(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 "
                      "validation - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 "
                      "validation - failed")

    def test_dhcp_relay_option_82_validation_disable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_option_82_validation_disable(dut01Obj)
        if(retValue):
            LogOutput('info', "Disable DHCP-Relay option 82 "
                      "validation - passed")
        else:
            LogOutput('error', "Disable DHCP-Relay option 82 "
                      "validation - failed")

    def test_dhcp_relay_option_82_drop_policy_enable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_option_82_drop_policy_enable(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 drop policy passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 drop policy "
                      "failed")

    def test_dhcp_relay_option_82_drop_policy_disable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_option_82_drop_policy_disable(dut01Obj)
        if(retValue):
            LogOutput('info', "Disable DHCP-Relay option 82 drop policy "
                      "passed")
        else:
            LogOutput('error', "Disable DHCP-Relay option 82 drop policy "
                      "failed")

    def test_dhcp_relay_option_82_keep_policy_enable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_option_82_keep_policy_enable(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 keep policy passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 keep policy "
                      "failed")

    def test_dhcp_relay_option_82_replace_policy_enable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_option_82_replace_policy_enable(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 replace "
                      " policy passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 replace policy "
                      "failed")

    def test_dhcp_relay_option_82_keep_policy_disable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_option_82_keep_policy_disable(dut01Obj)
        if(retValue):
            LogOutput('info', "Disable DHCP-Relay option 82 keep policy "
                      "passed")
        else:
            LogOutput('error', "Disable DHCP-Relay option 82 keep policy "
                      "failed")

    def test_dhcp_relay_option_82_status_when_relay_disabled(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_option_82_status_when_relay_disabled(dut01Obj)
        if(retValue):
            LogOutput('info', "Validation of DHCP-Relay option 82 status when "
                      "DHCP-Relay is disabled passed")
        else:
            LogOutput('error', "Validation of DHCP-Relay option 82 status when"
                      " DHCP-Relay is disabled failed")

    def test_helper_address_configuration_per_interface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = helper_address_configuration_per_interface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set helper-address configuration"
                      " per interface - passed")
        else:
            LogOutput('error', "Test to set helper-address configuration"
                      " per interface - failed")

    def test_maximum_helper_address_configuration_per_interface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = maximum_helper_address_configuration_per_interface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set maximum helper-address"
                      " configuration per interface - passed")
        else:
            LogOutput('error', "Test to set maximum helper-address"
                      " configuration per interface - failed")

    def test_same_helper_address_on_multiple_interface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = same_helper_address_on_multiple_interface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set same helper-address"
                      " configuration on multiple interface - passed")
        else:
            LogOutput('error', "Test to set same helper-address"
                      " configuration on multiple interface - failed")

    def test_configure_bootp_gateway_address(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = configure_bootp_gateway_address(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set bootp gateway"
                      " address on interface - passed")
        else:
            LogOutput('error', "Test to set bootp gateway"
                      " address on interface - failed")

    def test_add_helper_addresses(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = add_helper_addresses(dut01Obj)
        if(retValue):
            LogOutput('info', "Helper address addition stress test passed")
        else:
            LogOutput('error', "Helper address addition stress test failed")

    def test_delete_helper_addresses(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = delete_helper_addresses(dut01Obj)
        if(retValue):
            LogOutput('info', "Helper address deletion stress test passed")
        else:
            LogOutput('error', "Helper address deletion stress test failed")
