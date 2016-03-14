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


def udp_bcast_forwarder_enable(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    cmd = "ip udp-bcast-forward"
    devIntReturn = dut01.DeviceInteract(command=cmd)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enable UDP Broadcast Forwarder"

    if (exitContext(dut01) is False):
        return False

    cmdOut = "ovs-appctl -t ops-udpfwd udpfwd/dump"
    output = dut01.cmd(cmdOut)
    assert \
    'UDP Broadcast Forwarder is enabled' in output, \
    "Test to enable UDP Broadcast Forwarder failed"

    return True


def udp_bcast_forwarder_disable(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    cmd = "no ip udp-bcast-forward"
    devIntReturn = dut01.DeviceInteract(command=cmd)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to disable UDP " \
                         "Broadcast Forwarder"

    if (exitContext(dut01) is False):
        return False

    output = dut01.cmd("ovs-appctl -t ops-udpfwd udpfwd/dump")
    assert 'UDP Broadcast Forwarder is disabled' in output, \
           "Test to disable UDP Broadcast Forwarder failed"

    return True


def udp_forward_protocol_Test01(dut01):

    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="interface 1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    cmd = "ip forward-protocol udp 1.1.1.1 dns"
    devIntReturn = dut01.DeviceInteract(command=cmd)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to execute UDP forward-protocol"

    dut01.DeviceInteract(command="exit")

    if (exitContext(dut01) is False):
        return False

    cmdOut = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 1"
    output = dut01.cmd(cmdOut)
    assert '1.1.1.1' in output, "Test to set UDP forward-protocol " \
                                "configuration failed"

    return True


def udp_forward_protocol_Test02(dut01):
    # Verify UDP forward-protocol configuration for multiple
    # UDP ports on a specific interface

    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="interface 2")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    cmd1 = "ip forward-protocol udp 1.1.1.2 53"
    devIntReturn = dut01.DeviceInteract(command=cmd1)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configure UDP forward-protocol " \
                         "for server IP 1.1.1.2"

    cmd2 = "ip forward-protocol udp 1.1.1.2 137"
    devIntReturn = dut01.DeviceInteract(command=cmd2)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configure UDP forward-protocol " \
                         "for server IP 1.1.1.2"

    cmd3 = "ip forward-protocol udp 1.1.1.2 69"
    devIntReturn = dut01.DeviceInteract(command=cmd3)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configure UDP forward-protocol " \
                         "for server IP 1.1.1.2"

    dut01.DeviceInteract(command="exit")

    if (exitContext(dut01) is False):
        return False

    cmdOut = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 2"
    retBuffer = dut01.cmd(cmdOut)
    assert '1.1.1.2' in retBuffer \
        and '53' in retBuffer \
        and '69' in retBuffer \
        and '137' in retBuffer, "Test to verify UDP forward-protocol " \
                                "with multiple UDP ports " \
                                "on an interface failed"

    return True


def udp_forward_protocol_Test03(dut01):
    # Verify UDP forward-protocol configurations for multiple
    # server IP addresses for a single UDP port on an interface

    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="interface 3")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    cmd1 = "ip forward-protocol udp 1.1.1.3 123"
    devIntReturn = dut01.DeviceInteract(command=cmd1)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configure UDP forward-protocol " \
                         "for server IP 1.1.1.3"

    cmd2 = "ip forward-protocol udp 1.1.1.4 123"
    devIntReturn = dut01.DeviceInteract(command=cmd2)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configure UDP forward-protocol " \
                         "for server IP 1.1.1.4"

    cmd3 = "ip forward-protocol udp 1.1.1.5 123"
    devIntReturn = dut01.DeviceInteract(command=cmd3)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configure UDP forward-protocol " \
                         "for server IP 1.1.1.5"

    dut01.DeviceInteract(command="exit")

    if (exitContext(dut01) is False):
        return False

    cmdOut = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 3"
    retBuffer = dut01.cmd(cmdOut)
    assert '123' in retBuffer \
        and '1.1.1.3' in retBuffer \
        and '1.1.1.4' in retBuffer \
        and '1.1.1.5' in retBuffer, \
            "Test to verify UDP forward-protocol " \
            "for multiple server IPs but on a single " \
            "UDP port on an interface failed"

    return True


def udp_forward_protocol_Test04(dut01):
    # Verify UDP forward-protocol configuration for
    # subnet broadcast IP address on an interface

    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="interface 4")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    cmd1 = "ip forward-protocol udp 1.1.1.255 161"
    devIntReturn = dut01.DeviceInteract(command=cmd1)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configure UDP forward-protocol " \
                         "for server IP 1.1.1.255"

    dut01.DeviceInteract(command="exit")

    if (exitContext(dut01) is False):
        return False

    cmdOut = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 4"
    retBuffer = dut01.cmd(cmdOut)
    assert '161' in retBuffer \
        and '1.1.1.255' in retBuffer, \
            "Test to verify UDP forward-protocol " \
            "for subnet server IPs " \
            "on an interface failed"

    return True


def udp_forward_protocol_Test05(dut01):
    # Verify UDP forward-protocol configuration on same subnet
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="interface 5")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    cmd1 = "ip address 2.2.2.2/24"
    devIntReturn = dut01.DeviceInteract(command=cmd1)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configure an IP on an interface"

    cmd2 = "ip forward-protocol udp 2.2.2.255 161"
    devIntReturn = dut01.DeviceInteract(command=cmd2)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configure UDP forward-protocol " \
                         "for server IP 2.2.2.255"

    dut01.DeviceInteract(command="exit")

    if (exitContext(dut01) is False):
        return False

    cmdOut = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 5"
    retBuffer = dut01.cmd(cmdOut)
    assert '161' in retBuffer \
           and '2.2.2.255' in retBuffer, \
               "Test to verify UDP forward-protocol " \
               "on same subnet " \
               "on an interface failed"

    return True


def udp_forward_protocol_Test06(dut01):
    # Verify single UDP forward-protocol configuration
    # on multiple interfaces
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="interface 6")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    cmd2 = "ip forward-protocol udp 6.6.6.6 53"
    devIntReturn = dut01.DeviceInteract(command=cmd2)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configure UDP forward-protocol " \
                         "for server IP 6.6.6.6"
    dut01.DeviceInteract(command="exit")

    devIntReturn = dut01.DeviceInteract(command="interface 7")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    devIntReturn = dut01.DeviceInteract(command=cmd2)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configure UDP forward-protocol " \
                         "for server IP 6.6.6.6"
    dut01.DeviceInteract(command="exit")

    devIntReturn = dut01.DeviceInteract(command="interface 8")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    devIntReturn = dut01.DeviceInteract(command=cmd2)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configure UDP forward-protocol " \
                         "for server IP 6.6.6.6"

    dut01.DeviceInteract(command="exit")

    if (exitContext(dut01) is False):
        return False

    cmdOut = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 6"
    retBuffer = dut01.cmd(cmdOut)
    assert '53' in retBuffer \
           and '6.6.6.6' in retBuffer, \
               "Test to verify UDP forward-protocol " \
               "on multiple interfaces failed"

    cmdOut = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 7"
    retBuffer = dut01.cmd(cmdOut)
    assert '53' in retBuffer \
           and '6.6.6.6' in retBuffer, \
               "Test to verify UDP forward-protocol " \
               "on multiple interfaces failed"

    cmdOut = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 8"
    retBuffer = dut01.cmd(cmdOut)
    assert '53' in retBuffer \
           and '6.6.6.6' in retBuffer, \
               "Test to verify UDP forward-protocol " \
               "on multiple interfaces failed"

    return True


def udp_forward_protocol_max_per_interface(dut01):
    # Verify maximum UDP forward-protocol configurations per interface
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="interface 9")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    cmd1 = "ip forward-protocol udp 5.5.5.1 123"
    cmd2 = "ip forward-protocol udp 5.5.5.2 123"
    cmd3 = "ip forward-protocol udp 5.5.5.3 123"
    cmd4 = "ip forward-protocol udp 5.5.5.4 123"
    cmd5 = "ip forward-protocol udp 5.5.5.5 123"
    cmd6 = "ip forward-protocol udp 5.5.5.6 123"
    cmd7 = "ip forward-protocol udp 5.5.5.7 123"
    cmd8 = "ip forward-protocol udp 5.5.5.8 123"
    cmd9 = "ip forward-protocol udp 5.5.5.9 123"

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

    dut01.DeviceInteract(command="exit")

    if (exitContext(dut01) is False):
        return False

    cmdOut = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 9"
    retBuffer = dut01.cmd(cmdOut)
    assert '5.5.5.1' in retBuffer \
           and '5.5.5.2' in retBuffer \
           and '5.5.5.3' in retBuffer \
           and '5.5.5.4' in retBuffer \
           and '5.5.5.5' in retBuffer \
           and '5.5.5.6' in retBuffer \
           and '5.5.5.7' in retBuffer \
           and '5.5.5.8' in retBuffer, \
           "Test to verify maximum UDP forward-protocol " \
           "per interface failed"

    return True


# Support function to reboot the switch
def switch_reboot(deviceObj):
    LogOutput('info', "Reboot switch " + deviceObj.device)
    deviceObj.Reboot()
    rebootRetStruct = returnStruct(returnCode=0)
    return rebootRetStruct


def udp_forward_protocol_reboot(dut01):
    # Verify UDP forward-protocol configuration post reboot
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="interface 10")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Failed to enter Interface context"

    cmd1 = "ip forward-protocol udp 10.10.10.10 69"
    devIntReturn = dut01.DeviceInteract(command=cmd1)
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Unable to configue an UDP " \
                         "forward-protocol on the interface"

    if (exitContext(dut01) is False):
        return False

    # Before Reboot.
    cmdOut = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 10"
    retBuffer = dut01.cmd(cmdOut)
    assert '10.10.10.10' in retBuffer, \
           "UDP forward-protocol before reboot failed"

    # Perform reboot
    devRebootRetStruct = switch_reboot(dut01)
    if devRebootRetStruct.returnCode() != 0:
        LogOutput('error', "dut01 reboot - failed")
        assert(devRebootRetStruct.returnCode() == 0)
    else:
        LogOutput('info', "dut01 reboot - success")

    # Check for the configs post reboot
    time.sleep(30)

    # Post reboot
    cmdOut = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 10"
    retBuffer = dut01.cmd(cmdOut)
    assert '10.10.10.10' in retBuffer, \
           "UDP forward-protocol post reboot failed"

    return True


class Test_udp_bcast_forwarder_configuration:

    def setup_class(cls):
        # Test object will parse command line and formulate the env
        Test_udp_bcast_forwarder_configuration.testObj = \
            testEnviron(topoDict=topoDict)
        #    Get topology object
        Test_udp_bcast_forwarder_configuration.topoObj = \
            Test_udp_bcast_forwarder_configuration.testObj.topoObjGet()

    def teardown_class(cls):
        Test_udp_bcast_forwarder_configuration.topoObj.terminate_nodes()

#    def test_udp_bcast_forwarder_enable(self):
 #       dut01Obj = self.topoObj.deviceObjGet(device="dut01")
  #      retValue = udp_bcast_forwarder_enable(dut01Obj)
#        if(retValue):
 #           LogOutput('info', "Enable UDP Broadcast Forwarder - passed")
  #      else:
   #         LogOutput('error', "Enable UDP Broadcast Forwarder - failed")

    #def test_udp_bcast_forwarder_disable(self):
     #   dut01Obj = self.topoObj.deviceObjGet(device="dut01")
      #  retValue = udp_bcast_forwarder_disable(dut01Obj)
       # if(retValue):
        #    LogOutput('info', "Disable UDP Broadcast Forwarder - passed")
#        else:
 #           LogOutput('error', "Disable UDP Broadcast Forwarder - failed")

    # Verify UDP forward-protocol configuration on a specific interface
    def test_udp_forward_protocol_Test01(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = udp_forward_protocol_Test01(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set UDP forward-protocol "
                      "configuration per interface - passed")
        else:
            LogOutput('error', "Test to set UDP forward-protocol "
                      "configuration per interface - failed")

    # Verify UDP forward-protocol configuration for multiple
    # UDP ports on a specific interface
    def test_udp_forward_protocol_Test02(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = udp_forward_protocol_Test02(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to configure UDP "
                              "forward-protocol for multiple UDP "
                              "ports on an interface - passed")
        else:
            LogOutput('error', "Test to configure UDP "
                               "forward-protocol for multiple UDP "
                               "ports on an interface - failed")

    # Verify UDP forward-protocol configurations for multiple
    # server IP addresses for a single UDP port on an interface
    def test_udp_forward_protocol_Test03(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = udp_forward_protocol_Test03(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to configure UDP "
                              "forward-protocol for multiple "
                              "server IPs for a single UDP port "
                              "on an interface - passed")
        else:
            LogOutput('error', "Test to configure UDP "
                               "forward-protocol for multiple "
                               "server IPs for a single UDP port "
                               "on an interface - failed")

    # Verify UDP forward-protocol configuration for
    # subnet broadcast IP address on an interface
    def test_udp_forward_protocol_Test04(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = udp_forward_protocol_Test04(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to configure UDP "
                              "forward-protocol for subnet "
                              "server IPs for "
                              "on an interface - passed")
        else:
            LogOutput('error', "Test to configure UDP "
                               "forward-protocol for subnet "
                               "server IPs for "
                               "on an interface - failed")

    # Verify UDP forward-protocol configuration on same subnet
    def test_udp_forward_protocol_Test05(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = udp_forward_protocol_Test05(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to configure UDP "
                              "forward-protocol on same "
                              "subnet - passed")
        else:
            LogOutput('error', "Test to configure UDP "
                               "forward-protocol on same "
                               "subnet - failed")

    # Verify single UDP forward-protocol configuration
    # on multiple interfaces
    def test_udp_forward_protocol_Test06(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = udp_forward_protocol_Test06(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to configure same UDP "
                              "forward-protocol on multiple "
                              "interfaces - passed")
        else:
            LogOutput('error', "Test to configure same UDP "
                               "forward-protocol on multiple "
                               "interfaces - failed")

    # Verify maximum UDP forward-protocol configurations per interface
    def test_udp_forward_protocol_max_per_interface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = udp_forward_protocol_max_per_interface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to verify maximum UDP "
                              "forward-protocol per interface - passed")
        else:
            LogOutput('error', "Test to verify maximum UDP "
                               "forward-protocol per interface - failed")

#    # Verify UDP forward-protocol configuration post reboot
 #   def test_udp_forward_protocol_reboot(self):
  #      dut01Obj = self.topoObj.deviceObjGet(device="dut01")
#        retValue = udp_forward_protocol_reboot(dut01Obj)
  #      if(retValue):
   #         LogOutput('info', "Test to verify UDP "
    #                          "forward-protocol post reboot - passed")
     #   else:
      #      LogOutput('error', "Test to verify  UDP "
                               #"forward-protocol post reboot - failed")
