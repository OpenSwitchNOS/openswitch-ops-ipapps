# Copyright (C) 2016 Hewlett Packard Enterprise Development LP
#
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

from pytest import mark
from time import sleep

TOPOLOGY = """
#
# +-------+
# |       |     +-------+     +-------+
# |  hs1  <----->  sw1  <----->  sw2  |
# |       |     +-------+     +-------+
# +-------+
#

# Nodes
[type=openswitch name="Switch 1"] sw1
[type=openswitch name="Switch 2"] sw2
[type=host name="host 1"] h1

# Links
h1:if01 -- sw1:if01
sw1:if02 -- sw2:if01
"""


def configure(sw1, sw2, h1):
    # Host1 configuration
    print("Configuring host1 with IPv4 address")
    h1.libs.ip.interface('if01', addr='10.0.30.1/24', up=True)

    print("Configuring host1 with IPv6 address")
    h1.libs.ip.interface('if01', addr='1030::1/120', up=True)

    # Enabling interface 1 SW1
    print("Enabling interface1 on SW1")
    with sw1.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.no_shutdown()

    # Enabling interface 2 SW1
    print("Enabling interface2 on SW1")
    with sw1.libs.vtysh.ConfigInterface('if02') as ctx:
        ctx.no_shutdown()

    # Entering interface for link 1 SW1, giving an IPv4 address
    print("Configuring IPv4 address on link 1 SW1")
    with sw1.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.ip_address('10.0.30.2/24')

    # Entering interface for link 1 SW1, giving an IPv6 address
    print("Configuring IPv6 address on link 1 SW1")
    with sw1.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.ipv6_address('1030::2/120')

    # Entering interface for link 2 SW1, giving an IPv4 address
    print("Configuring IPv4 address on link 2 SW1")
    with sw1.libs.vtysh.ConfigInterface('if02') as ctx:
        ctx.ip_address('10.0.10.2/24')

    print("Configuring IPv6 address on link 2 SW1")
    with sw1.libs.vtysh.ConfigInterface('if02') as ctx:
        ctx.ipv6_address('1010::2/120')

    # Enabling interface 1 SW2
    print("Enabling interface1 on SW2")
    with sw2.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.no_shutdown()

    # Entering interface for link 1 SW2, giving an IPv4 address
    print("Configuring IPv4 address on link 1 SW2")
    with sw2.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.ip_address('10.0.10.3/24')

    # Entering interface for link 1 SW2, giving an IPv6 address
    print("Configuring IPv6 address on link 1 SW2")
    with sw2.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.ipv6_address('1010::3/120')

    # Configuring static routes
    print("Configuring static IPv4 route on host1")
    h1.libs.ip.add_route('10.0.10.0/24', '10.0.30.2')

    print("Configuring static IPv4 route on switch2")
    with sw2.libs.vtysh.Configure() as ctx:
        ctx.ip_route('10.0.30.0/24', '10.0.10.2')

    print("Configuring static IPv6 route on host1")
    h1.libs.ip.add_route('1010::0/120', '1030::2')

    print("Configuring static IPv6 route on switch2")
    with sw2.libs.vtysh.Configure() as ctx:
        ctx.ipv6_route('1030::/120', '1010::2')

    ret_buffer = sw1.libs.vtysh.show_running_config()
    print(ret_buffer)
    ret_buffer = sw2.libs.vtysh.show_running_config()
    print(ret_buffer)

    cmd_out = h1("netstat -rn")
    print("IPv4 Route table for workstation 1:\n" + str(cmd_out))


def check_interface_status(sw1, sw2):
    sw1p1 = sw1.ports['if01']
    sw1p2 = sw1.ports['if02']
    sw2p1 = sw2.ports['if01']

    iter = 0
    while iter != 2:
        retry = 0

        out = sw1("ovs-vsctl get interface {sw1p1} \
                  admin_state".format(**locals()), shell="bash")
        if 'down' in out:
            retry = 1

        out = sw1("ovs-vsctl get interface {sw1p2} \
                  admin_state".format(**locals()), shell="bash")
        if 'down' in out:
            retry = 1

        outt = sw2("ovs-vsctl get interface {sw2p1} \
                   admin_state".format(**locals()), shell="bash")
        if 'down' in out:
            retry = 1

        if (retry == 0):
            return True
        else:
            sleep(5)

        iter += 1

    if (retry != 0):
        return False

    return True


def ping_basic(sw2):
    # Ping IPv4-address from switch2 to host1
    print("Test ping IPv4-address from switch2 to host1")
    out = sw2.libs.vtysh.ping_repetitions('10.0.30.1', 1)

    assert out["packet_loss"] == 0


def ping_with_datafill_option(sw2):
    # Ping IPv4-address from switch2 to host1 with the data-fill parameter
    print("Test ping IPv4-address from switch2 to host1"
          " with the data-fill parameter")
    out = sw2.libs.vtysh.ping_repetitions('10.0.30.1 data-fill dee', 1)

    assert out["packet_loss"] == 0


def ping_with_datagram_size_option(sw2):
    # Ping IPv4-address from switch2 to host1 with the datagram-size parameter
    print("Test ping IPv4-address from switch2 to host1"
          " with the datagram-size parameter ")
    out = sw2.libs.vtysh.ping_repetitions('10.0.30.1 datagram-size 200', 1)
    # assert 'PING 10.0.30.1 (10.0.30.1) 200(228) bytes of data' \
    assert out["packet_loss"] == 0


def ping_with_interval_option(sw2):
    # Ping IPv4-address from switch2 to host1 with the interval parameter
    print("Test ping IPv4-address from switch2 to host1"
          " with the interval parameter")
    out = sw2.libs.vtysh.ping_repetitions('10.0.30.1 interval 2', 1)

    assert out["packet_loss"] == 0


def ping_with_repetition_option(sw2):
    # Ping IPv4-address with the repetition parameter from switch2 to host1
    print("Test ping IPv4-address from switch2 to host1"
          " with the repetition parameter")
    out = sw2.libs.vtysh.ping_repetitions('10.0.30.1', 2)

    assert out["packet_loss"] == 0


def ping_with_timeout_option(sw2):
    # Ping IPv4-address with the timeout parameter from switch2 to host1
    print("Test ping IPv4-address from switch2 to host1"
          " with the timeout parameter")
    out = sw2.libs.vtysh.ping_repetitions('10.0.30.1 timeout 2', 1)

    assert out["packet_loss"] == 0


def ping_with_tos_option(sw2):
    # Ping IPv4-address with the TOS parameter from switch2 to host1
    print("Test ping IPv4-address from switch2 to host1"
          " with the TOS parameter")
    out = sw2.libs.vtysh.ping_repetitions('10.0.30.1 tos 2', 1)

    assert out["packet_loss"] == 0


def ping_with_recordroute_option(sw2):
    # Ping IPv4-address with the ip-option record-route
    print("Test ping IPv4-address from switch2 to host1"
          " with the ip-option record-route")
    out = sw2.libs.vtysh.ping_repetitions('10.0.30.1 ip-option record-route',
                                          1)

    assert 'RR:' and out["packet_loss"] == 0


def ping_with_timestamp_option(sw2):
    # Ping IPv4-address with the ip-option include-timestamp
    print("Test ping IPv4-address from switch2 to host1"
          " with the ip-option include-timestamp")
    out = sw2.libs.vtysh.ping_repetitions('10.0.30.1 '
                                          'ip-option include-timestamp', 1)

    assert 'TS:' and out["packet_loss"] == 0


def ping_with_timestamp_and_address_option(sw2):
    # Ping IPv4-address with the ip-option include-timestamp-and-address
    print("Test ping IPv4-address from switch2 to host1"
          " with the ip-option include-timestamp-and-address")
    out = sw2.libs.vtysh.ping_repetitions('10.0.30.1 '
                                          'ip-option '
                                          'include-timestamp-and-address', 1)

    assert 'TS:' and out["packet_loss"] == 0


def ping6_basic(sw2):
    # Ping6 from switch2 to host1
    print("Test ping6 IPv6-address from switch2 to host1")
    out = sw2.libs.vtysh.ping6_repetitions('1030::1', 1)

    assert out["packet_loss"] == 0


def ping6_with_datafill_option(sw2):
    # Ping6 from switch2 to host1 with the data-fill parameter
    print("Test ping6 IPv6-address from switch2 to host1"
          " with the data-fill parameter")
    out = sw2.libs.vtysh.ping6_repetitions('1030::1 data-fill dee', 1)

    assert out["packet_loss"] == 0


def ping6_with_datagram_size_option(sw2):
    # Ping6 from switch2 to host1 with the datagram-size parameter
    print("Test ping6 IPv6-address from switch2 to host1"
          " with the datagram-size parameter")
    out = sw2.libs.vtysh.ping6_repetitions('1030::1 datagram-size 200', 1)

    assert 'PING 1030::1(1030::1) 200 data bytes' \
        and out["packet_loss"] == 0


def ping6_with_interval_option(sw2):
    # Ping6 from switch2 to host1 with the interval parameter
    print("Test ping6 IPv6-address from switch2 to host1"
          " with the interval parameter")
    out = sw2.libs.vtysh.ping6_repetitions('1030::1 interval 2', 1)

    assert out["packet_loss"] == 0


def ping6_with_repetition_option(sw2):
    # Ping6 from switch2 to host1 with the repetition parameter
    print("Test ping6 IPv6-address from switch1 to switch1"
          " with the repetitions parameter")
    out = sw2.libs.vtysh.ping6_repetitions('1030::1', 2)

    assert out["packet_loss"] == 0


def ping_network_unreachable(sw2):
    # Ping network unreachable case
    print("Test ping network unreachable case")
    out = sw2("ping 10.11.1.1")

    assert "Network is unreachable" in out


def ping_destination_unreachable(sw2):
    # Ping destination unreachable case
    print("Test ping destination host unreachable case")
    out = sw2('ping 10.0.10.5')

    assert "Destination Host Unreachable" in out


def ping_unknown_host(sw2):
    # Ping unknown host
    print("Test ping unknown host case")
    out = sw2('ping asdfrgqweewwe')

    assert "unknown host" in out


def ping6_network_unreachable(sw2):
    # Ping6 network unreachable case
    print("Test ping6 network unreachable case")
    out = sw2("ping6 1050::1")

    assert "Network is unreachable" in out


def ping6_unknown_host(sw2):
    # Ping6 unknown host
    print("Test ping unknown host case")
    out = sw2('ping6 asdfrgqweewwe')

    assert "unknown host" in out


def cleanup(sw1, sw2, h1):
    print("\nPerforming cleanup")
    print("unconfiguring static ipv4 route on host1")
    h1("ip -4 route del 10.0.10.0/24 via 10.0.30.2")

    print("unconfiguring static ipv6 route on host1")
    h1("ip -6 route del 1010::0/120 via 1030::2")

    print("Unconfiguring host1 with IPv4 address")
    h1.libs.ip.remove_ip('if01', '10.0.30.1/24')

    print("unConfiguring host1 with IPv6 address")
    h1.libs.ip.remove_ip('if01', '1030::1/120')

    # SW1
    print("Unconfiguring IPv4 address on link 1 SW1")
    with sw1.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.no_ip_address('10.0.30.2/24')

    print("Unconfiguring IPv6 address on link 1 SW1")
    with sw1.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.no_ipv6_address('1030::2/120')

    print("Unconfiguring IPv4 address on link 2 SW1")
    with sw1.libs.vtysh.ConfigInterface('if02') as ctx:
        ctx.no_ip_address('10.0.10.2/24')

    print("unConfiguring IPv6 address on link 2 SW1")
    with sw1.libs.vtysh.ConfigInterface('if02') as ctx:
        ctx.no_ipv6_address('1010::2/120')

    print("Disabling interface1 on SW1")
    with sw1.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.shutdown()

    print("Disabling interface2 on SW1")
    with sw1.libs.vtysh.ConfigInterface('if02') as ctx:
        ctx.shutdown()

    # SW2
    print("Unconfiguring IPv4 address on link 1 SW2")
    with sw2.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.no_ip_address('10.0.10.3/24')

    print("Unconfiguring IPv6 address on link 1 SW2")
    with sw2.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.no_ipv6_address('1010::3/120')

    # unconfiguring static routes
    with sw2.libs.vtysh.Configure() as ctx:
        ctx.no_ip_route("10.0.30.0/24", "10.0.10.2")
        ctx.no_ipv6_route("1030::/120", "1010::2")

    print("Disabling interface1 on SW2")
    with sw2.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.shutdown()


@mark.gate
def test_ft_utilities_ping(topology, step):
    sw1 = topology.get('sw1')
    sw2 = topology.get('sw2')
    h1 = topology.get('h1')

    assert sw1 is not None
    assert sw2 is not None
    assert h1 is not None

    # Get Device objects
    configure(sw1, sw2, h1)

    if (check_interface_status(sw1, sw2) is False):
        step("\n Since Interface state is down ping "
             "test cases are not executed")
        cleanup(sw1, sw2, h1)
        return True

    step("### Basic ping tests ###")
    ping_basic(sw2)

    step("\n### Ping with options tests ###")
    ping_with_datafill_option(sw2)
    ping_with_datagram_size_option(sw2)
    ping_with_interval_option(sw2)
    ping_with_timeout_option(sw2)
    ping_with_repetition_option(sw2)
    ping_with_tos_option(sw2)

    step("\n### Ping with extended options tests ###")
    ping_with_recordroute_option(sw2)
    ping_with_timestamp_option(sw2)
    ping_with_timestamp_and_address_option(sw2)

    print("\n### Basic ping6 tests ###")
    ping6_basic(sw2)

    print("\n### Ping6 test with options ###")
    ping6_with_datafill_option(sw2)
    ping6_with_datagram_size_option(sw2)
    ping6_with_interval_option(sw2)
    ping6_with_repetition_option(sw2)

    print("\n### Ping failure test cases ###")
    ping_network_unreachable(sw2)
    ping_destination_unreachable(sw2)
    ping_unknown_host(sw2)

    print("\n### Ping6 failure test cases ###")
    ping6_network_unreachable(sw2)
    ping6_unknown_host(sw2)

    cleanup(sw1, sw2, h1)
