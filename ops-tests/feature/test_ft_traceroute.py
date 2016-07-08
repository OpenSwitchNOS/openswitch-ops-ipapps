#  Copyright (C) 2016 Hewlett Packard Enterprise Development LP
#  Licensed under the Apache License, Version 2.0 (the "License"); you may
#  not use this file except in compliance with the License. You may obtain
#  a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#  License for the specific language governing permissions and limitations
#  under the License.

from pytest import mark

TOPOLOGY = """
#
#  +-------+
#  |       |     +-------+    +-------+     +-------+
#  |  hs1  <----->  sw1  <---->  sw2  <----->  sw3  |
#  |       |     +-------+    +-------+     +-------+
#  +-------+
#

#  Nodes
[type=openswitch name="Switch 1"] sw1
[type=openswitch name="Switch 2"] sw2
[type=openswitch name="Switch 3"] sw3
[type=host name="host 1"] h1

#  Links
sw1:if01 -- sw2:if01
sw2:if02 -- sw3:if01
sw1:if02 -- h1:if01
"""


def configure(sw1, sw2, sw3, h1):
    # Enabling interface 1 SW1
    print("Enabling interface1 on SW1")
    with sw1.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.no_shutdown()

    # Enabling interface 2 SW1
    print("Enabling interface2 on SW1")
    with sw1.libs.vtysh.ConfigInterface('if02') as ctx:
        ctx.no_shutdown()

    # Enabling interface 1 SW2
    print("Enabling interface1 on SW2")
    with sw2.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.no_shutdown()

    # Enabling interface 2 SW2
    print("Enabling interface2 on SW2")
    with sw2.libs.vtysh.ConfigInterface('if02') as ctx:
        ctx.no_shutdown()

    # Enabling interface 2 SW3
    print("Enabling interface2 on SW3")
    with sw3.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.no_shutdown()

    # Entering interface for link 1 SW1, giving an IPv4 address
    print("Configuring IPv4 address on link 1 SW1")
    with sw1.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.ip_address('10.0.30.1/24')

    # Entering interface for link 1 SW1, giving an IPv6 address
    print("Configuring IPv6 address on link 1 SW1")
    with sw1.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.ipv6_address('1030::1/120')

    #  Entering interface for link 2 SW1, giving an IPv4 address
    print("Configuring IPv4 address on link 2 SW1")
    with sw1.libs.vtysh.ConfigInterface('if02') as ctx:
        ctx.ip_address('10.0.10.2/24')

    print("Configuring IPv6 address on link 2 SW1")
    with sw1.libs.vtysh.ConfigInterface('if02') as ctx:
        ctx.ipv6_address('1010::2/120')

    # Entering interface for link 1 SW2, giving an IPv4 address
    print("Configuring IPv4 address on link 1 SW2")
    with sw2.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.ip_address('10.0.30.2/24')

    # Entering interface for link 1 SW2, giving an IPv6 address
    print("Configuring IPv6 address on link 1 SW2")
    with sw2.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.ipv6_address('1030::2/120')

    # Entering interface for link 2 SW2, giving an IPv4 address
    print("Configuring IPv4 address on link 2 SW2")
    with sw2.libs.vtysh.ConfigInterface('if02') as ctx:
        ctx.ip_address('10.0.40.2/24')

    # Entering interface for link 2 SW2, giving an IPv6 address
    print("Configuring IPv6 address on link 2 SW2")
    with sw2.libs.vtysh.ConfigInterface('if02') as ctx:
        ctx.ipv6_address('2030::2/120')

    # Entering interface for link 2 SW3, giving an IPv4 address
    print("Configuring IPv4 address on link 2 SW3")
    with sw3.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.ip_address('10.0.40.1/24')

    # Entering interface for link 2 SW3, giving an IPv6 address
    print("Configuring IPv6 address on link 2 SW3")
    with sw3.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.ipv6_address('2030::1/120')

    print("Configuring static ipv4 route on switch1")
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.ip_route('10.0.40.0/24', '10.0.30.2')

    print("Configuring static ipv4 route on switch2")
    with sw2.libs.vtysh.Configure() as ctx:
        ctx.ip_route('10.0.10.0/24', '10.0.30.1')

    print("Configuring static ipv4 route on switch3")
    with sw3.libs.vtysh.Configure() as ctx:
        ctx.ip_route('10.0.30.0/24', '10.0.40.2')

    print("Configuring static ipv4 route on switch3")
    with sw3.libs.vtysh.Configure() as ctx:
        ctx.ip_route('10.0.10.0/24', '10.0.40.2')

    print("Configuring static ipv6 route on switch 1")
    with sw1.libs.vtysh.Configure() as ctx:
        ctx.ipv6_route('2030::/120', '1030::2')

    print("Configuring static ipv6 route on switch 2")
    with sw2.libs.vtysh.Configure() as ctx:
        ctx.ipv6_route('1010::/120', '1030::1')

    print("Configuring static ipv6 route on switch 3")
    with sw3.libs.vtysh.Configure() as ctx:
        ctx.ipv6_route('1030::/120', '2030::2')

    print("Configuring static ipv6 route on switch 3")
    with sw3.libs.vtysh.Configure() as ctx:
        ctx.ipv6_route('1010::/120', '2030::2')

    #  Configure host 1
    print("\nConfiguring host 1 IPv4")
    h1.libs.ip.interface('if01', addr='10.0.10.1/24', up=True)

    print("\nConfiguring host 1 IPv6")
    h1.libs.ip.interface('if01', addr='1010::1/120', up=True)

    # Configuring static routes
    # Configuring static routes on host 1
    print("Configuring routes on the workstations")
    h1.libs.ip.add_route('default', '10.0.10.2')
    h1.libs.ip.add_route('1030::0/120', '1010::2')
    h1.libs.ip.add_route('2030::0/120', '1010::2')

    ret_buffer = sw1.libs.vtysh.show_running_config()
    print(ret_buffer)
    ret_buffer = sw2.libs.vtysh.show_running_config()
    print(ret_buffer)
    ret_buffer = sw3.libs.vtysh.show_running_config()
    print(ret_buffer)


def traceroute_basic(sw3, h1):
    # Traceroute with IPv4-address from switch3 to host1
    print("Test traceroute with IPv4-address from switch3 to host1")
    out = sw3("traceroute 10.0.10.1")

    assert '10.0.10.1' in out

    # Traceroute with hostname from switch3 to host1
    print("Test traceroute with hostname from switch3 to host1")
    out = sw3("traceroute h1")

    # assert 'host1' in out


def traceroute_with_options(sw3):
    # Traceroute from switch3 to host1 with the maxttl parameter
    print("Test traceroute from switch3 to host1 with the maxttl parameter")
    out = sw3("traceroute 10.0.10.1 maxttl 2")

    assert '10.0.10.1' in out

    # Traceroute from switch3 to host1 with the minttl parameter
    print("Test traceroute from switch3 to host1 with the minttl parameter")
    out = sw3("traceroute 10.0.10.1 minttl 1")

    assert '10.0.10.1' in out

    # Traceroute from switch3 to host1 with the dstport parameter
    print("Test traceroute from switch3 to host1 with the dstport parameter")
    out = sw3("traceroute 10.0.10.1 dstport 33434")
    assert '10.0.10.1' in out

    # Traceroute from switch3 to host1 with the probes parameter
    print("Test traceroute from switch3 to host1 with the probes parameter")
    out = sw3("traceroute 10.0.10.1 probes 3")

    assert '10.0.10.1' in out

    # Traceroute from switch3 to host1 with the timeout parameter
    print("Test traceroute from switch3 to host1 with the timeout parameter")
    out = sw3("traceroute 10.0.10.1 timeout 3")

    assert '10.0.10.1' in out


def traceroute6_basic(sw3):
    # Traceroute with IPv6-address from switch3 to host1
    print("Test traceroute with IPv6-address from switch3 to host1")
    out = sw3("traceroute6 1010::1")

    assert 'traceroute to 1010::1' in out


def traceroute6_with_options(sw3):
    # Traceroute6 from switch3 to host1 with the maxttl parameter
    print("Test traceroute6 from switch3 to host1 with the maxttl parameter")
    out = sw3("traceroute6 1010::1 maxttl 2")

    assert 'traceroute to 1010::1' in out

    # Traceroute6 from switch3 to host1 with the dstport parameter
    print("Test traceroute6 from switch3 to host1 with the dstport parameter")
    out = sw3("traceroute6 1010::1 dstport 334")

    assert 'traceroute to 1010::1' in out

    # Traceroute6 from switch3 to host1 with the probes parameter
    print("Test traceroute6 from switch3 to host1 with the probes parameter")
    out = sw3("traceroute6 1010::1 probes 3")

    assert "traceroute to 1010::1" in out

    # Traceroute6 from switch3 to host1 with the timeout parameter
    print("Test traceroute6 from switch3 to host1 with the timeout parameter")
    out = sw3("traceroute6 1010::1 timeout 3")

    assert "traceroute to 1010::1" in out


def traceroute_failure_cases(sw3):
    # Traceroute network unreachable case
    print("Test traceroute network unreachable case")
    out = sw3.libs.vtysh.traceroute("10.11.1.1")

    assert "Network is unreachable" == out['network_unreachable']

    # Traceroute destination unreachable case
    print("Test traceroute destination host unreachable case")
    out = sw3.libs.vtysh.traceroute("10.0.40.5")

    assert '!H' == out['dst_unreachable']

    # Traceroute unknown host
    print("Test traceroute unknown host case")
    out = sw3("traceroute asdfrgqweewwe")

    assert 'traceroute: unknown host' in out

    # Traceroute with maxttl is lesthan the number of hops case
    print("Test traceroute with maxttl is lesthan the number of hops case")
    out = sw3.libs.vtysh.traceroute("10.0.10.1", "", "1")

    assert '2 10.0.10.1' not in out

    # Traceroute with minttl and maxxttl are lesthan the number of hops case
    print("Test traceroute with  minttl and maxttl is lesthan the number"
          " of hops case")
    out = sw3.libs.vtysh.traceroute("10.0.10.1", "1", "1")

    assert '10.0.40.2' == out[1]['int_hop']

    # Traceroute with the probes exceeds the maximum supported by the platform
    print("Test traceroute with probes exceeds the maximum supported by "
          "the platform case")
    out = sw3("traceroute 10.0.10.1 probes 6")

    assert 'Unknown command' in out


def traceroute6_failure_cases(sw3):
    # Traceroute6 network unreachable case
    print("Test traceroute6 network unreachable case")
    out = sw3.libs.vtysh.traceroute6("1050::4")

    assert 'Network is unreachable' == out['network_unreachable']

    # Traceroute6 destination unreachable case( this case is not working
    # correctly : recheck)
    print("Test traceroute6 destination host unreachable case")
    out = sw3.libs.vtysh.traceroute6("2030::4")

    assert '!H' == out['dst_unreachable']

    # Traceroute6 unknown host
    print("Test traceroute6 unknown host case")
    out = sw3("traceroute6 asdfrgqweewwe")

    assert 'traceroute: unknown host' in out

    # Traceroute6 with maxttl is lesthan the number of hops case
    print("Test traceroute6 with maxttl is lesthan the number of hops case")
    out = sw3.libs.vtysh.traceroute6("1010::1", "1")

    assert '2 1010::1' not in out

    # Traceroute6 with probes exceeds the maximum supported by the platform
    print("Test traceroute6 with probes exceeds the maximum supported by "
          "the platform case")
    out = sw3("traceroute6 1010::1 probes 6")

    assert 'Unknown command' in out


@mark.gate
def test_traceroute(topology, step):
    sw1 = topology.get('sw1')
    sw2 = topology.get('sw2')
    sw3 = topology.get('sw3')
    h1 = topology.get('h1')

    assert sw1 is not None
    assert sw2 is not None
    assert sw3 is not None
    assert h1 is not None

    configure(sw1, sw2, sw3, h1)

    step("# # #  Basic traceroute tests # # # ")
    traceroute_basic(sw3, h1)

    step("\n# # #  Traceroute with options tests # # # ")
    traceroute_with_options(sw3)

    step("\n# # #  Basic traceroute6 tests # # # ")
    traceroute6_basic(sw3)

    step("\n# # #  Traceroute6 test with options # # # ")
    traceroute6_with_options(sw3)

    step("\n# # #  Traceroute failure test cases # # # ")
    traceroute_failure_cases(sw3)

    step("\n# # #  Traceroute6 failure test cases # # # ")
    traceroute6_failure_cases(sw3)
