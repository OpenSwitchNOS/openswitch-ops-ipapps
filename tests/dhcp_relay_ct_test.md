# DHCP-Relay Test Cases

## Contents
- [Verify the dhcp-relay configuration](#verify-the-dhcp-relay-configuration)
    - [Enable dhcp-relay globally](#enable-dhcp-relay-globally)
    - [Enable dhcp-relay hop count increment](#enable-dhcp-relay-hop-count-increment)
    - [Enable dhcp-relay option 82](#enable-dhcp-relay-option-82)
    - [Enable dhcp-relay option 82 validation](#enable-dhcp-relay-option-82-validation)
    - [Enable dhcp-relay option 82 validation with replace policy](#enable-dhcp-relay-option-82-validation-with-replace-policy)
    - [Enable dhcp-relay option 82 validation with drop policy](#enable-dhcp-relay-option-82-validation-with-drop-policy)
    - [Enable dhcp-relay option 82 validation with replace policy and remote ID](#enable-dhcp-relay-option-82-validation-with-replace-policy-and-remote-id)
    - [Enable dhcp-relay option 82 validation with drop policy and remote ID](#enable-dhcp-relay-option-82-validation-with-drop-policy-and-remote-id)
    - [Configure dhcp-relay option 82 with keep policy](#configure-dhcp-relay-option-82-with-keep-policy)
    - [Configure dhcp-relay option 82 with keep policy and mac remote ID](#configure-dhcp-relay-option-82-with-keep-policy-and-mac-remote-id)
    - [Configure dhcp-relay option 82 with keep policy and ip remote ID](#configure-dhcp-relay-option-82-with-keep-policy-and-mac-remote-id)
    - [Configure dhcp-relay option 82 with drop policy and validation](#configure-dhcp-relay-option-82-with-drop-policy-and-validation)
    - [Configure dhcp-relay option 82 with replace policy and validation](#configure-dhcp-relay-option-82-with-replace-policy-and-validation)
    - [Configure dhcp-relay option 82 drop policy with validation and remote ID](#configure-dhcp-relay-option-82-drop-policy-with-validation-and-remote-id)
    - [Configure dhcp-relay option 82 keep policy with validation and remote ID](#configure-dhcp-relay-option-82-replace-policy-with-validation-and-remote-id)
    - [Configure dhcp-relay option 82 policy](#configure-dhcp-relay-option-82-policy)
    - [Configure dhcp-relay option 82 remote ID](#configure-dhcp-relay-option-82-remote-id)
    - [Configure dhcp-relay option 82 remote ID](#configure-dhcp-relay-option-82-remote-id)
    - [Disable dhcp-relay globally](#disable-dhcp-relay-globally)
    - [Disable dhcp-relay hop count increment](#disable-dhcp-relay-hop-count-increment)
    - [Disable dhcp-relay option 82](#disable-dhcp-relay-option-82)
    - [Disable dhcp-relay option 82 validation](#disable-dhcp-relay-option-82-validation)
    - [Verify the dhcp-relay status](#verify-the-dhcp-relay-status)
    - [Verify the dhcp-relay configuration status in show-running](#verify-the-dhcp-relay-configuration-status-in-show-running)
    - [Verify dhcp-relay post reboot](#verify-dhcp-relay-post-reboot)
- [Verify relay destination address configuration](#verify-relay-destination-address-configuration)
    - [Verify the helper address configuration for unicast server IP](#verify-the-helper-address-configuration-for-unicast-server-ip)
    - [Verify the helper address configuration for multicast server IP](#verify-the-helper-address-configuration-for-multicast-server-ip)
    - [Verify the helper address configuration for broadcast server IP](#verify-the-helper-address-configuration-for-broadcast-server-ip)
    - [Verify the helper address configuration for loopback server IP on an interface](#verify-the-helper-address-configuration-for-loopback-server-ip-on-an-interface)
    - [Verify the helper address configuration for invalid server IP on an interface](#verify-the-helper-address-configuration-for-invalid-server-ip-on-an-interface)
    - [Verify the helper address configuration on an interface when IP routing is disabled](#verify-the-helper-address-configuration-on-an-interface-when-ip-routing-is-disabled)
    - [Verify the helper address configuration on a sub-interface](#verify-the-helper-address-configuration-on-a-sub-interface)
    - [Verify the helper address configuration on a split-interface](#verify-the-helper-address-configuration-on-a-split-interface)
    - [Verify the helper address configuration on a vlan-interface](#verify-the-helper-address-configuration-on-a-vlan-interface)
    - [Verify the helper address configuration when dhcp-server is enabled](#verify-the-helper-address-configuration-when-dhcp-server-is-enabled)
    - [Verify the dhcp-server configuration when the helper address is configured](#verify-the-dhcp-server-configuration-when-the-helper-address-is-configured)
    - [Verify the helper address configuration post reboot](#verify-the-helper-address-configuration-post-reboot)
    - [Verify the multiple helper address configurations on a single interface](#verify-the-multiple-helper-address-configurations-on-a-single-interface)
    - [Verify the single helper address configuration on different interfaces](#verify-the-single-helper-address-configuration-on-different-interfaces)
    - [Verify the maximum helper address configurations per interface](#verify-the-maximum-helper-address-configurations-per-interface)
    - [Verify the helper address record duplication](#verify-the-helper-address-record-duplication)
    - [Verify the helper address status](#verify-the-helper-address-status)
    - [Verify the helper address status on the specified interface](#verify-the-helper-address-status-on-the-specified-interface)
    - [Verify the helper address configuration status in show-running](#verify-the-helper-address-configuration-status-in-show-running)
    - [Verify the helper address configuration status on the specified interface in show-running](#verify-the-helper-address-configuration-status-on-the-specified-interface-in-show-running)
- [Verify relay bootp gateway configuration](#verify-relay-bootp-gateway-configuration)
    - [Verify the bootp gateway configuration for configured IP on an interface](#verify-the-bootp-gateway-configuration-for-invalid-server-ip-on-an-interface)
    - [Verify the bootp gateway configuration for unconfigured IP on an interface](#verify-the-bootp-gateway-configuration-for-invalid-server-ip-on-an-interface)
    - [Verify the bootp gateway configuration for invalid IP on an interface](#verify-the-bootp-gateway-configuration-for-invalid-server-ip-on-an-interface)
    - [Verify the bootp gateway status](#verify-the-bootp-gateway-status)
    - [Verify the bootp gateway status on the specified interface](#verify-the-bootp-gateway-status-on-the-specified-interface)
    - [Verify the bootp gateway configuration status in show-running](#verify-the-bootp-gateway-configuration-status-in-show-running)
    - [Verify the bootp gateway configuration status on the specified interface in show-running](#verify-the-bootp-gateway-configuration-status-on-the-specified-interface-in-show-running)
## Verify the dhcp-relay configuration
### Objective
To verify if the dhcp-relay configuration is enabled/disabled.
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

### Enable dhcp-relay globally
### Description
Enable the dhcp-relay on dut01 using the `dhcp-relay` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay enable state is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay enable state is not reflected in the `show dhcp-relay` command.

### Enable dhcp-relay hop count increment
### Description
Enable the dhcp-relay hop count increment on dut01 using the `dhcp-relay hop-count-increment` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay hop count increment enable state is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay hop count increment enable state is not reflected in the `show dhcp-relay` command.

### Enable dhcp-relay option 82
### Description
Enable the dhcp-relay option 82 on dut01 using the `dhcp-relay option 82 replace` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 enable state is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 enable state is not reflected in the `show dhcp-relay` command.

### Enable dhcp-relay option 82 validation
### Description
Enable the dhcp-relay option 82 validation on dut01 using the `dhcp-relay option 82 validate` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 validation enable state is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 validation enable state is not reflected in the `show dhcp-relay` command.

### Enable dhcp-relay option 82 validation with replace policy
### Description
Enable the dhcp-relay option 82 validation with replace policy on dut01 using the `dhcp-relay option 82 validate replace` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 validation enable state and replace policy are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 validation enable state and replace policy are not reflected in the `show dhcp-relay` command.

### Enable dhcp-relay option 82 validation with drop policy
### Description
Enable the dhcp-relay option 82 validation with drop policy on dut01 using the `dhcp-relay option 82 validate drop` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 validation enable state and drop policy are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 validation enable state and drop policy are not reflected in the `show dhcp-relay` command.

### Enable dhcp-relay option 82 validation with replace policy and mac remote ID
### Description
Enable the dhcp-relay option 82 validation with replace policy and mac remote ID on dut01 using the `dhcp-relay option 82 validate replace mac` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 validation enable state, replace policy and mac remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 validation enable state, replace policy and mac remote ID are not reflected in the `show dhcp-relay` command.

### Enable dhcp-relay option 82 validation with replace policy and ip remote ID
### Description
Enable the dhcp-relay option 82 validation with replace policy and ip remote ID on dut01 using the `dhcp-relay option 82 validate replace ip` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 validation enable state, replace policy and ip remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 validation enable state, replace policy and ip remote ID are not reflected in the `show dhcp-relay` command.

### Enable dhcp-relay option 82 validation with drop policy and mac remote ID
### Description
Enable the dhcp-relay option 82 validation with drop policy and mac remote ID on dut01 using the `dhcp-relay option 82 validate drop mac` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 validation enable state, drop policy and mac remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 validation enable state, drop policy and mac remote ID are not reflected in the `show dhcp-relay` command.

### Enable dhcp-relay option 82 validation with drop policy and ip remote ID
### Description
Enable the dhcp-relay option 82 validation with drop policy and ip remote ID on dut01 using the `dhcp-relay option 82 validate drop ip` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 validation enable state, drop policy and ip remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 validation enable state, drop policy and ip remote ID are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with keep policy
### Description
Configure the dhcp-relay option 82 with keep policy on dut01 using the `dhcp-relay option 82 keep` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with keep policy is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with keep policy is not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with keep policy and mac remote ID
### Description
Configure the dhcp-relay option 82 with keep policy and mac remote ID on dut01 using the `dhcp-relay option 82 keep mac` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with keep policy and mac remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with keep policy and mac remote ID are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with keep policy and ip remote ID
### Description
Configure the dhcp-relay option 82 with keep policy and ip remote ID on dut01 using the `dhcp-relay option 82 keep ip` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with keep policy and ip remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with keep policy and ip remote ID are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with replace policy
### Description
Configure the dhcp-relay option 82 with replace policy on dut01 using the `dhcp-relay option 82 replace` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with replace policy is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with replace policy is not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with replace policy and validation
### Description
Configure the dhcp-relay option 82 with replace policy and validation on dut01 using the `dhcp-relay option 82 replace validation` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with replace policy and validation enable state are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with replace policy and validation enable state are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 replace policy with validation and mac remote ID
### Description
Configure the dhcp-relay option 82 replace policy with validation and mac remote ID on dut01 using the `dhcp-relay option 82 replace validation` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with replace policy, validation enable state and mac remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with replace policy validation enable state and mac remote ID are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 replace policy with validation and ip remote ID
### Description
Configure the dhcp-relay option 82 replace policy with validation and ip remote ID on dut01 using the `dhcp-relay option 82 replace validation` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with replace policy, validation enable state and ip remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with replace policy validation enable state and ip remote ID are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with drop policy
### Description
Configure the dhcp-relay option 82 with drop policy on dut01 using the `dhcp-relay option 82 drop` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with drop policy is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with drop policy is not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with drop policy and validation
### Description
Configure the dhcp-relay option 82 with drop policy and validation on dut01 using the `dhcp-relay option 82 drop validation` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with drop policy and validation enable state are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with drop policy and validation enable state are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 drop policy with validation and mac remote ID
### Description
Configure the dhcp-relay option 82 drop policy with validation and mac remote ID on dut01 using the `dhcp-relay option 82 drop validation mac` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with drop policy, validation enable state and mac remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with drop policy validation enable state and mac remote ID are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 drop policy with validation and ip remote ID
### Description
Configure the dhcp-relay option 82 drop policy with validation and ip remote ID on dut01 using the `dhcp-relay option 82 drop validation ip` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with drop policy, validation enable state and ip remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with drop policy validation enable state and ip remote ID are not reflected in the `show dhcp-relay` command.

### Disable dhcp-relay globally
### Description
Disable the dhcp-relay on dut01 using the `no dhcp-relay` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay disable state is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay disable state is not reflected in the `show dhcp-relay` command.

### Disable dhcp-relay hop count increment
### Description
Disable the dhcp-relay hop count increment on dut01 using the `no dhcp-relay hop-count-increment` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay hop count increment disable state is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay hop count increment disable state is not reflected in the `show dhcp-relay` command.

### Disable dhcp-relay option 82
### Description
Disable the dhcp-relay option 82 on dut01 using the `no dhcp-relay option 82` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 disable state is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 disable state is not reflected in the `show dhcp-relay` command.

### Disable dhcp-relay option 82 validation
### Description
Disable the dhcp-relay option 82 validation on dut01 using the `no dhcp-relay option 82 validate` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 validation disable state is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 validation disable state is not reflected in the `show dhcp-relay` command.

### Verify the dhcp-relay configuration status in show-running
### Description
Verify the dhcp-relay configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
The user verifies the current dhcp-relay configuration status is reflected in the `show running-configuration` command.
#### Test fail criteria
The user verifies the current dhcp-relay configuration status is not reflected in the `show running-configuration` command.

### Verify the dhcp-relay status
### Description
Verify the dhcp-relay status using the `show dhcp-relay` command.
### Test result criteria
#### Test pass criteria
The user verifies the current dhcp-relay status is reflected in the `show dhcp-relay` command.
#### Test fail criteria
The user verifies the current dhcp-relay status is not reflected in the `show dhcp-relay` command.

### Verify dhcp-relay post reboot
### Description
Verify the dhcp-relay configuration is retained after the device reboots.
### Test result criteria
#### Test pass criteria
The dhcp-relay configuration is retained post reboot.
#### Test fail criteria
The dhcp-relay configuration is not retained post reboot.

## Verify relay destination address configuration
### Objective
To configure a helper address on an interface.
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

### Verify the helper address configuration for unicast server IP
### Description
Verify the helper address configuration with a unicast server IP on an intrerface.
### Test result criteria
#### Test pass criteria
The user is able to configure the helper address with a unicast server IP on an interface.
#### Test fail criteria
The user is unable to configure the helper address with a unicast server IP on an interface.

### Verify the helper address configuration for multicast server IP
### Description
Verify the helper address configuration with a multicast server IP on an intrerface.
### Test result criteria
#### Test pass criteria
The user is unable to configure the helper address, an error is displayed on the console.
#### Test fail criteria
The user is able to configure the helper address with a multicast server IP on an interface.

### Verify the helper address configuration for broadcast server IP
### Description
Verify the helper address configuration with a broadcast server IP on an intrerface.
### Test result criteria
#### Test pass criteria
The user is unable to configure the helper address, and an error is displayed on the console.
#### Test fail criteria
The user is able to configure the helper address with a broadcast server IP on an interface.

### Verify the helper address configuration for invalid server IP on an interface
### Description
Verify the helper address configuration with server IP as 0.0.0.0 on an interface.
### Test result criteria
#### Test pass criteria
The user is unable to configure the helper address, and an error is displayed on the console.
#### Test fail criteria
The user is able to configure the helper address on an interface.

### Verify the helper address configuration for loopback server IP on an interface
### Description
Verify the helper address configuration with loopback server IP on an interface.
### Test result criteria
#### Test pass criteria
The user is unable to configure the helper address, and an error is displayed on the console.
#### Test fail criteria
The user is able to configure the helper address on an interface.

### Verify the helper address configuration on an interface when IP routing is disabled
### Description
Disable the IP routing on an interface of dut01 when a helper address is configured.
### Test result criteria
#### Test pass criteria
The user verifies that the helper address configurations on the interface are not present.
#### Test fail criteria
The user verifies that the helper address configurations on the interface are present.

### Verify the helper address configuration on a sub-interface
### Description
Verify the helper address configuration on a sub-interface.
### Test result criteria
#### Test pass criteria
The user is able to configure the helper address on a sub-interface.
#### Test fail criteria
The user is unable to configure the helper address on a sub-interface.

### Verify the helper address configuration on a split-interface
### Description
Verify the helper address configuration on a split-interface.
### Test result criteria
#### Test pass criteria
The user is able to configure the helper address on a split-interface.
#### Test fail criteria
The user is unable to configure the helper address on a split-interface.

### Verify the helper address configuration on a vlan-interface
### Description
Verify the helper address configuration on a vlan-interface.
### Test result criteria
#### Test pass criteria
The user is able to configure the helper address on a vlan-interface.
#### Test fail criteria
The user is unable to configure the helper address on a vlan-interface.

### Verify the helper address configuration when dhcp-server is enabled
### Description
Configure the helper address on an interface of dut01 when dhcp-server is enabled.
### Test result criteria
#### Test pass criteria
The user verifies that the helper address is not configured on the interface.
#### Test fail criteria
The user verifies that the helper address is configured on the interface.

### Verify the dhcp-server configuration when the helper address is configured
### Description
Enable the dhcp-server on an interface of dut01 when a helper address is configured.
### Test result criteria
#### Test pass criteria
The user verifies that the dhcp-server is not enabled on the interface.
#### Test fail criteria
The user verifies that the dhcp-server is enabled on the interface.

### Verify the helper address configuration post reboot
### Description
Verify the helper address configuration is retained after the device reboots.
### Test result criteria
#### Test pass criteria
The helper address configurations is retained post reboot.
#### Test fail criteria
The helper address configurations is not retained post reboot.

### Verify the multiple helper address configurations on a single interface
### Description
Verify the multiple helper address configurations on the same interface of dut01.
### Test result criteria
#### Test pass criteria
The user is able to configure the multiple helper address on a single interface.
#### Test fail criteria
The user is unable to configure multiple helper addresses on a single interface.

### Verify the single helper address configuration on different interfaces
### Description
Verify the single helper address on different interfaces of dut01.
### Test result criteria
#### Test pass criteria
The user is able to configure a single helper address on different interfaces.
#### Test fail criteria
The user is unable to configure a single helper address on different interfaces.

### Verify the maximum helper address configurations per interface
### Description
Verify the maximum (16) helper address configurations per interface.
### Test result criteria
#### Test pass criteria
The user is able to configure a maximum of 16 helper addresses on a single interface.
#### Test fail criteria
The user is unable to configure a maximum of 16 helper addresses on a single interface.

### Verify the helper address record duplication
### Description
Verify the helper address record duplication by configuring the same helper address on the same interface multiple times.
### Test result criteria
#### Test pass criteria
The user is unable to configure the same helper address on the interface multiple times.
#### Test fail criteria
The user is able to configure the same helper address on the interface multiple times.

### Verify the helper address status
### Description
Verify the helper address status on an interface using the `show ip helper-address` command.
### Test result criteria
#### Test pass criteria
The user is able to verify the helper address status on an interface.
#### Test fail criteria
The user is unable to to verify the helper address status on an interface.

### Verify the helper address status on the specified interface
### Description
Verify the helper address status on the specified interface using the `show ip helper-address interface IFNAME` command.
### Test result criteria
#### Test pass criteria
The user is able to verify the helper address status on the specified interface.
#### Test fail criteria
The user is unable to to verify the helper address status on the specified interface.

### Verify the helper address configuration status in show-running
### Description
Verify the helper address configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
The user verifies the current helper address configuration status is reflected in the `show` command.
#### Test fail criteria
The user verifies the current helper address configuration status is not reflected in the `show` command.

### Verify the helper address configuration status on the specified interface in show-running
### Description
Verify the helper address configuration status on the specified interface using the `show running-configuration interface IFNAME` command.
### Test result criteria
#### Test pass criteria
The user verifies the current helper address configuration status on the specified interface is reflected in the `show` command.
#### Test fail criteria
The user verifies the current helper address configuration status on the specified interface is not reflected in the `show` command.

## Verify relay bootp gateway configuration
### Objective
To configure a bootp gateway on an interface.
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
### Verify the bootp gateway configuration for configured IP on an interface
### Description
Verify the bootp gateway configuration with a configured IP on an interface.
### Test result criteria
#### Test pass criteria
The user is able to configure the bootp gateway with a configured IP on an interface.
#### Test fail criteria
The user is unable to configure the bootp gateway with a configured IP on an interface.

### Verify the bootp gateway unconfigured for configured IP on an interface
### Description
Verify the bootp gateway unconfigured with a configured IP on an interface.
### Test result criteria
#### Test pass criteria
The user is unable to configure the bootp gateway with a configured IP on an interface.
#### Test fail criteria
The user is able to configure the bootp gateway with a configured IP on an interface.

### Verify the bootp gateway status
### Description
Verify the bootp gateway status on an interface using the `show dhcp-relay bootp-gateway` command.
### Test result criteria
#### Test pass criteria
The user is able to verify the bootp gateway status on an interface.
#### Test fail criteria
The user is unable to to verify the bootp gateway status on an interface.

### Verify the bootp gateway status on the specified interface
### Description
Verify the bootp gateway status on the specified interface using the `show dhcp-relay bootp-gateway interface IFNAME` command.
### Test result criteria
#### Test pass criteria
The user is able to verify the bootp gateway status on the specified interface.
#### Test fail criteria
The user is unable to to verify the bootp gateway status on the specified interface.

### Verify the bootp-gateway configuration status in show-running
### Description
Verify the helper address configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
The user verifies the current helper address configuration status is reflected in the `show` command.
#### Test fail criteria
The user verifies the current helper address configuration status is not reflected in the `show` command.

### Verify the bootp-gateway configuration status on the specified interface in show-running
### Description
Verify the helper address configuration status on the specified interface using the `show running-configuration interface IFNAME` command.
### Test result criteria
#### Test pass criteria
The user verifies the current bootp-gateway configuration status on the specified interface is reflected in the `show` command.
#### Test fail criteria
The user verifies the current bootp-gateway configuration status on the specified interface is not reflected in the `show` command.