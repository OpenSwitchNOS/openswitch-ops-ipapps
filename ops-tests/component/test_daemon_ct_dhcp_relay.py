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


def dhcp_relay_enable(sw1):
    sw1("configure terminal")
    print("Test to enable DHCP relay")
    sw1("dhcp-relay")
    sw1("end")

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump", shell="bash")
    assert 'DHCP Relay : 1' in output


def dhcp_relay_disable(sw1):
    sw1("configure terminal")
    print("Test to disable DHCP relay")
    sw1("no dhcp-relay")
    sw1("end")

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump", shell="bash")
    assert 'DHCP Relay : 0' in output


def dhcp_relay_hop_count_increment_enable(sw1):
    sw1("configure terminal")
    print("Test to enable DHCP relay hop count increment")
    sw1("dhcp-relay hop-count-increment")
    sw1("end")

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump", shell="bash")
    assert 'DHCP Relay hop-count-increment : 1' in output


def dhcp_relay_hop_count_increment_disable(sw1):
    sw1("configure terminal")
    print("Test to disable DHCP relay hop count increment")
    sw1("no dhcp-relay hop-count-increment")
    sw1("end")

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump", shell="bash")
    assert 'DHCP Relay hop-count-increment : 0' in output


def dhcp_relay_option_82_enable(sw1):
    sw1("configure terminal")
    print("Test to enable DHCP relay option 82")
    sw1("dhcp-relay option 82 replace")
    sw1("end")

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump", shell="bash")
    assert 'DHCP Relay Option82 : 1' in output


def dhcp_relay_option_82_disable(sw1):
    sw1("configure terminal")
    print("Test to disable DHCP relay option 82")
    sw1("no dhcp-relay option 82")
    sw1("end")

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump", shell="bash")
    assert 'DHCP Relay Option82 : 0' in output


def dhcp_relay_option_82_validation_enable(sw1):
    sw1("configure terminal")
    print("Test to enable DHCP relay option 82 response validation")
    sw1("dhcp-relay option 82 validate")
    sw1("end")

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump", shell="bash")
    assert 'DHCP Relay Option82 validate : 1' in output


def dhcp_relay_option_82_validation_disable(sw1):
    sw1("configure terminal")
    print("Test to disable DHCP relay option 82 response validation")
    sw1("no dhcp-relay option 82 validate")
    sw1("end")

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump", shell="bash")
    assert 'DHCP Relay Option82 validate : 0' in output


def dhcp_relay_option_82_drop_policy_enable(sw1):
    sw1("configure terminal")
    print("Test to enable DHCP relay option 82 drop policy")
    sw1("dhcp-relay option 82 drop")
    sw1("end")

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump", shell="bash")
    assert 'DHCP Relay Option82 policy : drop' in output


def dhcp_relay_option_82_drop_policy_disable(sw1):
    sw1("configure terminal")
    print("Test to disable DHCP relay option 82 drop policy")
    sw1("no dhcp-relay option 82")
    sw1("end")

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump", shell="bash")
    assert 'DHCP Relay Option82 policy : drop' not in output


def dhcp_relay_option_82_keep_policy_enable(sw1):
    sw1("configure terminal")
    print("Test to enable DHCP relay option 82 keep policy")
    sw1("dhcp-relay option 82 keep")
    sw1("end")

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump", shell="bash")
    assert 'DHCP Relay Option82 policy : keep' in output


def dhcp_relay_option_82_replace_policy_enable(sw1):
    sw1("configure terminal")
    print("Test to enable DHCP relay option 82 replace policy")
    sw1("dhcp-relay option 82 replace")
    sw1("end")

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump", shell="bash")
    assert 'DHCP Relay Option82 policy : replace' in output


def dhcp_relay_option_82_keep_policy_disable(sw1):
    sw1("configure terminal")
    print("Test to disable DHCP relay option 82 keep policy")
    sw1("no dhcp-relay option 82")
    sw1("end")

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump", shell="bash")
    assert 'DHCP Relay Option82 policy : keep' not in output


def dhcp_relay_option_82_status_when_relay_disabled(sw1):
    sw1("configure terminal")
    print("Test to check status of DHCP relay option 82"
          " when DHCP-Relay is disabled")
    sw1("dhcp-relay option 82 keep")
    sw1("no dhcp-relay")
    sw1("end")

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump", shell="bash")
    assert 'DHCP Relay Option82 : 0' in output


def helper_address_configuration_per_interface(sw1):
    sw1("configure terminal")
    sw1("interface 1")
    sw1("ip helper-address 192.168.10.1")
    sw1("end")

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 1",
                 shell="bash")
    assert '192.168.10.1' and \
        'client request dropped packets = 0' and \
        'client request valid packets = 0' and \
        'server request dropped packets = 0' and \
        'server request valid packets = 0' and \
        'client request dropped packets with option 82 = 0' and \
        'client request valid packets with option 82 = 0' and \
        'server request dropped packets with option 82 = 0' and \
        'server request valid packets with option 82 = 0' in output

    # Remove configuration
    sw1("configure terminal")
    sw1("interface 1")
    sw1("no ip helper-address 192.168.10.1")
    sw1("end")


def maximum_helper_address_configuration_per_interface(sw1):
    sw1("configure terminal")
    sw1("interface 2")

    sw1("ip helper-address 192.168.10.1")
    sw1("ip helper-address 192.168.10.2")
    sw1("ip helper-address 192.168.10.3")
    sw1("ip helper-address 192.168.10.4")
    sw1("ip helper-address 192.168.10.5")
    sw1("ip helper-address 192.168.10.6")
    sw1("ip helper-address 192.168.10.7")
    sw1("ip helper-address 192.168.10.8")

    sw1("end")

    ret_buffer = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 2",
                     shell="bash")
    assert '192.168.10.1' in ret_buffer and '192.168.10.2' in ret_buffer \
        and '192.168.10.3' in ret_buffer and '192.168.10.4' in ret_buffer \
        and '192.168.10.5' in ret_buffer and '192.168.10.6' in ret_buffer \
        and '192.168.10.7' in ret_buffer and '192.168.10.8' in ret_buffer

    # Remove configuration
    sw1("configure terminal")
    sw1("interface 2")
    sw1("no ip helper-address 192.168.10.1")
    sw1("no ip helper-address 192.168.10.2")
    sw1("no ip helper-address 192.168.10.3")
    sw1("no ip helper-address 192.168.10.4")
    sw1("no ip helper-address 192.168.10.5")
    sw1("no ip helper-address 192.168.10.6")
    sw1("no ip helper-address 192.168.10.7")
    sw1("no ip helper-address 192.168.10.8")
    sw1("end")


def same_helper_address_on_multiple_interface(sw1):
    sw1("configure terminal")
    sw1("interface 1")
    sw1("ip helper-address 10.10.10.1")

    sw1("interface 3")
    sw1("ip helper-address 10.10.10.1")

    sw1("interface 4")
    sw1("ip helper-address 10.10.10.1")

    sw1("interface 5")
    sw1("ip helper-address 10.10.10.1")

    sw1("end")

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 5",
                 shell="bash")
    lines = output.split('\n')
    for line in lines:
        if "10.10.10.1" in line:
            out = line.split(',')
            count = int(out[1])

    assert count == 4

    # Remove configuration
    sw1("configure terminal")
    sw1("interface 1")
    sw1("no ip helper-address 10.10.10.1")
    sw1("interface 3")
    sw1("no ip helper-address 10.10.10.1")
    sw1("interface 4")
    sw1("no ip helper-address 10.10.10.1")
    sw1("interface 5")
    sw1("no ip helper-address 10.10.10.1")
    sw1("end")


def configure_bootp_gateway_address(sw1):
    sw1("configure terminal")
    sw1("interface 1")
    sw1("ip address 9.0.0.1/8")
    sw1("ip bootp-gateway 9.0.0.1")
    sw1("end")

    ret_buffer = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 1",
                     shell="bash")
    assert '9.0.0.1' in ret_buffer

    # Remove configuration
    sw1("configure terminal")
    sw1("interface 1")
    sw1("no ip address 9.0.0.1/8")
    sw1("no ip bootp-gateway 9.0.0.1")
    sw1("end")

    # After unconfiguring also check value in daemon
    ret_buffer = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 1",
                     shell="bash")
    assert '9.0.0.1' not in ret_buffer


def add_helper_addresses(sw1):

    # Create IP pool , adding 100 IP addresses
    global ipPool
    ipPool = []

    # 192.168.1.1 to 192.168.1.32
    for i in range(1, 33):
        ip = "192.168.1." + str(i)
        ipPool.append(ip)

    # 10.10.1.1 to 10.10.1.32
    for i in range(1, 33):
        ip = "10.10.1." + str(i)
        ipPool.append(ip)

    # 192.168.1.1 to 192.168.1.16
    for i in range(1, 17):
        ip = "192.168.1." + str(i)
        ipPool.append(ip)

    # 9.10.1.1 to 9.10.1.20
    for i in range(1, 21):
        ip = "9.10.1." + str(i)
        ipPool.append(ip)

    # 100 IP addresses
    print(" number of ip address = %d" % len(ipPool))

    sw1("configure terminal")
    # enter interface 11
    sw1("interface 11")

    for k in range(0, 8):
        cmd = "ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 12
    sw1("interface 12")

    for k in range(8, 16):
        cmd = "ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 13
    sw1("interface 13")

    for k in range(16, 24):
        cmd = "ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 14
    sw1("interface 14")

    for k in range(24, 32):
        cmd = "ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 15
    sw1("interface 15")
    for k in range(32, 40):
        cmd = "ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 16
    sw1("interface 16")

    for k in range(40, 48):
        cmd = "ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 17
    sw1("interface 17")

    for k in range(48, 56):
        cmd = "ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 18
    sw1("interface 18")
    for k in range(56, 64):
        cmd = "ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 19
    sw1("interface 19")
    for k in range(64, 72):
        cmd = "ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 20
    sw1("interface 20")
    for k in range(72, 80):
        cmd = "ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 21
    sw1("interface 21")
    for k in range(80, 88):
        cmd = "ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 22
    sw1("interface 22")
    for k in range(88, 96):
        cmd = "ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 23
    sw1("interface 23")
    for k in range(96, 100):
        cmd = "ip helper-address " + ipPool[k]
        sw1(cmd)

    sw1("end")

    # Check output
    count = 0
    output = sw1("ovs-appctl -t ops-relay udpfwd/dump", shell="bash")
    lines = output.split('\n')
    for line in lines:
        if 'Interface ' in line:
            out = line.split(':')
            count += int(out[1])

    # for debug
    print(" length of ipPool = %d" % len(ipPool))
    print(" value of count = %d" % count)

    if len(ipPool) != count:
        print(" Adding Helper Addresses failed")
        return False


def delete_helper_addresses(sw1):
    sw1("configure terminal")
    # enter interface 11
    sw1("interface 11")

    for k in range(0, 8):
        cmd = " no ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 12
    sw1("interface 12")

    for k in range(8, 16):
        cmd = " no ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 13
    sw1("interface 13")

    for k in range(16, 24):
        cmd = " no ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 14
    sw1("interface 14")

    for k in range(24, 32):
        cmd = " no ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 15
    sw1("interface 15")

    for k in range(32, 40):
        cmd = " no ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 16
    sw1("interface 16")

    for k in range(40, 48):
        cmd = " no ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 17
    sw1("interface 17")

    for k in range(48, 56):
        cmd = " no ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 18
    sw1("interface 18")

    for k in range(56, 64):
        cmd = " no ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 19
    sw1("interface 19")

    for k in range(64, 72):
        cmd = " no ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 20
    sw1("interface 20")

    for k in range(72, 80):
        cmd = " no ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 21
    sw1("interface 21")

    for k in range(80, 88):
        cmd = " no ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 22
    sw1("interface 22")

    for k in range(88, 96):
        cmd = " no ip helper-address " + ipPool[k]
        sw1(cmd)

    # enter interface 23
    sw1("interface 23")

    for k in range(96, 100):
        cmd = " no ip helper-address " + ipPool[k]
        sw1(cmd)

    sw1("end")

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 11",
                 shell="bash")
    assert 'No servers are configured on this interface :11' in output

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 12",
                 shell="bash")
    assert 'No servers are configured on this interface :12' in output

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 13",
                 shell="bash")
    assert 'No servers are configured on this interface :13' in output

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 14",
                 shell="bash")
    assert 'No servers are configured on this interface :14' in output

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 15",
                 shell="bash")
    assert 'No servers are configured on this interface :15' in output

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 16",
                 shell="bash")
    assert 'No servers are configured on this interface :16' in output

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 17",
                 shell="bash")
    assert 'No servers are configured on this interface :17' in output

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 18",
                 shell="bash")
    assert 'No servers are configured on this interface :18' in output

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 19",
                 shell="bash")
    assert 'No servers are configured on this interface :19' in output

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 20",
                 shell="bash")
    assert 'No servers are configured on this interface :20' in output

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 21",
                 shell="bash")
    assert 'No servers are configured on this interface :21' in output

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 22",
                 shell="bash")
    assert 'No servers are configured on this interface :22' in output

    output = sw1("ovs-appctl -t ops-relay udpfwd/dump interface 23",
                 shell="bash")
    assert 'No servers are configured on this interface :23' in output


def test_ipapps_dhcp_relay_configuration(topology, step):
    sw1 = topology.get('sw1')

    assert sw1 is not None

    dhcp_relay_enable(sw1)

    dhcp_relay_disable(sw1)

    dhcp_relay_hop_count_increment_enable(sw1)

    dhcp_relay_hop_count_increment_disable(sw1)

    dhcp_relay_option_82_enable(sw1)

    dhcp_relay_option_82_disable(sw1)

    dhcp_relay_option_82_validation_enable(sw1)

    dhcp_relay_option_82_validation_disable(sw1)

    dhcp_relay_option_82_drop_policy_enable(sw1)

    dhcp_relay_option_82_drop_policy_disable(sw1)

    dhcp_relay_option_82_keep_policy_enable(sw1)

    dhcp_relay_option_82_replace_policy_enable(sw1)

    dhcp_relay_option_82_keep_policy_disable(sw1)

    dhcp_relay_option_82_status_when_relay_disabled(sw1)

    helper_address_configuration_per_interface(sw1)

    maximum_helper_address_configuration_per_interface(sw1)

    same_helper_address_on_multiple_interface(sw1)

    configure_bootp_gateway_address(sw1)

    add_helper_addresses(sw1)

    delete_helper_addresses(sw1)
