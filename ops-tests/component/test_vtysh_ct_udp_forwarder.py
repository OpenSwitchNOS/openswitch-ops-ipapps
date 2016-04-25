# Copyright (C) 2015 Hewlett Packard Enterprise Development LP
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


def udp_bcast_forwarder_enable(sw1):
    sw1('configure terminal')
    sw1("ip udp-bcast-forward")
    sw1('end')

    out = sw1("show ip forward-protocol")
    assert re.search("(UDP B\w+ Forwarder : enabled)", out) is not None
    return True


def udp_bcast_forwarder_disable(sw1):
    sw1('configure terminal')
    sw1("no ip udp-bcast-forward")
    sw1("end")

    out = sw1("show ip forward-protocol")
    assert re.search("(UDP B\w+ Forwarder : disabled)", out) is not None
    return True


def show_udp_bcast_forwarder(sw1):
    sw1('configure terminal')
    # Enable the UDP Bcast Forwarder and then verify the show command.
    sw1('ip udp-bcast-forward')
    sw1("end")

    out = sw1("show ip forward-protocol")
    assert 'enabled' in out
    return True


def udp_bcast_forwarder_running_config_test(sw1):
    sw1('configure terminal')
    sw1("ip udp-bcast-forward")
    sw1('end')

    out = sw1("show running-config")
    assert 'ip udp-bcast-forward' in out
    return True


def forward_protocol_invalid_params(sw1):
    sw1('configure terminal')
    interface = 17
    sw1('interface %s' % interface)

    server_ip = "15.15.15.15"
    # Invalid UDP Port number
    u_port = 81
    cmd = "ip forward-protocol udp "+server_ip+" "+str(u_port)

    out = sw1(cmd)
    assert 'Invalid UDP portname/portnumber entered.' in out

    # Invalid UDP Protocol name
    u_protocol = "invalidUDP"
    cmd = "ip forward-protocol udp "+server_ip+" "+u_protocol

    out = sw1(cmd)
    assert 'Invalid UDP portname/portnumber entered.' in out
    sw1("end")
    return True


def forward_protocol_unicast_server_ip(sw1):
    # Enter the Configure terminal and configure the forward-protocol
    # on an interface.
    sw1('configure terminal')
    sw1('interface 1')

    cmd = "ip forward-protocol udp 1.1.1.1 53"
    sw1(cmd)
    sw1("end")

    out = sw1("show ip forward-protocol")
    assert '  1.1.1.1                53' in out
    return True


def forward_protocol_unconfigure_unicast_server_ip(sw1):
    # Enter the Configure terminal and unconfigure the forward-protocol
    # on an interface.
    sw1('configure terminal')
    sw1('interface 1')

    cmd = "no ip forward-protocol udp 1.1.1.1 53"
    sw1(cmd)
    sw1("end")

    out = sw1("show ip forward-protocol")
    assert '  1.1.1.1                53' not in out
    return True


def forward_protocol_configure_bcast_fwd_disable(sw1):
    # Disable the UDP Bcast Forwarder
    udp_bcast_forwarder_disable(sw1)

    cmd = "ip forward-protocol udp 1.1.1.5 69"
    sw1('configure terminal')
    sw1('interface 1')

    sw1(cmd)
    sw1("end")

    out = sw1("show ip forward-protocol")
    assert '  1.1.1.5                69' in out

    # Re-enable the UDP Bcast Forwarder
    udp_bcast_forwarder_enable(sw1)

    return True


def forward_protocol_subnet_broadcast_server_ip(sw1):
    sw1('configure terminal')
    interface = 2
    sw1('interface %s' % interface)

    cmd = "ip forward-protocol udp 10.0.0.255 69"
    sw1(cmd)
    sw1("end")

    out = sw1("show ip forward-protocol")
    assert '  10.0.0.255                69' in out
    return True


def forward_protocol_configure_sub_interface(sw1):
    sw1('configure terminal')
    interface = 1.1  # sub-interface
    sw1('interface %s' % interface)

    cmd = "ip forward-protocol udp 10.0.1.1 123"
    sw1(cmd)
    sw1("end")

    out = sw1("show ip forward-protocol")
    assert '  10.0.1.1                123' in out
    return True


def forward_protocol_configure_split_interface(sw1):
    sw1('configure terminal')
    interface = 54-4  # split-interface
    sw1('interface %s' % interface)

    cmd = "ip forward-protocol udp 10.0.1.11 137"
    sw1(cmd)
    sw1("end")

    out = sw1("show ip forward-protocol")
    assert '  10.0.1.11                137' in out
    return True


def forward_protocol_configure_multiple_test01(sw1):
    # Test to configure multiple server IPs for multiple UDP
    # ports but on single interface
    sw1('configure terminal')
    interface = 2
    sw1('interface %s' % interface)

    cmd1 = "ip forward-protocol udp 4.4.4.1 53"
    cmd2 = "ip forward-protocol udp 4.4.4.2 69"
    cmd3 = "ip forward-protocol udp 4.4.4.4 161"
    cmd4 = "ip forward-protocol udp 4.4.4.9 123"

    sw1(cmd1)
    sw1(cmd2)
    sw1(cmd3)
    sw1(cmd4)
    sw1("end")

    show_cmd = "show ip forward-protocol interface "+str(interface)
    out = sw1(show_cmd)
    assert '4.4.4.9                123' and \
           '4.4.4.4                161' and \
           '4.4.4.1                53' and \
           '4.4.4.2                69'  \
           in out
    return True


def forward_protocol_configure_multiple_test02(sw1):
    # Test to configure same server IP and same UDP port
    # but on multiple interfaces
    sw1('configure terminal')
    interface = 5
    sw1('interface %s' % interface)

    cmd1 = "ip forward-protocol udp 11.11.11.11 69"
    sw1(cmd1)
    sw1("end")

    cmd2 = "show ip forward-protocol interface "+str(interface)
    out = sw1(cmd2)
    assert 'Interface: 5' and \
           '11.11.11.11                69' in out

    sw1('configure terminal')
    interface = 6
    sw1('interface %s' % interface)

    sw1(cmd1)
    sw1("end")

    cmd2 = "show ip forward-protocol interface "+str(interface)
    out = sw1(cmd2)
    assert 'Interface: 6' and \
           '11.11.11.11                69' in out

    sw1('configure terminal')
    interface = 7
    sw1('interface %s' % interface)

    sw1(cmd1)
    sw1("end")

    cmd2 = "show ip forward-protocol interface "+str(interface)
    out = sw1(cmd2)
    assert 'Interface: 7' and \
           '11.11.11.11                69' in out
    return True


def forward_protocol_configure_multiple_test03(sw1):
    # Test to configure multiple server IP but on same UDP port
    # and on single interface
    sw1('configure terminal')
    interface = 8
    sw1('interface %s' % interface)

    cmd1 = "ip forward-protocol udp 11.11.11.11 161"
    cmd2 = "ip forward-protocol udp 11.11.11.22 161"
    cmd3 = "ip forward-protocol udp 11.11.11.33 161"

    sw1(cmd1)
    sw1(cmd2)
    sw1(cmd3)
    sw1("end")

    cmd4 = "show ip forward-protocol interface "+str(interface)
    out = sw1(cmd4)
    assert 'Interface: 8' and \
           '11.11.11.11                161' and \
           '11.11.11.22                161' and \
           '11.11.11.33                161' \
           in out
    return True


def forward_protocol_configure_multiple_test04(sw1):
    # Test to configure same server IP but multiple UDP ports
    # on single interface
    sw1('configure terminal')
    interface = 9
    sw1('interface %s' % interface)

    cmd1 = "ip forward-protocol udp 11.11.0.11 123"
    cmd2 = "ip forward-protocol udp 11.11.0.11 161"
    cmd3 = "ip forward-protocol udp 11.11.0.11 137"

    sw1(cmd1)
    sw1(cmd2)
    sw1(cmd3)
    sw1("end")

    cmd4 = "show ip forward-protocol interface "+str(interface)
    out = sw1(cmd4)
    assert 'Interface: 9' and \
           '11.11.0.11                123' and \
           '11.11.0.11                161' and \
           '11.11.0.11                137' \
           in out
    return True


def forward_protocol_configure_multiple_test05(sw1):
    # Test to configure same server IP but multiple UDP ports
    # on multiple interfaces
    sw1('configure terminal')
    interface = 10
    sw1('interface %s' % interface)

    cmd1 = "ip forward-protocol udp 8.8.8.8 69"
    cmd2 = "ip forward-protocol udp 8.8.8.8 123"
    cmd3 = "ip forward-protocol udp 8.8.8.8 137"

    sw1(cmd1)
    sw1('end')

    sw1('configure terminal')
    interface = 11
    sw1('interface %s' % interface)

    sw1(cmd2)
    sw1('end')

    sw1('configure terminal')
    interface = 12
    sw1('interface %s' % interface)

    sw1(cmd3)
    sw1('end')

    out = sw1("show ip forward-protocol")
    assert '  8.8.8.8                69' and \
           '  8.8.8.8                123' and \
           '  8.8.8.8                137' \
           in out
    return True


def forward_protocol_unconfigure_non_exist(sw1):
    sw1('configure terminal')
    sw1('interface 1')

    cmd = "no ip forward-protocol udp 1.1.1.1 53"
    out = sw1(cmd)

    assert 'UDP forward-protocol is not present.' in out
    sw1('end')
    return True


def forward_protocol_configure_multicast_server_ip1(sw1):
    sw1('configure terminal')
    sw1('interface 1')

    cmd = "ip forward-protocol udp 244.0.0.0 123"
    out = sw1(cmd)

    assert 'Broadcast, multicast and loopback addresses are not allowed.' \
           in out
    sw1('end')
    return True


def forward_protocol_configure_broadcast_server_ip(sw1):
    sw1('configure terminal')
    sw1('interface 1')

    cmd = "ip forward-protocol udp 255.255.255.255 dns"
    out = sw1(cmd)

    assert 'Broadcast, multicast and loopback addresses are not allowed.' \
           in out
    sw1('end')
    return True


def forward_protocol_configure_loopback_server_ip(sw1):
    sw1('configure terminal')
    sw1('interface 1')

    cmd = "ip forward-protocol udp 127.0.0.0 dns"
    out = sw1(cmd)

    assert 'Broadcast, multicast and loopback addresses are not allowed.' \
           in out
    sw1('end')
    return True


def forward_protocol_configure_multicast_server_ip2(sw1):
    sw1('configure terminal')
    sw1('interface 1')

    cmd = "ip forward-protocol udp 244.0.0.0 dns"
    out = sw1(cmd)

    assert 'Broadcast, multicast and loopback addresses are not allowed.' \
           in out
    sw1('end')
    return True


def forward_protocol_configure_invalid_server_ip(sw1):
    sw1('configure terminal')
    sw1('interface 1')

    cmd = "ip forward-protocol udp 0.0.0.0 snmp"
    out = sw1(cmd)

    assert 'Broadcast, multicast and loopback addresses are not allowed.' \
           in out
    sw1('end')
    return True


def forward_protocol_configure_duplication(sw1):
    sw1('configure terminal')
    sw1('interface 1')

    cmd1 = "ip forward-protocol udp 4.4.4.1 53"
    cmd2 = "ip forward-protocol udp 4.4.4.1 53"

    sw1(cmd1)
    out = sw1(cmd2)

    assert 'This entry already exists.' in out
    sw1('end')
    return True


def forward_protocol_maximum_entry_per_port(sw1):
    sw1('configure terminal')
    interface = 3
    sw1('interface %s' % interface)

    cmd1 = "ip forward-protocol udp 5.5.5.1 123"
    cmd2 = "ip forward-protocol udp 5.5.5.2 123"
    cmd3 = "ip forward-protocol udp 5.5.5.3 123"
    cmd4 = "ip forward-protocol udp 5.5.5.4 123"
    cmd5 = "ip forward-protocol udp 5.5.5.5 123"
    cmd6 = "ip forward-protocol udp 5.5.5.6 123"
    cmd7 = "ip forward-protocol udp 5.5.5.7 123"
    cmd8 = "ip forward-protocol udp 5.5.5.8 123"
    cmd9 = "ip forward-protocol udp 5.5.5.9 123"
    cmd10 = "ip forward-protocol udp 5.5.5.10 123"
    cmd11 = "ip forward-protocol udp 5.5.5.11 123"
    cmd12 = "ip forward-protocol udp 5.5.5.12 123"
    cmd13 = "ip forward-protocol udp 5.5.5.13 123"
    cmd14 = "ip forward-protocol udp 5.5.5.14 123"
    cmd15 = "ip forward-protocol udp 5.5.5.15 123"
    cmd16 = "ip forward-protocol udp 5.5.5.16 123"
    cmd17 = "ip forward-protocol udp 5.5.5.17 123"

    out = sw1(cmd1)
    out = sw1(cmd2)
    out = sw1(cmd3)
    out = sw1(cmd4)
    out = sw1(cmd5)
    out = sw1(cmd6)
    out = sw1(cmd7)
    out = sw1(cmd8)
    out = sw1(cmd9)
    out = sw1(cmd10)
    out = sw1(cmd11)
    out = sw1(cmd12)
    out = sw1(cmd13)
    out = sw1(cmd14)
    out = sw1(cmd15)
    out = sw1(cmd16)
    out = sw1(cmd17)

    assert 'Command failed' in out
    sw1('end')
    return True


def forward_protocol_in_running_config(sw1):
    sw1('configure terminal')
    interface = 19
    sw1('interface %s' % interface)

    cmd1 = "ip forward-protocol udp 3.3.3.1 53"
    cmd2 = "ip forward-protocol udp 3.3.3.2 69"
    sw1(cmd1)
    sw1(cmd2)
    sw1('end')

    out = sw1("show running-config")
    assert 'ip forward-protocol udp 3.3.3.1 53' and \
           'ip forward-protocol udp 3.3.3.2 69' in out
    return True


def udp_forwarder_complete_post_reboot(sw1):
    # Enable UDP Bcast Forwarding
    udp_bcast_forwarder_enable(sw1)
    # Set a forward-protocol
    forward_protocol_unicast_server_ip(sw1)

    # Save the running to start-up config
    run_cfg = "copy running-config startup-config"
    sw1(run_cfg)

    # Verify the configurations are saved in startup configs
    start_cfg = "show startup-config"
    out = sw1(start_cfg)

    assert 'ip forward-protocol udp 1.1.1.1 53' and \
           'ip udp-bcast-forward' in out

    # Perform reboot
    # Check for the configs post reboot
    sleep(30)

    out = sw1("show running-config")
    assert 'ip forward-protocol udp 1.1.1.1 53' and \
           'ip udp-bcast-forward' in out
    return True


def test_udp_bcast_forwarder_configuration(topology):
    sw1 = topology.get('sw1')

    assert sw1 is not None

    # Test to enable UDP Bcast Forwarding
    assert udp_bcast_forwarder_enable(sw1)

    # Test to disable UDP Bcast Forwarding
    assert udp_bcast_forwarder_disable(sw1)

    # Test to show UDP Bcast Forwarding status
    assert show_udp_bcast_forwarder(sw1)

    # Test to show running config for UDP Bcast Forwarding
    assert udp_bcast_forwarder_running_config_test(sw1)

    # Test to configure forward-protocol with invalid params
    assert forward_protocol_invalid_params(sw1)

    # Test to configure forward-protocol Unicast Server IP
    assert forward_protocol_unicast_server_ip(sw1)

    # Test to unconfigure forward-protocol Unicast Server IP
    assert forward_protocol_unconfigure_unicast_server_ip(sw1)

    # Test to configure forward-protocol unicast server IP
    # when UDP Broadcast Forwarding is disabled
    assert forward_protocol_configure_bcast_fwd_disable(sw1)

    # Test to configure forward-protocol on a sub-interface
    assert forward_protocol_configure_sub_interface(sw1)

    # Test to configure forward-protocol on a split-interface
    assert forward_protocol_configure_split_interface(sw1)

    # Test to configure forward-protocol subnet broadcast server IP
    assert forward_protocol_subnet_broadcast_server_ip(sw1)

    # Test to configure multiple server IPs for multiple UDP
    # ports but on single interface
    assert forward_protocol_configure_multiple_test01(sw1)

    # Test to configure same server IP and same UDP port
    # but on multiple interfaces
    assert forward_protocol_configure_multiple_test02(sw1)

    # Test to configure multiple server IP but on same UDP port
    # and on single interface
    assert forward_protocol_configure_multiple_test03(sw1)

    # Test to configure same server IP but multiple UDP ports
    # on single interface
    assert forward_protocol_configure_multiple_test04(sw1)

    # Test to configure same server IP but multiple UDP ports
    # on multiple interfaces
    assert forward_protocol_configure_multiple_test05(sw1)

    # Test to unconfigure a non-existing forward-protocol server IP
    assert forward_protocol_unconfigure_non_exist(sw1)

    # Test to configure a Multicast server IP
    assert forward_protocol_configure_multicast_server_ip1(sw1)

    # Test to configure an anynet Broadcast server IP
    assert forward_protocol_configure_broadcast_server_ip(sw1)

    # Test to configure a Loopback server IP
    assert forward_protocol_configure_loopback_server_ip(sw1)

    # Test to configure a Multicast server IP
    assert forward_protocol_configure_multicast_server_ip2(sw1)

    # Test to configure an invalid server IP
    assert forward_protocol_configure_invalid_server_ip(sw1)

    # Test to configure duplicate server IP
    assert forward_protocol_configure_duplication(sw1)

    # Test to verify the UDP Broadcast Forwarder details in
    # show running
    assert forward_protocol_in_running_config(sw1)
