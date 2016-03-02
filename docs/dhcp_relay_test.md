# DHCP-Relay Daemon Test Cases

## Contents
- [Verify DHCP-relay configuration](#verify-dhcp-relay-configuration)
    - [Enable DHCP-relay globally](#enable-dhcp-relay-globally)
    - [Disable DHCP-relay globally](#disable-dhcp-relay-globally)
- [Verify relay helper address configuration](#verify-relay-helper-address-configuration)
    - [Verify relay helper address configuration on an specific interface](#verify-relay-helper-address-configuration-on-an-specific-interface)
    - [Verify the maximum helper-address configurations per interface](#verify-the-maximum-helper-address-configurations-per-interface)
    - [Verify relay helper address configuration on all interfaces](#verify-relay-helper-address-configuration-on-all-interfaces)
    - [Verify the single helper-address configuration on different interfaces](#verify-the-single-helper-address-configuration-on-different-interfaces)
    - [Verify maximum number of helper address configuration on all interfaces](#verify-maximum-number-of-helper-address-configuration-on-all-interfaces)

## Verify DHCP-relay configuration
### Objective
To verify the DHCP-relay configuration whether it is enabled/disabled.

### Requirements
One switch is required for this test.

### Setup
#### Topology Diagram
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
Enable the DHCP-relay on dut01 using `dhcp-relay`.
### Test Result Criteria
#### Test Pass Criteria
Verify DHCP-relay state in the unixctl dump output is true.
#### Test Fail Criteria
This test fails if DHCP-relay state in the unixctl dump output is false.

### Disable DHCP-relay globally
### Description
Disable the DHCP-relay on dut01 using `no dhcp-relay`.
### Test Result Criteria
#### Test Pass Criteria
Verify DHCP-relay state in the unixctl dump output is false.
#### Test Fail Criteria
This test fails if DHCP-relay state in the unixctl dump output is not false.

## Verify relay helper address configuration
### Objective
To verify helper addresses are correctly updated in daemon local cache.
### Requirements
One switch is required for this test.

### Setup
#### Topology Diagram
```ditaa

                          +----------------+
                          |                |
                          |     DUT01      |
                          |                |
                          |                |
                          +----------------+
```

### Verify relay helper address configuration on an specific interface
### Description
Add Helper address on an interface using appropriate CLI. Verify that unixctl dump output of an interface displays the helper address.
### Test Result Criteria
#### Test Pass Criteria
The unixctl dump output displays the configured helper addresses.
#### Test Fail Criteria
This test fails if configured helper addresses are not displayed in unixctl dump output.

### Verify the maximum helper-address configurations per interface
### Description
Add Helper address on an interface using Appropriate CLI. Verify that unixctl dump output of an interface displays 16 helper address.
### Test Result Criteria
#### Test Pass Criteria
The unixctl dump output displays 16 helper-address.
#### Test Fail Criteria
This test fails if configured helper addresses(16 addresses) are not displayed in unixctl dump output.

### Verify relay helper address configuration on all interfaces
### Description
Add Helper address on interfaces using appropriate CLI. Verify that unixctl dump output displays the interfaces along with helper address.
### Test Result Criteria
#### Test Pass Criteria
The unixctl dump output displays the configured helper addresses on all interfaces.
#### Test Fail Criteria
This test fails if configured helper addresses are not displayed in unixctl dump output.

### Verify the single helper-address configuration on different interfaces
### Description
Add Helper address on interfaces using appropriate CLI. Verify that unixctl dump output displays the interfaces along with helper address.
### Test Result Criteria
#### Test Pass Criteria
The unixctl dump output displays the configured helper addresses on all interfaces.
#### Test Fail Criteria
This test fails if configured helper addresses are not displayed in unixctl dump output.

### Verify maximum number of helper address configuration on all interfaces
### Description
This is a Daemon stress test to check stability. Add around 100 helper address on an interfaces using appropriate CLI. Verify that unixctl dump output displays the interfaces along with helper address. Remove 50 helper address using appropriate CLI and verify the output of unixctl dump.
### Test Result Criteria
#### Test Pass Criteria
The unixctl dump output displays the configured helper addresses on all interfaces.
#### Test Fail Criteria
This test fails if configured helper addresses are not displayed in unixctl dump output.