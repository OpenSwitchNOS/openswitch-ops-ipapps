# DHCP Relay Test Cases

## Contents
- [Verify the dhcp-relay configuration](#verify-the-dhcp-relay-configuration)
    - [Enable dhcp-relay globally](#enable-dhcp-relay-globally)
    - [Enable dhcp-relay hop-count-increment](#enable-dhcp-relay-hop-count-increment)
    - [Enable dhcp-relay option 82](#enable-dhcp-relay-option-82)
    - [Enable dhcp-relay option 82 validation](#enable-dhcp-relay-option-82-validation)
    - [Enable dhcp-relay option 82 validation with replace policy](#enable-dhcp-relay-option-82-validation-with-replace-policy)
    - [Enable dhcp-relay option 82 validation with replace policy and mac remote ID](#enable-dhcp-relay-option-82-validation-with-replace-policy-and-mac-remote-id)
    - [Enable dhcp-relay option 82 validation with replace policy and ip remote ID](#enable-dhcp-relay-option-82-validation-with-replace-policy-and-ip-remote-id)
    - [Enable dhcp-relay option 82 validation with drop policy](#enable-dhcp-relay-option-82-validation-with-drop-policy)
    - [Enable dhcp-relay option 82 validation with drop policy and mac remote ID](#enable-dhcp-relay-option-82-validation-with-drop-policy-and-mac-remote-id)
    - [Enable dhcp-relay option 82 validation with drop policy and ip remote ID](#enable-dhcp-relay-option-82-validation-with-drop-policy-and-ip-remote-id)
    - [Configure dhcp-relay option 82 with keep policy](#configure-dhcp-relay-option-82-with-keep-policy)
    - [Configure dhcp-relay option 82 with keep policy and mac remote ID](#configure-dhcp-relay-option-82-with-keep-policy-and-mac-remote-id)
    - [Configure dhcp-relay option 82 with keep policy and ip remote ID](#configure-dhcp-relay-option-82-with-keep-policy-and-mac-remote-id)
    - [Configure dhcp-relay option 82 with replace policy](#configure-dhcp-relay-option-82-with-replace-policy)
    - [Configure dhcp-relay option 82 with replace policy and mac remote ID](#configure-dhcp-relay-option-82-with-replace-policy-and-mac-remote-id)
    - [Configure dhcp-relay option 82 with replace policy and IP remote ID](#configure-dhcp-relay-option-82-with-replace-policy-and-mac-remote-id)
    - [Configure dhcp-relay option 82 replace policy with mac remote ID and validation](#configure-dhcp-relay-option-82-replace-policy-with-mac-remote-id-and-validation)
    - [Configure dhcp-relay option 82 replace policy with ip remote ID and validation](#configure-dhcp-relay-option-82-replace-policy-with-ip-remote-id-and-validation)
    - [Configure dhcp-relay option 82 with replace policy and validation](#configure-dhcp-relay-option-82-with-replace-policy-and-validation)
    - [Configure dhcp-relay option 82 replace policy with validation and mac remote ID](#configure-dhcp-relay-option-82-replace-policy-with-validation-and-mac-remote-id)
    - [Configure dhcp-relay option 82 replace policy with validation and IP remote ID](#configure-dhcp-relay-option-82-replace-policy-with-validation-and-ip-remote-id)
    - [Configure dhcp-relay option 82 with drop policy](#configure-dhcp-relay-option-82-with-drop-policy)
    - [Configure dhcp-relay option 82 with drop policy and mac remote ID](#configure-dhcp-relay-option-82-with-keep-policy-and-mac-remote-id)
    - [Configure dhcp-relay option 82 with drop policy and ip remote ID](#configure-dhcp-relay-option-82-with-keep-policy-and-mac-remote-id)
    - [Configure dhcp-relay option 82 drop policy with mac remote ID and validation](#configure-dhcp-relay-option-82-drop-policy-with-mac-remote-id-and-validation)
    - [Configure dhcp-relay option 82 drop policy with ip remote ID and validation](#configure-dhcp-relay-option-82-drop-policy-with-ip-remote-id-and-validation)
    - [Configure dhcp-relay option 82 with drop policy and validation](#configure-dhcp-relay-option-82-with-drop-policy-and-validation)
    - [Configure dhcp-relay option 82 drop policy with validation and mac remote ID](#configure-dhcp-relay-option-82-drop-policy-with-validation-and-mac-remote-id)
    - [Configure dhcp-relay option 82 drop policy with validation and IP remote ID](#configure-dhcp-relay-option-82-drop-policy-with-validation-and-ip-remote-id)
    - [Disable dhcp-relay globally](#disable-dhcp-relay-globally)
    - [Disable dhcp-relay hop-count-increment](#disable-dhcp-relay-hop-count-increment)
    - [Disable dhcp-relay option 82](#disable-dhcp-relay-option-82)
    - [Disable dhcp-relay option 82 validation](#disable-dhcp-relay-option-82-validation)
    - [Verify DHCP relay status](#verify-dhcp-relay-status)
    - [Verify DHCP relay configuration status in show running-configuration](#verify-dhcp-relay-configuration-status-in-show-running-configuration)
    - [Verify DHCP relay hop-count-increment status in show running-configuration](#verify-dhcp-relay-hop-count-increment-configuration-status-in-show-running-configuration)
    - [Verify DHCP relay option 82 with validation configuration status in show running-configuration](#verify-dhcp-relay-option-82-with-validation-configuration-status-in-show-running-configuration)
    - [Verify DHCP relay option 82 with drop policy configuration status in show running-configuration](#verify-dhcp-relay-option-82-with-drop-policy-configuration-status-in-show-running-configuration)
    - [Verify DHCP relay option 82 with replace policy configuration status in show running-configuration](#verify-dhcp-relay-option-82-with-replace-policy-configuration-status-in-show-running-configuration)
    - [Verify DHCP relay option 82 with keep policy configuration status in show running-configuration](#verify-dhcp-relay-option-82-with-keep-policy-configuration-status-in-show-running-configuration)
    - [Verify DHCP relay option 82 with keep policy and IP remote ID configuration status in show running-configuration](#verify-dhcp-relay-option-82-with-keep-policy-and-ip-remote-id-configuration-status-in-show-running-configuration)
    - [Verify DHCP relay option 82 with drop policy and IP remote ID configuration status in show running-configuration](#verify-dhcp-relay-option-82-with-drop-policy-and-ip-remote-id-configuration-status-in-show-running-configuration)
    - [Verify DHCP relay option 82 with replace policy and IP remote ID configuration status in show running-configuration](#verify-dhcp-relay-option-82-with-replace-policy-and-ip-remote-id-configuration-status-in-show-running-configuration)
    - [Verify DHCP relay option 82 with drop policy and validation configuration status in show running-configuration](#verify-dhcp-relay-option-82-with-drop-policy-and-validation-configuration-status-in-show-running-configuration)
    - [Verify DHCP relay option 82 with replace policy and validation configuration status in show running-configuration](#verify-dhcp-relay-option-82-with-replace-policy-and-validation-configuration-status-in-show-running-configuration)
    - [Verify DHCP relay option 82 drop policy with validation and IP remote ID configuration status in show running-configuration](#verify-dhcp-relay-option-82-drop-policy-with-validation-and_ip_remote_id_configuration-status-in-show-running-configuration)
    - [Verify DHCP relay option 82 replace policy with validation and IP remote ID configuration status in show running-configuration](#verify-dhcp-relay-option-82-replace-policy-with-validation-and_ip_remote_id_configuration-status-in-show-running-configuration)
    - [Verify DHCP relay post reboot](#verify-dhcp-relay-post-reboot)
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
    - [Verify the helper address configuration when DHCP server is enabled](#verify-the-helper-address-configuration-when-dhcp-server-is-enabled)
    - [Verify the helper address configuration post reboot](#verify-the-helper-address-configuration-post-reboot)
    - [Verify the multiple helper address configurations on a single interface](#verify-the-multiple-helper-address-configurations-on-a-single-interface)
    - [Verify the single helper address configuration on different interfaces](#verify-the-single-helper-address-configuration-on-different-interfaces)
    - [Verify the maximum helper address configurations per interface](#verify-the-maximum-helper-address-configurations-per-interface)
    - [Verify the helper address record duplication](#verify-the-helper-address-record-duplication)
    - [Verify the helper address status](#verify-the-helper-address-status)
    - [Verify the helper address when the BOOTP gateway status is unconfigured](#verify-the-helper-address-status-when-the-bootp-gateway-is-unconfigured)
    - [Verify the helper address status on the specified interface](#verify-the-helper-address-status-on-the-specified-interface)
    - [Verify the helper address configuration status in show running-configuration](#verify-the-helper-address-configuration-status-in-show-running-configuration)
    - [Verify the helper address configuration status on the specified interface in show running-configuration](#verify-the-helper-address-configuration-status-on-the-specified-interface-in-show-running-configuration)
- [Verify relay BOOTP gateway configuration](#verify-relay-bootp-gateway-configuration)
    - [Verify the BOOTP gateway configuration for configured IP on an interface](#verify-the-bootp-gateway-configuration-for-configured-ip-on-an-interface)
    - [Verify the BOOTP gateway configuration for configured IP on a sub-interface](#verify-the-bootp-gateway-configuration-for-configured-ip-on-a-sub-interface)
    - [Verify the BOOTP gateway configuration for configured secondary IP on an interface](#verify-the-bootp-gateway-configuration-for-configured-secondary-ip-on-an-interface)
    - [Verify the BOOTP gateway configuration for configured secondary IP on a sub-interface](#verify-the-bootp-gateway-configuration-for-configured-secondary-ip-on-a-sub-interface)
    - [Verify the BOOTP gateway configuration for unconfigured IP on an interface](#verify-the-bootp-gateway-configuration-for-unconfigured-ip-on-an-interface)
    - [Verify the BOOTP gateway unconfiguration for configured BOOTP gateway on an interface](#verify-the-bootp-gateway-unconfiguration-for-configured-bootp-gateway-on-an-interface)
    - [Verify the BOOTP gateway unconfiguration for configured IP on an interface](#verify-the-bootp-gateway-unconfiguration-for-configured-ip-on-an-interface)
    - [Verify the BOOTP gateway unconfiguration for unconfigured IP on an interface](#verify-the-bootp-gateway-unconfiguration-for-unconfigured-ip-on-an-interface)
    - [Verify the BOOTP gateway configuration for invalid IP](#verify-the-bootp-gateway-configuration-for-invalid-ip)
    - [Verify the BOOTP gateway status](#verify-the-bootp-gateway-status)
    - [Verify the BOOTP gateway status when the helper address is unconfigured](#verify-the-bootp-gateway-status-when-the-helper-address-is-unconfigured)
    - [Verify the BOOTP gateway status on the specified interface](#verify-the-bootp-gateway-status-on-the-specified-interface)
    - [Verify the BOOTP gateway configuration status in show running-configuration](#verify-the-bootp-gateway-configuration-status-in-show-running-configuration)
    - [Verify the BOOTP gateway configuration status on the specified interface in show running-configuration](#verify-the-bootp-gateway-configuration-status-on-the-specified-interface-in-show-running-configuration)
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

### Enable DHCP relay globally
### Description
Enable DHCP relay on dut01 using the `dhcp-relay` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay enable state is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay enable state is not reflected in the `show dhcp-relay` command.

### Enable dhcp-relay hop-count-increment
### Description
Enable the DHCP relay hop count increment on dut01 using the `dhcp-relay hop-count-increment` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay hop-count-increment enable state is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay hop-count-increment enable state is not reflected in the `show dhcp-relay` command.

### Enable dhcp-relay option 82
### Description
Enable the DHCP relay option 82 on dut01 using the `dhcp-relay option 82 (replace|drop|keep|validate)` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 enable state is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 enable state is not reflected in the `show dhcp-relay` command.

### Enable dhcp-relay option 82 validation
### Description
Enable the DHCP relay option 82 validation on dut01 using the `dhcp-relay option 82 validate` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 validation enable state is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 validation enable state is not reflected in the `show dhcp-relay` command.

### Enable dhcp-relay option 82 validation with replace policy
### Description
Enable the DHCP relay option 82 validation with replace policy on dut01 using the `dhcp-relay option 82 validate replace` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 validation enable state and replace policy are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 validation enable state and replace policy are not reflected in the `show dhcp-relay` command.

### Enable dhcp-relay option 82 validation with replace policy and mac remote ID
### Description
Enable the DHCP relay option 82 validation with replace policy and mac remote ID on dut01 using the `dhcp-relay option 82 validate replace mac` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 validation enable state, replace policy and mac remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 validation enable state, replace policy and mac remote ID are not reflected in the `show dhcp-relay` command.

### Enable dhcp-relay option 82 validation with replace policy and IP remote ID
### Description
Enable the DHCP relay option 82 validation with replace policy and IP remote ID on dut01 using the `dhcp-relay option 82 validate replace ip` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 validation enable state, replace policy and IP remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 validation enable state, replace policy and IP remote ID are not reflected in the `show dhcp-relay` command.

## Enable dhcp-relay option 82 validation with drop policy
### Description
Enable the DHCP relay option 82 validation with drop policy on dut01 using the `dhcp-relay option 82 validate drop` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 validation enable state and drop policy are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 validation enable state and drop policy are not reflected in the `show dhcp-relay` command.

### Enable dhcp-relay option 82 validation with drop policy and mac remote ID
### Description
Enable the DHCP relay option 82 validation with drop policy and mac remote ID on dut01 using the `dhcp-relay option 82 validate drop mac` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 validation enable state, drop policy and mac remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 validation enable state, drop policy and mac remote ID are not reflected in the `show dhcp-relay` command.

### Enable dhcp-relay option 82 validation with drop policy and IP remote ID
### Description
Enable the DHCP relay option 82 validation with drop policy and IP remote ID on dut01 using the `dhcp-relay option 82 validate drop ip` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 validation enable state, drop policy and IP remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 validation enable state, drop policy and IP remote ID are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with keep policy
### Description
Configure the DHCP relay option 82 with keep policy on dut01 using the `dhcp-relay option 82 keep` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with keep policy is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with keep policy is not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with keep policy and mac remote ID
### Description
Configure the DHCP relay option 82 with keep policy and mac remote ID on dut01 using the `dhcp-relay option 82 keep mac` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with keep policy and mac remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with keep policy and mac remote ID are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with keep policy and IP remote ID
### Description
Configure the DHCP relay option 82 with keep policy and IP remote ID on dut01 using the `dhcp-relay option 82 keep ip` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with keep policy and IP remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with keep policy and IP remote ID are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with replace policy
### Description
Configure the DHCP relay option 82 with replace policy on dut01 using the `dhcp-relay option 82 replace` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with replace policy is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with replace policy is not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with replace policy and mac remote ID
### Description
Configure the DHCP relay option 82 with replace policy and mac remote ID on dut01 using the `dhcp-relay option 82 replace mac` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with replace policy and mac remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with replace policy and mac remote ID are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 replace policy with mac remote ID and validation
### Description
Configure the DHCP relay option 82 replace policy with mac remote ID and validation on dut01 using the `dhcp-relay option 82 replace mac validation` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 replace policy with mac remote ID and validation enable state are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 replace policy with mac remote ID and validation enable state are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with replace policy and IP remote ID
### Description
Configure the DHCP relay option 82 with replace policy and IP remote ID on dut01 using the `dhcp-relay option 82 replace ip` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with replace policy and IP remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with replace policy and IP remote ID are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 replace policy with IP remote ID and validation
### Description
Configure the DHCP relay option 82 replace policy with IP remote ID and validation on dut01 using the `dhcp-relay option 82 replace ip validation` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 replace policy with IP remote ID and validation enable state are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 replace policy with IP remote ID and validation enable state are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with replace policy and validation
### Description
Configure the DHCP relay option 82 with replace policy and validation on dut01 using the `dhcp-relay option 82 replace validation` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 with replace policy and validation enable state are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 with replace policy and validation enable state are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 replace policy with validation and mac remote ID
### Description
Configure the DHCP relay option 82 replace policy with validation and mac remote ID on dut01 using the `dhcp-relay option 82 replace validation mac` command.
### Test result criteria
#### Test pass criteria
Verify that the dhcp-relay option 82 replace policy with validation enable state and mac remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that the dhcp-relay option 82 replace policy with validation enable state and mac remote ID are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 replace policy with validation and IP remote ID
### Description
Configure the DHCP relay option 82 replace policy with validation and IP remote ID on dut01 using the `dhcp-relay option 82 replace validation ip` command.
### Test result criteria
#### Test pass criteria
Verify that dhcp-relay option 82 with replace policy, validation enable state and IP remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that dhcp-relay option 82 with replace policy, validation enable state and IP remote ID are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with drop policy
### Description
Configure the DHCP relay option 82 with drop policy on dut01 using the `dhcp-relay option 82 drop` command.
### Test result criteria
#### Test pass criteria
Verify that dhcp-relay option 82 with drop policy is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that dhcp-relay option 82 with drop policy is not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with drop policy and mac remote ID
### Description
Configure the DHCP relay option 82 with drop policy and mac remote ID on dut01 using the `dhcp-relay option 82 drop mac` command.
### Test result criteria
#### Test pass criteria
Verify that dhcp-relay option 82 with drop policy and mac remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that dhcp-relay option 82 with drop policy and mac remote ID are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 drop policy with mac remote ID and validation
### Description
Configure the DHCP relay option 82 drop policy with mac remote ID and validation on dut01 using the `dhcp-relay option 82 drop mac validation` command.
### Test result criteria
#### Test pass criteria
Verify that dhcp-relay option 82 drop policy with mac remote ID and validation enable state are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that dhcp-relay option 82 drop policy with mac remote ID and validation enable state are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with drop policy and IP remote ID
### Description
Configure the DHCP relay option 82 with drop policy and IP remote ID on dut01 using the `dhcp-relay option 82 drop ip` command.
### Test result criteria
#### Test pass criteria
Verify that dhcp-relay option 82 with drop policy and IP remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that dhcp-relay option 82 with drop policy and IP remote ID are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 drop policy with IP remote ID and validation
### Description
Configure the DHCP relay option 82 drop policy with IP remote ID and validation on dut01 using the `dhcp-relay option 82 drop ip validation` command.
### Test result criteria
#### Test pass criteria
Verify that dhcp-relay option 82 drop policy with IP remote ID and validation enable state are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that dhcp-relay option 82 drop policy with IP remote ID and validation enable state are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 with drop policy and validation
### Description
Configure the DHCP relay option 82 with drop policy and validation on dut01 using the `dhcp-relay option 82 drop validation` command.
### Test result criteria
#### Test pass criteria
Verify that dhcp-relay option 82 with drop policy and validation enable state are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that dhcp-relay option 82 with drop policy and validation enable state are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 drop policy with validation and mac remote ID
### Description
Configure the DHCP relay option 82 drop policy with validation and mac remote ID on dut01 using the `dhcp-relay option 82 drop validation mac` command.
### Test result criteria
#### Test pass criteria
Verify that dhcp-relay option 82 drop policy with validation enable state and mac remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that dhcp-relay option 82 drop policy with validation enable state and mac remote ID are not reflected in the `show dhcp-relay` command.

### Configure dhcp-relay option 82 drop policy with validation and IP remote ID
### Description
Configure the DHCP relay option 82 drop policy with validation and IP remote ID on dut01 using the `dhcp-relay option 82 drop validation ip` command.
### Test result criteria
#### Test pass criteria
Verify that dhcp-relay option 82 drop policy with validation enable state and IP remote ID are reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that dhcp-relay option 82 drop policy with validation enable state and IP remote ID are not reflected in the `show dhcp-relay` command.

### Disable dhcp-relay globally
### Description
Disable the DHCP relay on dut01 using the `no dhcp-relay` command.
### Test result criteria
#### Test pass criteria
Verify that dhcp-relay disable state is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that dhcp-relay disable state is not reflected in the `show dhcp-relay` command.

### Disable dhcp-relay hop-count-increment
### Description
Disable the DHCP relay hop count increment on dut01 using the `no dhcp-relay hop-count-increment` command.
### Test result criteria
#### Test pass criteria
Verify that dhcp-relay hop-count-increment disable state is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that dhcp-relay hop-count-increment disable state is not reflected in the `show dhcp-relay` command.

### Disable dhcp-relay option 82
### Description
Disable the DHCP relay option 82 on dut01 using the `no dhcp-relay option 82` command.
### Test result criteria
#### Test pass criteria
Verify that dhcp-relay option 82 disable state is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that dhcp-relay option 82 disable state is not reflected in the `show dhcp-relay` command.

### Disable dhcp-relay option 82 validation
### Description
Disable the DHCP relay option 82 validation on dut01 using the `no dhcp-relay option 82 validate` command.
### Test result criteria
#### Test pass criteria
Verify that dhcp-relay option 82 validation disable state is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify that dhcp-relay option 82 validation disable state is not reflected in the `show dhcp-relay` command.

### Verify DHCP relay configuration status in show running-configuration
### Description
Verify the DHCP relay configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
Verify the current dhcp-relay configuration status is reflected in the `show running-configuration` command.
#### Test fail criteria
Verify the current dhcp-relay configuration status is not reflected in the `show running-configuration` command.

### Verify DHCP relay hop-count-increment configuration status in show running-configuration
### Description
Verify the DHCP relay hop-count-increment configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
Verify the current dhcp-relay hop-count-increment configuration status is reflected in the `show running-configuration` command.
#### Test fail criteria
Verify the current dhcp-relay hop-count-increment configuration status is not reflected in the `show running-configuration` command.

### Verify DHCP relay option 82 with validation configuration status in show running-configuration
### Description
Verify the DHCP relay option 82 with validation configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
Verify the current dhcp-relay option 82 with validation configuration status is reflected in the `show running-configuration` command.
#### Test fail criteria
Verify the current dhcp-relay option 82 with validation configuration status is not reflected in the `show running-configuration` command.

### Verify DHCP relay option 82 with drop policy configuration status in show running-configuration
### Description
Verify the DHCP relay option 82 with drop policy configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
Verify the current dhcp-relay option 82 with drop policy configuration status is reflected in the `show running-configuration` command.
#### Test fail criteria
Verify the current dhcp-relay option 82 with drop policy configuration status is not reflected in the `show running-configuration` command.

### Verify DHCP relay option 82 with keep policy configuration status in show running-configuration
### Description
Verify the DHCP relay option 82 with keep policy configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
Verify the current dhcp-relay option 82 with keep policy configuration status is reflected in the `show running-configuration` command.
#### Test fail criteria
Verify the current dhcp-relay option 82 with keep policy configuration status is not reflected in the `show running-configuration` command.

### Verify DHCP relay option 82 with replace policy configuration status in show running-configuration
### Description
Verify the DHCP relay option 82 with replace policy configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
Verify the current dhcp-relay option 82 with replace policy configuration status is reflected in the `show running-configuration` command.
#### Test fail criteria
Verify the current dhcp-relay option 82 with replace policy configuration status is not reflected in the `show running-configuration` command.

### Verify DHCP relay option 82 with keep policy and IP remote ID configuration status in show running-configuration
### Description
Verify the DHCP relay option 82 with keep policy and IP remote ID configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
Verify the current dhcp-relay option 82 with keep policy and IP remote ID configuration status are reflected in the `show running-configuration` command.
#### Test fail criteria
Verify the current dhcp-relay option 82 with keep policy and IP remote ID configuration status are not reflected in the `show running-configuration` command.

### Verify DHCP relay option 82 with replace policy and validation configuration status in show running-configuration
### Description
Verify the DHCP relay option 82 with replace policy and validation configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
Verify the current dhcp-relay option 82 with replace policy and validation enable state configuration status are reflected in the `show running-configuration` command.
#### Test fail criteria
Verify the current dhcp-relay option 82 with replace policy and validation enable state configuration status are not reflected in the `show running-configuration` command.

### Verify DHCP relay option 82 with drop policy and validation configuration status in show running-configuration
### Description
Verify the DHCP relay option 82 with drop policy and validation configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
Verify the current dhcp-relay option 82 with drop policy and validation enable state configuration status are reflected in the `show running-configuration` command.
#### Test fail criteria
Verify the current dhcp-relay option 82 with drop policy and validation enable state configuration status are not reflected in the `show running-configuration` command.

### Verify DHCP relay option 82 with replace policy and IP remote ID configuration status in show running-configuration
### Description
Verify the DHCP relay option 82 with replace policy and IP remote ID configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
Verify the current dhcp-relay option 82 with replace policy and IP remote ID configuration status are reflected in the `show running-configuration` command.
#### Test fail criteria
Verify the current dhcp-relay option 82 with replace policy and IP remote ID configuration status are not reflected in the `show running-configuration` command.

### Verify DHCP relay option 82 with drop policy and IP remote ID configuration status in show running-configuration
### Description
Verify the DHCP relay option 82 with drop policy and IP remote ID configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
Verify the current dhcp-relay option 82 with drop policy and IP remote ID configuration status are reflected in the `show running-configuration` command.
#### Test fail criteria
Verify the current dhcp-relay option 82 with drop policy and IP remote ID configuration status are not reflected in the `show running-configuration` command.

### Verify DHCP relay option 82 replace policy with validation and IP remote ID configuration status in show running-configuration
### Description
Verify the DHCP relay option 82 replace policy with validation and IP remote ID configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
Verify the current dhcp-relay option 82 replace policy with validation enable state and IP remote ID configuration status is reflected in the `show running-configuration` command.
#### Test fail criteria
Verify the current dhcp-relay option 82 replace policy with validation enable state and IP remote ID configuration status is not reflected in the `show running-configuration` command.

### Verify DHCP relay option 82 drop policy with validation and IP remote ID configuration status in show running-configuration
### Description
Verify the DHCP relay option 82 drop policy with validation and IP remote ID configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
Verify the current dhcp-relay option 82 drop policy with validation enable state and IP remote ID configuration status are reflected in the `show running-configuration` command.
#### Test fail criteria
Verify the current dhcp-relay option 82 drop policy with validation enable state and IP remote ID configuration status are not reflected in the `show running-configuration` command.

### Verify DHCP relay status
### Description
Verify the DHCP relay status using the `show dhcp-relay` command.
### Test result criteria
#### Test pass criteria
Verify the current dhcp-relay status is reflected in the `show dhcp-relay` command.
#### Test fail criteria
Verify the current dhcp-relay status is not reflected in the `show dhcp-relay` command.

### Verify DHCP relay post reboot
### Description
Verify the DHCP relay configuration is retained after the device reboots.
### Test result criteria
#### Test pass criteria
The DHCP relay configuration is retained post reboot.
#### Test fail criteria
The DHCP relay configuration is not retained post reboot.

## Verify relay destination address configuration
### Objective
To configure a helper address on an interface.
### Requirements
The requirements for this test case are:
 - Docker version 1.7 or above
 - Action switch Docker instance/physical hardware switch

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
The user is unable to configure the helper addressn, and an error is displayed on the console.
#### Test fail criteria
The user is able to configure the helper address with a multicast server IP on an interface.

### Verify the helper address configuration for broadcast server IP
### Description
Verify the helper address configuration with a broadcast server IP on an intrerface.
### Test result criteria
#### Test pass criteria
The user is able to configure the helper address with a broadcast server IP on an interface.
#### Test fail criteria
The user is unable to configure the helper address, and an error is displayed on the console.

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

### Verify the helper address configuration when DHCP server is enabled
### Description
Configure the helper address on an interface of dut01 when DHCP server is enabled.
### Test result criteria
#### Test pass criteria
The user verifies that the helper address is not configured on the interface.
#### Test fail criteria
The user verifies that the helper address is configured on the interface.

### Verify the DHCP server configuration when the helper address is configured
### Description
Enable the DHCP server on an interface of dut01 when a helper address is configured.
### Test result criteria
#### Test pass criteria
The user verifies that the DHCP server is not enabled on the interface.
#### Test fail criteria
The user verifies that the DHCP server is enabled on the interface.

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
Verify multiple helper address configurations on the same interface of dut01.
### Test result criteria
#### Test pass criteria
The user is able to configure multiple helper address on a single interface.
#### Test fail criteria
The user is unable to configure multiple helper addresses on a single interface.

### Verify a single helper address configuration on different interfaces
### Description
Verify a single helper address on different interfaces of dut01.
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

### Verify the helper address when the BOOTP gateway status is unconfigured
### Description
Verify the helper address status when the BOOTP gateway status is unconfigured on an interface using the `show ip helper-address` command.
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

### Verify the helper address configuration status in show running-configuration
### Description
Verify the helper address configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
The user verifies the current helper address configuration status is reflected in the `show` command.
#### Test fail criteria
The user verifies the current helper address configuration status is not reflected in the `show` command.

### Verify the helper address configuration status on the specified interface in show running-configuration
### Description
Verify the helper address configuration status on the specified interface using the `show running-configuration interface IFNAME` command.
### Test result criteria
#### Test pass criteria
The user verifies the current helper address configuration status on the specified interface is reflected in the `show` command.
#### Test fail criteria
The user verifies the current helper address configuration status on the specified interface is not reflected in the `show` command.

## Verify relay BOOTP gateway configuration
### Objective
To configure a BOOTP gateway on an interface.
### Requirements
The requirements for this test case are:
 - Docker version 1.7 or above
 - Action switch Docker instance/physical hardware switch

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
### Verify the BOOTP gateway configuration for configured IP on an interface
### Description
Verify the BOOTP gateway configuration with a configured IP on an interface.
### Test result criteria
#### Test pass criteria
The user is able to configure the BOOTP gateway with a configured IP on an interface.
#### Test fail criteria
The user is unable to configure the BOOTP gateway with a configured IP on an interface.

### Verify the BOOTP gateway configuration for configured IP on a sub-interface
### Description
Verify the BOOTP gateway configuration with a configured IP on a sub-interface.
### Test result criteria
#### Test pass criteria
The user is able to configure the BOOTP gateway with a configured IP on a sub-interface.
#### Test fail criteria
The user is unable to configure the BOOTP gateway with a configured IP on a sub-interface.

### Verify the BOOTP gateway configuration for configured secondary IP on an interface
### Description
Verify the BOOTP gateway configuration with a configured secondary IP on an interface.
### Test result criteria
#### Test pass criteria
The user is able to configure the BOOTP gateway with a configured secondary IP on an interface.
#### Test fail criteria
The user is unable to configure the BOOTP gateway with a configured secondary IP on an interface.

### Verify the BOOTP gateway configuration for configured secondary IP on a sub-interface
### Description
Verify the BOOTP gateway configuration with a configured secondary IP on a sub-interface.
### Test result criteria
#### Test pass criteria
The user is able to configure the BOOTP gateway with a secondary configured IP on a sub-interface.
#### Test fail criteria
The user is unable to configure the BOOTP gateway with a secondary configured IP on a sub-interface.

### Verify the BOOTP gateway configuration for unconfigured IP on an interface
### Description
Verify the BOOTP gateway configuration with a unconfigured IP on an interface.
### Test result criteria
#### Test pass criteria
The user is unable to configure the BOOTP gateway with a unconfigured IP on an interface.
#### Test fail criteria
The user is able to configure the BOOTP gateway with a unconfigured IP on an interface.

### Verify the BOOTP gateway configuration for invalid IP
### Description
Verify the BOOTP gateway configuration with an invalid IP.
### Test result criteria
#### Test pass criteria
The user is unable to configure the BOOTP gateway with an invalid IP.
#### Test fail criteria
The user is able to configure the BOOTP gateway with an invalid IP.

### Verify the BOOTP gateway unconfiguration for configured BOOTP gateway on an interface
### Description
Verify the BOOTP gateway unconfigured with a configured BOOTP gateway on an interface.
### Test result criteria
#### Test pass criteria
The user is able to unconfigure the BOOTP gateway with a configured BOOTP gateway on an interface.
#### Test fail criteria
The user is unable to unconfigure the BOOTP gateway with a configured BOOTP gateway on an interface.

### Verify the BOOTP gateway unconfiguration for configured IP on an interface
### Description
Verify the BOOTP gateway unconfigured with a configured IP on an interface.
### Test result criteria
#### Test pass criteria
The user is unable to unconfigure the BOOTP gateway with a configured IP on an interface.
#### Test fail criteria
The user is able to unconfigure the BOOTP gateway with a configured IP on an interface.

### Verify the BOOTP gateway unconfiguration for unconfigured IP on an interface
### Description
Verify the BOOTP gateway unconfigured with a configured IP on an interface.
### Test result criteria
#### Test pass criteria
The user is unable to unconfigure the BOOTP gateway with a unconfigured IP on an interface.
#### Test fail criteria
The user is able to unconfigure the BOOTP gateway with a unconfigured IP on an interface.

### Verify the BOOTP gateway status
### Description
Verify the BOOTP gateway status on an interface using the `show dhcp-relay bootp-gateway` command.
### Test result criteria
#### Test pass criteria
The user is able to verify the BOOTP gateway status on an interface.
#### Test fail criteria
The user is unable to to verify the BOOTP gateway status on an interface.

### Verify the BOOTP gateway status when the helper address is unconfigured
### Description
Verify the BOOTP gateway status when the helper address is unconfigured on an interface using the `show dhcp-relay bootp-gateway` command.
### Test result criteria
#### Test pass criteria
The user is able to verify the BOOTP gateway status on an interface.
#### Test fail criteria
The user is unable to to verify the BOOTP gateway status on an interface.

### Verify the BOOTP gateway status on the specified interface
### Description
Verify the BOOTP gateway status on the specified interface using the `show dhcp-relay bootp-gateway interface IFNAME` command.
### Test result criteria
#### Test pass criteria
The user is able to verify the BOOTP gateway status on the specified interface.
#### Test fail criteria
The user is unable to to verify the BOOTP gateway status on the specified interface.

### Verify the BOOTP gateway configuration status in show running-configuration
### Description
Verify the BOOTP gateway configuration status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
The user verifies the current BOOTP gateway configuration status is reflected in the `show` command.
#### Test fail criteria
The user verifies the current BOOTP gateway configuration status is not reflected in the `show` command.

### Verify the BOOTP gateway configuration status on the specified interface in show running-configuration
### Description
Verify the BOOTP gateway configuration status on the specified interface using the `show running-configuration interface IFNAME` command.
### Test result criteria
#### Test pass criteria
The user verifies the current BOOTP gateway configuration status on the specified interface is reflected in the `show` command.
#### Test fail criteria
The user verifies the current BOOTP gateway configuration status on the specified interface is not reflected in the `show` command.