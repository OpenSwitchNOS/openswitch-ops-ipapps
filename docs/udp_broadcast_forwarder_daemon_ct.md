# UDP Broadcast Forwarder Daemon Test Cases

## Contents
- [Verify the UDP broadcast forwarder configuration](#verify-the-udp-broadcast-forwarder-configuration)
    - [Enable UDP broadcast forwarding globally](#enable-udp-broadcast-forwarding-globally)
    - [Disable UDP broadcast forwarding globally](#disable-udp-broadcast-forwarding-globally)
- [Verify UDP forward protocol configurations](#verify-udp-forward-protocol-configurations)
    - [Verify the UDP forward protocol configuration on a specific interface](#verify-the-udp-forward-protocol-configuration-on-a-specific-interface)
    - [Verify the UDP forward protocol configuration for multiple UDP ports on a specific interface](#verify-the-udp-forward-protocol-configuration-for-multiple-udp-ports-on-a-specific-interface)
    - [Verify the UDP forward protocol configurations for multiple IP addresses for a single UDP port on a specific interface](#verify-the-udp-forward-protocol-configurations-for-multiple-ip-addresses-for-a-single-udp-port-on-a-specific-interface)
    - [Verify the UDP forward protocol configuration for a subnet broadcast IP address on a specific interface](#verify-the-udp-forward-protocol-configuration-for-a-subnet-broadcast-ip-address-on-a-specific-interface)
    - [Verify the UDP forward protocol configuration on the same interface](#verify-the-udp-forward-protocol-configurations-on-the-same-interface)
    - [Verify a single UDP forward protocol configuration on multiple interfaces](#verify-a-single-udp-forward-protocol-configuration-on-multiple-interfaces)
    - [Verify the UDP forward protocol configuration post reboot](#verify-the-udp-forward-protocol-configuration-post-reboot)
    - [Verify the maximum UDP forward protocol configurations per interface](#verify-the-maximum-udp-forward-protocol-configurations-per-interface)
    - [Verify the UDP forward protocol servers configured count per interface](#verify-the-udp-forward-protocol-servers-configured-count-per-interface)
    - [Verify the UDP forward protocol servers count per UDP port on an interface](#verify-the-udp-forward-protocol-servers-count-per-udp-port-on-an-interface)
    - [Verify the UDP forward protocol configurations along with the DHCP relay helper address configuration on an interface](#verify-the-udp-forward-protocol-configurations-along-with-the-dhcp-relay-helper-address-configuration-on-an-interface)


## Verify the UDP broadcast forwarder configuration
### Objective
To verify the UDP broadcast forwarding configuration is enabled/disabled.

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

### Enable UDP broadcast forwarding globally
### Description
Enable the UDP broadcast forwarding on dut01 using the `ip udp-bcast-forward` command.
### Test result criteria
#### Test pass criteria
The user verifies the UDP broadcast forwarding state as true using the unixctl dump output.
#### Test fail criteria
The user verifies the UDP broadcast forwarding state as false using the unixctl dump output.

### Disable UDP broadcast forwarding globally
### Description
Disable the UDP broadcast forwarding on dut01 using the `no ip udp-bcast-forward` command.
### Test result criteria
#### Test pass criteria
The user verifies the UDP broadcast forwarding state as false using the unixctl dump output.
#### Test fail criteria
The user verifies the UDP broadcast forwarding state as true using the unixctl dump output.

## Verify UDP forward protocol configurations
### Objective
To verify that the UDP forward protocol configurations are properly updated in the daemon's local cache.
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

### Verify the UDP forward protocol configuration on a specific interface
### Description
Configure the UDP forward protocol on an interface using appropriate CLI commands. Verify that the unixctl dump output of an interface displays the UDP forward protocol.
### Test result criteria
#### Test pass criteria
The user verifies that the unixctl dump output displays the configured UDP forward protocol.
#### Test fail criteria
The user verifies that the unixctl dump output does not display the configured UDP forward protocol.

### Verify the UDP forward protocol configuration for multiple UDP ports on a specific interface
### Description
Configure the UDP forward protocol on an interface for multiple UDP ports using appropriate CLI commands. Verify that the unixctl dump output of the interface displays the UDP forward protocol.
### Test result criteria
#### Test pass criteria
The user verifies that the unixctl dump output displays the configured UDP forward protocol.
#### Test fail criteria
The user verifies that the unixctl dump output does not display the configured UDP forward protocol.

### Verify the UDP forward protocol configurations for multiple IP addresses for a single UDP port on a specific interface
### Description
Configure the UDP forward protocol on an interface for multiple IP addresses for a single UDP port using appropriate CLI commands. Verify that the unixctl dump output of the interface displays the UDP forward protocol.
### Test result criteria
#### Test pass criteria
The user verifies that the unixctl dump output displays the configured UDP forward protocol.
#### Test fail criteria
The user verifies that the unixctl dump output does not display the configured UDP forward protocol.

### Verify the UDP forward protocol configuration for a subnet broadcast IP address on a specific interface
### Description
Configure the UDP forward protocol on an interface for a subnet broadcast IP using appropriate CLI commands. Verify that the unixctl dump output of the interface displays the UDP forward protocol.
### Test result criteria
#### Test pass criteria
The user verifies that the unixctl dump output displays the configured UDP forward protocol.
#### Test fail criteria
The user verifies that the unixctl dump output does not display the configured UDP forward protocol.

### Verify the UDP forward protocol configurations on the same interface
### Description
Configure the UDP forward protocol on an interface for a server IP on the same interface using appropriate CLI commands. Verify that the unixctl dump output of the interface displays the UDP forward protocol.
### Test result criteria
#### Test pass criteria
The user verifies that the unixctl dump output displays the configured UDP forward protocol.
#### Test fail criteria
The user verifies that the unixctl dump output does not display the configured UDP forward protocol.

### Verify a single UDP forward protocol configuration on multiple interfaces
### Description
Configure the same UDP forward protocol on multiple interfaces using appropriate CLI commands. Verify that the unixctl dump output of the interfaces displays the UDP forward protocol.
### Test result criteria
#### Test pass criteria
The user verifies that the unixctl dump output displays the configured UDP forward protocol.
#### Test fail criteria
The user verifies that the unixctl dump output does not display the configured UDP forward protocol.

### Verify the UDP forward protocol configuration post reboot
### Description
Configure the UDP forward protocol on an interface using appropriate CLI commands. Perform a reboot of the switch. Verify that the unixctl dump output of the interdface displays the UDP forward protocol post reboot.
### Test result criteria
#### Test pass criteria
The user verifies that the unixctl dump output displays the configured UDP forward protocol post reboot.
#### Test fail criteria
The user verifies that the unixctl dump output does not display the configured UDP forward protocol post reboot.

### Verify the maximum UDP forward protocol configurations per interface
### Description
Configure the UDP forward protocol on an interface using appropriate CLI commands. Verify that the unixctl dump output of the interface displays eight UDP forward protocol configurations.
### Test result criteria
#### Test pass criteria
The user verifies that the unixctl dump output displays eight UDP forward protocol configurations.
#### Test fail criteria
The user verifies that the unixctl dump output displays more than eight UDP forward protocol configurations.

### Verify the UDP forward protocol servers configured count per interface
### Description
Configure the multiple UDP forward protocol servers on an interface using appropriate CLI commands. Verify the unixctl dump count of servers configured on the interface.
### Test result criteria
#### Test pass criteria
The user verifies the count for the number of configured servers in the unixctl dump output is same as the number of servers configured.
#### Test fail criteria
The user verifies the countfor the number of configured servers in the unixctl dump output is not same as the number of servers configured.

### Verify the UDP forward protocol servers count per UDP port on an interface
### Description
Configure the UDP forward protocol servers for different UDP ports on an interface using appropriate CLI commands. Verify the unixctl dump count of servers configured for that UDP port on the interface.
### Test result criteria
#### Test pass criteria
The user verifies that the total number of servers configured per port on an interface is equal to the overall number of UDP forward protocol servers configured on that interface.
#### Test fail criteria
The user verifies that the total number of servers configured per port on an interface is not equal to the overall number of UDP forward protocol servers configured on that interface.

### Verify the UDP forward protocol configurations along with the dhcp-relay helper address configuration on an interface
### Description
Configure the UDP forward protocol and DHCP relay helper address on an interface using appropriate CLI commands. Verify that the unixctl dump output of the interface displays both the UDP forward protocol configuration and the DHCP relay helper address configuration.
### Test result criteria
#### Test pass criteria
The user verifies that the unixctl dump output displays both the UDP forward protocol configuration and the DHCP relay helper address configuration.
#### Test fail criteria
The user verifies that the unixctl dump output does not display either the UDP forward protocol configurations or the DHCP relay helper address configuration.
