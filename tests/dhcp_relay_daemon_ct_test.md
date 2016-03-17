# DHCP-Relay Daemon Test Cases

## Contents
- [Verify DHCP-relay configuration](#verify-dhcp-relay-configuration)
    - [Enable DHCP-relay globally](#enable-dhcp-relay-globally)
    - [Disable DHCP-relay globally](#disable-dhcp-relay-globally)
- [Verify the relay helper address configuration](#verify-the-relay-helper-address-configuration)
    - [Verify the helper address configuration on a specific interface](#verify-the-helper-address-configuration-on-a-specific-interface)
    - [Verify the maximum helper address configurations per interface](#verify-the-maximum-helper-address-configurations-per-interface)
    - [Verify the same helper address configuration on different interfaces](#verify-the-same-helper-address-configuration-on-different-interfaces)
    - [Verify the maximum number of helper address configurations on all interfaces](#verify-the-maximum-number-of-helper-address-configurations-on-all-interfaces)
    - [Verify helper address deletion on all interfaces](#verify-helper-address-deletion-on-all-interfaces)

## Verify DHCP-relay configuration
### Objective
To verify if the DHCP-relay configuration is enabled/disabled.

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

### Enable DHCP-relay globally
### Description
Enable the DHCP-relay on dut01 using the `dhcp-relay` command.
### Test result Criteria
#### Test pass criteria
Verify that the DHCP-relay state in the unixctl dump output is true.
#### Test fail criteria
Verify that the DHCP-relay state in the unixctl dump output is false.

### Disable DHCP-relay globally
### Description
Disable the DHCP-relay on dut01 using the `no dhcp-relay` command.
### Test result Criteria
#### Test pass criteria
Verify that the DHCP-relay state in the unixctl dump output is false.
#### Test fail criteria
Verify that the DHCP-relay state in the unixctl dump output is true.

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

### Verify the helper address configuration on a specific interface
### Description
1. Configure helper addresses on an interface using the `ip helper-address <adddress>` command.
2. Verify that the unixctl dump output of an interface displays the helper addresses.
### Test result Criteria
#### Test pass criteria
The unixctl dump output displays the interface along with the helper addresses.
#### Test fail criteria
The unixctl dump output does not display interface with the helper addresses.

### Verify the maximum helper address configurations per interface
### Description
1. Configure the maximum helper addresses(eight addresses) on an interface using `ip helper-address <adddress>` command.
2. Verify that the unixctl dump output of an interface displays eight helper addresses.
### Test result Criteria
#### Test pass criteria
The unixctl dump output for the interface displays eight helper addresses.
#### Test fail criteria
The unixctl dump output for the interface does not display eight helper addresses.

### Verify the same helper address configuration on different interfaces
### Description
1. Configure helper address on an interface (Ex: interface 2) using `ip helper-address <adddress>` command.
2. Configure the same helper address on another interface (Ex: interface 3) using `ip helper-address <adddress>` command.
3. Configure the same helper address on another interface (Ex: interface 4) using `ip helper-address <adddress>` command
4. Verify that the unixctl dump output of an interface (Ex: interface 2) displays reference count for the ip address as three.
### Test result Criteria
#### Test pass criteria
The unixctl dump output displays the correct reference count.
#### Test fail criteria
The unixctl dump output does not display the correct reference count.

### Verify the maximum number of helper address configurations on all interfaces
### Description
This is a daemon performance test to check stability. 100 IP addresses are configured to check stability.
1. Configure eight helper addresses on interface 1 using `ip helper-address <adddress>` command.
2. Repeat the above step for other interfaces from interface two to interface 13.
3. Verify that unixctl dump output displays all 100 ip helper addresses.
### Test result Criteria
#### Test pass criteria
The unixctl dump output displays the the interfaces along with helper addresses.
#### Test fail criteria
This test fails if helper addresses are not displayed in unixctl dump output.

### Verify helper address deletion on all interfaces
### Description
This is a daemon performance test to check stability. 50 IP addresses are unconfigured to check stability.
1. Unconfigure eight helper addresses on interface 1 using `no ip helper-address <adddress>` command.
2. Repeat the above step for other interfaces from interface two to interface 6.
3. Verify that unixctl dump output for an interface displays no helper addresses.
### Test result Criteria
#### Test pass criteria
The unixctl dump output for an interface displays no helper addresses.
#### Test fail criteria
This test fails if helper addresses for an interface are displayed in unixctl dump output.