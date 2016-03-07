# DHCP-Relay Daemon Test Cases

## Contents
- [Verify DHCP-relay configuration](#verify-dhcp-relay-configuration)
    - [Enable DHCP-relay globally](#enable-dhcp-relay-globally)
    - [Disable DHCP-relay globally](#disable-dhcp-relay-globally)
- [Verify relay helper address configuration](#verify-relay-helper-address-configuration)
    - [Verify helper address configuration on an specific interface](#verify-helper-address-configuration-on-an-specific-interface)
    - [Verify the maximum helper-address configuration per interface](#verify-the-maximum-helper-address-configuration-per-interface)
    - [Verify same helper-address configuration on different interfaces](#verify-same-helper-address-configuration-on-different-interfaces)
    - [Verify maximum number of helper address configuration on all interfaces](#verify-maximum-number-of-helper-address-configuration-on-all-interfaces)
    - [Verify helper address deletion on all interfaces](#verify-helper-address-deletion-on-all-interfaces)

## Verify DHCP-relay configuration
### Objective
To verify the DHCP-relay configuration whether it is enabled/disabled.

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
Enable the DHCP-relay on dut01 using `dhcp-relay` command.
### Test Result Criteria
#### Test Pass Criteria
Verify DHCP-relay state in the unixctl dump output is true.
#### Test Fail Criteria
This test fails if DHCP-relay state in the unixctl dump output is false.

### Disable DHCP-relay globally
### Description
Disable the DHCP-relay on dut01 using `no dhcp-relay` command.
### Test Result Criteria
#### Test Pass Criteria
Verify DHCP-relay state in the unixctl dump output is false.
#### Test Fail Criteria
This test fails if DHCP-relay state in the unixctl dump output is not false.

## Verify relay helper address configuration
### Objective
To verify helper addresses are properly updated in daemon local cache.
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

### Verify helper address configuration on an specific interface
### Description
Configure helper addresses on an interface using appropriate CLI. Verify that unixctl dump output of an interface displays the helper addresses.
### Test Result Criteria
#### Test Pass Criteria
The unixctl dump output displays the interface along with helper addresses.
#### Test Fail Criteria
This test fails if helper addresses are not displayed in unixctl dump output.

### Verify the maximum helper-address configuration per interface
### Description
Configure maximum helper addresses(8 addresses) on an interface using appropriate CLI. Verify that unixctl dump output of an interface displays 8 helper addresses.
### Test Result Criteria
#### Test Pass Criteria
The unixctl dump output for the interface displays 8 helper-addresses.
#### Test Fail Criteria
This test fails if helper addresses(8 addresses) are not displayed in unixctl dump output.

### Verify relay helper address configuration on all interfaces
### Description
Configure helper addresses on all interfaces using appropriate CLI. Verify that unixctl dump output displays the interfaces along with helper addresses.
### Test Result Criteria
#### Test Pass Criteria
The unixctl dump output displays the the interfaces along with helper addresses.
#### Test Fail Criteria
This test fails if helper addresses are not displayed in unixctl dump output.

### Verify same helper-address configuration on different interfaces
### Description
Configure same helper address on different interfaces using appropriate CLI. Verify that unixctl dump output displays the interfaces along with helper address.
### Test Result Criteria
#### Test Pass Criteria
The unixctl dump output displays the interfaces along with the helper address.
#### Test Fail Criteria
This test fails if helper address is not displayed in unixctl dump output.

### Verify maximum number of helper address configuration on all interfaces
### Description
This is a daemon performance test to check stability. Configure a total of 100 helper addresses on the interfaces using appropriate CLI. Verify that unixctl dump output displays the interfaces along with helper addresses.
### Test Result Criteria
#### Test Pass Criteria
The unixctl dump output displays the the interfaces along with helper addresses.
#### Test Fail Criteria
This test fails if helper addresses are not displayed in unixctl dump output.

### Verify helper address deletion on all interfaces
### Description
This is a daemon performance test to check stability. Remove 50 helper addresses using appropriate CLI and verify the output of unixctl dump. Verify that unixctl dump output displays no helper addresses.
### Test Result Criteria
#### Test Pass Criteria
The unixctl dump output displays no helper addresses.
#### Test Fail Criteria
This test fails if helper addresses are displayed in unixctl dump output.