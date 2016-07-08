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


def dhcp_relay_enable(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay")

    out = sw1("do show dhcp-relay")
    assert re.search("(DHCP Relay Agent\s+:\s+?enabled)", out) is not None
    sw1("end")
    return True


def dhcp_relay_disable(sw1):
    sw1("configure terminal")
    sw1("no dhcp-relay")

    out = sw1("do show dhcp-relay")
    assert re.search("(DHCP Relay Agent\s*:\s*?disabled)", out) is not None
    sw1("end")
    return True


def dhcp_relay_hop_count_increment_enable(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay hop-count-increment")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert "DHCP Request Hop Count Increment : enabled" in out
    return True


def dhcp_relay_hop_count_increment_disable(sw1):
    sw1("configure terminal")
    sw1("no dhcp-relay hop-count-increment")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert "DHCP Request Hop Count Increment : disabled" in out
    return True


def dhcp_relay_option_82_enable(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 replace")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None
    return True


def dhcp_relay_option_82_validation_enable(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 validate")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Response [V|v]alidation\s*:\s*?enabled)", out) \
        is not None
    return True


def relay_option_82_with_validation_and_replace(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 validate replace")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?enabled)", out) is not None\
        and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?replace)", out)\
        is not None
    return True


def relay_option_82_with_validation_replace_and_mac_rid(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 validate replace mac")
    sw1("end")

    sw1("show dhcp-relay")
    """assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?enabled)", out) is not None\
        and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?replace)", out)\
        is not None and re.search("(Remote ID\s*:\s*?mac)", out) is not None"""
    return True


def relay_option_82_with_validation_replace_and_ip_rid(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 validate replace ip")
    sw1("end")

    sw1("show dhcp-relay")
    """assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?enabled)", out) is not None\
        and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?replace)", out)\
        is not None and re.search("(Remote ID\s*:\s*?ip)", out) is not None"""
    return True


def relay_option_82_with_validation_and_drop(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 validate drop")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?enabled)", out) is not None\
        and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?drop)", out) \
        is not None
    return True


def relay_option_82_with_validation_drop_and_mac_rid(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 validate drop mac")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?enabled)", out) is not None\
        and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?drop)", out) \
        is not None and re.search("(Remote ID\s*:\s*?mac)", out) is not None
    return True


def relay_option_82_with_validation_drop_and_ip_rid(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 validate drop ip")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?enabled)", out) is not None\
        and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?drop)", out) \
        is not None and re.search("(Remote ID\s*:\s*?ip)", out) is not None
    return True


def relay_option_82_with_keep_policy(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 keep")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?disabled)", out) is not \
        None and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?keep)",
                           out) is not None
    return True


def relay_option_82_with_keep_and_mac_rid(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 keep mac")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?disabled)", out) is not \
        None and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?keep)",
                           out) is not None and \
        re.search("(Remote ID\s*:\s*?mac)", out) is not None
    return True


def relay_option_82_with_keep_and_ip_rid(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 keep ip")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?disabled)", out) is not \
        None and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?keep)",
                           out) is not None and \
        re.search("(Remote ID\s*:\s*?ip)", out) is not None
    return True


def relay_option_82_with_replace_policy(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 replace")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?replace)", out) \
        is not None
    return True


def relay_option_82_with_replace_and_mac_rid(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 replace mac")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?replace)", out) \
        is not None and re.search("(Remote ID\s*:\s*?mac)", out) is not None
    return True


def relay_option_82_replace_with_mac_rid_validation(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 replace mac [V|v]alidation")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?enabled)", out) is not None\
        and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?replace)", out)\
        is not None and re.search("(Remote ID\s*:\s*?mac)", out) is not None
    return True


def relay_option_82_with_replace_and_ip_rid(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 replace ip")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?replace)", out) \
        is not None and re.search("(Remote ID\s*:\s*?ip)", out) is not None
    return True


def relay_option_82_replace_with_ip_rid_validation(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 replace ip [V|v]alidation")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?enabled)", out) is not None\
        and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?replace)", out)\
        is not None and re.search("(Remote ID\s*:\s*?ip)", out) is not None
    return True


def relay_option_82_with_replace_and_validation(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 replace validate")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?enabled)", out) is not None\
        and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?replace)", out)\
        is not None
    return True


def relay_option_82_with_replace_validation_and_mac_rid(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 replace validate mac")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?enabled)", out) is not None\
        and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?replace)", out)\
        is not None and re.search("(Remote ID\s*:\s*?mac)", out) is not None
    return True


def relay_option_82_with_replace_validation_and_ip_rid(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 replace validate ip")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?enabled)", out) is not None\
        and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?replace)", out)\
        is not None and re.search("(Remote ID\s*:\s*?ip)", out) is not None
    return True


def relay_option_82_with_drop_policy(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 drop")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?drop)", out) \
        is not None
    return True


def relay_option_82_with_drop_and_mac_rid(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 drop mac")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?drop)", out) \
        is not None and re.search("(Remote ID\s*:\s*?mac)", out) is not None
    return True


def relay_option_82_drop_with_mac_rid_validation(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 drop mac validate")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?enabled)", out) is not None\
        and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?drop)", out) \
        is not None and re.search("(Remote ID\s*:\s*?mac)", out) is not None
    return True


def relay_option_82_with_drop_and_ip_rid(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 drop ip")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?drop)", out) \
        is not None and re.search("(Remote ID\s*:\s*?ip)", out) is not None
    return True


def relay_option_82_drop_with_ip_rid_validation(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 drop ip validate")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?enabled)", out) is not None\
        and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?drop)", out) \
        is not None and re.search("(Remote ID\s*:\s*?ip)", out) is not None
    return True


def relay_option_82_with_drop_and_validation(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 drop validate")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?enabled)", out) is not None\
        and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?drop)", out) \
        is not None
    return True


def relay_option_82_with_drop_validation_and_mac_rid(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 drop validate mac")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?enabled)", out) is not None\
        and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?drop)", out) \
        is not None and re.search("(Remote ID\s*:\s*?mac)", out) is not None
    return True


def relay_option_82_with_drop_validation_and_ip_rid(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 drop validate ip")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?enabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?enabled)", out) is not None\
        and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?drop)", out) \
        is not None and re.search("(Remote ID\s*:\s*?ip)", out) is not None
    return True


def dhcp_relay_option_82_disable(sw1):
    sw1("configure terminal")
    sw1("no dhcp-relay option 82")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Option 82\s*:\s*?disabled)", out) is not None and \
        re.search("(Response [V|v]alidation\s*:\s*?disabled)", out) is not \
        None and re.search("(Option 82 [H|h]andle [P|p]olicy\s*:\s*?replace)",
                           out) \
        is not None and re.search("(Remote ID\s*:\s*?mac)", out) is not None
    return True


def dhcp_relay_option_82_validation_disable(sw1):
    sw1("configure terminal")
    sw1("no dhcp-relay option 82 validate")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(Response [V|v]alidation\s*:\s*?disabled)", out) \
        is not None
    return True


def show_dhcp_relay(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(DHCP Relay Agent\s*:\s*?enabled)", out) is not None
    return True


def show_dhcp_relay_statistics(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(DHCP Relay Agent\s*:\s*?enabled)", out) is not None \
        and "DHCP Relay Statistics:" and \
        "Client Requests       Server Responses" and \
        "Valid      Dropped    Valid      Dropped" in out
    return True


def show_dhcp_relay_option82_statistics(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay")
    sw1("end")

    out = sw1("show dhcp-relay")
    assert re.search("(DHCP Relay Agent\s*:\s*?enabled)", out) is not None \
        and "DHCP Relay Option 82 Statistics:" and \
        "Client Requests       Server Responses" and \
        "Valid      Dropped    Valid      Dropped" in out
    return True


def dhcp_relay_running_config_test(sw1):
    sw1("configure terminal")
    sw1("no dhcp-relay")
    sw1("end")

    out = sw1("show running-config")
    assert "no dhcp-relay" in out
    return True


def dhcp_relay_hop_count_increment_running_config_test(sw1):
    sw1("configure terminal")
    sw1("no dhcp-relay hop-count-increment")
    sw1("end")

    out = sw1("show running-config")
    assert "no dhcp-relay hop-count-increment" in out
    return True


def relay_option82_drop_running_config_test(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 drop")
    sw1("end")

    out = sw1("show running-config")
    assert "dhcp-relay option 82 drop" in out
    return True


def relay_option82_keep_running_config_test(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 keep")
    sw1("end")

    out = sw1("show running-config")
    assert "dhcp-relay option 82 keep" in out
    return True


def relay_option82_replace_running_config_test(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 replace")
    sw1("end")

    out = sw1("show running-config")
    assert "dhcp-relay option 82 replace" in out
    return True


def relay_option82_replace_with_validation_running_config_test(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 replace validate")
    sw1("end")

    out = sw1("show running-config")
    assert "dhcp-relay option 82 replace validate" in out
    return True


def relay_option82_drop_with_validation_running_config_test(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 drop validate")
    sw1("end")

    out = sw1("show running-config")
    assert "dhcp-relay option 82 drop validate" in out
    return True


def relay_option82_keep_with_ip_rid_running_config_test(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 keep ip")
    sw1("end")

    out = sw1("show running-config")
    assert "dhcp-relay option 82 keep ip" in out
    return True


def relay_option82_replace_with_ip_rid_running_config_test(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 replace ip")
    sw1("end")

    out = sw1("show running-config")
    assert "dhcp-relay option 82 replace ip" in out
    return True


def relay_option82_drop_with_ip_rid_running_config_test(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 drop ip")
    sw1("end")

    out = sw1("show running-config")
    assert "dhcp-relay option 82 drop ip" in out
    return True


def relay_option82_replace_validation_ip_rid_running_config_test(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 replace validate ip")
    sw1("end")

    out = sw1("show running-config")
    assert "dhcp-relay option 82 replace validate ip" in out
    return True


def relay_option82_drop_validation_ip_rid_running_config_test(sw1):
    sw1("configure terminal")
    sw1("dhcp-relay option 82 drop validate ip")
    sw1("end")

    out = sw1("show running-config")
    assert "dhcp-relay option 82 drop validate ip" in out
    return True


def helper_address_configuration_for_unicast_server_ip(sw1):
    sw1("configure terminal")
    sw1("interface 1")
    sw1("ip helper-address 192.168.10.1")
    sw1("end")

    out = sw1("show ip helper-address")
    assert "Interface: 1" and "192.168.10.1" in out
    return True


def helper_address_configuration_on_split_interface(sw1):
    sw1("configure terminal")
    sw1("interface 50")
    sw1._shells['vtysh']._prompt = (
        '.*Do you want to continue [y/n]?'
    )

    sw1('split')

    sw1._shells['vtysh']._prompt = (
        '(^|\n)switch(\\([\\-a-zA-Z0-9]*\\))?#'
    )

    sw1('y')
    sw1(' ')
    sw1("interface 50-4")
    sw1("ip helper-address 192.168.80.1")
    sw1("end")

    out = sw1("show ip helper-address")
    assert "Interface: 50-4" and "192.168.80.1" in out
    return True


def helper_address_configuration_on_sub_interface(sw1):
    sw1("configure terminal")
    sw1("interface 1.4")
    sw1("ip helper-address 192.168.90.1")
    sw1("end")

    out = sw1("show ip helper-address")
    assert "Interface: 1.4" and "192.168.90.1" in out
    return True


def helper_address_unconfiguration_for_unicast_server_ip(sw1):
    sw1("configure terminal")
    sw1("interface 1")
    sw1("no ip helper-address 192.168.10.1")
    sw1("end")

    out = sw1("show ip helper-address")
    assert "Interface: 1" and "192.168.10.1" not in out
    return True


def helper_address_unconfig_for_un_configd_helper_address(sw1):
    sw1("configure terminal")
    sw1("interface 1")
    out = sw1("no ip helper-address 192.168.40.1")
    sw1("end")

    assert "Helper-address is not present." in out
    return True


def helper_address_configuration_for_multicast_server_ip(sw1):
    sw1("configure terminal")
    sw1("interface 1")
    out = sw1("ip helper-address 224.0.0.0")
    sw1("end")

    assert "Multicast and loopback addresses are not allowed." in out
    return True


def helper_address_configuration_for_broadcast_server_ip(sw1):
    sw1("configure terminal")
    sw1("interface 1")
    out = sw1("ip helper-address 255.255.255.255")
    sw1("end")

    # assert "Interface: 1" and "225.255.255.255" in out
    out = sw1("show ip helper-address interface 1")
    assert "IP Helper Addresses\n\n Interface: 1\n" \
        "  IP Helper Address\n  -----------------\n  255.255.255.255" in out
    return True


def helper_address_configuration_for_loopback_server_ip(sw1):
    sw1("configure terminal")
    sw1("interface 1")
    out = sw1("ip helper-address 127.0.0.1")
    sw1("end")

    assert "Multicast and loopback addresses are not allowed." in out
    return True


def helper_address_configuration_for_invalid_server_ip(sw1):
    sw1("configure terminal")
    sw1("interface 1")
    out = sw1("ip helper-address 275.255.255.255")
    sw1("end")

    assert "% Unknown command." in out
    return True


def helper_address_record_duplication__onsingle_interface(sw1):
    sw1("configure terminal")
    sw1("interface 1")
    out = sw1("ip helper-address 192.168.20.1")
    out = sw1("ip helper-address 192.168.20.1")
    sw1("end")

    assert "This entry already exists." in out
    return True


def multiple_helper_address_config_for_single_interface(sw1):
    sw1("configure terminal")
    sw1("interface 2")
    sw1("ip helper-address 192.168.10.2")
    sw1("ip helper-address 192.168.10.3")
    sw1("ip helper-address 192.168.10.4")
    sw1("end")

    out = sw1("show ip helper-address")
    assert "Interface: 2 and 192.168.10.2" and "192.168.10.3" \
        and "192.168.10.4" in out
    return True


def maximum_helper_address_configuration_per_interface(sw1):
    sw1("configure terminal")
    sw1("interface 3")
    sw1("ip helper-address 192.168.100.2")
    sw1("ip helper-address 192.168.100.3")
    sw1("ip helper-address 192.168.100.4")
    sw1("ip helper-address 192.168.100.5")
    sw1("ip helper-address 192.168.100.6")
    sw1("ip helper-address 192.168.100.7")
    sw1("ip helper-address 192.168.100.8")
    sw1("ip helper-address 192.168.100.9")
    out = sw1("ip helper-address 192.168.100.10")
    sw1("end")

    assert "Maximum allowed helper addresses already configured " \
        "on the interface." in out
    return True


def single_helper_address_config_on_different_interfaces(sw1):
    sw1("configure terminal")
    sw1("interface 5")
    sw1("ip helper-address 192.168.10.12")
    sw1("end")

    sw1("configure terminal")
    sw1("interface 6")
    sw1("ip helper-address 192.168.10.12")
    sw1("end")

    sw1("configure terminal")
    sw1("interface 7")
    sw1("ip helper-address 192.168.10.12")
    sw1("end")

    out = sw1("show ip helper-address")
    assert "Interface: 11" and "192.168.10.12" and \
        "192.168.10.12" and "192.168.10.12" in out
    return True


def show_helper_address_status(sw1):
    sw1("configure terminal")
    sw1("interface 11")
    sw1("ip helper-address 192.168.30.2")
    sw1("end")

    out = sw1("show ip helper-address")
    assert "Interface: 11" and "192.168.30.2" in out
    return True


def show_helper_address_and_bootp_gateway_config_on_interface(sw1):
    sw1("configure terminal")
    sw1("interface 2")
    sw1("ip helper-address 192.168.90.1")
    sw1("ip address 10.0.0.1/24")
    sw1("ip bootp-gateway 10.0.0.1")
    sw1("no ip bootp-gateway 10.0.0.1")
    sw1("end")

    out = sw1("show ip helper-address")
    assert "Interface: 2" and "192.168.90.1" in out
    return True


def show_helper_address_status_on_specified_interface(sw1):
    sw1("configure terminal")
    sw1("interface 11")
    sw1("ip helper-address 192.168.50.2")
    sw1("end")

    out = sw1("show ip helper-address interface 11")
    assert "Interface: 11" and "192.168.50.2" in out
    return True


def helper_address_running_config_test(sw1):
    sw1("configure terminal")
    sw1("interface 4")
    sw1("ip helper-address 192.168.90.11")
    sw1("end")

    out = sw1("show running-config")
    assert "Interface: 8" and \
        "ip helper-address 192.168.90.11" in out
    return True


def helper_address_interface_running_config_test(sw1):
    sw1("configure terminal")
    sw1("interface 8")
    sw1("ip helper-address 192.168.90.77")
    sw1("ip helper-address 192.168.90.78")
    sw1("end")

    out = sw1("show running-config interface 8")
    assert "Interface: 8" and "ip helper-address 192.168.90.77" and \
        "ip helper-address 192.168.90.78" in out
    return True


def bootp_gateway_config_for_config_ip_on_interface(sw1):
    sw1("configure terminal")
    sw1("interface 18")
    sw1("ip address 10.0.80.1/24")
    sw1("ip bootp-gateway 10.0.80.1")
    sw1("end")

    out = sw1("show dhcp-relay bootp-gateway")
    assert "18                   10.0.80.1" in out
    return True


def bootp_gateway_config_for_config_ip_on_sub_interface(sw1):
    sw1("configure terminal")
    sw1("interface 10.1")
    sw1("ip address 20.0.80.1/24")
    sw1("ip bootp-gateway 20.0.80.1")
    sw1("end")

    out = sw1("show dhcp-relay bootp-gateway")
    assert "10.1                 20.0.80.1" in out
    return True


def bootp_gateway_config_for_config_secondary_ip_on_interface(sw1):
    sw1("configure terminal")
    sw1("interface 14")
    sw1("ip address 50.0.80.1/24")
    sw1("ip bootp-gateway 50.0.80.1")
    sw1("end")

    out = sw1("show dhcp-relay bootp-gateway")
    assert "14                   50.0.80.1" in out
    return True


def bootp_gateway_config_for_config_secondary_ip_on_sub_interface(sw1):
    sw1("configure terminal")
    sw1("interface 5.2")
    sw1("ip address 30.0.80.1/24")
    sw1("ip bootp-gateway 30.0.80.1")
    sw1("end")

    out = sw1("show dhcp-relay bootp-gateway")
    assert "5.2                  30.0.80.1" in out
    return True


def bootp_gateway_config_for_unconfig_ip_on_interface(sw1):
    sw1("configure terminal")
    sw1("interface 19")
    out = sw1("ip bootp-gateway 10.0.10.1")
    sw1("end")

    assert "The IP address 10.0.10.1 is not yet configured" \
        " on this interface." in out
    return True


def bootp_gateway_config_for_invalid_ip(sw1):
    sw1("configure terminal")
    sw1("interface 20")
    out = sw1("ip bootp-gateway 275.255.255.255")
    sw1("end")

    assert "% Unknown command." in out
    return True


def bootp_gateway_unconfig_for_configbootp_gateway_on_interface(sw1):
    sw1("configure terminal")
    sw1("interface 21")
    sw1("ip address 10.0.20.1/24")
    sw1("ip bootp-gateway 10.0.20.1")
    sw1("no ip bootp-gateway 10.0.20.1")
    sw1("end")

    out = sw1("show dhcp-relay bootp-gateway")
    assert "21                    10.0.20.1" not in out
    return True


def bootp_gateway_unconfig_for_config_ip_on_interface(sw1):
    sw1("configure terminal")
    sw1("interface 22")
    sw1("ip address 10.0.30.1/24")
    out = sw1("no ip bootp-gateway 10.0.30.1")
    sw1("end")

    assert "The BOOTP Gateway 10.0.30.1 is not configured" \
        " on this interface." in out
    return True


def bootp_gateway_unconfig_for_unconfig_ip_on_interface(sw1):
    sw1("configure terminal")
    sw1("interface 23")
    out = sw1("no ip bootp-gateway 10.0.40.1")
    sw1("end")

    assert "The BOOTP Gateway 10.0.40.1 is not " \
        "configured on this interface." in out
    return True


def show_bootp_gateway_status(sw1):
    sw1("configure terminal")
    sw1("interface 17")
    sw1("ip address 10.0.10.2/24")
    sw1("ip bootp-gateway 10.0.10.2")
    sw1("end")

    out = sw1("show dhcp-relay bootp-gateway")
    assert "17                   10.0.10.2" in out
    return True


def show_bootp_gateway_and_helper_address_config_on_interface(sw1):
    sw1("configure terminal")
    sw1("interface 16")
    sw1("ip helper-address 192.168.90.1")
    sw1("ip address 10.0.60.2/24")
    sw1("ip bootp-gateway 10.0.60.2")
    sw1("no ip helper-address 192.168.90.1")
    sw1("end")

    out = sw1("show dhcp-relay bootp-gateway")
    assert "16                   10.0.60.2" in out
    return True


def show_bootp_gateway_status_on_specified_interface(sw1):
    sw1("configure terminal")
    sw1("interface 26")
    sw1("ip address 20.0.0.2/24")
    sw1("ip bootp-gateway 20.0.0.2")
    sw1("end")

    out = sw1("show dhcp-relay bootp-gateway interface 26")
    assert "26                   20.0.0.2" in out
    return True


def bootp_gateway_running_config_test(sw1):
    sw1("configure terminal")
    sw1("interface 27")
    sw1("ip address 30.0.0.3/24")
    sw1("ip bootp-gateway 30.0.0.3")
    sw1("end")

    out = sw1("show running-config")
    assert "Interface: 27" and \
        "ip bootp-gateway 30.0.0.3" in out
    return True


def bootp_gateway_interface_running_config_test(sw1):
    sw1("configure terminal")
    sw1("interface 28")
    sw1("ip address 40.0.0.6/24")
    sw1("ip bootp-gateway 40.0.0.6")
    sw1("end")

    out = sw1("show running-config interface 28")
    assert "Interface: 28" and \
        "ip bootp-gateway 40.0.0.6" in out
    return True


def dhcp_relay_complete_post_reboot(sw1):
    # Disable DHCP-Relay
    dhcp_relay_disable(sw1)
    # Set a helper-address
    helper_address_configuration_for_unicast_server_ip(sw1)

    # Save the running to start-up config
    run_cfg = "copy running-config startup-config"
    sw1(run_cfg)

    # Verify the configurations are saved in startup configs
    start_cfg = "show startup-config"
    out = sw1(start_cfg)

    assert "ip helper-address 192.168.10.1" and \
        "no dhcp-relay" in out

    # Check for the configs post reboot
    sleep(30)

    out = sw1("show running-config")
    assert "ip helper-address 192.168.10.1" and \
        "no dhcp-relay" in out
    return True


def test_dhcp_relay_ipapps_configuration(topology, step):
    sw1 = topology.get("sw1")

    assert sw1 is not None

    dhcp_relay_enable(sw1)

    dhcp_relay_disable(sw1)

    dhcp_relay_hop_count_increment_enable(sw1)

    dhcp_relay_option_82_enable(sw1)

    dhcp_relay_option_82_validation_enable(sw1)

    relay_option_82_with_validation_and_replace(sw1)

    relay_option_82_with_validation_replace_and_mac_rid(sw1)

    relay_option_82_with_validation_replace_and_ip_rid(sw1)

    relay_option_82_with_validation_and_drop(sw1)

    relay_option_82_with_validation_drop_and_mac_rid(sw1)

    relay_option_82_with_validation_drop_and_ip_rid(sw1)

    relay_option_82_with_keep_policy(sw1)

    relay_option_82_with_keep_and_mac_rid(sw1)

    relay_option_82_with_keep_and_ip_rid(sw1)

    relay_option_82_with_replace_policy(sw1)

    relay_option_82_with_replace_and_mac_rid(sw1)

    relay_option_82_with_replace_and_ip_rid(sw1)

    relay_option_82_with_replace_and_validation(sw1)

    relay_option_82_with_replace_validation_and_mac_rid(sw1)

    relay_option_82_with_replace_validation_and_ip_rid(sw1)

    relay_option_82_with_drop_policy(sw1)

    relay_option_82_with_keep_and_mac_rid(sw1)

    relay_option_82_with_drop_and_ip_rid(sw1)

    relay_option_82_with_drop_and_validation(sw1)

    relay_option_82_with_drop_validation_and_mac_rid(sw1)

    relay_option_82_with_drop_validation_and_ip_rid(sw1)

    dhcp_relay_hop_count_increment_disable(sw1)

    dhcp_relay_option_82_disable(sw1)

    dhcp_relay_option_82_validation_disable(sw1)

    show_dhcp_relay(sw1)

    show_dhcp_relay_statistics(sw1)

    show_dhcp_relay_option82_statistics(sw1)

    dhcp_relay_running_config_test(sw1)

    dhcp_relay_hop_count_increment_running_config_test(sw1)

    relay_option82_drop_running_config_test(sw1)

    relay_option82_keep_running_config_test(sw1)

    relay_option82_replace_running_config_test(sw1)

    relay_option82_replace_with_validation_running_config_test(sw1)

    relay_option82_drop_with_validation_running_config_test(sw1)

    relay_option82_keep_with_ip_rid_running_config_test(sw1)

    relay_option82_replace_with_ip_rid_running_config_test(sw1)

    relay_option82_drop_with_ip_rid_running_config_test(sw1)

    relay_option82_replace_validation_ip_rid_running_config_test(sw1)

    relay_option82_drop_validation_ip_rid_running_config_test(sw1)

    helper_address_configuration_for_unicast_server_ip(sw1)

    helper_address_unconfiguration_for_unicast_server_ip(sw1)

    helper_address_configuration_on_split_interface(sw1)

    helper_address_configuration_on_sub_interface(sw1)

    helper_address_unconfig_for_un_configd_helper_address(sw1)

    helper_address_configuration_for_multicast_server_ip(sw1)

    helper_address_configuration_for_broadcast_server_ip(sw1)

    helper_address_configuration_for_loopback_server_ip(sw1)

    helper_address_configuration_for_invalid_server_ip(sw1)

    helper_address_record_duplication__onsingle_interface(sw1)

    multiple_helper_address_config_for_single_interface(sw1)

    maximum_helper_address_configuration_per_interface(sw1)

    single_helper_address_config_on_different_interfaces(sw1)

    show_helper_address_status(sw1)

    show_helper_address_and_bootp_gateway_config_on_interface(sw1)

    show_helper_address_status_on_specified_interface(sw1)

    helper_address_running_config_test(sw1)

    helper_address_interface_running_config_test(sw1)

    bootp_gateway_config_for_config_ip_on_interface(sw1)

    bootp_gateway_config_for_config_ip_on_sub_interface(sw1)

    bootp_gateway_config_for_config_secondary_ip_on_interface(sw1)

    bootp_gateway_config_for_config_secondary_ip_on_sub_interface(sw1)

    bootp_gateway_config_for_unconfig_ip_on_interface(sw1)

    bootp_gateway_config_for_invalid_ip(sw1)

    bootp_gateway_unconfig_for_configbootp_gateway_on_interface(sw1)

    bootp_gateway_unconfig_for_config_ip_on_interface(sw1)

    bootp_gateway_unconfig_for_unconfig_ip_on_interface(sw1)

    show_bootp_gateway_status(sw1)

    show_bootp_gateway_and_helper_address_config_on_interface(sw1)

    show_bootp_gateway_status_on_specified_interface(sw1)

    bootp_gateway_running_config_test(sw1)

    bootp_gateway_interface_running_config_test(sw1)
