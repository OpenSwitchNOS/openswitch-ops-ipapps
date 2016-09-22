# Source-interface Selection CLI Test Cases

## Contents
- [Verify source-interface configuration](#verify-source-interface-configuration)
    - [Verify source-interface IP address configuration to the TFTP protocol](#verify-source-interface-ip-address-configuration-to-the-tftp-protocol)
    - [Verify source-interface configuration to the TFTP protocol](#verify-source-interface-configuration-to-the-tftp-protocol)
    - [Verify source-interface IP address configuration to the TACACS protocol](#verify-source-interface-ip-address-configuration-to-the-tacacs-protocol)
    - [Verify source-interface configuration to the TACACS protocol](#verify-source-interface-configuration-to-the-tacacs-protocol)
    - [Verify source-interface IP address configuration to the RADIUS protocol](#verify-source-interface-ip-address-configuration-to-the-radius-protocol)
    - [Verify source-interface configuration to the RADIUS protocol](#verify-source-interface-configuration-to-the-radius-protocol)
    - [Verify source-interface IP address configuration for all the specified protocols](#verify-source-interface-ip-address-configuration-for-all-the-specified-protocols)
    - [Verify source-interface configuration for all the specified protocols](#verify-source-interface-configuration-for-all-the-specified-protocols)
    - [Verify the validation of source-interface IP address](#verify-the-validation-of-source-interface-ip-address)
    - [Verify source-interface IP address configuration with unconfigured IP address](#verify-source-interface-ip-address-configuration-with-unconfigured-ip-address)
    - [Verify source-interface configuration with unconfigured interface](#verify-source-interface-configuration-with-unconfigured-interface)
    - [Verify source-interface unconfiguration to the TFTP protocol with unconfigured source-interface](#verify-source-interface-unconfiguration-to-the-tftp-protocol-with-unconfigured-source-interface)
    - [Verify source-interface unconfiguration to the TACACS protocol with unconfigured source-interface](#verify-source-interface-unconfiguration-to-the-tacacs-protocol-with-unconfigured-source-interface)
    - [Verify source-interface unconfiguration to the RADIUS protocol with unconfigured source-interface](#verify-source-interface-unconfiguration-to-the-radius-protocol-with-unconfigured-source-interface)
    - [Verify source-interface unconfiguration for all the specified protocols with unconfigured source-interface](#verify-source-interface-unconfiguration-for-all-the-specified-protocols-with-unconfigured-source-interface)
    - [Verify source-interface unconfiguration to the TFTP protocol](#verify-source-interface-unconfiguration-to-the-tftp-protocol)
    - [Verify source-interface unconfiguration to the TACACS protocol](#verify-source-interface-unconfiguration-to-the-tacacs-protocol)
    - [Verify source-interface unconfiguration to the RADIUS protocol](#verify-source-interface-unconfiguration-to-the-radius-protocol)
    - [Verify source-interface unconfiguration for all the specified protocols](#verify-source-interface-unconfiguration-for-all-the-specified-protocols)
    - [Verify source-interface configuration in show-running](#verify-source-interface-configuration-in-show-running)
    - [Verify source-interface configuration post reboot](#verify-source-interface-configuration-post-reboot)
## Verify source-interface configuration
### Objective
To verify source-interface configurations and verify the negative scenarios as well.
### Requirements
The requirements for this test case are:
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

### Verify source-interface IP address configuration to the TFTP protocol
### Description
Verify source-interface IP address configuration to the TFTP protocol
on dut01 using the `ip source-interface tftp <A.B.C.D>` command.
### Test result criteria
#### Test pass criteria
The user verifies that the source-interface IP address to the TFTP protocol is
reflected in the respective `show` command.
#### Test fail criteria
The user verifies that the source-interface IP address to the TFTP protocol is
not reflected in the respective `show` command.

### Verify source-interface configuration to the TFTP protocol
### Description
Verify source-interface configuration to the TFTP protocol on dut01 using
the `ip source-interface tftp interface <IFNAME>` command.
### Test result criteria
#### Test pass criteria
The user verifies that the source-interface to the TFTP protocol is
reflected in the respective `show` command.
#### Test fail criteria
The user verifies that the source-interface to the TFTP protocol is
not reflected in the respective `show` command.

### Verify source-interface IP address configuration to the TACACS protocol
### Description
Verify source-interface IP address configuration to the TACACS protocol
on dut01 using the `ip source-interface tacacs <A.B.C.D>` command.
### Test result criteria
#### Test pass criteria
The user verifies that the source-interface IP address to the TACACS protocol is
reflected in the respective `show` command.
#### Test fail criteria
The user verifies that the source-interface IP address to the TACACS protocol is
not reflected in the respective `show` command.

### Verify source-interface configuration to the TACACS protocol
### Description
Verify source-interface configuration to the TACACS protocol on dut01 using
the `ip source-interface tacacs interface <IFNAME>` command.
### Test result criteria
#### Test pass criteria
The user verifies that the source-interface to the TACACS protocol is
reflected in the respective `show` command.
#### Test fail criteria
The user verifies that the source-interface to the TACACS protocol is
not reflected in the respective `show` command.

### Verify source-interface IP address configuration to the RADIUS protocol
### Description
Verify source-interface IP address configuration to the RADIUS protocol
on dut01 using the `ip source-interface radius <A.B.C.D>` command.
### Test result criteria
#### Test pass criteria
The user verifies that the source-interface IP address to the RADIUS protocol is
reflected in the respective `show` command.
#### Test fail criteria
The user verifies that the source-interface IP address to the RADIUS protocol is
not reflected in the respective `show` command.

### Verify source-interface configuration to the RADIUS protocol
### Description
Verify source-interface configuration to the RADIUS protocol on dut01 using
the `ip source-interface radius interface <IFNAME>` command.
### Test result criteria
#### Test pass criteria
The user verifies that the source-interface to the RADIUS protocol is
reflected in the respective `show` command.
#### Test fail criteria
The user verifies that the source-interface to the RADIUS protocol is
not reflected in the respective `show` command.

### Verify source-interface IP address configuration for all the specified protocols
### Description
Verify source-interface IP address configuration for all the specified protocols
on dut01 using the `ip source-interface all <A.B.C.D>` command.
### Test result criteria
#### Test pass criteria
The user verifies that the source-interface IP address for all the specified protocols is
reflected in the respective `show` command.
#### Test fail criteria
The user verifies that the source-interface IP address for all the specified protocols is
not reflected in the respective `show` command.

### Verify source-interface configuration for all the specified protocols
### Description
Verify source-interface configuration for all the specified protocols on dut01 using
the `ip source-interface all interface <IFNAME>` command.
### Test result criteria
#### Test pass criteria
The user verifies that the source-interface for all the specified protocols is
reflected in the respective `show` command.
#### Test fail criteria
The user verifies that the source-interface for all the specified protocols is
not reflected in the respective `show` command.

### Verify the validation of source-interface IP address
### Description
Verify the validation of source-interface IP address with invalid IP address
on dut01 using the `ip source-interface tftp <A.B.C.D>` command.
### Test result criteria
#### Test pass criteria
The user is unable to configure the source-interface IP address.
#### Test fail criteria
The user is able to configure the source-interface IP address.

### Verify source-interface IP address configuration with unconfigured IP address
### Description
Verify source-interface IP address configuration with unconfigured IP address
on dut01 using the `ip source-interface tftp <A.B.C.D>` command.
### Test result criteria
#### Test pass criteria
The user is unable to configure the source-interface IP address.
#### Test fail criteria
The user is able to configure the source-interface IP address.

### Verify source-interface configuration with unconfigured interface
### Description
Verify source-interface configuration with unconfigured interface
on dut01 using the `ip source-interface tftp interface <IFNAME>` command.
### Test result criteria
#### Test pass criteria
The user is unable to configure the source-interface.
#### Test fail criteria
The user is able to configure the source-interface.

### Verify source-interface unconfiguration to the TFTP protocol
### Description
Verify source-interface unconfiguration to the TFTP protocol on dut01 using
the `no ip source-interface tftp` command.
### Test result criteria
#### Test pass criteria
The user verifies that the source-interface to the TFTP protocol is
not reflected in the respective `show` command.
#### Test fail criteria
The user verifies that the source-interface to the TFTP protocol is
reflected in the respective `show` command.

### Verify source-interface unconfiguration to the TACACS protocol
### Description
Verify source-interface unconfiguration to the TACACS protocol on dut01 using
the `no ip source-interface tacacs` command.
### Test result criteria
#### Test pass criteria
The user verifies that the source-interface to the TACACS protocol is
not reflected in the respective `show` command.
#### Test fail criteria
The user verifies that the source-interface to the TACACS protocol is
reflected in the respective `show` command.

### Verify source-interface unconfiguration to the RADIUS protocol
### Description
Verify source-interface unconfiguration to the RADIUS protocol on dut01 using
the `no ip source-interface radius` command.
### Test result criteria
#### Test pass criteria
The user verifies that the source-interface to the RADIUS protocol is
not reflected in the respective `show` command.
#### Test fail criteria
The user verifies that the source-interface to the RADIUS protocol is
reflected in the respective `show` command.

### Verify source-interface unconfiguration for all the specified protocols
### Description
Verify source-interface unconfiguration for all the specified protocols on dut01 using
the `no ip source-interface all` command.
### Test result criteria
#### Test pass criteria
The user verifies that the source-interface for all the specified protocols is
not reflected in the respective `show` command.
#### Test fail criteria
The user verifies that the source-interface for all the specified protocols is
reflected in the respective `show` command.

### Verify source-interface unconfiguration to the TFTP protocol with unconfigured source-interface
### Description
Verify source-interface unconfiguration to the TFTP protocol with unconfigured
source-interface on dut01 using the `no ip source-interface tftp` command.
### Test result criteria
#### Test pass criteria
The user is unable to configure the source-interface to the TFTP protocol.
#### Test fail criteria
The user is able to configure the source-interface to the TFTP protocol.

### Verify source-interface unconfiguration to the TACACS protocol with unconfigured source-interface
### Description
Verify source-interface unconfiguration to the TACACS protocol with unconfigured
source-interface on dut01 using the `no ip source-interface tacacs` command.
### Test result criteria
#### Test pass criteria
The user is unable to configure the source-interface to the TACACS protocol.
#### Test fail criteria
The user is able to configure the source-interface to the TACACS protocol.

### Verify source-interface unconfiguration to the RADIUS protocol with unconfigured source-interface
### Description
Verify source-interface unconfiguration to the RADIUS protocol with unconfigured
source-interface on dut01 using the `no ip source-interface radius` command.
### Test result criteria
#### Test pass criteria
The user is unable to configure the source-interface to the RADIUS protocol.
#### Test fail criteria
The user is able to configure the source-interface to the RADIUS protocol.

### Verify source-interface unconfiguration for all the specified protocols with unconfigured source-interface
### Description
Verify source-interface unconfiguration for all the specified protocols with
unconfigured source-interface on dut01 using the `no ip source-interface all` command.
### Test result criteria
#### Test pass criteria
The user is unable to configure the source-interface for all
the specified protocols.
#### Test fail criteria
The user is able to configure the source-interface for all
the specified protocols.

### Verify source-interface configuration in show-running
### Description
Verify the source-interface configuration using the
`show running-configuration` command.
### Test result criteria
#### Test pass criteria
The user verifies that the current source-interface configuration
is reflected in the `show` command.
#### Test fail criteria
The user verifies that the current source-interface configuration
is not reflected in the `show` command.

### Verify source-interface configuration post reboot
### Description
Verify that the source-interface configuration is retained after the device reboots.
### Test Result Criteria
#### Test pass criteria
The source-interface configurations is retained post reboot.
#### Test fail criteria
The source-interface configurations is not retained post reboot.