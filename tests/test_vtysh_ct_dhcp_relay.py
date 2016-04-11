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
    assert 'DHCP Relay Agent                 : enabled' in cmdOut, "Test " \
        "to enable dhcp-relay failed"

    return True


def dhcp_relay_disable(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="no dhcp-relay")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to disable dhcp-relay failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'DHCP Relay Agent                 : disabled' in cmdOut, "Test " \
        "to enable dhcp-relay failed"

    return True


def dhcp_relay_hop_count_increment_enable(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay \
    hop-count-increment")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay hop-count " \
        "increment failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'DHCP Request Hop Count Increment : enabled' in cmdOut, "Test " \
        "to enable dhcp-relay hop-count increment failed"

    return True


def dhcp_relay_hop_count_increment_disable(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="no dhcp-relay \
    hop-count-increment")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to disable dhcp-relay hop-count " \
        "increment failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'DHCP Request Hop Count Increment : disabled' in cmdOut, "Test " \
        "to disable dhcp-relay hop-count increment failed"

    return True


def dhcp_relay_option_82_enable(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    replace")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' in cmdOut, "Test " \
        "to enable dhcp-relay option 82 failed"

    return True


def dhcp_relay_option_82_validation_enable(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    validate")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 " \
        "response validation failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Response Validation              : enabled' in cmdOut, "Test " \
        "to enable dhcp-relay option 82 response validation failed"

    return True


def relay_option_82_with_validation_and_replace(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    validate replace")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "validation and replace policy failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : enabled' and \
        'Option 82 Handle Policy          : replace' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with validation and " \
        "replace policy failed"

    return True


def relay_option_82_with_validation_replace_and_mac_rid(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    validate replace mac")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "validation, replace policy and mac remote ID failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : enabled' and \
        'Option 82 Handle Policy          : replace' and \
        'Remote ID                        : mac' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "validation, replace policy and mac remote ID failed"

    return True


def relay_option_82_with_validation_replace_and_ip_rid(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    validate replace ip")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "validation, replace policy and ip remote ID failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : enabled' and \
        'Option 82 Handle Policy          : replace' and \
        'Remote ID                        : ip' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "validation, replace policy and ip remote ID failed"

    return True


def relay_option_82_with_validation_and_drop(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    validate drop")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "validation and drop policy failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : enabled' and \
        'Option 82 Handle Policy          : drop' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with validation and " \
        "drop policy failed"

    return True


def relay_option_82_with_validation_drop_and_mac_rid(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    validate drop mac")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "validation, drop policy and mac remote ID failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : enabled' and \
        'Option 82 Handle Policy          : drop' and \
        'Remote ID                        : mac' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "validation, drop policy and mac remote ID failed"

    return True


def relay_option_82_with_validation_drop_and_ip_rid(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    validate drop ip")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "validation, drop policy and ip remote ID failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : enabled' and \
        'Option 82 Handle Policy          : drop' and \
        'Remote ID                        : ip' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "validation, drop policy and ip remote ID failed"

    return True


def relay_option_82_with_keep_policy(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    keep")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "keep policy failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : disabled' and \
        'Option 82 Handle Policy          : keep' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with keep policy failed"

    return True


def relay_option_82_with_keep_and_mac_rid(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    keep mac")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "keep policy and mac remote ID failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : disabled' and \
        'Option 82 Handle Policy          : keep' and \
        'Remote ID                        : mac' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "keep policy and mac remote ID failed"

    return True


def relay_option_82_with_keep_and_ip_rid(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    keep ip")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "keep policy and ip remote ID failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : disabled' and \
        'Option 82 Handle Policy          : keep' and \
        'Remote ID                        : ip' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "keep policy and ip remote ID failed"

    return True


def relay_option_82_with_replace_policy(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    replace")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "replace policy failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Option 82 Handle Policy          : replace' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with replace policy failed"

    return True


def relay_option_82_with_replace_and_mac_rid(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    replace mac")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "replace policy and mac remote ID failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Option 82 Handle Policy          : replace' and \
        'Remote ID                        : mac' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "replace policy and mac remote ID failed"

    return True


def relay_option_82_replace_with_mac_rid_validation(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    replace mac validation")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "replace policy, mac remote ID and validation failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : enabled' and \
        'Option 82 Handle Policy          : replace' and \
        'Remote ID                        : mac' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "replace policy, mac remote ID and validation failed"

    return True


def relay_option_82_with_replace_and_ip_rid(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    replace ip")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "replace policy and ip remote ID failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Option 82 Handle Policy          : replace' and \
        'Remote ID                        : ip' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "replace policy and ip remote ID failed"

    return True


def relay_option_82_replace_with_ip_rid_validation(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    replace ip validation")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "replace policy, ip remote ID and validation failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : enabled' and \
        'Option 82 Handle Policy          : replace' and \
        'Remote ID                        : ip' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "replace policy, ip remote ID and validation failed"

    return True


def relay_option_82_with_replace_and_validation(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    replace validate")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        " replace policy and validation failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : enabled' and \
        'Option 82 Handle Policy          : replace' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with replace policy and " \
        "validation failed"

    return True


def relay_option_82_with_replace_validation_and_mac_rid(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    replace validate mac")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "replace policy, validation and mac remote ID failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : enabled' and \
        'Option 82 Handle Policy          : replace' and \
        'Remote ID                        : mac' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "replace policy, validation and mac remote ID failed"

    return True


def relay_option_82_with_replace_validation_and_ip_rid(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    replace validate ip")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "replace policy, validation and ip remote ID failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : enabled' and \
        'Option 82 Handle Policy          : replace' and \
        'Remote ID                        : ip' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "replace policy, validation and ip remote ID failed"

    return True


def relay_option_82_with_drop_policy(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    drop")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "drop policy failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Option 82 Handle Policy          : drop' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with drop policy failed"

    return True


def relay_option_82_with_drop_and_mac_rid(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    drop mac")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "drop policy and mac remote ID failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Option 82 Handle Policy          : drop' and \
        'Remote ID                        : mac' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "drop policy and mac remote ID failed"

    return True


def relay_option_82_drop_with_mac_rid_validation(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    drop mac validate")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "drop policy, mac remote ID and validation failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : enabled' and \
        'Option 82 Handle Policy          : drop' and \
        'Remote ID                        : mac' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "drop policy, mac remote ID and validation failed"

    return True


def relay_option_82_with_drop_and_ip_rid(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    drop ip")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "drop policy and ip remote ID failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Option 82 Handle Policy          : drop' and \
        'Remote ID                        : ip' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "drop policy and ip remote ID failed"

    return True


def relay_option_82_drop_with_ip_rid_validation(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    drop ip validate")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "drop policy, ip remote ID and validation failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : enabled' and \
        'Option 82 Handle Policy          : drop' and \
        'Remote ID                        : ip' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "drop policy, ip remote ID and validation failed"

    return True


def relay_option_82_with_drop_and_validation(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    drop validate")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        " drop policy and validation failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : enabled' and \
        'Option 82 Handle Policy          : drop' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with drop policy and " \
        "validation failed"

    return True


def relay_option_82_with_drop_validation_and_mac_rid(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    drop validate mac")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "drop policy, validation and mac remote ID failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : enabled' and \
        'Option 82 Handle Policy          : drop' and \
        'Remote ID                        : mac' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "drop policy, validation and mac remote ID failed"

    return True


def relay_option_82_with_drop_validation_and_ip_rid(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    drop validate ip")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay option 82 with " \
        "drop policy, validation and ip remote ID failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : enabled' and \
        'Response Validation              : enabled' and \
        'Option 82 Handle Policy          : drop' and \
        'Remote ID                        : ip' in cmdOut, "Test to " \
        "enable dhcp-relay option 82 with " \
        "drop policy, validation and ip remote ID failed"

    return True


def dhcp_relay_option_82_disable(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="no dhcp-relay option 82")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to disable dhcp-relay option 82 failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Option 82                        : disabled' and \
        'Response Validation              : disbled' and \
        'Option 82 Handle Policy          : replace' and \
        'Remote ID                        : mac' in cmdOut, "Test to " \
        "disable dhcp-relay option 82 failed"

    return True


def dhcp_relay_option_82_validation_disable(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="no dhcp-relay option 82 \
    validate")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to disable dhcp-relay option 82 " \
        "response validation failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'Response Validation              : disabled' in cmdOut, "Test " \
        "to disable dhcp-relay option 82 response validation failed"

    return True


def show_dhcp_relay(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'DHCP Relay Agent                 : enabled' in cmdOut, "Test  " \
        "to show dhcp-relay configuration failed"

    return True


def show_dhcp_relay_statistics(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'DHCP Relay Agent                 : enabled' and \
        'DHCP Relay Statistics:' and \
        'Client Requests       Server Responses' and \
        'Valid      Dropped    Valid      Dropped' in cmdOut, "Test  " \
        "to show dhcp-relay statistics failed"

    return True


def show_dhcp_relay_option82_statistics(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to enable dhcp-relay failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay")
    assert 'DHCP Relay Agent                 : enabled' and \
        'DHCP Relay Option 82 Statistics:' and \
        'Client Requests       Server Responses' and \
        'Valid      Dropped    Valid      Dropped' in cmdOut, "Test  " \
        "to show dhcp-relay option 82 statistics failed"

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


def dhcp_relay_hopCountIncrementRunningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="no dhcp-relay \
    hop-count-increment")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show dhcp-relay hop count increment "\
        "in running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'no dhcp-relay hop-count-increment' in cmdOut, "Test to show "\
        "dhcp-relay hop count increment in running config failed"

    return True


def relay_option82_dropRunningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    drop")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show dhcp-relay option 82 drop policy" \
        "in running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'dhcp-relay option 82 drop' in cmdOut, "Test to show dhcp-relay " \
        "option 82 drop policy running config failed"

    return True


def relay_option82_keepRunningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    keep")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show dhcp-relay option 82 keep policy" \
        "in running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'dhcp-relay option 82 keep' in cmdOut, "Test to show dhcp-relay " \
        "option 82 keep policy running config failed"

    return True


def relay_option82_replaceRunningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    replace")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show dhcp-relay option 82 replace policy" \
        "in running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'dhcp-relay option 82 replace' in cmdOut, "Test to show " \
        "dhcp-relay option 82 replace policy running config failed"

    return True


def relay_option82_replace_with_validationRunningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    replace validate")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show dhcp-relay option 82 replace policy" \
        "with validation in running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'dhcp-relay option 82 replace validate' in cmdOut, "Test to " \
        "show dhcp-relay option 82 replace policy with " \
        "validation running config failed"

    return True


def relay_option82_drop_with_validationRunningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    drop validate")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show dhcp-relay option 82 drop policy" \
        "with validation in running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'dhcp-relay option 82 drop validate' in cmdOut, "Test to " \
        "show dhcp-relay option 82 drop policy with " \
        "validation running config failed"

    return True


def relay_option82_keep_with_ip_ridRunningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    keep ip")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show dhcp-relay option 82 keep policy" \
        "with ip remote id in running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'dhcp-relay option 82 keep ip' in cmdOut, "Test to " \
        "show dhcp-relay option 82 keep policy with " \
        "ip remote id running config failed"

    return True


def relay_option82_replace_with_ip_ridRunningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    replace ip")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show dhcp-relay option 82 replace policy" \
        "with ip remote id in running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'dhcp-relay option 82 replace ip' in cmdOut, "Test to " \
        "show dhcp-relay option 82 replace policy with " \
        "ip remote id running config failed"

    return True


def relay_option82_drop_with_ip_ridRunningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    drop ip")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show dhcp-relay option 82 drop policy" \
        "with ip remote id in running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'dhcp-relay option 82 drop ip' in cmdOut, "Test to " \
        "show dhcp-relay option 82 drop policy with " \
        "ip remote id running config failed"

    return True


def relay_option82_replace_validation_ip_ridRunningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    replace validate ip")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show dhcp-relay option 82 replace policy" \
        "with validatio and ip remote id in running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'dhcp-relay option 82 replace validate ip' in cmdOut, "Test " \
        "to show dhcp-relay option 82 replace policy with " \
        "validation and ip remote id running config failed"

    return True


def relay_option82_drop_validation_ip_ridRunningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="dhcp-relay option 82 \
    drop validate ip")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show dhcp-relay option 82 drop policy" \
        "with validatio and ip remote id in running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'dhcp-relay option 82 drop validate ip' in cmdOut, "Test to " \
        "show dhcp-relay option 82 drop policy with " \
        "validation and ip remote id running config failed"

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
    assert 'Helper-address is not present.' in cmdOut, \
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
    assert 'Multicast and loopback addresses are not allowed.' \
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
    assert 'Interface: 1' and '225.255.255.255' in cmdOut, "Test to " \
        "validate helper-address configuration for broadcast server IP failed"

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
    assert 'Multicast and loopback addresses are not allowed.' \
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


def showHelperAddress_and_bootpGatewayConfig_onInterface(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 2) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.90.1")
    devIntReturn = dut01.DeviceInteract(command="ip address \
    10.0.0.1/24")
    devIntReturn = dut01.DeviceInteract(command="ip bootp-gateway \
    10.0.0.1")
    devIntReturn = dut01.DeviceInteract(command="no ip bootp-gateway \
    10.0.0.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show helper-address configuration " \
        "when the bootp-gateway is unconfigured on an interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show ip helper-address")
    assert 'Interface: 2' and '192.168.90.1' in cmdOut, \
        "Test to show helper-address configuration " \
        "when the bootp-gateway is unconfigured on an interface failed"

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


def bootpGateway_configFor_configIP_onInterface(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 18) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip address \
    10.0.80.1/24")
    devIntReturn = dut01.DeviceInteract(command="ip bootp-gateway \
    10.0.80.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to set bootp-gateway configuration " \
        "for configured IP address on an interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay bootp-gateway")
    assert '18                   10.0.80.1' in cmdOut, \
        "Test to set bootp-gateway configuration " \
        "for configured IP address on an interface failed"

    return True


def bootpGateway_configFor_configIP_onSubInterface(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 10.1) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip address \
    20.0.80.1/24")
    devIntReturn = dut01.DeviceInteract(command="ip bootp-gateway \
    20.0.80.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to set bootp-gateway configuration " \
        "for configured IP address on a sub interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay bootp-gateway")
    assert '10.1                 20.0.80.1' in cmdOut, \
        "Test to set bootp-gateway configuration " \
        "for configured IP address on a sub interface failed"

    return True


def bootpGateway_configFor_configSecondaryIP_onInterface(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 14) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip address \
    50.0.80.1/24")
    devIntReturn = dut01.DeviceInteract(command="ip bootp-gateway \
    50.0.80.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to set bootp-gateway configuration " \
        "for configured secondary IP address on an interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay bootp-gateway")
    assert '14                   50.0.80.1' in cmdOut, \
        "Test to set bootp-gateway configuration " \
        "for configured secondary IP address on an interface failed"

    return True


def bootpGateway_configFor_configSecondaryIP_onSubInterface(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 5.2) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip address \
    30.0.80.1/24")
    devIntReturn = dut01.DeviceInteract(command="ip bootp-gateway \
    30.0.80.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to set bootp-gateway configuration " \
        "for configured secondary IP address on a sub interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay bootp-gateway")
    assert '5.2                  30.0.80.1' in cmdOut, \
        "Test to set bootp-gateway configuration " \
        "for configured secondary IP address on a sub interface failed"

    return True


def bootpGateway_configFor_unconfigIP_onInterface(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 19) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip bootp-gateway \
    10.0.10.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to set bootp-gateway configuration " \
        "for unconfigured IP address on an interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = devIntReturn.get('buffer')
    assert 'The IP address 10.0.10.1 is not yet configured ' \
        'on this interface.' in cmdOut, "Test to set bootp-gateway " \
        "configuration for unconfigured IP address on an interface failed"

    return True


def bootpGateway_configFor_invalidIP(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 20) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip bootp-gateway \
    275.255.255.255")
    retCode = devIntReturn.get('returnCode')
    assert retCode != 0, "Test to set bootp-gateway configuration " \
        "for invalid IP address interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = devIntReturn.get('buffer')
    assert '% Unknown command.' in cmdOut, "Test to set bootp-gateway " \
        "configuration for invalid IP address interface failed"

    return True


def bootpGateway_unconfigFor_configbootpGateway_onInterface(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 21) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip address \
    10.0.20.1/24")
    devIntReturn = dut01.DeviceInteract(command="ip bootp-gateway \
    10.0.20.1")
    devIntReturn = dut01.DeviceInteract(command="no ip bootp-gateway \
    10.0.20.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to unset bootp-gateway configuration " \
        "for configured bootp-gateway on an interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay bootp-gateway")
    assert '21                    10.0.20.1' not in cmdOut, \
        "Test to unset bootp-gateway unconfiguration " \
        "for configured bootp-gateway on an interface failed"

    return True


def bootpGateway_unconfigFor_configIP_onInterface(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 22) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip address \
    10.0.30.1/24")
    devIntReturn = dut01.DeviceInteract(command="no ip bootp-gateway \
    10.0.30.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to unset bootp-gateway configuration " \
        "for configured IP address on an interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = devIntReturn.get('buffer')

    assert 'The BOOTP Gateway 10.0.30.1 is not configured ' \
        'on this interface.' in cmdOut, "Test" \
        " to unset bootp-gateway configuration for unconfigured" \
        " IP address on an interface failed"

    return True


def bootpGateway_unconfigFor_unconfigIP_onInterface(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 23) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="no ip bootp-gateway \
    10.0.40.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to unset bootp-gateway configuration " \
        "for unconfigured IP address on an interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = devIntReturn.get('buffer')
    assert 'The BOOTP Gateway 10.0.40.1 is not ' \
        'configured on this interface.' in cmdOut, "Test" \
        " to unset bootp-gateway configuration for unconfigured" \
        " IP address on an interface failed"

    return True


def show_bootp_gateway_status(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 17) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip address \
    10.0.10.2/24")
    devIntReturn = dut01.DeviceInteract(command="ip bootp-gateway \
    10.0.10.2")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show bootp-gateway configuration failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay bootp-gateway")
    assert '17                   10.0.10.2' in cmdOut, \
        "Test to show bootp-gateway configuration failed"
    return True


def showBootpGateway_and_helperAddressConfig_onInterface(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 16) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip helper-address \
    192.168.90.1")
    devIntReturn = dut01.DeviceInteract(command="ip address \
    10.0.20.2/24")
    devIntReturn = dut01.DeviceInteract(command="ip bootp-gateway \
    10.0.20.2")
    devIntReturn = dut01.DeviceInteract(command="no ip helper-address \
    192.168.90.1")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show bootp-gateway configuration " \
        "when the helper-address is unconfigured on an interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay bootp-gateway")
    assert '16                   10.0.20.2' in cmdOut, \
        "Test to set bootp-gateway configuration " \
        "for configured IP address on an interface failed"

    return True


def show_bootp_gateway_statusOnSpecifiedInterface(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 26) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip address \
    10.0.0.2/24")
    devIntReturn = dut01.DeviceInteract(command="ip bootp-gateway \
    10.0.0.2")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show bootp-gateway configuration " \
        "on specified interface failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show dhcp-relay bootp-gateway \
    interface 26")
    assert '26                   10.0.0.2' in cmdOut, \
        "Test to show bootp-gateway configuration on " \
        "specified interface failed"
    return True


def bootp_gateway_runningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 27) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip address \
    10.0.0.3/24")
    devIntReturn = dut01.DeviceInteract(command="ip bootp-gateway \
    10.0.0.3")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show bootp-gateway configuration " \
        "in running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config")
    assert 'Interface: 27' and \
        'ip bootp-gateway 10.0.0.3' in cmdOut, "Test to " \
        "show bootp-gateway configuration " \
        "in running config failed"
    return True


def bootp_gateway_interface_runningConfigTest(dut01):
    if (enterConfigShell(dut01) is False):
        return False

    if (enterInterfaceContext(dut01, 28) is False):
        return False

    devIntReturn = dut01.DeviceInteract(command="ip address \
    10.0.0.6/24")
    devIntReturn = dut01.DeviceInteract(command="ip bootp-gateway \
    10.0.0.6")
    retCode = devIntReturn.get('returnCode')
    assert retCode == 0, "Test to show bootp-gateway configuration " \
        "in interface running config failed"
    dut01.DeviceInteract(command="end")

    cmdOut = dut01.cmdVtysh(command="show running-config \
    interface 28")
    assert 'Interface: 28' and \
        'ip bootp-gateway 10.0.0.6' in cmdOut, "Test to " \
        "show bootp-gateway configuration " \
        "in interface running config failed"
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

    def test_dhcp_relay_hop_count_increment_enable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_hop_count_increment_enable(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay hop count "
                              "increment - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay hop count "
                               "increment - failed")

    def test_dhcp_relay_option_82_enable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_option_82_enable(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 - passed")
        else:
            LogOutput('error', "Enable DDHCP-Relay option 82 - failed")

    def test_dhcp_relay_option_82_validation_enable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_option_82_validation_enable(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 "
                              "validation - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 "
                               "validation - failed")

    def test_relay_option_82_with_validation_and_replace(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_validation_and_replace(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 validation with "
                              "replace policy - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 validation with "
                               "replace policy - failed")

    def test_relay_option_82_with_validation_replace_and_mac_rid(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            relay_option_82_with_validation_replace_and_mac_rid(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 validation with "
                              "replace policy and mac remote ID - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 validation with "
                               "replace policy and mac remote ID - failed")

    def test_relay_option_82_with_validation_replace_and_ip_rid(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_validation_replace_and_ip_rid(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 validation with "
                              "replace policy and ip remote ID - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 validation with "
                               "replace policy and ip remote ID - failed")

    def test_relay_option_82_with_validation_and_drop(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_validation_and_drop(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 validation with "
                              "drop policy - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 validation with "
                               "drop policy - failed")

    def test_relay_option_82_with_validation_drop_and_mac_rid(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_validation_drop_and_mac_rid(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 validation with "
                              "drop policy and mac remote ID - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 validation with "
                               "drop policy and mac remote ID - failed")

    def test_relay_option_82_with_validation_drop_and_ip_rid(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_validation_drop_and_ip_rid(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 validation with "
                              "drop policy and ip remote ID - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 validation with "
                               "drop policy and ip remote ID - failed")

    def test_relay_option_82_with_keep_policy(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_keep_policy(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 with "
                              "keep policy - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 with "
                               "keep policy - failed")

    def test_relay_option_82_with_keep_and_mac_rid(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_keep_and_mac_rid(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 with "
                              "keep policy and mac remote ID - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 with "
                               "keep policy and mac remote ID - failed")

    def test_relay_option_82_with_keep_and_ip_rid(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_keep_and_ip_rid(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 with "
                              "keep policy and ip remote ID - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 with "
                               "keep policy and ip remote ID - failed")

    def test_relay_option_82_with_replace_policy(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_replace_policy(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 with "
                              "replace policy - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 with "
                               "replace policy - failed")

    def test_relay_option_82_with_replace_and_mac_rid(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_replace_and_mac_rid(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 with "
                              "replace policy and mac remote ID - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 with "
                               "replace policy and mac remote ID - failed")

    def test_relay_option_82_with_replace_and_ip_rid(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_replace_and_ip_rid(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 with "
                              "replace policy and ip remote ID - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 with "
                               "replace policy and ip remote ID - failed")

    def test_relay_option_82_with_replace_and_validation(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_replace_and_validation(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 with "
                              "replace policy and validation - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 with "
                               "replace policy and validation - failed")

    def test_relay_option_82_with_replace_validation_and_mac_rid(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            relay_option_82_with_replace_validation_and_mac_rid(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 with "
                              "replace policy, validation and mac "
                              "remote ID - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 with "
                               "replace policy, validation and mac "
                               " remote ID - failed")

    def test_relay_option_82_with_replace_validation_and_ip_rid(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_replace_validation_and_ip_rid(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 with "
                              "replace policy, validation and ip "
                              "remote ID - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 with "
                               "replace policy, validation and ip "
                               " remote ID - failed")

    def test_relay_option_82_with_drop_policy(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_drop_policy(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 with "
                              "drop policy - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 with "
                               "drop policy - failed")

    def test_relay_option_82_with_drop_and_mac_rid(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_keep_and_mac_rid(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 with "
                              "drop policy and mac remote ID - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 with "
                               "drop policy and mac remote ID - failed")

    def test_relay_option_82_with_drop_and_ip_rid(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_drop_and_ip_rid(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 with "
                              "drop policy and ip remote ID - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 with "
                               "drop policy and ip remote ID - failed")

    def test_relay_option_82_with_drop_and_validation(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_drop_and_validation(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 with "
                              "drop policy and validation - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 with "
                               "drop policy and validation - failed")

    def test_relay_option_82_with_drop_validation_and_mac_rid(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_drop_validation_and_mac_rid(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 with "
                              "drop policy, validation and mac "
                              "remote ID - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 with "
                               "drop policy, validation and mac "
                               " remote ID - failed")

    def test_relay_option_82_with_drop_validation_and_ip_rid(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option_82_with_drop_validation_and_ip_rid(dut01Obj)
        if(retValue):
            LogOutput('info', "Enable DHCP-Relay option 82 with "
                              "drop policy, validation and ip "
                              "remote ID - passed")
        else:
            LogOutput('error', "Enable DHCP-Relay option 82 with "
                               "drop policy, validation and ip "
                               " remote ID - failed")

    def test_dhcp_relay_hop_count_increment_disable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_hop_count_increment_disable(dut01Obj)
        if(retValue):
            LogOutput('info', "Disable DHCP-Relay option 82 hop count "
                              "increment - passed")
        else:
            LogOutput('error', "Disable DHCP-Relay option 82 hop count "
                               "increment - failed")

    def test_dhcp_relay_option_82_disable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_option_82_disable(dut01Obj)
        if(retValue):
            LogOutput('info', "Disable DHCP-Relay option 82 - passed")
        else:
            LogOutput('info', "Disable DHCP-Relay option 82 - failed")

    def test_dhcp_relay_option_82_validation_disable(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_option_82_validation_disable(dut01Obj)
        if(retValue):
            LogOutput('info', "Disable DHCP-Relay option 82 "
                              "validation - passed")
        else:
            LogOutput('error', "Disable DHCP-Relay option 82 "
                               "validation - failed")

    def test_show_dhcp_relay(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = show_dhcp_relay(dut01Obj)
        if(retValue):
            LogOutput('info', "Show DHCP-Relay configuration - passed")
        else:
            LogOutput('error', "Show DHCP-Relay configuration - failed")

    def test_show_dhcp_relay_statistics(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = show_dhcp_relay_statistics(dut01Obj)
        if(retValue):
            LogOutput('info', "Show DHCP-Relay statistics - passed")
        else:
            LogOutput('error', "Show DHCP-Relay statistics - failed")

    def test_show_dhcp_relay_option82_statistics(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = show_dhcp_relay_option82_statistics(dut01Obj)
        if(retValue):
            LogOutput('info', "Show DHCP-Relay option 82 "
                              "statistics - passed")
        else:
            LogOutput('error', "Show DHCP-Relay option 82 "
                               "statistics - failed")

    def test_dhcp_relayRunningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relayRunningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Show DHCP-Relay "
                              "running configuration - passed")
        else:
            LogOutput('error', "Show DHCP-Relay "
                               "running configuration - failed")

    def test_dhcp_relay_hopCountIncrementRunningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = dhcp_relay_hopCountIncrementRunningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Show DHCP-Relay hop count increment "
                              "running configuration - passed")
        else:
            LogOutput('error', "DHCP-Relay hop count increment "
                               "running configuration - failed")

    def test_relay_option82_dropRunningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option82_dropRunningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Show DHCP-Relay with drop policy "
                              "running configuration - passed")
        else:
            LogOutput('error', "Show DHCP-Relay with drop policy "
                               "running configuration - failed")

    def test_relay_option82_keepRunningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option82_keepRunningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Show DHCP-Relay with keep policy "
                              "running configuration - passed")
        else:
            LogOutput('error', "Show DHCP-Relay with keep policy "
                               "running configuration - failed")

    def test_relay_option82_replaceRunningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = relay_option82_replaceRunningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Show DHCP-Relay with replace policy "
                              "running configuration - passed")
        else:
            LogOutput('error', "Show DHCP-Relay with replace policy "
                               "running configuration - failed")

    def test_relay_option82_replace_with_validationRunningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            relay_option82_replace_with_validationRunningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Show DHCP-Relay with replace policy and "
                              "validation running configuration - passed")
        else:
            LogOutput('error', "Show DHCP-Relay with replace policy and "
                               "validation running configuration - failed")

    def test_relay_option82_drop_with_validationRunningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            relay_option82_drop_with_validationRunningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Show DHCP-Relay with drop policy and "
                              "validation running configuration - passed")
        else:
            LogOutput('error', "Show DHCP-Relay with drop policy and "
                               "validation running configuration - failed")

    def test_relay_option82_keep_with_ip_ridRunningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            relay_option82_keep_with_ip_ridRunningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Show DHCP-Relay with keep policy and "
                              "ip remote id running configuration - passed")
        else:
            LogOutput('error', "Show DHCP-Relay with keep policy and "
                               "ip remote id running configuration - failed")

    def test_relay_option82_replace_with_ip_ridRunningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            relay_option82_replace_with_ip_ridRunningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Show DHCP-Relay with replace policy and "
                              "ip remote id running configuration - passed")
        else:
            LogOutput('error', "Show DHCP-Relay with replace policy and "
                               "ip remote id running configuration - failed")

    def test_relay_option82_drop_with_ip_ridRunningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            relay_option82_drop_with_ip_ridRunningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Show DHCP-Relay with drop policy and "
                              "ip remote id running configuration - passed")
        else:
            LogOutput('error', "Show DHCP-Relay with drop policy and "
                               "ip remote id running configuration - failed")

    def test_relay_option82_replace_validation_ip_ridRunningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            relay_option82_replace_validation_ip_ridRunningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Show DHCP-Relay with replace policy, "
                              "validation and ip remote id running "
                              "configuration - passed")
        else:
            LogOutput('error', "Show DHCP-Relay with replace policy, "
                               "validation and ip remote id running "
                               "configuration -- failed")

    def test_relay_option82_drop_validation_ip_ridRunningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            relay_option82_drop_validation_ip_ridRunningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Show DHCP-Relay with drop policy, "
                              "validation and ip remote id running "
                              "configuration - passed")
        else:
            LogOutput('error', "Show DHCP-Relay with drop policy, "
                               "validation and ip remote id running "
                               "configuration -- failed")

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

    def test_showHelperAddress_and_bootpGatewayConfig_onInterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            showHelperAddress_and_bootpGatewayConfig_onInterface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to show helper-address configuration "
                              "when the bootp-gateway is unconfigured "
                              "on specified interface - passed")
        else:
            LogOutput('error', "Test to show helper-address configuration "
                               "when the bootp-gateway is unconfigured "
                               "on specified interface - failed")

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
            LogOutput('info', "Test to show helper-address "
                              "configuration in interface "
                              "running config - passed")
        else:
            LogOutput('error', "Test to show helper-address "
                               "configuration in interface "
                               "running config - failed")

    def test_bootpGateway_configFor_configIP_onInterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            bootpGateway_configFor_configIP_onInterface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set bootp-gateway configuration "
                              "for configured IP address on an "
                              "interface - passed")
        else:
            LogOutput('error', "Test to set bootp-gateway configuration "
                               "for configured IP address on an "
                               "interface - failed")

    def test_bootpGateway_configFor_configIP_onSubInterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            bootpGateway_configFor_configIP_onSubInterface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set bootp-gateway configuration "
                              "for configured IP address on a "
                              "sub interface - passed")
        else:
            LogOutput('error', "Test to set bootp-gateway configuration "
                               "for configured IP address on a "
                               "sub interface - failed")

    def test_bootpGateway_configFor_configSecondaryIP_onInterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            bootpGateway_configFor_configSecondaryIP_onInterface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set bootp-gateway configuration "
                              "for configured secondary IP address on an "
                              "interface - passed")
        else:
            LogOutput('error', "Test to set bootp-gateway configuration "
                               "for configured secondary IP address on an "
                               "interface - failed")

    def test_bootpGateway_configFor_configSecondaryIP_onSubInterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            bootpGateway_configFor_configSecondaryIP_onSubInterface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set bootp-gateway configuration "
                              "for configured secondary IP address on a "
                              "sub interface - passed")
        else:
            LogOutput('error', "Test to set bootp-gateway configuration "
                               "for configured secondary IP address on a "
                               "sub interface - failed")

    def test_bootpGateway_configFor_unconfigIP_onInterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            bootpGateway_configFor_unconfigIP_onInterface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set bootp-gateway configuration "
                              "for unconfigured IP address on an "
                              "interface - passed")
        else:
            LogOutput('error', "Test to set bootp-gateway configuration "
                               "for unconfigured IP address on an "
                               "interface - failed")

    def test_bootpGateway_configFor_invalidIP(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = bootpGateway_configFor_invalidIP(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to set bootp-gateway configuration "
                              "for configured IP address on an "
                              "interface - passed")
        else:
            LogOutput('error', "Test to set bootp-gateway configuration "
                               "for configured IP address on an "
                               "interface - failed")

    def test_bootpGateway_unconfigFor_configbootpGateway_onInterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            bootpGateway_unconfigFor_configbootpGateway_onInterface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to unset bootp-gateway configuration "
                              "for configured bootp-gateway on an "
                              "interface - passed")
        else:
            LogOutput('error', "Test to unset bootp-gateway configuration "
                               "for configured bootp-gateway on an "
                               "interface - failed")

    def test_bootpGateway_unconfigFor_configIP_onInterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            bootpGateway_unconfigFor_configIP_onInterface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to unset bootp-gateway configuration "
                              "for configured IP address on an "
                              "interface - passed")
        else:
            LogOutput('error', "Test to unset bootp-gateway configuration "
                               "for configured IP address on an "
                               "interface - failed")

    def test_bootpGateway_unconfigFor_unconfigIP_onInterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            bootpGateway_unconfigFor_unconfigIP_onInterface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to unset bootp-gateway configuration "
                              "for unconfigured IP address on an "
                              "interface - passed")
        else:
            LogOutput('error', "Test to unset bootp-gateway configuration "
                               "for unconfigured IP address on an "
                               "interface - failed")

    def test_show_bootp_gateway_status(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = show_bootp_gateway_status(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to show bootp-gateway "
                              "configuration - passed")
        else:
            LogOutput('error', "Test to show bootp-gateway "
                               "configuration - failed")

    def test_showBootpGateway_and_helperAddressConfig_onInterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = \
            showBootpGateway_and_helperAddressConfig_onInterface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to show bootp-gateway configuration "
                              "when the helper-address is unconfigured "
                              "on specified interface - passed")
        else:
            LogOutput('error', "Test to show bootp-gateway configuration "
                               "when the helper-address is unconfigured "
                               "on specified interface - failed")

    def test_show_bootp_gateway_statusOnSpecifiedInterface(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = show_bootp_gateway_statusOnSpecifiedInterface(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to show bootp-gateway "
                              "configuration on specified "
                              "interface - passed")
        else:
            LogOutput('error', "Test to show bootp-gateway "
                               "configuration on specified "
                               "interface - failed")

    def test_bootp_gateway_runningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = bootp_gateway_runningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to show bootp-gateway "
                              "configuration in running "
                              "config - passed")
        else:
            LogOutput('error', "Test to show bootp-gateway "
                               "configuration in running "
                               "config - failed")

    def test_bootp_gateway_interface_runningConfigTest(self):
        dut01Obj = self.topoObj.deviceObjGet(device="dut01")
        retValue = bootp_gateway_interface_runningConfigTest(dut01Obj)
        if(retValue):
            LogOutput('info', "Test to show bootp-gateway"
                              "configuration in interface "
                              "running config - passed")
        else:
            LogOutput('error', "Test to show bootp-gateway "
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
