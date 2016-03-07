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
Configure helper addresses on an interface using appropriate CLI commands. Verify that the unixctl dump output of an interface displays the helper addresses.
### Test result Criteria
#### Test pass criteria
The unixctl dump output displays the interface along with the helper addresses.
#### Test fail criteria
The unixctl dump output does not display interface with the helper addresses.

### Verify the maximum helper address configurations per interface
### Description
Configure the maximum helper addresses(eight addresses) on an interface using appropriate CLI commands. Verify that the unixctl dump output of an interface displays eight helper addresses.
### Test result Criteria
#### Test pass criteria
The unixctl dump output for the interface displays eight helper addresses.
#### Test fail criteria
The unixctl dump output for the interface does not display eight helper addresses.

### Verify the same helper address configuration on different interfaces
### Description
Configure the same helper address on different interfaces using appropriate CLI commands. Verify that the unixctl dump output displays the interfaces along with the helper address.
### Test result Criteria
#### Test pass criteria
The unixctl dump output displays the interfaces along with the helper address.
#### Test fail criteria
The unixctl dump output does not display the interfaces with the helper address.

### Verify the maximum number of helper address configurations on all interfaces
### Description
This is a daemon performance test to check stability. Configure a total of 100 helper addresses on the interfaces using appropriate CLI. Verify that unixctl dump output displays the interfaces along with helper addresses.
### Test result Criteria
#### Test pass criteria
The unixctl dump output displays the the interfaces along with helper addresses.
#### Test fail criteria
This test fails if helper addresses are not displayed in unixctl dump output.

### Verify helper address deletion on all interfaces
### Description
This is a daemon performance test to check stability. Remove 50 helper addresses using appropriate CLI and verify the output of unixctl dump. Verify that unixctl dump output displays no helper addresses.
### Test result Criteria
#### Test pass criteria
The unixctl dump output displays no helper addresses.
#### Test fail criteria
This test fails if helper addresses are displayed in unixctl dump output.