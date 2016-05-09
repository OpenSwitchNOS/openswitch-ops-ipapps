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

from time import sleep
import re

TOPOLOGY = """
#
# +-------+
# |  sw1  |
# +-------+
#

# Nodes
[type=openswitch name="Switch 1"] sw1
"""


def dhcpv6_relay_enable(sw1):
    sw1("configure terminal")
    sw1("dhcpv6-relay")

    out = sw1("do show dhcpv6-relay")
    assert re.search("(DHCPV6 Relay Agent\s+:\s+?enabled)", out) is not None
    sw1("end")
    return True


def dhcpv6_relay_disable(sw1):
    sw1("configure terminal")
    sw1("no dhcpv6-relay")

    out = sw1("do show dhcpv6-relay")
    assert re.search("(DHCPV6 Relay Agent\s*:\s*?disabled)", out) is not None
    sw1("end")
    return True


def dhcpv6_relay_option_79_enable(sw1):
    sw1("configure terminal")
    sw1("dhcpv6-relay option 79")

    out = sw1("do show dhcpv6-relay")
    assert re.search("(Option 79\s*:\s*?enabled)", out) is not None
    sw1("end")
    return True


def dhcpv6_relay_option_79_disable(sw1):
    sw1("configure terminal")
    sw1("no dhcpv6-relay option 79")

    out = sw1("do show dhcpv6-relay")
    assert re.search("(Option 79\s*:\s*?disabled)", out) is not None
    sw1("end")
    return True


def show_dhcpv6_relay(sw1):
    sw1("configure terminal")
    sw1("dhcpv6-relay")
    sw1("end")

    out = sw1("show dhcpv6-relay")
    assert re.search("(DHCPV6 Relay Agent\s*:\s*?enabled)", out) is not None
    return True


def show_dhcpv6_relay_statistics(sw1):
    sw1("configure terminal")
    sw1("dhcpv6-relay")
    sw1("end")

    out = sw1("show dhcpv6-relay")
    assert re.search("(DHCPV6 Relay Agent\s*:\s*?enabled)", out) is not None \
        and "DHCPV6 Relay Statistics:" and \
        "Client Requests       Server Responses" and \
        "Valid      Dropped    Valid      Dropped" in out
    return True


def dhcpv6_relay_running_config_test(sw1):
    sw1("configure terminal")
    sw1("dhcpv6-relay")
    sw1("end")

    out = sw1("show running-config")
    assert "dhcpv6-relay" in out
    return True


def dhcpv6_relay_option79_running_config_test(sw1):
    sw1("configure terminal")
    sw1("dhcpv6-relay option 79")
    sw1("end")

    out = sw1("show running-config")
    assert "dhcpv6-relay option 79" in out
    return True


def test_dhcp_relay_ipapps_configuration(topology, step):
    sw1 = topology.get("sw1")

    assert sw1 is not None

    dhcpv6_relay_enable(sw1)

    dhcpv6_relay_disable(sw1)

    dhcpv6_relay_option_79_enable(sw1)

    dhcpv6_relay_option_79_disable(sw1)

    show_dhcpv6_relay(sw1)

    show_dhcpv6_relay_statistics(sw1)

    dhcpv6_relay_running_config_test(sw1)

    dhcpv6_relay_option79_running_config_test(sw1)
