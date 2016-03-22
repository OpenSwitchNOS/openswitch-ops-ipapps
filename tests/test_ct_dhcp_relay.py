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
    assert '192.168.10.1' in output, "Test to set helper-address"
    " configuration failed"

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

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump interface 4")
    assert 'Server Ip ref count :4' in output, "Test to set same"
    " helper-address configuration failed"

    return True


def all_interfaces(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="interface 5")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    devIntReturn = dut01.DeviceInteract(command="ip helper-address 30.10.10.1")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address 30.10.10.2")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, " Failed to execute helper-address"
    " configuration command"
    dut01.DeviceInteract(command="exit")

    dut01.DeviceInteract(command="int 6")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address 20.10.10.1")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address 40.10.10.2")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, " Failed to execute helper-address"
    " configuration command"
    dut01.DeviceInteract(command="exit")

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    assert '30.10.10.1' in output \
        and '30.10.10.2' in output \
        and '20.10.10.1' in output \
        and '40.10.10.2' in output, "Test to set helper-address"
    "configuration failed"

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

    def test_dhcp_relay_disable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_disable(dut01Obj)
        if(retValue):
            LogOutput('info', "Disable DHCP-Relay - passed")
        else:
            LogOutput('error', "Disable DHCP-Relay - failed")

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

    def test_all_interfaces(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = all_interfaces(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to display complete dump passed")
        else:
            LogOutput('error', "Test to display complete dump failed")
