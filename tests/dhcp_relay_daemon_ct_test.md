# DHCP Relay Daemon Test Cases

## Contents
- [Verify dhcp-relay configuration](#verify-dhcp-relay-configuration)
    - [Enable dhcp-relay globally](#enable-dhcp-relay-globally)
    - [Disable dhcp-relay globally](#disable-dhcp-relay-globally)
- [Verify dhcp-relay option 82 configuration](#verify-dhcp-relay-option-82-configuration)
    - [Enable dhcp-relay hop-count-increment](#enable-dhcp-relay-hop-count-increment)
    - [Disable dhcp-relay hop-count-increment](#disable-dhcp-relay-hop-count-increment)
    - [Enable dhcp-relay option 82](#enable-dhcp-relay-option-82)
    - [Disable dhcp-relay option 82](#disable-dhcp-relay-option-82)
    - [Enable dhcp-relay option 82 validation](#enable-dhcp-relay-option-82-validation)
    - [Disable dhcp-relay option 82 validation](#disable-dhcp-relay-option-82-validation)
    - [Enable dhcp-relay option 82 with drop policy](#enable-dhcp-relay-option-82-with-drop-policy)
    - [Disable dhcp-relay option 82 with drop policy](#disable-dhcp-relay-option-82-with-drop-policy)
    - [Enable dhcp-relay option 82 with keep policy](#enable-dhcp-relay-option-82-with-keep-policy)
    - [Enable dhcp-relay option 82 with replace policy](#enable-dhcp-relay-option-82-with-replace-policy)
    - [Disable dhcp-relay and check status of option 82 configuration](#disable-dhcp-relay-and-check-status-of-option-82-configuration)
- [Verify the relay helper address configuration](#verify-the-relay-helper-address-configuration)
    - [Verify the helper address configuration on a specific interface](#verify-the-helper-address-configuration-on-a-specific-interface)
    - [Verify the maximum helper address configurations per interface](#verify-the-maximum-helper-address-configurations-per-interface)
    - [Verify the same helper address configuration on different interfaces](#verify-the-same-helper-address-configuration-on-different-interfaces)
    - [Verify the maximum number of helper address configurations on all interfaces](#verify-the-maximum-number-of-helper-address-configurations-on-all-interfaces)
    - [Verify helper address deletion on all interfaces](#verify-helper-address-deletion-on-all-interfaces)
    - [Verify bootp gateway configuration](#verify-bootp-gateway-configuration)

## Verify dhcp-relay configuration
### Objective
To verify if the DHCP relay configuration is enabled/disabled.

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

### Enable dhcp-relay globally
### Description
Enable dhcp-relay on dut01 using the `dhcp-relay` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCP relay state in the unixctl dump output is true.
#### Test fail criteria
Verify that the DHCP relay state in the unixctl dump output is false.

### Disable dhcp-relay globally
### Description
Disable dhcp-relay on dut01 using the `no dhcp-relay` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCP relay state in the unixctl dump output is false.
#### Test fail criteria
Verify that the DHCP relay state in the unixctl dump output is true.

## Verify dhcp-relay option 82 configuration
### Objective
To verify that dhcp-relay option 82 configuration is enabled/disabled.

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
### Enable dhcp-relay hop-count-increment
### Description
Enable dhcp-relay hop-count-increment on dut01 using the `dhcp-relay hop-count-increment` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCP relay hop count increment state in the unixctl dump output is true.
#### Test fail criteria
Verify that the DHCP relay hop count increment state in the unixctl dump output is false.

### Disable dhcp-relay hop-count-increment
### Description
Disable dhcp-relay hop-count-increment on dut01 using the `no dhcp-relay hop-count-increment` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCP relay hop count increment state in the unixctl dump output is false.
#### Test fail criteria
Verify that the DHCP relay hop count increment state in the unixctl dump output is true.

### Enable dhcp-relay option 82
### Description
Enable dhcp-relay option 82 on dut01 using the `dhcp-relay option 82 replace` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCP relay option 82 state in the unixctl dump output is true.
#### Test fail criteria
Verify that the DHCP relay option 82 state in the unixctl dump output is false.

### Disable dhcp-relay option 82
### Description
Disable dhcp-relay option 82 on dut01 using the `no dhcp-relay option 82` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCP relay option 82 state in the unixctl dump output is false.
#### Test fail criteria
Verify that the DHCP relay option 82 state in the unixctl dump output is true.

### Enable dhcp-relay option 82 validation
### Description
Enable dhcp-relay option 82 validation on dut01 using the `dhcp-relay option 82 validate` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCP relay option 82 validation state in the unixctl dump output is true.
#### Test fail criteria
Verify that the DHCP relay option 82 validation state in the unixctl dump output is false.

### Disable dhcp-relay option 82 validation
### Description
Disable dhcp-relay option 82 validation on dut01 using the `no dhcp-relay option 82 validate` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCP relay option 82 validation state in the unixctl dump output is false.
#### Test fail criteria
Verify that the DHCP relay option 82 validation state in the unixctl dump output is true.

### Enable dhcp-relay option 82 with drop policy
### Description
Enable dhcp-relay option 82 with drop policy on dut01 using the `dhcp-relay option 82 drop` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCP relay option 82 policy state in the unixctl dump output is 'drop'.
#### Test fail criteria
Verify that the DHCP relay option 82 policy state in the unixctl dump output is not 'drop'.

### Disable dhcp-relay option 82 with drop policy
### Description
Disable dhcp-relay option 82 with drop policy on dut01 using the `no dhcp-relay option 82` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCP relay option 82 policy state in the unixctl dump output is not 'drop'.
#### Test fail criteria
Verify that the DHCP relay option 82 policy state in the unixctl dump output is 'drop'.

### Enable dhcp-relay option 82 with keep policy
### Description
Enable dhcp-relay option 82 with keep policy on dut01 using the `dhcp-relay option 82 keep` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCP relay option 82 policy state in the unixctl dump output is 'keep'.
#### Test fail criteria
Verify that the DHCP relay option 82 policy state in the unixctl dump output is not 'keep'.

### Enable dhcp-relay option 82 with replace policy
### Description
Enable dhcp-relay option 82 with replace policy on dut01 using the `dhcp-relay option 82 replace` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCP relay option 82 policy state in the unixctl dump output is 'replace'.
#### Test fail criteria
Verify that the DHCP relay option 82 policy state in the unixctl dump output is not 'replace'.

### Disable dhcp-relay option 82 with replace policy
### Description
Disable dhcp-relay option 82 with replace policy on dut01 using the `no dhcp-relay option 82` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCP relay option 82 policy state in the unixctl dump output is not 'replace'.
#### Test fail criteria
Verify that the DHCP relay option 82 policy state in the unixctl dump output is 'replace'.

### Disable dhcp-relay and check status of option 82 configuration
### Description
Disable dhcp-relay on dut01 using the `no dhcp-relay` command.
### Test result criteria
#### Test pass criteria
Verify that the DHCP relay option 82 state in the unixctl dump output is false.
#### Test fail criteria
Verify that the DHCP relay option 82 state in the unixctl dump output is true.

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
2. Verify that the unixctl dump output of the interface displays the helper addresses.

### Test result criteria
#### Test pass criteria
The unixctl dump output displays the interface along with the helper addresses.
#### Test fail criteria
The unixctl dump output does not display interface with the helper addresses.

### Verify the maximum helper address configurations per interface
### Description
1. Configure the maximum helper addresses(eight addresses) on an interface using the `ip helper-address <adddress>` command.
2. Verify that the unixctl dump output of the interface displays eight helper addresses.

### Test result criteria
#### Test pass criteria
The unixctl dump output for the interface displays eight helper addresses.
#### Test fail criteria
The unixctl dump output for the interface does not display eight helper addresses.

### Verify the same helper address configuration on different interfaces
### Description
1. Configure a helper address on an interface (for example, interface 2) using the `ip helper-address <adddress>` command.
2. Configure the same helper address on another interface (for example, interface 3) using the `ip helper-address <adddress>` command.
3. Configure the same helper address on another interface (for example, interface 4) using the `ip helper-address <adddress>` command
4. Verify that the unixctl dump output of an interface (for example, interface 2) displays the reference count for the IP address as three.

### Test result criteria
#### Test pass criteria
The unixctl dump output displays the correct reference count.
#### Test fail criteria
The unixctl dump output does not display the correct reference count.

### Verify the maximum number of helper address configurations on all interfaces
### Description
This is a daemon performance test to check stability. Configure 100 IP addresses to check stability.
1. Configure eight helper addresses on interface 1 using the `ip helper-address <adddress>` command.
2. Repeat the above step for other interfaces from interface 2 to interface 13.
3. Verify that the unixctl dump output displays all 100 IP helper addresses.

### Test result criteria
#### Test pass criteria
The unixctl dump output displays the the interfaces along with the helper addresses.
#### Test fail criteria
This test fails if the helper addresses are not displayed in the unixctl dump output.

### Verify helper address deletion on all interfaces
### Description
This is a daemon performance test to check stability. Unconfigure 50 IP addresses to check stability.
1. Unconfigure eight helper addresses on interface 1 using the `no ip helper-address <adddress>` command.
2. Repeat the above step for other interfaces from interface 2 to interface 6.
3. Verify that the unixctl dump output for an interface displays no helper addresses.

### Test result criteria
#### Test pass criteria
The unixctl dump output for an interface displays no helper addresses.
#### Test fail criteria
This test fails if helper addresses for an interface are displayed in the unixctl dump output.

### Verify bootp gateway configuration
### Description
1. Configure bootp gateway address on an interface using the `ip bootp-gateway <adddress>` command.
2. Verify that the unixctl dump output of the interface displays the bootp gateway address.

### Test result criteria
#### Test pass criteria
The unixctl dump output displays the interface along with the bootp gateway address.
#### Test fail criteria
The unixctl dump output does not display interface with the bootp gateway address.
