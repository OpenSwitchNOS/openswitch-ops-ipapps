# DHCPV6 Relay Test Cases

## Contents
- [Verify the dhcpv6-relay configuration](#verify-the-dhcpv6-relay-configuration)
    - [Enable dhcpv6-relay globally](#enable-dhcpv6-relay-globally)
    - [Enable dhcpv6-relay option 79](#enable-dhcpv6-relay-option-79)
    - [Disable dhcpv6-relay globally](#disable-dhcpv6-relay-globally)
    - [Disable dhcpv6-relay option 79](#disable-dhcpv6-relay-option-79)
    - [Verify dhcpv6-relay status](#verify-dhcpv6-relay-status)
    - [Verify dhcpv6-relay statistics](#verify-dhcpv6-relay-statistics)
    - [Verify dhcpv6-relay configuration status in show running-configuration](#verify-dhcpv6-relay-configuration-status-in-show-running-configuration)
    - [Verify dhcpv6-relay option 79 configuration status in show running-configuration](#verify-dhcpv6-relay-option-79-configuration-status-in-show-running-configuration)
    - [Verify dhcpv6-relay post reboot](#verify-dhcpv6-relay-post-reboot)

## Verify the dhcpv6-relay configuration
### Objective
To verify if the dhcpv6-relay configuration is enabled/disabled.

### Requirements

 - Docker version 1.7 or above
 - Action switch docker instance/physical hardware switch

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
The dhcpv6-relay enable state is reflected in the `show dhcpv6-relay` command.
#### Test fail criteria
The dhcpv6-relay enable state is not reflected in the `show dhcpv6-relay` command.

### Enable dhcpv6-relay option 79
### Description
Enable the dhcpv6-relay option 79 on dut01 using the `dhcpv6-relay option 79` command.
### Test result criteria
#### Test pass criteria
The dhcpv6-relay option 79 enable state is reflected in the `show dhcpv6-relay` command.
#### Test fail criteria
The dhcpv6-relay option 79 enable state is not reflected in the `show dhcpv6-relay` command.

### Disable dhcpv6-relay globally
### Description
Disable the dhcpv6-relay on dut01 using the `no dhcpv6-relay` command.
### Test result criteria
#### Test pass criteria
The dhcpv6-relay disable state is reflected in the `show dhcpv6-relay` command.
#### Test fail criteria
The dhcpv6-relay disable state is not reflected in the `show dhcpv6-relay` command.

### Disable dhcpv6-relay option 79
### Description
Disable the dhcpv6-relay option 79 on dut01 using the `no dhcpv6-relay option 79` command.
### Test result criteria
#### Test pass criteria
The dhcpv6-relay option 79 disable state is reflected in the `show dhcpv6-relay` command.
#### Test fail criteria
The dhcpv6-relay option 79 disable state is not reflected in the `show dhcpv6-relay` command.


### Verify dhcpv6-relay configuration status in show running-configuration
### Description
Verify the dhcpv6-relay configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
The current dhcpv6-relay configuration status is reflected in the `show running-configuration` command.
#### Test fail criteria
The current dhcpv6-relay configuration status is not reflected in the `show running-configuration` command.

### Verify dhcpv6-relay option 79 configuration status in show running-configuration
### Description
Verify the dhcpv6-relay option 79 configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
The current dhcpv6-relay option 79 configuration status is reflected in the `show running-configuration` command.
#### Test fail criteria
The current dhcpv6-relay option 79 configuration status is not reflected in the `show running-configuration` command.

### Verify dhcpv6-relay status
### Description
Verify the dhcpv6-relay status using the `show dhcpv6-relay` command.
### Test result criteria
#### Test pass criteria
The current dhcpv6-relay status is reflected in the `show dhcpv6-relay` command.
#### Test fail criteria
The current dhcpv6-relay status is not reflected in the `show dhcpv6-relay` command.

### Verify dhcpv6-relay statistics
### Description
Verify the dhcpv6-relay statistics using the `show dhcpv6-relay` command.
### Test result criteria
#### Test pass criteria
The dhcpv6-relay statistics are reflected in the `show dhcpv6-relay` command.
#### Test fail criteria
The dhcpv6-relay statistics are not reflected in the `show dhcpv6-relay` command.

### Verify dhcpv6-relay post reboot
### Description
Verify the dhcpv6-relay configuration is retained after the device reboots.
### Test result criteria
#### Test pass criteria
The dhcpv6-relay configuration is retained post reboot.
#### Test fail criteria
The dhcpv6-relay configuration is not retained post reboot.