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

TOPOLOGY = """
#
# +-------+
# |  sw1  |
# +-------+
#

# Nodes
[type=openswitch name="Switch 1"] sw1
"""


def udp_bcast_forwarder_enable(sw1):
    sw1("configure terminal")
    cmd = "ip udp-bcast-forward"
    sw1(cmd)
    sw1("end")

    out = "ovs-appctl -t ops-udpfwd udpfwd/dump"
    output = sw1(out, shell="bash")
    assert 'UDP Bcast Forwarder : 1' in output
    return True


def udp_bcast_forwarder_disable(sw1):
    sw1("configure terminal")
    cmd = "no ip udp-bcast-forward"
    sw1(cmd)
    sw1("end")

    output = sw1("ovs-appctl -t ops-udpfwd udpfwd/dump", shell="bash")
    assert 'UDP Bcast Forwarder : 0' in output
    return True


def udp_forward_protocol_test01(sw1):
    sw1("configure terminal")
    sw1("interface 1")
    cmd = "ip forward-protocol udp 1.1.1.1 dns"
    sw1(cmd)
    sw1("end")

    out = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 1"
    output = sw1(out, shell="bash")
    assert '1.1.1.1' in output
    return True


def udp_forward_protocol_unconfigure(sw1):
    sw1("configure terminal")
    # Configure
    sw1("interface 20")
    cmd = "ip forward-protocol udp 1.1.1.1 rip"
    sw1(cmd)
    sw1("end")

    out = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 20"
    output = sw1(out, shell="bash")
    assert '1.1.1.1' in output

    # Unconfigure
    sw1("configure terminal")
    sw1("interface 20")
    cmd = "no ip forward-protocol udp 1.1.1.1 rip"
    sw1(cmd)
    sw1("end")

    out = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 20"
    output = sw1(out, shell="bash")
    assert '1.1.1.1' not in output
    return True


# Verify UDP forward-protocol configuration for multiple
# UDP ports on a specific interface
def udp_forward_protocol_test02(sw1):
    sw1("configure terminal")
    sw1("interface 2")
    cmd1 = "ip forward-protocol udp 1.1.1.2 53"
    sw1(cmd1)
    cmd2 = "ip forward-protocol udp 1.1.1.2 137"
    sw1(cmd2)
    cmd3 = "ip forward-protocol udp 1.1.1.2 69"
    sw1(cmd3)
    sw1("end")

    out = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 2"
    ret_buffer = sw1(out, shell="bash")
    assert '1.1.1.2' in ret_buffer and '53' in ret_buffer \
        and '69' in ret_buffer and '137' in ret_buffer
    return True


# Verify UDP forward-protocol configurations for multiple
# server IP addresses for a single UDP port on an interface
def udp_forward_protocol_test03(sw1):
    sw1("configure terminal")
    sw1("interface 3")
    cmd1 = "ip forward-protocol udp 1.1.1.3 123"
    sw1(cmd1)
    cmd2 = "ip forward-protocol udp 1.1.1.4 123"
    sw1(cmd2)
    cmd3 = "ip forward-protocol udp 1.1.1.5 123"
    sw1(cmd3)
    sw1("end")

    out = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 3"
    ret_buffer = sw1(out, shell="bash")
    assert '123' in ret_buffer and '1.1.1.3' in ret_buffer \
        and '1.1.1.4' in ret_buffer and '1.1.1.5' in ret_buffer
    return True


# Verify UDP forward-protocol configuration for
# subnet broadcast IP address on an interface
def udp_forward_protocol_test04(sw1):
    sw1("configure terminal")
    sw1("interface 4")
    cmd1 = "ip forward-protocol udp 1.1.1.255 161"
    sw1(cmd1)
    sw1("end")

    out = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 4"
    ret_buffer = sw1(out, shell="bash")
    assert '161' in ret_buffer and '1.1.1.255' in ret_buffer
    return True


# Verify UDP forward-protocol configuration on same subnet
def udp_forward_protocol_test05(sw1):
    sw1("configure terminal")
    sw1("interface 5")
    cmd1 = "ip address 2.2.2.2/24"
    sw1(cmd1)
    cmd2 = "ip forward-protocol udp 2.2.2.255 161"
    sw1(cmd2)
    sw1("end")

    out = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 5"
    ret_buffer = sw1(out, shell="bash")
    assert '161' in ret_buffer and '2.2.2.255' in ret_buffer
    return True


# Verify single UDP forward-protocol configuration
# on multiple interfaces
def udp_forward_protocol_test06(sw1):
    sw1("configure terminal")
    sw1("interface 6")
    cmd2 = "ip forward-protocol udp 6.6.6.6 53"
    sw1(cmd2)

    sw1("interface 7")
    sw1(cmd2)

    sw1("interface 8")
    sw1(cmd2)
    sw1("end")

    out = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 6"
    ret_buffer = sw1(out, shell="bash")
    assert '53' in ret_buffer and '6.6.6.6' in ret_buffer

    out = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 7"
    ret_buffer = sw1(out, shell="bash")
    assert '53' in ret_buffer and '6.6.6.6' in ret_buffer

    out = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 8"
    ret_buffer = sw1(out, shell="bash")
    assert '53' in ret_buffer and '6.6.6.6' in ret_buffer
    return True


# Verify maximum UDP forward-protocol configurations per interface
def udp_forward_protocol_max_per_interface(sw1):
    sw1("configure terminal")
    sw1("interface 9")

    cmd1 = "ip forward-protocol udp 5.5.5.1 123"
    cmd2 = "ip forward-protocol udp 5.5.5.2 123"
    cmd3 = "ip forward-protocol udp 5.5.5.3 123"
    cmd4 = "ip forward-protocol udp 5.5.5.4 123"
    cmd5 = "ip forward-protocol udp 5.5.5.5 123"
    cmd6 = "ip forward-protocol udp 5.5.5.6 123"
    cmd7 = "ip forward-protocol udp 5.5.5.7 123"
    cmd8 = "ip forward-protocol udp 5.5.5.8 123"
    cmd9 = "ip forward-protocol udp 5.5.5.9 123"

    sw1(cmd1)
    sw1(cmd2)
    sw1(cmd3)
    sw1(cmd4)
    sw1(cmd5)
    sw1(cmd6)
    sw1(cmd7)
    sw1(cmd8)
    sw1(cmd9)

    sw1("end")

    out = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 9"
    ret_buffer = sw1(out, shell="bash")
    assert '5.5.5.1' in ret_buffer and '5.5.5.2' in ret_buffer \
           and '5.5.5.3' in ret_buffer and '5.5.5.4' in ret_buffer \
           and '5.5.5.5' in ret_buffer and '5.5.5.6' in ret_buffer \
           and '5.5.5.7' in ret_buffer and '5.5.5.8' in ret_buffer
    return True


# Verify number of servers configured per interface
def udp_forward_protocol_server_entry01(sw1):
    sw1("configure terminal")
    sw1("interface 11")
    cmd = "ip forward-protocol udp 1.1.1.1 dns"
    sw1(cmd)

    cmd = "ip forward-protocol udp 1.1.1.1 ntp"
    sw1(cmd)

    cmd = "ip forward-protocol udp 1.1.1.1 161"
    sw1(cmd)
    sw1("end")

    out = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 11"
    output = sw1(out, shell="bash")
    assert 'Interface 11: 3' in output
    return True


# Verify individual number of servers configured per UDP port
# on an interface
def udp_forward_protocol_server_entry02(sw1):
    sw1("configure terminal")
    sw1("interface 12")
    cmd = "ip forward-protocol udp 191.1.1.1 dns"
    sw1(cmd)

    cmd = "ip forward-protocol udp 191.1.1.1 ntp"
    sw1(cmd)

    cmd = "ip forward-protocol udp 191.1.1.1 161"
    sw1(cmd)

    cmd = "ip forward-protocol udp 191.1.1.1 timep"
    sw1(cmd)

    cmd = "ip forward-protocol udp 191.1.1.1 tftp"
    sw1(cmd)

    cmd = "ip forward-protocol udp 191.1.1.1 162"
    sw1(cmd)
    sw1("end")

    # Check output
    count = 0
    out = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 12"
    output = sw1(out, shell="bash")
    lines = output.split('\n')
    for line in lines:
        if "Interface" in line:
            count_on_int = line.split(':')
            total = int(count_on_int[1])
        if "191.1.1.1" in line:
            count_per_port = line.split(',')
            count += int(count_per_port[1])

    assert total == count
    return True


# Verify the configuration of dhcp-relay helper address and
# UDP forward-protocol server addresses together
def udp_forward_protocol_server_entry03(sw1):
    sw1("configure terminal")
    sw1("interface 13")
    cmd = "ip forward-protocol udp 12.12.12.12 dns"
    sw1(cmd)

    cmd = "ip forward-protocol udp 1.1.1.12 ntp"
    sw1(cmd)

    cmd = "ip forward-protocol udp 1.1.12.1 snmp"
    sw1(cmd)

    cmd = "ip helper-address 12.1.1.1"
    sw1(cmd)
    sw1("end")

    out = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 13"
    output = sw1(out, shell="bash")
    assert 'Interface 13: 4' in output \
           and 'Port 67 - 12.1.1.1' in output \
           and 'Port 53 - 12.12.12.12' in output \
           and 'Port 123 - 1.1.1.12' in output \
           and 'Port 161 - 1.1.12.1' in output
    return True


# Verify UDP forward-protocol configuration post reboot
def udp_forward_protocol_reboot(sw1):
    sw1("configure terminal")
    sw1("interface 10")
    cmd1 = "ip forward-protocol udp 10.10.10.10 69"
    sw1(cmd1)
    sw1("end")

    # Before Reboot.
    out = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 10"
    ret_buffer = sw1(out, shell="bash")
    assert '10.10.10.10' in ret_buffer

    # Check for the configs post reboot
    sleep(30)

    # Post reboot
    out = "ovs-appctl -t ops-udpfwd udpfwd/dump interface 10"
    ret_buffer = sw1(out, shell="bash")
    assert '10.10.10.10' in ret_buffer
    return True


def test_ipapps_udp_bcast_forwarder_configuration(topology, step):
    sw1 = topology.get('sw1')

    assert sw1 is not None

    udp_bcast_forwarder_enable(sw1)

    udp_bcast_forwarder_disable(sw1)

    step("Verify UDP forward-protocol configuration on a specific interface")
    udp_forward_protocol_test01(sw1)

    step("Verify UDP forward-protocol unconfiguration on a specific interface")
    udp_forward_protocol_unconfigure(sw1)

    step("Verify UDP forward-protocol configuration for multiple")
    step("UDP ports on a specific interface")
    udp_forward_protocol_test02(sw1)

    step("Verify UDP forward-protocol configurations for multiple")
    step("server IP addresses for a single UDP port on an interface")
    udp_forward_protocol_test03(sw1)

    step("Verify UDP forward-protocol configuration for")
    step("subnet broadcast IP address on an interface")
    udp_forward_protocol_test04(sw1)

    step("Verify UDP forward-protocol configuration on same subnet")
    udp_forward_protocol_test05(sw1)

    step("Verify single UDP forward-protocol configuration")
    step("on multiple interfaces")
    udp_forward_protocol_test06(sw1)

    step("Verify maximum UDP forward-protocol configurations per interface")
    udp_forward_protocol_max_per_interface(sw1)

    step("Verify number of servers configured per interface")
    udp_forward_protocol_server_entry01(sw1)

    step("Verify individual number of servers configured per UDP port")
    step("on an interface")
    udp_forward_protocol_server_entry02(sw1)

    step("Verify the configuration of dhcp-relay helper address and")
    step("UDP forward-protocol server addresses together")
    udp_forward_protocol_server_entry03(sw1)
