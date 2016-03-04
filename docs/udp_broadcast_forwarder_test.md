# UDP broadcast forwarder daemon Test Cases

## Contents
- [Verify UDP broadcast forwarder configuration](#verify-udp-broadcast-forwarder-configuration)
    - [Enable UDP broadcast forwarding globally](#enable-udp-broadcast-forwarding-globally)
    - [Disable UDP broadcast forwarding globally](#disable-udp-broadcast-forwarding-globally)
- [Verify UDP forward-protocol configurations](#verify-udp-forward-protocol-configurations)
    - [Verify UDP forward-protocol configuration on a specific interface](#verify-udp-forward-protocol-configuration-on-a-specific-interface)
    - [Verify UDP forward-protocol configuration for multiple UDP ports on a specific interface](#verify-udp-forward-protocol-configuration-for-multiple-udp-ports-on-a-specific-interface)
    - [Verify UDP forward-protocol configurations for multiple IP addresses for a single UDP port on a specific interface](#verify-udp-forward-protocol-configurations-for-multiple-ip-addresses-for-a-single-udp-port-on-a-specific-interface)
    - [Verify UDP forward-protocol configuration for subnet broadcast IP address on a specific interface](#verify-udp-forward-protocol-configuration-for-subnet-broadcast-ip-address-on-a-specific-interface)
    - [Verify UDP forward-protocol configuration on same subnet](#verify-udp-forward-protocol-configurations-on-same-subnet)
    - [Verify single UDP forward-protocol configuration on multiple interfaces](#verify-single-udp-forward-protocol-configuration-on-multiple-interfaces)
    - [Verify UDP forward-protocol configuration post reboot](#verify-udp-forward-protocol-configuration-post-reboot)
    - [Verify maximum UDP forward-protocol configurations per interface](#verify-maximum-udp-forward-protocol-configurations-per-interface)


## Verify UDP broadcast forwarder configuration
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
#### Test Fail Criteria
The user verifies the UDP broadcast forwarding state as true using the unixctl dump output.

## Verify UDP forward-protocol configurations
### Objective
To verify UDP forward-protocol configurations are properly updated in daemon local cache.
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

### Verify UDP forward-protocol configuration on a specific interface
### Description
Configure UDP forward-protocol on an interface using appropriate CLI. Verify that unixctl dump output of an interface displays the UDP forward-protocol.
### Test result criteria
#### Test pass criteria
The user verifies unixctl dump output displays the configured UDP forward-protocol.
#### Test fail criteria
The user verifies unixctl dump output does not display the configured UDP forward-protocol.

### Verify UDP forward-protocol configuration for multiple UDP ports on a specific interface
### Description
Configure UDP forward-protocol on an interface for multiple UDP ports using appropriate CLI. Verify that unixctl dump output of an interface displays the UDP forward-protocol.
### Test result criteria
#### Test pass criteria
The user verifies unixctl dump output displays the configured UDP forward-protocol.
#### Test fail criteria
The user verifies unixctl dump output does not display the configured UDP forward-protocol.

### Verify UDP forward-protocol configurations for multiple IP addresses for a single UDP port on a specific interface
### Description
Configure UDP forward-protocol on an interface for multiple IP addresses for a single UDP port using appropriate CLI. Verify that unixctl dump output of an interface displays the UDP forward-protocol.
### Test result criteria
#### Test pass criteria
The user verifies unixctl dump output displays the configured UDP forward-protocol.
#### Test fail criteria
The user verifies unixctl dump output does not display the configured UDP forward-protocol.

### Verify UDP forward-protocol configuration for subnet broadcast IP address on a specific interface
### Description
Configure UDP forward-protocol on an interface for a subnet broadcast IP using appropriate CLI. Verify that unixctl dump output of an interface displays the UDP forward-protocol.
### Test result criteria
#### Test pass criteria
The user verifies unixctl dump output displays the configured UDP forward-protocol.
#### Test fail criteria
The user verifies unixctl dump output does not display the configured UDP forward-protocol.

### Verify UDP forward-protocol configurations on same subnet
### Description
Configure UDP forward-protocol on an interface for a server IP on same subnet using appropriate CLI. Verify that unixctl dump output of an interface displays the UDP forward-protocol.
### Test result criteria
#### Test pass criteria
The user verifies unixctl dump output displays the configured UDP forward-protocol.
#### Test fail criteria
The user verifies unixctl dump output does not display the configured UDP forward-protocol.

### Verify single UDP forward-protocol configuration on multiple interfaces
### Description
Configure the same UDP forward-protocol on multiple interfaces using appropriate CLI. Verify that unixctl dump output of these interfaces displays the UDP forward-protocol.
### Test result criteria
#### Test pass criteria
The user verifies unixctl dump output displays the configured UDP forward-protocol.
#### Test fail criteria
The user verifies unixctl dump output does not display the configured UDP forward-protocol.

### Verify UDP forward-protocol configuration post reboot
### Description
Configure UDP forward-protocol on an interface using appropriate CLI. Perform reboot of the switch. Verify that unixctl dump output of an interdface displays the UDP forward-protocol post reboot.
### Test result criteria
#### Test pass criteria
The user verifies unixctl dump output displays the configured UDP forward-protocol post reboot.
#### Test fail criteria
The user verifies unixctl dump output does not display the configured UDP forward-protocol post reboot.

### Verify maximum UDP forward-protocol configurations per interface
### Description
Configure UDP forward-protocol on an interface using Appropriate CLI. Verify that unixctl dump output of an interface displays 16 UDP forward-protocol.
### Test result criteria
#### Test pass criteria
The user verifies unixctl dump output displays 16 UDP forward-protocols.
#### Test fail criteria
The user verifies unixctl dump output displays more than 16 UDP forward-protocols.
