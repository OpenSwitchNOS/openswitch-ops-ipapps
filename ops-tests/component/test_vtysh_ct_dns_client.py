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

'''
FIXME - Commenting the script as the dns-client feature is not merged.
        This needs to be enabled, when dns-client is enabled as well.
        This script is not tested, as there was a change in priorities
        so only the coding is done covering all the cases supported for the
        dns-client feature, but no testing is done. This will be done
        when dns-client feature is enabled.

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


# Test to disable DNS client
def dns_client_disable(sw1):
    sw1('configure terminal')
    sw1("no ip dns")
    sw1("end")

    out = sw1("show ip dns")
    assert re.search("(DNS Client Mode: Disabled)", out) is not None
    return True


# Test to enable DNS client
def dns_client_enable(sw1):
    sw1('configure terminal')
    sw1("ip dns")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(DNS Client Mode: Enabled)", out) is not None
    return True


# Test to configure a DNS client domain-name
def dns_client_domain_name_config(sw1):
    sw1('configure terminal')
    sw1("ip dns domain-name domain.com")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(DNS Domain name : domain.com)", out) is not None
    return True


# Test to configure a DNS client domain-list
def dns_client_domain_list_config(sw1):
    sw1('configure terminal')
    sw1("ip dns domain-list domainlist1.com")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(DNS Domain list : domainlist1.com)", out) is not None
    return True


# Test to unconfigure a DNS client domain-list
def dns_client_domain_list_unconfig(sw1):
    sw1('configure terminal')
    sw1("no ip dns domain-list domainlist1.com")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(DNS Domain list : domainlist1.com)", out) is None
    return True


# Test to configure multiple DNS client domain-list
def dns_client_domain_list_multiple_config(sw1):
    sw1('configure terminal')
    sw1("ip dns domain-list domainlist1.com")
    sw1("ip dns domain-list domainlist2.com")
    sw1("ip dns domain-list domainlist3.com")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(DNS Domain list : domainlist1.com, domainlist2.com, \
                     domainlist3.com)", out) is not None
    return True


# Test to unconfigure multiple DNS client domain-list
def dns_client_domain_list_multiple_unconfig(sw1):
    sw1('configure terminal')
    sw1("no ip dns domain-list domainlist1.com")
    sw1("no ip dns domain-list domainlist2.com")
    sw1("no ip dns domain-list domainlist3.com")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(DNS Domain list : domainlist1.com, domainlist2.com, \
                     domainlist3.com)", out) is None
    return True


# Test to configure a DNS client IPv4 name-server
def dns_client_v4name_server_config(sw1):
    sw1('configure terminal')
    sw1("ip dns server-address 1.1.1.1")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(Name Server(s) : 1.1.1.1)", out) is not None
    return True


# Test to unconfigure a DNS client IPv4 name-server
def dns_client_v4name_server_unconfig(sw1):
    sw1('configure terminal')
    sw1("no ip dns server-address 1.1.1.1")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(Name Server(s) : 1.1.1.1)", out) is None
    return True


# Test to configure multiple DNS client IPv4 name-server
def dns_client_v4name_server_multiple_config(sw1):
    sw1('configure terminal')
    sw1("ip dns server-address 2.2.2.2")
    sw1("ip dns server-address 3.3.3.3")
    sw1("ip dns server-address 4.4.4.4")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(Name Server(s) : 2.2.2.2, \
                       3.3.3.3, 4.4.4.4)", out) is not None
    return True


# Test to unconfigure multiple DNS client IPv4 name-server
def dns_client_v4name_server_multiple_unconfig(sw1):
    sw1('configure terminal')
    sw1("no ip dns server-address 2.2.2.2")
    sw1("no ip dns server-address 3.3.3.3")
    sw1("no ip dns server-address 4.4.4.4")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(Name Server(s) : 2.2.2.2, \
                       3.3.3.3, 4.4.4.4)", out) is None
    return True


# Test to configure a DNS client IPv6 name-server
def dns_client_v6name_server_config(sw1):
    sw1('configure terminal')
    sw1("ip dns server-address a::32")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(Name Server(s) : a::32)", out) is not None
    return True


# Test to unconfigure a DNS client IPv6 name-server
def dns_client_v6name_server_unconfig(sw1):
    sw1('configure terminal')
    sw1("no ip dns server-address a::32")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(Name Server(s) : a::32)", out) is None
    return True


# Test to configure multiple DNS client IPv6 name-server
def dns_client_v4name_server_multiple_config(sw1):
    sw1('configure terminal')
    sw1("ip dns server-address b::12")
    sw1("ip dns server-address c::51")
    sw1("ip dns server-address 123::56")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(Name Server(s) : b::12, \
                       c::51, 123::56)", out) is not None
    return True


# Test to unconfigure multiple DNS client IPv6 name-server
def dns_client_v6name_server_multiple_unconfig(sw1):
    sw1('configure terminal')
    sw1("no ip dns server-address b::12")
    sw1("no ip dns server-address c::51")
    sw1("no ip dns server-address 123::56")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(Name Server(s) : b::12, \
                       c::51, 123::56)", out) is None
    return True


# Test to configure both IPv4 and IPv6 name-servers
def dns_client_name_server_config(sw1):
    sw1('configure terminal')
    sw1("ip dns server-address 18.18.18.18")
    sw1("ip dns server-address E::08")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(Name Server(s) : 18.18.18.18, \
                       E::08)", out) is not None
    return True


# Test to unconfigure both IPv4 and IPv6 name-servers
def dns_client_name_server_unconfig(sw1):
    sw1('configure terminal')
    sw1("no ip dns server-address 18.18.18.18")
    sw1("no ip dns server-address E::08")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(Name Server(s) : 18.18.18.18, \
                       E::08)", out) is None
    return True


# Test to configure a DNS client hoat
def dns_client_host_config(sw1):
    sw1('configure terminal')
    sw1("ip dns host host1 1.2.3.4")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(host1            1.2.3.4)", out) is not None
    return True


# Test to unconfigure a DNS client host
def dns_client_host_unconfig(sw1):
    sw1('configure terminal')
    sw1("no ip dns host host1 1.2.3.4")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("((host1            1.2.3.4)", out) is None
    return True


# Test to configure multiple DNS client hosts
def dns_client_host_multiple_config(sw1):
    sw1('configure terminal')
    sw1("ip dns host host2 2.3.4.5")
    sw1("ip dns host host3 8::53")
    sw1("ip dns host host2 a::9")
    sw1("ip dns host host4 121.1.1.1")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(host2            2.3.4.5)", out) is not None
    assert re.search("(host3            8::53)", out) is not None
    assert re.search("(host2            a::9)", out) is not None
    assert re.search("(host4            121.1.1.1)", out) is not None
    return True


# Test to unconfigure multiple DNS client hosts
def dns_client_hosts_multiple_unconfig(sw1):
    sw1('configure terminal')
    sw1("no ip dns host host2 2.3.4.5")
    sw1("no ip dns host host3 8::53")
    sw1("no ip dns host host2 a::9")
    sw1("no ip dns host host4 121.1.1.1")
    sw1('end')

    out = sw1("show ip dns")
    out = sw1("show ip dns")
    assert re.search("(host2            2.3.4.5)", out) is None
    assert re.search("(host3            8::53)", out) is None
    assert re.search("(host2            a::9)", out) is None
    assert re.search("(host4            121.1.1.1)", out) is None
    return True


# Test to verify maximum domain lists
def dns_client_max_domain_list_config(sw1):
    sw1('configure terminal')
    sw1("ip dns domain-list domainlist1.com")
    sw1("ip dns domain-list domainlist2.com")
    sw1("ip dns domain-list domainlist3.com")
    sw1("ip dns domain-list domainlist4.com")
    sw1("ip dns domain-list domainlist5.com")
    sw1("ip dns domain-list domainlist6.com")
    maxOut = sw1("ip dns domain-list domainlist7.com")
    sw1('end')

    assert re.search("(Maximum domain lists are configured, \
                     Failed to configure domain7.com)", maxOut) is not None

    out = sw1("show ip dns")
    assert re.search("(DNS Domain list : domainlist1.com, domainlist2.com, \
                     domainlist3.com, domainlist4.com, domainlist5.com, \
                     domainlist6.com)", out) is not None

    # Unconfigure these domain lists
    sw1('configure terminal')
    sw1("no ip dns domain-list domainlist1.com")
    sw1("no ip dns domain-list domainlist2.com")
    sw1("no ip dns domain-list domainlist3.com")
    sw1("no ip dns domain-list domainlist4.com")
    sw1("no ip dns domain-list domainlist5.com")
    sw1("no ip dns domain-list domainlist6.com")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(DNS Domain list : domainlist1.com, domainlist2.com, \
                     domainlist3.com, domainlist4.com, domainlist5.com, \
                     domainlist6.com)", out) is None

    return True


# Test to verify maximum name servers
def dns_client_max_name_servers_config(sw1):
    sw1('configure terminal')
    sw1("ip dns server-address 1.1.1.1")
    sw1("ip dns server-address a::12")
    sw1("ip dns server-address 1.1.1.2")
    maxOut = sw1("ip dns server-address 1.1.1.3")
    sw1('end')

    assert re.search("(Maximum name servers are configured, \
                     Failed to configure 1.1.1.3)", maxOut) is not None

    out = sw1("show ip dns")
    assert re.search("(Name Server(s) : 1.1.1.1, a::12, \
                     1.1.1.2)", out) is not None

    # Unconfigure these name servers
    sw1('configure terminal')
    sw1("no ip dns server-address 1.1.1.1")
    sw1("no ip dns server-address a::12")
    sw1("no ip dns server-address 1.1.1.2")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(Name Server(s) : 1.1.1.1, a::12, \
                     1.1.1.2)", out) is None

    return True


# Test to verify maximum hosts
def dns_client_max_hosts_config(sw1):
    sw1('configure terminal')
    sw1("ip dns host host1 5.6.7.8")
    sw1("ip dns host host2 1.11.1.1")
    sw1("ip dns host host2 b::32")
    sw1("ip dns host host3 5.5.5.5")
    sw1("ip dns host host4 1.11.1.5")
    sw1("ip dns host host5 2.2.2.2")
    sw1("ip dns host host6 123::12")
    maxOut = sw1("ip dns host host7 1.1.1.1")
    sw1('end')

    assert re.search("(Maximum hosts are configured, \
                     Failed to configure host7)", maxOut) is not None

    out = sw1("show ip dns")
    assert re.search("(host4            1.11.1.5 \
                       host1            5.6.7.8 \
                       host3            5.5.5.5 \
                       host2            1.11.1.1
                       host5            2.2.2.2 \
                       host2            b::32 \
                       host6            123::12)", out) is not None

    # Unconfigure these hosts
    sw1('configure terminal')
    sw1("no ip dns host host1 5.6.7.8")
    sw1("no ip dns host host2 1.11.1.1")
    sw1("no ip dns host host2 b::32")
    sw1("no ip dns host host3 5.5.5.5")
    sw1("no ip dns host host4 1.11.1.5")
    sw1("no ip dns host host5 2.2.2.2")
    sw1("no ip dns host host6 123::12")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(host4            1.11.1.5 \
                       host1            5.6.7.8 \
                       host3            5.5.5.5 \
                       host2            1.11.1.1
                       host5            2.2.2.2 \
                       host2            b::32 \
                       host6            123::12)", out) is None


    return True


# Test to verify invalid inputs
def dns_client_invalid_inputs(sw1):
    sw1('configure terminal')
    # Verify invalid domain-name entry
    out = sw1("ip dns domain-name 1234")

    assert re.search("(Invalid Domain name)", out) is not None

    # Verify invalid domain-list entry
    out = sw1("ip dns domain-list 1234")

    assert re.search("(Invalid Domain list)", out) is not None

    # Verify invalid name server entry
    out = sw1("ip dns server-address 255.255.255.255")

    assert re.search("(IPv4 :Broadcast, multicast and \
                      loopback addresses are not allowed)", out) is not None

    out = sw1("ip dns server-address FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF")

    assert re.search("(IPv6 :Broadcast, multicast and \
                      loopback addresses are not allowed)", out) is not None


    # Verify invalid hosts entry
    out = sw1("ip dns host 123 1.1.1.1")
    assert re.search("(Invalid host name)", out) is not None

    out = sw1("ip dns host host8 255.255.255.255")
    assert re.search("(IPv4 :Broadcast, multicast and \
                      loopback addresses are not allowed)", out) is not None

    out = sw1("ip dns host host8 FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF")
    assert re.search("(IPv6 :Broadcast, multicast and \
                      loopback addresses are not allowed)", out) is not None
    sw1('end')

    return True


# Test to verify unconfigure of non-existing entry
def dns_client_non_existing_entry(sw1):
    sw1('configure terminal')

    # Verify unconfigure of non-existing domain-name entry
    sw1("ip dns domain-name domain")
    out = sw1("no ip dns domain-name domain.com")
    assert re.search("(domain name entry not found.)", out) is not None

    sw1("no ip dns domain-name domain")

    # Verify unconfigure of non-existing domain-list entry
    sw1("ip dns domain-list domainlist1")
    out = sw1("no ip dns domain-list domainlist5")
    assert re.search("(Domain list entry is not present.)", out) is not None

    sw1("no ip dns domain-list domainlist1")

    # Verify unconfigure of non-existing name server entry
    sw1("ip dns server-address 1.11.1.1")
    out = sw1("no ip dns serevr-address 99.9.9.9")
    assert re.search("(Name server entry is not present.)", out) is not None

    sw1("no ip dns server-address 1.11.1.1")

    # Verify unconfigure of non-existing host entry
    sw1("ip dns host host7 6.6.6.6")
    out = sw1("no ip dns host host61 3.3.33.3")
    assert re.search("(host entry not found.)", out) is not None

    sw1("no ip dns host host7 6.6.6.6")

    sw1('end')

    return True


# Test to verify a stress scenario
def dns_client_stress_scenario(sw1):
    sw1('configure terminal')
    sw1("ip dns domain-name domain.com")

    # Configure maximum domain-lists
    sw1("ip dns domain-list domainlist1.com")
    sw1("ip dns domain-list domainlist2.com")
    sw1("ip dns domain-list domainlist3.com")
    sw1("ip dns domain-list domainlist4.com")
    sw1("ip dns domain-list domainlist5.com")
    sw1("ip dns domain-list domainlist6.com")

    # Configure maximum name servers
    sw1("ip dns server-address 1.1.1.1")
    sw1("ip dns server-address a::12")
    sw1("ip dns server-address 1.1.1.2")

    # Configure maxmimum hosts
    sw1("ip dns host host1 5.6.7.8")
    sw1("ip dns host host2 1.11.1.1")
    sw1("ip dns host host2 b::32")
    sw1("ip dns host host3 5.5.5.5")
    sw1("ip dns host host4 1.11.1.5")
    sw1("ip dns host host5 2.2.2.2")
    sw1("ip dns host host6 123::12")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(Domain Name : domain.com)", out) is not None
    assert re.search("(DNS Domain list : domainlist1.com, domainlist2.com, \
                       domainlist3.com, domainlist4.com, domainlist5.com, \
                       domainlist6.com)", out) is not None
    assert re.search("(Name Server(s) : 1.1.1.1, a::12, 1.1.1.2)", out) is not None
    assert re.search("(host4            1.11.1.5 \
                       host1            5.6.7.8 \
                       host3            5.5.5.5 \
                       host2            1.11.1.1
                       host5            2.2.2.2 \
                       host2            b::32 \
                       host6            123::12)", out) is not None

    # Clear these settings
    sw1('configure terminal')
    sw1("no ip dns domain-name domain.com")
    sw1("no ip dns domain-list domainlist1.com")
    sw1("no ip dns domain-list domainlist2.com")
    sw1("no ip dns domain-list domainlist3.com")
    sw1("no ip dns domain-list domainlist4.com")
    sw1("no ip dns domain-list domainlist5.com")
    sw1("no ip dns domain-list domainlist6.com")
    sw1("no ip dns server-address 1.1.1.1")
    sw1("no ip dns server-address a::12")
    sw1("no ip dns server-address 1.1.1.2")
    sw1("no ip dns host host1 5.6.7.8")
    sw1("no ip dns host host2 1.11.1.1")
    sw1("no ip dns host host2 b::32")
    sw1("no ip dns host host3 5.5.5.5")
    sw1("no ip dns host host4 1.11.1.5")
    sw1("no ip dns host host5 2.2.2.2")
    sw1("no ip dns host host6 123::12")
    sw1('end')

    return True


# Test to verify configurations post reboot
def dns_client_post_reboot(sw1):
    sw1('configure terminal')
    sw1("ip dns domain-name domainreboot.com")
    sw1("ip dns domain-list rebootlist1.com")
    sw1("ip dns server-address 88.89.90.91")
    sw1("ip dns server-address 123::123")
    sw1("ip dns host host1 63.65.67.69")
    sw1("ip dns host host2 53::32")
    sw1('end')

    out = sw1("show ip dns")
    assert re.search("(Domain Name : domainreboot.com)", out) is not None
    assert re.search("(DNS Domain list : rebootlist1.com)", out) is not None
    assert re.search("(Name Server(s) : 88.89.90.91, 123::123)", out) is not None
    assert re.search("(host1            63.65.67.69 \
                       host2            53::32)", out) is not None

    ## Perform reboot here
    # FIXME Incorporate APIs to reboot

    sleep(30)
    out = sw1("show ip dns")
    assert re.search("(Domain Name : domainreboot.com)", out) is not None
    assert re.search("(DNS Domain list : rebootlist1.com)", out) is not None
    assert re.search("(Name Server(s) : 88.89.90.91, 123::123)", out) is not None
    assert re.search("(host1            63.65.67.69 \
                       host2            53::32)", out) is not None

    return True


def test_dns_client_configuration(topology, step):
    sw1 = topology.get('sw1')

    assert sw1 is not None

    # Test to disable DNS client
    assert dns_client_disable(sw1)

    # Test to enable DNS client
    assert dns_client_enable(sw1)

    # Test to show running configuration for DNS client
    assert dns_client_running_config_test(sw1)

    # Test to configure a DNS client domain-name
    assert dns_client_domain_name_config(sw1)

    # Test to unconfigure a DNS client domain-name
    assert dns_client_domain_name_unconfig(sw1)

    # Test to configure a DNS client domain-name multiple times
    assert dns_client_domain_name_multiple_config(sw1)

    # Test to configure a DNS client domain-list
    assert dns_client_domain_list_config(sw1)

    # Test to unconfigure a DNS client domain-list
    assert dns_client_domain_list_unconfig(sw1)

    # Test to configure multiple DNS client domain-list
    assert dns_client_domain_list_multiple_config(sw1)

    # Test to configure multiple DNS client domain-list
    assert dns_client_domain_list_multiple_unconfig(sw1)

    # Test to configure a DNS client IPv4 name-server
    assert dns_client_v4name_server_config(sw1)

    # Test to unconfigure a DNS client IPv4 name-server
    assert dns_client_v4name_server_unconfig(sw1)

    # Test to configure multiple DNS client IPv4 name-server
    assert dns_client_v4name_server_multiple_config(sw1)

    # Test to unconfigure multiple DNS client IPv4 name-server
    assert dns_client_v4name_server_multiple_unconfig(sw1)

    # Test to configure a DNS client IPv6 name-server
    assert dns_client_v6name_server_config(sw1)

    # Test to unconfigure a DNS client IPv6 name-server
    assert dns_client_v6name_server_unconfig(sw1)

    # Test to configure multiple DNS client IPv6 name-server
    assert dns_client_v6name_server_multiple_config(sw1)

    # Test to unconfigure multiple DNS client IPv6 name-server
    assert dns_client_v6name_server_multiple_unconfig(sw1)

    # Test to configure both IPv4 and IPv6 name-servers
    assert dns_client_name_server_config(sw1)

    # Test to configure a DNS client host
    assert dns_client_host_config(sw1)

    # Test to unconfigure a DNS client host
    assert dns_client_host_unconfig(sw1)

    # Test to configure multiple DNS client hosts
    assert dns_client_host_multiple_config(sw1)

    # Test to unconfigure multiple DNS client hosts
    assert dns_client_host_multiple_unconfig(sw1)

    # Test to verify maximum domain lists
    assert dns_client_max_domain_list_config(sw1)

    # Test to verify maximum name servers
    assert dns_client_max_name_servers_config(sw1)

    # Test to verify maximum hosts
    assert dns_client_max_hosts_config(sw1)

    # Test to verify invalid inputs
    assert dns_client_invalid_inputs(sw1)

    # Test to verify unconfigure of non-existing entry
    assert dns_client_non_existing_entry(sw1)

    # Test to verify a stress scenario
    assert dns_client_stress_scenario(sw1)

    # Test to verify configurations post reboot
    assert dns_client_post_reboot(sw1)
'''
