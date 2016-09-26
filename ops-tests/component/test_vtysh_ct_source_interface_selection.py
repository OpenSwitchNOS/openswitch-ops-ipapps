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

TOPOLOGY = """
#
# +-------+
# |  sw1  |
# +-------+
#

# Nodes
[type=openswitch name="Switch 1"] sw1
"""


def test_source_interface_configuration(topology, step):
    sw1 = topology.get('sw1')

    assert sw1 is not None

    step('Set source IP address configuration with unconfigured IP address')
    sw1('configure terminal')
    out = sw1("ip source-interface all 10.0.0.1")
    assert 'Specified IP address is not configured on any interface.' \
        in out

    step('Set source port configuration with no IP address configured port')
    out = sw1("ip source-interface all interface 1")
    assert 'No IP address is configured on the specified interface.' \
        in out

    step('Validate source IP address configuration with invalid IP address')
    sw1('configure terminal')
    out = sw1("ip source-interface all 300.0.0.1")
    assert '% Unknown command.' in out

    step('Validate source IP address configuration with loopback IP address')
    sw1('configure terminal')
    out = sw1("ip source-interface all 127.0.0.1")
    assert 'Broadcast, multicast and loopback addresses are not allowed.' \
        in out

    step('Validate source IP address configuration with multicast IP address')
    sw1('configure terminal')
    out = sw1("ip source-interface all 224.0.0.0")
    assert 'Broadcast, multicast and loopback addresses are not allowed.' \
        in out

    step('Validate source IP address configuration with broadcast IP address')
    sw1('configure terminal')
    out = sw1("ip source-interface all 255.255.255.255")
    assert 'Broadcast, multicast and loopback addresses are not allowed.' \
        in out

    step('Unset source IP address configuration with unconfigured IP address ' \
    'for all the defined protocols')
    out = sw1("no ip source-interface all")
    assert 'Common source interface is not configured.' \
        in out

    step('Unset source IP address configuration with unconfigured IP address ' \
    'for TFTP protocol')
    sw1('configure terminal')
    out = sw1("no ip source-interface tftp")
    assert 'TFTP Source interface is not configured.' \
        in out

    step('Set source IP address to all the defined protocols')
    sw1("interface 1")
    sw1("ip address 1.1.1.1/24")
    sw1("exit")

    sw1("ip source-interface all 1.1.1.1")
    out = sw1("do show ip source-interface")
    assert 'tftp            1.1.1.1' in out

    step('Set source IP address to tftp protocol')
    sw1("interface 2")
    sw1("ip address 2.2.2.2/24")
    sw1("exit")
    sw1("ip source-interface tftp 2.2.2.2")
    out = sw1("do show ip source-interface tftp")
    assert 'tftp            2.2.2.2' in out

    step('Set source IP address to tacacs protocol')
    sw1("interface 3")
    sw1("ip address 3.3.3.3/24")
    sw1("exit")
    sw1("ip source-interface tacacs 3.3.3.3")
    out = sw1("do show ip source-interface tacacs")
    assert 'tacacs          3.3.3.3' in out

    step('Set source IP address to radius protocol')
    sw1("interface 4")
    sw1("ip address 4.4.4.4/24")
    sw1("exit")
    sw1("ip source-interface radius 4.4.4.4")
    out = sw1("do show ip source-interface radius")
    assert 'radius          4.4.4.4' in out

    step('Set source IP interface to tftp protocol')
    sw1("ip source-interface tftp interface 1")
    out = sw1("do show ip source-interface tftp")
    assert 'tftp            1' in out

    step('Set source IP interface to tacacs protocol')
    sw1("ip source-interface tacacs interface 1")
    out = sw1("do show ip source-interface tacacs")
    assert 'tacacs          1' in out

    step('Set source IP interface to radius protocol')
    sw1("ip source-interface radius interface 1")
    out = sw1("do show ip source-interface radius")
    assert 'radius          1' in out

    step('Set source IP interface to all the defined protocols')
    sw1("no ip source-interface tftp")
    sw1("ip source-interface all interface 2")
    out = sw1("do show ip source-interface")
    assert 'tftp            2' in out

    step('Unset source IP to all the defined protocols')
    sw1("no ip source-interface all")
    out = sw1("do show ip source-interface")
    assert '' in out

    step('Unset source IP to tftp protocols')
    sw1("ip source-interface tftp 1.1.1.1")
    sw1("no ip source-interface tftp")
    out = sw1("do show ip source-interface tftp")
    assert '' in out

    step('Unset source IP to tacacs protocols')
    sw1("ip source-interface tacacs 1.1.1.1")
    sw1("no ip source-interface tacacs")
    out = sw1("do show ip source-interface tacacs")
    assert '' in out

    step('Unset source IP to radius protocols')
    sw1("ip source-interface radius 1.1.1.1")
    sw1("no ip source-interface radius")
    out = sw1("do show ip source-interface radius")
    assert '' in out

    step('show running config of source-interface to tftp protocol')
    sw1("ip source-interface tftp 2.2.2.2")
    out = sw1("do show running-config")
    assert 'ip source-interface tftp 2.2.2.2' in out

    step('show running config of source-interface to tacacs protocol')
    sw1("ip source-interface tacacs 2.2.2.2")
    out = sw1("do show running-config")
    assert 'ip source-interface tacacs 2.2.2.2' in out

    step('show running config of source-interface to radius protocol')
    sw1("ip source-interface radius 2.2.2.2")
    out = sw1("do show running-config")
    assert 'ip source-interface radius 2.2.2.2' in out

    step('show running config of source-interface to all the defined protocols')
    sw1("ip source-interface all interface 2")
    out = sw1("do show running-config")
    assert 'ip source-interface all interface 2' in out
