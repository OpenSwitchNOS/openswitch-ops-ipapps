# DHCPv6 Relay Daemon Test Cases

## Contents
- [Verify dhcpv6-relay configuration](#verify-dhcpv6-relay-configuration)
    - [Enable dhcpv6-relay globally](#enable-dhcpv6-relay-globally)
    - [Disable dhcpv6-relay globally](#disable-dhcpv6-relay-globally)
- [Verify dhcpv6-relay option 79 configuration](#verify-dhcpv6-relay-option-79-configuration)
    - [Enable dhcpv6-relay option 79](#enable-dhcpv6-relay-option-79)
    - [Disable dhcpv6-relay option 79](#disable-dhcpv6-relay-option-79)
    - [Disable dhcpv6-relay and check status of option 79 configuration](#disable-dhcpv6-relay-and-check-status-of-option-79-configuration)
- [Verify the relay helper address configuration](#verify-the-relay-helper-address-configuration)
    - [Verify the unicast helper address configuration on a specific interface](#verify-the-unicast-helper-address-configuration-on-a-specific-interface)
    - [Verify the multicast helper address configuration on a specific interface](#verify-the-multicast-helper-address-configuration-on-a-specific-interface)
    - [Verify the maximum helper address configurations per interface](#verify-the-maximum-helper-address-configurations-per-interface)
    - [Enable and disable dhcpv6 relay when maximum helper addresses are configured on an interface](#enable-and-disable-dhcpv6-relay-when-maximum-helper-addresses-are-configured-on-an-interface])
    - [Verify the same unicast helper address configuration on different interfaces](#verify-the-same-unicast-helper-address-configuration-on-different-interfaces)
    - [Verify the same multicast helper address configuration on different interfaces](#verify-the-same-multicast-helper-address-configuration-on-different-interfaces)
    - [Verify the same multicast helper address configuration on a specific interface with different outgoing interface](#verify-the-same-multicast-helper-address-configuration-on-a-specific-interface-with-different-outgoing-interface)
    - [Verify the maximum number of helper address configurations on all interfaces](#verify-the-maximum-number-of-helper-address-configurations-on-all-interfaces)
    - [Verify helper address deletion on all interfaces](#verify-helper-address-deletion-on-all-interfaces)
- [Verify the dhcpv6-relay statistics](#verify-the-dhcpv6-relay-statistics)
    - [Verify the dhcpv6-relay statistics on a specific interface](#verify-the-dhcpv6-relay-statistics-on-a-specific-interface)

## Verify dhcpv6-relay configuration
### Objective
To verify if the DHCPv6 relay configuration is enabled/disabled.

### Requirements
One switch is required for this test.

### Setup
#### Topology diagram
```ditaa

                          +----------------+
                          |                |
                          |     DUT01      |
                          |                |
                          |                |
                          +----------------+
```

### Enable dhcpv6-relay globally
### Description
Enable dhcpv6-relay on dut01 using the `dhcpv6-relay` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCPv6 relay state in the unixctl dump output is true.
#### Test fail criteria
Verify that the DHCPv6 relay state in the unixctl dump output is false.

### Disable dhcpv6-relay globally
### Description
Disable dhcpv6-relay on dut01 using the `no dhcpv6-relay` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCPv6 relay state in the unixctl dump output is false.
#### Test fail criteria
Verify that the DHCPv6 relay state in the unixctl dump output is true.

## Verify dhcpv6-relay option 79 configuration
### Objective
To verify that dhcpv6-relay option 79 configuration is enabled/disabled.

### Requirements
One switch is required for this test.

### Setup
#### Topology diagram
```ditaa

                          +----------------+
                          |                |
                          |     DUT01      |
                          |                |
                          |                |
                          +----------------+
```
### Enable dhcpv6-relay option 79
### Description
Enable dhcpv6-relay option 79 on dut01 using the `dhcpv6-relay option 79` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCPv6 relay option 79 state in the unixctl dump output is true.
#### Test fail criteria
Verify that the DHCPv6 relay option 79 state in the unixctl dump output is false.

### Disable dhcpv6-relay option 79
### Description
Disable dhcpv6-relay option 79 on dut01 using the `no dhcpv6-relay option 79` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCPv6 relay option 79 state in the unixctl dump output is false.
#### Test fail criteria
Verify that the DHCPv6 relay option 79 state in the unixctl dump output is true.

### Disable dhcpv6-relay and check status of option 79 configuration
### Description
Disable dhcpv6-relay on dut01 using the `no dhcpv6-relay` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCPv6 relay option 79 state in the unixctl dump output is false.
#### Test fail criteria
Verify that the DHCPv6 relay option 79 state in the unixctl dump output is true.

## Verify the relay helper address configuration
### Objective
To verify helper addresses are properly updated in the daemon's local cache.
### Requirements
One switch is required for this test.

### Setup
#### Topology diagram
```ditaa

                          +----------------+
                          |                |
                          |     DUT01      |
                          |                |
                          |                |
                          +----------------+
```

### Verify the unicast helper address configuration on a specific interface
### Description
1. Configure unicast helper addresses on an interface using the `ipv6 helper-address unicast <adddress>` command.
2. Verify that the unixctl dump output of the interface displays the helper addresses.

### Test result criteria
#### Test pass criteria
The unixctl dump output displays the interface along with the helper addresses.
#### Test fail criteria
The unixctl dump output does not display interface with the helper addresses.

### Verify the multicast helper address configuration on a specific interface
### Description
1. Configure multicast helper addresses on an interface using the `ipv6 helper-address multicast <adddress> egress <ifname>` command.
2. Verify that the unixctl dump output of the interface displays the helper addresses along with the egress ifnames.

### Test result criteria
#### Test pass criteria
The unixctl dump output displays the interface along with the helper addresses and egress ifnames.
#### Test fail criteria
The unixctl dump output does not display interface with the helper addresses and egress ifnames.

### Verify the maximum helper address configurations per interface
### Description
1. Configure the maximum helper addresses(eight addresses) on an interface (combination of four unicast and four multicast addresses).
2. Configure four unicast helper addresses on an interface using the `ipv6 helper-address unicast <adddress>` command.
3. Configure four multicast helper addresses on an interface using the `ipv6 helper-address multicast <adddress> egress <ifname>` command.
4. Verify that the unixctl dump output of the interface displays the all the helper addresses along with the egress ifnames.
5. Shutdown the interface using `shutdown` command from interface context.
6. Verify that the unixctl dump output of the interface displays the all the helper addresses along with the egress ifnames.
7. Perform no shutdown on the interface using `no shutdown` command from interface context.
8. Verify that the unixctl dump output of the interface displays the all the helper addresses along with the egress ifnames.

### Test result criteria
#### Test pass criteria
The unixctl dump output for the interface displays eight helper addresses along with the egress ifnames..
#### Test fail criteria
The unixctl dump output for the interface does not display eight helper addresses along with the egress ifnames.

### Enable and disable dhcpv6 relay when maximum helper addresses are configured on an interface
### Description
1. Enable dhcpv6-relay on dut01 using the `dhcpv6-relay` command.
2. Configure the maximum helper addresses(eight addresses) on an interface (combination of four unicast and four multicast addresses).
3. Configure four unicast helper addresses on an interface using the `ipv6 helper-address unicast <adddress>` command.
4. Configure four multicast helper addresses on an interface using the `ipv6 helper-address multicast <adddress> egress <ifname>` command.
5. Verify that the unixctl dump output of the interface displays the dhcpv6 relay status and all the helper addresses along with the egress ifnames.
6. Disable dhcpv6-relay on dut01 using the `no dhcpv6-relay` command.
7. Verify that the unixctl dump output of the interface displays the dhcpv6 relay status and all the helper addresses along with the egress ifnames.

### Test result criteria
#### Test pass criteria
All the above steps are executed successfully.
#### Test fail criteria
Any step failures.

### Verify the same unicast helper address configuration on different interfaces
### Description
1. Configure a unicast helper address on an interface (for example, interface 2) using the `ipv6 helper-address unicast <adddress>` command.
2. Configure the same helper address on another interface (for example, interface 3) using the `ipv6 helper-address unicast <adddress>` command.
3. Configure the same helper address on another interface (for example, interface 4) using the `ipv6 helper-address unicast <adddress>` command
4. Verify that the unixctl dump output of an interface (for example, interface 2) displays the reference count for the IPv6 address as three.

### Test result criteria
#### Test pass criteria
The unixctl dump output displays the correct reference count.
#### Test fail criteria
The unixctl dump output does not display the correct reference count.

### Verify the same multicast helper address configuration on different interfaces
### Description
1. Configure a multicast helper address on an interface (for example, interface 3) using the `ipv6 helper-address multicast <adddress>` command.
2. Configure the same helper address on another interface (for example, interface 4) using the `ipv6 helper-address multicast <adddress>` command.
3. Configure the same helper address on another interface (for example, interface 5) using the `ipv6 helper-address multicast <adddress>` command
4. Verify that the unixctl dump output of an interface (for example, interface 3) displays the reference count for the IPv6 address as three.

### Test result criteria
#### Test pass criteria
The unixctl dump output displays the correct reference count.
#### Test fail criteria
The unixctl dump output does not display the correct reference count.

### Verify the same multicast helper address configuration on a specific interface with different outgoing interface
### Description
1. Configure a helper address on an interface (for example, interface 3) using the `ipv6 helper-address multicast <adddress> egress <ifname>` command.
2. Configure the same helper address on interface 3 again with different egress ifname using the `ipv6 helper-address multicast <adddress> egress <ifname>` command.
3. Verify that the unixctl dump output of the interface 3 displays the helper addresses along with the egress ifnames.

### Test result criteria
#### Test pass criteria
The unixctl dump output displays the interface along with the helper addresses and egress ifnames.
#### Test fail criteria
The unixctl dump output for the interface does not display eight helper addresses along with the egress ifnames.

### Verify the maximum number of helper address configurations on all interfaces
### Description
This is a daemon performance test to check stability. Configure 100 IPv6 addresses(combination of unicast and multicast ipv6 addresses) to check stability.
1. Configure a combination of helper addresses(total eight) on interface 1 using `ipv6 helper-address unicast <adddress>` command and `ipv6 helper-address multicast <adddress> egress <ifname>` command.
2. Repeat the above step for other interfaces from interface 2 to interface 13.
3. Verify that the unixctl dump output displays all 100 IPv6 helper addresses along with egress ifnames.

### Test result criteria
#### Test pass criteria
The unixctl dump output displays the the interfaces along with the helper addresses and egress ifnames.
#### Test fail criteria
This test fails if the helper addresses are not displayed in the unixctl dump output egress ifnames.

### Verify helper address deletion on all interfaces
### Description
This is a daemon performance test to check stability. Unconfigure 100 IP addresses to check stability.
1. Unconfigure a combination of helper addresses(total eight) on interface 1 using `no ipv6 helper-address unicast <adddress>` command and `no ipv6 helper-address multicast <adddress> egress <ifname>` command.
2. Repeat the above step for other interfaces from interface 2 to interface 13.
3. Verify that the unixctl dump output for an interface displays no helper addresses.

### Test result criteria
#### Test pass criteria
The unixctl dump output for an interface displays no helper addresses.
#### Test fail criteria
This test fails if helper addresses for an interface are displayed in the unixctl dump output.

## Verify the dhcpv6-relay statistics
### Objective
To verify the dhcpv6-relay statistics are properly updated in the daemon's local cache.
### Requirements
One switch is required for this test.

### Setup
#### Topology diagram
```ditaa

                          +----------------+
                          |                |
                          |     DUT01      |
                          |                |
                          |                |
                          +----------------+
```
### Verify the dhcpv6-relay statistics on a specific interface
### Description
1. Configure helper addresses on an interface using the `ipv6 helper-address unicast <adddress>` command and `ipv6 helper-address multicast <adddress> egress <ifname>` command.
2. Verify that the unixctl dump output of the interface displays the dhcpv6-relay statistics.

### Test result criteria
#### Test pass criteria
The unixctl dump output displays the dhcpv6-relay statistics.
#### Test fail criteria
The unixctl dump output does not display the dhcpv6-relay statistics.
