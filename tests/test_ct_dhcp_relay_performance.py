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
    print " no of ip address = %d" % len(ipPool)

    if (enterConfigShell(dut01) is False):
        return False

    #enter interface 1
    devIntReturn = dut01.DeviceInteract(command="interface 1")

    for k in range(0, 8):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 2
    devIntReturn = dut01.DeviceInteract(command="interface 2")

    for k in range(8, 16):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 3
    devIntReturn = dut01.DeviceInteract(command="interface 3")

    for k in range(16, 24):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 4
    devIntReturn = dut01.DeviceInteract(command="interface 4")

    for k in range(24, 32):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 5
    devIntReturn = dut01.DeviceInteract(command="interface 5")
    for k in range(32, 40):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 6
    devIntReturn = dut01.DeviceInteract(command="interface 6")

    for k in range(40, 48):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 7
    devIntReturn = dut01.DeviceInteract(command="interface 7")

    for k in range(48, 56):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 8
    devIntReturn = dut01.DeviceInteract(command="interface 8")
    for k in range(56, 64):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 9
    devIntReturn = dut01.DeviceInteract(command="interface 9")
    for k in range(64, 72):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 10
    devIntReturn = dut01.DeviceInteract(command="interface 10")
    for k in range(72, 80):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 11
    devIntReturn = dut01.DeviceInteract(command="interface 11")
    for k in range(80, 88):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 12
    devIntReturn = dut01.DeviceInteract(command="interface 12")
    for k in range(88, 96):
        cmd = "ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " configuration command"

    dut01.DeviceInteract(command="exit")

    #enter interface 13
    devIntReturn = dut01.DeviceInteract(command="interface 13")
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
    temp = []
    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    lines = output.split('\n')
    for line in lines:
        if "Server IP Address" in line:
            ip = line.split(':')
            temp.append(ip[1])

    #for debug
    print " length of temp = %d" % len(temp)
    print " length of ipPool = %d" % len(ipPool)

    if len(ipPool) != len(temp):
        print " Adding Helper Addresses failed"
        return False

    return True


def delete_helper_addresses(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    #enter interface 1
    devIntReturn = dut01.DeviceInteract(command="interface 1")

    for k in range(0, 8):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    #enter interface 2
    devIntReturn = dut01.DeviceInteract(command="interface 2")

    for k in range(8, 16):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    #enter interface 3
    devIntReturn = dut01.DeviceInteract(command="interface 3")

    for k in range(16, 24):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    #enter interface 4
    devIntReturn = dut01.DeviceInteract(command="interface 4")

    for k in range(24, 32):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    #enter interface 5
    devIntReturn = dut01.DeviceInteract(command="interface 5")

    for k in range(32, 40):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    #enter interface 6
    devIntReturn = dut01.DeviceInteract(command="interface 6")

    for k in range(40, 48):
        cmd = " no ip helper-address " + ipPool[k]
        devIntReturn = dut01.DeviceInteract(command=cmd)
        retCode = devIntReturn.get('returnCode')
        assert retCode == 0, "Failed to execute helper-address"
        " deletion command"

    dut01.DeviceInteract(command="exit")

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 1")
    assert 'No helper address configured on this interface' in output, "Helper"
    " address deletion on interface 1 failed"

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 2")
    assert 'No helper address configured on this interface' in output, "Helper"
    " address deletion on interface 2 failed"

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 3")
    assert 'No helper address configured on this interface' in output, "Helper"
    " address deletion on interface 3 failed"

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 4")
    assert 'No helper address configured on this interface' in output, "Helper"
    " address deletion on interface 4 failed"

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 5")
    assert 'No helper address configured on this interface' in output, "Helper"
    " address deletion on interface 5 failed"

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 6")
    assert 'No helper address configured on this interface' in output, "Helper"
    " address deletion on interface 6 failed"

    return True


class Test_stress_dhcp_relay_configuration:

    def setup_class(cls):
        # Test object will parse command line and formulate the env
        Test_stress_dhcp_relay_configuration.testObj = \
            testEnviron(topoDict=topoDict)
        #    Get topology object
        Test_stress_dhcp_relay_configuration.topoObj = \
            Test_stress_dhcp_relay_configuration.testObj.topoObjGet()

    def teardown_class(cls):
        Test_stress_dhcp_relay_configuration.topoObj.terminate_nodes()

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
