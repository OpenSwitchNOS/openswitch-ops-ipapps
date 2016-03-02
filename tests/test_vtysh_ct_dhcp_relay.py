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


def dhcp_relay_enable(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'DHCP Relay Agent :enabled' in cmdOut, "Test to enable " \
                                                  "dhcp-relay failed"

    return True


def dhcp_relay_disable(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="no dhcp-relay")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to disable dhcp-relay failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'DHCP Relay Agent :disabled' in cmdOut, "Test to disable " \
                                                   "dhcp-relay failed"

    return True


def dhcp_relayRunningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="no dhcp-relay")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show dhcp-relay in running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'no dhcp-relay' in cmdOut, "Test to show dhcp-relay in " \
                                      "running config failed"

    return True


def show_dhcp_relay(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'DHCP Relay Agent :enabled' in cmdOut, "Test to show " \
                                                  "dhcp-relay " \
                                                  "configuration failed"

    return True


def helper_addressConfigurationFor_unicastServerIp(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.10.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to set helper-address configuration " \
                         "for unicast server IP failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip helper-address")
    assert 'Interface: 1' and '192.168.10.1' in cmdOut, \
        "Test to set helper-address configuration for " \
        "unicast server IP failed"

    return True


def helper_addressConfigurationOn_splitInterface(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 50-4) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.80.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to set helper-address configuration " \
                         "on split interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip helper-address")
    assert 'Interface: 50-4' and '192.168.80.1' in cmdOut, \
        "Test to set helper-address configuration on split interface failed"

    return True


def helper_addressConfigurationOn_subInterface(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1.4) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.90.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to set helper-address configuration " \
                         "on sub interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip helper-address")
    assert 'Interface: 1.4' and '192.168.90.1' in cmdOut, \
        "Test to set helper-address configuration on sub interface failed"

    return True


def helper_addressUnconfigurationFor_unicastServerIp(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="no ip helper-address \
    192.168.10.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to unset helper-address configuration " \
                         "for unicast server IP failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip helper-address")
    assert 'Interface: 1' and '192.168.10.1' not in cmdOut, \
        "Test to unset helper-address configuration " \
        "for unicast server IP failed"

    return True


def helper_addressUnconfigFor_unConfigdHelperAddress(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="no ip helper-address \
    192.168.40.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to unset helper-address configuration " \
                         "for unconfigured helper-address failed"
    dut01.DeviceInteract(command="end")

    cmdOut = devIntReturn.get('buffer')
    assert 'No such entries are present.' in cmdOut, \
        "Test to unset helper-address configuration for " \
        "unconfigured helper-address failed"

    return True


def helper_addressConfigurationFor_multicastServerIp(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address 224.0.0.0")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to validate helper-address configuration " \
                         "for multicast server IP failed"
    dut01.DeviceInteract(command="end")

    cmdOut = devIntReturn.get('buffer')
    assert 'Broadcast, multicast and loopback addresses are not allowed.' \
        in cmdOut, "Test to validate helper-address configuration " \
        "for multicast server IP failed"

    return True


def helper_addressConfigurationFor_broadcastServerIp(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    225.255.255.255")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to validate helper-address configuration " \
                         "for broadcast server IP failed"
    dut01.DeviceInteract(command="end")

    cmdOut = devIntReturn.get('buffer')
    assert 'Broadcast, multicast and loopback addresses are not allowed.' \
        in cmdOut, "Test to validate helper-address configuration " \
        "for broadcast server IP failed"

    return True


def helper_addressConfigurationFor_loopbackServerIp(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    127.0.0.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to validate helper-address configuration " \
                         "for loopback server IP failed"
    dut01.DeviceInteract(command="end")

    cmdOut = devIntReturn.get('buffer')
    assert 'Broadcast, multicast and loopback addresses are not allowed.' \
        in cmdOut, "Test to validate helper-address configuration " \
        "for loopback server IP failed"

    return True


def helper_addressConfigurationFor_invalidServerIp(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    275.255.255.255")
    retCode = devIntReturn.get('returnCode')
    assert retCode != 0, "Test to validate helper-address configuration for " \
                         "invalid server IP failed"
    dut01.DeviceInteract(command="end")

    cmdOut = devIntReturn.get('buffer')
    assert '% Unknown command.' in cmdOut, "Test to validate " \
        "helper-address configuration for invalid server IP failed"

    return True


def helper_addressRecordDuplication_OnsingleInterface(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 1) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.20.1")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.20.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to check helper-address record duplication " \
                         "on single interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = devIntReturn.get('buffer')
    assert 'This entry already exists.' in cmdOut, \
        "Test to check  helper-address record duplication " \
        "on single interface failed"

    return True


def multipleHelper_addressConfigFor_singleInterface(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 2) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.10.2")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.10.3")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.10.4")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to set multiple helper-address " \
                         "configuration for single interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip helper-address")
    assert 'Interface: 2' and '192.168.10.2' and '192.168.10.3' \
        and '192.168.10.4' in cmdOut, "Test to set multiple helper-address " \
        "configuration for single interface failed"

    return True


def maximumHelper_addressConfiguration_perInterface(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 3) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.100.2")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.100.3")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.100.4")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.100.5")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.100.6")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.100.7")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.100.8")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.100.9")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.100.10")

    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to set maximum helper-address " \
                         "configuration per interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = devIntReturn.get('buffer')
    assert 'Maximum allowed helper addresses already configured ' \
        'on the interface.' in cmdOut, "Test to validate helper-address " \
        "configuration for invalid server IP failed"

    return True


def singleHelper_addressConfigOn_differentInterfaces(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 5) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.10.12")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to set single helper-address configuration " \
                         "on different interfaces failed"
    dut01.DeviceInteract(command="end")

    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 6) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.10.12")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to set single helper-address configuration " \
                         "on different interfaces failed"
    dut01.DeviceInteract(command="end")

    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 7) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.10.12")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to set single helper-address configuration " \
                         "on different interfaces failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip helper-address")
    assert 'Interface: 11' and '192.168.10.12' and \
        '192.168.10.12' and '192.168.10.12' in cmdOut, \
        "Test to set single helper-address configuration for " \
        "different interfaces failed"

    return True


def show_helper_address_status(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 11) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.30.2")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show helper-address configuration failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip helper-address")
    assert 'Interface: 11' and '192.168.30.2' in cmdOut, \
        "Test to show helper-address configuration failed"
    return True


def show_helper_address_statusOnSpecifiedInterface(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 11) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.50.2")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show helper-address configuration " \
                         " on specified interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip helper-address interface 11")
    assert 'Interface: 11' and '192.168.50.2' in cmdOut, \
        "Test to show helper-address configuration on " \
        "specified interface failed"
    return True


def helper_address_runningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 4) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.90.11")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show helper-address configuration in " \
                         "running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'Interface: 8' and \
        'ip helper-address 192.168.90.11' in cmdOut, "Test to show " \
        "helper-address configuration in running config failed"
    return True


def helper_address_interface_runningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 8) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.90.77")
    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.90.78")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show helper-address configuration in " \
        "running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config interface 8")
    assert 'Interface: 8' and 'ip helper-address 192.168.90.77' and \
        'ip helper-address 192.168.90.78' in cmdOut, "Test to show " \
        "helper-address configuration in running config failed"

    return True


# Support function to reboot the switch
def switch_reboot(deviceObj):
    LogOutput('info', "Reboot switch " + deviceObj.device)
    deviceObj.Reboot()
    rebootRetStruct = returnStruct(returnCode=0)
    return rebootRetStruct


def dhcp_relay_complete_post_reboot(dut01):

    # Disable DHCP-Relay
    dhcp_relay_disable(dut01)
    # Set a helper-address
    helper_addressConfigurationFor_unicastServerIp(dut01)

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
    assert 'ip helper-address 192.168.10.1' and \
           'no dhcp-relay' in startCmdOut, \
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
    assert 'ip helper-address 192.168.10.1' and \
           'no dhcp-relay' in cmdOut, \
           "Test to verify Dhcp-relay configs " \
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

    def test_show_dhcp_relay(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = show_dhcp_relay(dut01Obj)
        if(retValue):
            LogOutput('info', "Show DHCP-Relay configuration - passed")
        else:
            LogOutput('error', "Show DHCP-Relay configuration - failed")

    def test_dhcp_relayRunningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relayRunningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Show DHCP-Relay "
                              "running configuration - passed")
        else:
            LogOutput('error', "Show DHCP-Relay "
                               "running configuration - failed")

    def test_helper_addressConfigurationFor_unicastServerIp(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            helper_addressConfigurationFor_unicastServerIp(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set helper-address configuration "
                              "for unicast server IP - passed")
        else:
            LogOutput('error', "Test to set helper-address configuration "
                               "for unicast server IP - failed")

    def test_helper_addressUnconfigurationFor_unicastServerIp(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = helper_addressUnconfigurationFor_unicastServerIp(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to unset helper-address configuration "
                              "for unicast server IP - passed")
        else:
            LogOutput('error', "Test to unset helper-address configuration "
                               "for unicast server IP - failed")

    def test_helper_addressConfigurationOn_splitInterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            helper_addressConfigurationOn_splitInterface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set helper-address configuration "
                              "on split interface - passed")
        else:
            LogOutput('error', "Test to set helper-address configuration "
                               "on split interface - failed")

    def test_helper_addressConfigurationOn_subInterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            helper_addressConfigurationOn_subInterface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set helper-address configuration "
                              "on sub interface - passed")
        else:
            LogOutput('error', "Test to set helper-address configuration "
                               "on sub interface - failed")

    def test_helper_addressUnconfigFor_unConfigdHelperAddress(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = helper_addressUnconfigFor_unConfigdHelperAddress(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to unset helper-address configuration "
                              "for unconfigured helper-address - passed")
        else:
            LogOutput('error', "Test to unset helper-address configuration "
                               "for unconfigured helper-address - failed")

    def test_helper_addressConfigurationFor_multicastServerIp(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = helper_addressConfigurationFor_multicastServerIp(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to validate helper-address "
                              "configuration for multicast "
                              "server IP - passed")
        else:
            LogOutput('error', "Test to validate helper-address "
                               "configuration for multicast "
                               "server IP - failed")

    def test_helper_addressConfigurationFor_broadcastServerIp(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = helper_addressConfigurationFor_broadcastServerIp(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to validate helper-address "
                              "configuration for broadcast "
                              "server IP - passed")
        else:
            LogOutput('error', "Test to validate helper-address "
                               "configuration for broadcast "
                               "server IP - failed")

    def test_helper_addressConfigurationFor_loopbackServerIp(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = helper_addressConfigurationFor_loopbackServerIp(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to validate helper-address "
                              "configuration for loopback server IP - passed")
        else:
            LogOutput('error', "Test to validate helper-address "
                               "configuration for loopback "
                               "server IP - failed")

    def test_helper_addressConfigurationFor_invalidServerIp(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = helper_addressConfigurationFor_invalidServerIp(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to validate helper-address "
                              "configuration for invalid server IP - passed")
        else:
            LogOutput('error', "Test to validate helper-address "
                               "configuration for invalid server IP - failed")

    def test_helper_addressRecordDuplication_OnsingleInterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = helper_addressRecordDuplication_OnsingleInterface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to check helper-address record "
                              "duplication on single interface - passed")
        else:
            LogOutput('error', "Test to check helper-address record "
                               "duplication on single interface - failed")

    def test_multipleHelper_addressConfigFor_singleInterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = multipleHelper_addressConfigFor_singleInterface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set multiple helper-address "
                              "configuration for single interface - passed")
        else:
            LogOutput('error', "Test to set multiple helper-address "
                               "configuration for single interface - failed")

    def test_maximumHelper_addressConfiguration_perInterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = maximumHelper_addressConfiguration_perInterface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set maximum helper-address "
                              "configuration per interface - passed")
        else:
            LogOutput('error', "Test to set maximum helper-address "
                               "configuration per interface - failed")

    def test_singleHelper_addressConfigOn_differentInterfaces(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = singleHelper_addressConfigOn_differentInterfaces(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set single helper-address "
                              "configuration on different "
                              "interfaces - passed")
        else:
            LogOutput('error', "Test to set single helper-address "
                               "configuration on different "
                               "interfaces - failed")

    def test_show_helper_address_status(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = show_helper_address_status(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to show helper-address "
                              "configuration - passed")
        else:
            LogOutput('error', "Test to show helper-address "
                               "configuration - failed")

    def test_show_helper_address_statusOnSpecifiedInterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = show_helper_address_statusOnSpecifiedInterface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to show helper-address "
                              "configuration on specified "
                              "interface - passed")
        else:
            LogOutput('error', "Test to show helper-address "
                               "configuration on specified "
                               "interface - failed")

    def test_helper_address_runningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = helper_address_runningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to show helper-address "
                              "configuration in running "
                              "config - passed")
        else:
            LogOutput('error', "Test to show helper-address "
                               "configuration in running "
                               "config - failed")

    def test_helper_address_interface_runningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = helper_address_interface_runningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to show helper-address"
                              "configuration in interface "
                              "running config - passed")
        else:
            LogOutput('error', "Test to show helper-address "
                               "configuration in interface "
                               "running config - failed")

#   def test_dhcp_relay_complete_post_reboot(self):
#       dut01Obj = self.topoObj.deviceObjGet(device="dut01")
#       retValue = dhcp_relay_complete_post_reboot(dut01Obj)
#       if(retValue):
#           LogOutput('info', "DHCP-Relay "
#                             "post Reboot - passed")
#       else:
#           LogOutput('error', "DHCP-Relay "
#                              "post Reboot - failed")
