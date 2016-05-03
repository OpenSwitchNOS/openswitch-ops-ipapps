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


def dhcpv6_relay_enable(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcpv6-relay")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcpv6-relay failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcpv6-relay")
    assert 'DHCPV6 Relay Agent : enabled' in cmdOut, "Test " \
        "to enable dhcpv6-relay failed"

    return True


def dhcpv6_relay_disable(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="no dhcpv6-relay")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to disable dhcpv6-relay failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcpv6-relay")
    assert 'DHCPV6 Relay Agent : disabled' in cmdOut, "Test " \
        "to enable dhcpv6-relay failed"

    return True


def dhcpv6_relay_option_79_enable(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcpv6-relay option 79")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcpv6-relay option 79 failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcpv6-relay")
    assert 'Option 79          : enabled' in cmdOut, "Test " \
        "to enable dhcpv6-relay option 79 failed"

    return True


def dhcpv6_relay_option_79_disable(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="no dhcpv6-relay option 79")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to disable dhcpv6-relay option 79 failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcpv6-relay")
    assert 'Option 79          : disabled' in cmdOut, \
        "Test to disable dhcpv6-relay option 82 failed"

    return True


def show_dhcpv6_relay(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcpv6-relay")
    devIntReturn = dut01.DeviceInteract(command="dhcpv6-relay option 79")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show dhcpv6-relay configuration failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcpv6-relay")
    assert 'DHCPV6 Relay Agent : enabled' and \
        'Option 79          : enabled' in cmdOut, "Test  " \
        "to show dhcpv6-relay configuration failed"

    return True


def show_dhcpv6_relay_statistics(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcpv6-relay")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show dhcpv6-relay statistics failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcpv6-relay")
    assert 'DHCPV6 Relay Agent : enabled' and \
        'Client Requests       Server Responses' and \
        'Valid      Dropped    Valid      Dropped' in \
        cmdOut, "Test to show dhcpv6-relay statistics failed"

    return True


def dhcpv6_relayRunningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcpv6-relay")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show dhcpv6-relay in running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'dhcpv6-relay' in cmdOut, "Test to show dhcpv6-relay in " \
        "running config failed"

    return True


def dhcpv6_relay_option79_RunningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcpv6-relay option 79")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show dhcpv6-relay option 79 " \
        "in running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'dhcpv6-relay option 79' in cmdOut, "Test to show dhcpv6-relay " \
        "option 79 running config failed"

    return True


# Support function to reboot the switch
def switch_reboot(deviceObj):
    LogOutput('info', "Reboot switch " + deviceObj.device)
    deviceObj.Reboot()
    rebootRetStruct = returnStruct(returnCode=0)
    return rebootRetStruct


def dhcp_relay_complete_post_reboot(dut01):

    # Enable dhcpv6-relay
    dhcpv6_relay_enable(dut01)

    # Enable dhcpv6-relay option 79
    dhcpv6_relay_option_79_enable(dut01)

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
    assert 'dhcpv6-relay' and \
           'dhcpv6-relay option 79' in startCmdOut, \
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
    assert 'dhcpv6-relay' and \
           'dhcpv6-relay option 79' in cmdOut, \
           "Test to verify dhcpv6-relay configs " \
           "post reboot - failed"
    return True


class Test_dhcp_relay_configuration:
    def setup_class(cls):
        # Test object will parse command line and formulate the env.
        Test_dhcp_relay_configuration.testObj =\
            testEnviron(topoDict=topoDict, defSwitchContext="vtyShell")
        #    Get topology object.
        Test_dhcp_relay_configuration.topoObj = \
            Test_dhcp_relay_configuration.testObj.topoObjGet()

    def teardown_class(cls):
        Test_dhcp_relay_configuration.topoObj.terminate_nodes()

    def test_dhcpv6_relay_enable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcpv6_relay_enable(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable dhcpv6-relay - passed")
        else:
            LogOutput('error', "Enable dhcpv6-relay - failed")

    def test_dhcpv6_relay_disable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcpv6_relay_disable(dut01Obj)
        if(retValue):
            LogOutput('info', "Disable dhcpv6-relay - passed")
        else:
            LogOutput('error', "Disable dhcpv6-relay - failed")

    def test_dhcpv6_relay_option_79_enable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcpv6_relay_option_79_enable(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable dhcpv6-relay option 82 - passed")
        else:
            LogOutput('error', "Enable Ddhcpv6-relay option 82 - failed")

    def test_dhcpv6_relay_option_82_disable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcpv6_relay_option_82_disable(dut01Obj)
        if(retValue):
            LogOutput('info', "Disable dhcpv6-relay option 82 - passed")
        else:
            LogOutput('info', "Disable dhcpv6-relay option 82 - failed")

    def test_show_dhcpv6_relay(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = show_dhcpv6_relay(dut01Obj)
        if(retValue):
            LogOutput('info', "Show dhcpv6-relay configuration - passed")
        else:
            LogOutput('error', "Show dhcpv6-relay configuration - failed")

    def test_show_dhcpv6_relay_statistics(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = show_dhcpv6_relay_statistics(dut01Obj)
        if(retValue):
            LogOutput('info', "Show dhcpv6-relay statistics - passed")
        else:
            LogOutput('error', "Show dhcpv6-relay statistics - failed")

    def test_dhcpv6_relayRunningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcpv6_relayRunningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Show dhcpv6-relay "
                              "running configuration - passed")
        else:
            LogOutput('error', "Show dhcpv6-relay "
                               "running configuration - failed")

    def test_dhcpv6_relay_option79_RunningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcpv6_relay_option79_RunningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Show dhcpv6-relay option 79 running "
                              "configuration - passed")
        else:
            LogOutput('error', "Show dhcpv6-relay option 79 running "
                               "configuration -- failed")

#   def test_dhcp_relay_complete_post_reboot(self):
#       dut01Obj = self.topoObj.deviceObjGet(device="dut01")
#       retValue = dhcp_relay_complete_post_reboot(dut01Obj)
#       if(retValue):
#           LogOutput('info', "dhcpv6-relay "
#                             "post Reboot - passed")
#       else:
#           LogOutput('error', "dhcpv6-relay "
#                              "post Reboot - failed")
