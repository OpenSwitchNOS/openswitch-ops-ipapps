# UDP Broadcast Forwarding Test Cases

## Contents
- [Verify UDP broadcast forwarding configuration](#verify-udp-broadcast-forwarding-configuration)
    - [Enable UDP broadcast forwarding](#enable-udp-broadcast-forwarding)
    - [Disable UDP broadcast forwarding](#disable-udp-broadcast-forwarding)
    - [Verify UDP broadcast forwarding status](#verify-udp-broadcast-forwarding-status)
    - [Verify UDP broadcast forwarding status in show-running](#verify-udp-broadcast-forwarding-status-in-show-running)
    - [Verify UDP broadcast forwarding post reboot](#verify-udp-broadcast-forwarding-post-reboot)
- [Verify UDP forward-protocol configuration](#verify-udp-forward-protocol-configuration)
    - [Verify UDP forward-protocol configuration for unicast server IP](#verify-udp-forward-protocol-configuration-for-unicast-server-ip)
    - [Verify UDP forward-protocol configuration for subnet broadcast server IP](#verify-udp-forward-protocol-configuration-for-subnet-broadcast-server-ip)
    - [Verify UDP forward-protocol status](#verify-udp-forward-protocol-status)
    - [Verify UDP forward-protocol status in show-running](#verify-udp-forward-protocol-status-in-show-running)
    - [Verify UDP forward-protocol status on interface](#verify-udp-forward-protocol-status-on-interface)
    - [Verify UDP forward-protocol post reboot](#verify-udp-forward-protocol-post-reboot)
    - [Verify the maximum UDP forward-protocol configurations per interface](#verify-the-maximum-udp-forward-protocol-configurations-per-interface)
    - [Verify UDP forward-protocol configuration when UDP broadcast forwading is disabled](#verify-udp-forward-protocol-configuration-when-udp-broadcast-forwarding-is-disabled)
    - [Verify UDP forward-protocol on an interface when IP routing is disabled](#verify-udp-forward-protocol-on-an-interface-when-ip-routing-is-disabled)
    - [Verify UDP forward-protocol on multiple interfaces](#verify-udp-forward-protocol-on-multiple-interfaces)
    - [Verify UDP forward-protocol for multiple server IPs on a single UDP port on an interface](#verify-udp-forward-protocol-for-a-multiple-server-ips-on-a-single-udp-port-on-an-interface)
    - [Verify UDP forward-protocol for a single server IP on multiple UDP ports on an interface](#verify-udp-forward-protocol-for-a-single-server-ip-on-multiple-udp-ports-on-an-interface)
    - [Verify UDP forward-protocol for a single server IP on multiple UDP ports on multiple interfaces](#verify-udp-forward-protocol-for-a-single-server-ip-on-multiple-udp-ports-on-multiple-interfaces)
- [Verify negative test cases](#verify-negative-test-cases)
    - [Verify UDP forward-protocol record duplication on an interface](#verify-udp-forward-protocol-record-duplication-on-an-interface)
    - [Verify UDP forward-protocol configuration for invalid server IP on an interface](#verify-udp-forward-protocol-configuration-for-invalid-server-ip-on-an-interface)
    - [Verify UDP forward-protocol configuration for multicast server IP on an interface](#verify-udp-forward-protocol-configuration-for-multicast-server-ip-on-an-interface)
    - [Verify UDP forward-protocol configuration for anynet broadcasr server IP on an interface](#verify-udp-forward-protocol-configuration-for-anynet-broadcast-server-ip-on-an-interface)
    - [Verify UDP forward-protocol configuration for loopback server IP on an interface](#verify-udp-forward-protocol-configuration-for-loopback-server-ip-on-an-interface)


## Verify UDP broadcast forwarding configuration
### Objective
To verify enablement/disablement of the UDP broadcast forwarding.
### Requirements
The requirements for this test case are:
 - Docker version 1.7 or above
 - Action switch docker instance/physical hardware switch

### Setup
#### Topology diagram
```ditaa

                          +----------------+
                          |                |
                          |     dut01      |
                          |                |
                          |                |
                          +----------------+
```

### Enable UDP broadcast forwarding
### Description
Enable the UDP broadcast forwarding on dut01 using the `ip udp-bcast-forward` command.
### Test result criteria
#### Test pass criteria
The user verifies that the UDP broadcast forwarding enable state is reflected in the `show` command.
#### Test fail criteria
The user verifies that the UDP broadcast forwarding enable state is not reflected in the `show` command.

### Disable UDP broadcast forwarding
### Description
Disable the UDP broadcast forwarding on dut01 using the `no ip udp-bcast-forward` command.
### Test result criteria
#### Test pass criteria
The user verifies that the UDP broadcast forwarding disable state is reflected in the `show` command.
#### Test fail criteria
The user verifies that the UDP broadcast forwarding disable state is not reflected in the `show` command.

### Verify UDP broadcast forwarding status
### Description
Verify the UDP broadcast forwarding status using the `show ip forward-protocol` command.
### Test result criteria
#### Test pass criteria
The user verifies that the current UDP broadcast status is reflected in the `show` command.
#### Test fail criteria
The user verifies that the current UDP broadcast status is not reflected in the `show` command.

### Verify UDP broadcast forwarding status in show-running
### Description
Verify the UDP broadcast forwarding status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
The user verifies that the current UDP broadcast status is reflected in the `show` command.
#### Test fail criteria
The user verifies that the current UDP broadcast status is not reflected in the `show` command.

### Verify UDP broadcast forwarding post reboot
### Description
Verify that the UDP broadcast forwarding configuration is retained after the device reboots.
### Test Result Criteria
#### Test pass criteria
The UDP broadcast forwarding configurations is retained post reboot.
#### Test fail criteria
The UDP broadcast forwarding configurations is not retained post reboot.

## Verify UDP forward-protocol configuration
### Objective
To configure a forward-protocol server on an interface.
### Requirements
The requirements for this test case are:
 - Docker version 1.7 or above
 - Action switch docker instance/physical hardware switch
 - A topology with links between dut01 and switch01 and between dut01 and switch02 which are configured with IPv4 addresses

### Setup
#### Topology diagram
```ditaa

                          +----------------+
                          |                |
                          |     dut01      |
                          |                |
                          |                |
                          +----------------+
```

### Verify UDP forward-protocol configuration for unicast server IP
### Description
Verify the UDP forward-protocol configuration with unicast server IP on an intrerface.
### Test result criteria
#### Test pass criteria
The user is able to configure UDP forward-protocol with unicast server IP on an interface.
#### Test fail criteria
The user is unable to configure UDP forward-protocol with unicast server IP on an interface.

### Verify UDP forward-protocol configuration for subnet broadcast server IP
### Description
Verify the UDP forward-protocol configuration with subnet broadcast server IP on an interface.
### Test result criteria
#### Test pass criteria
The user is able to configure UDP forward-protocol with subnet broadcast server IP on an interface.
#### Test fail criteria
The user is unable to configure UDP forward-protocol with subnet broadcast server IP on an interface.

### Verify UDP forward-protocol status
### Description
Verify the UDP forward-protocol server status on an interface using the `show ip forward-protocol` command.
### Test result criteria
#### Test pass criteria
The user is able to verify the current UDP forward-protocol status on an interface.
#### Test fail criteria
The user is unable to verify the current UDP forward-protocol status on an interface.

### Verify UDP forward-protocol status in show-running
### Description
Verify the UDP forward-protocol configurations are retained using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
The user verifies the current UDP forward-protocol configurations is reflected in the `show` command.
#### Test fail criteria
The user verifies the current UDP forward-protocol configurations is not reflected in the `show` command.

### Verify UDP forward-protocol status on interface
### Description
Verify the UDP forward-protocol configurations is retained on its specific interface using the `show interface <IFNAME>` command.
### Test result criteria
#### Test pass criteria
The user verifies the current UDP forward-protocol configurations is reflected in the `show` command.
#### Test fail criteria
The user verifies the current UDP forward-protocol configurations is not reflected in the `show` command.

### Verify UDP forward-protocol post reboot
### Description
Verify the UDP forward-protocol configuration is retained after the device reboots.
### Test result criteria
#### Test pass criteria
The UDP forward-protocol configurations is retained post reboot.
#### Test fail criteria
The UDP forward-protocol configurations is not retained post reboot.

### Verify the maximum UDP forward-protocol configurations per interface
### Description
Verify the maximum (8) UDP forward-protocol configurations per interface.
### Test result criteria
#### Test pass criteria
The user is able to configure a maximum of 8 UDP forward-protocol configurations.
#### Test fail criteria
The user is unable to configure a maximum of 8 UDP forward-protocol configurations.

### Verify UDP forward-protocol configuration when UDP broadcast forwarding is disabled
### Description
Verify the UDP forward-protocol configuration on an interface when UDP broadcast forwarding is disabled.
### Test result criteria
#### Test pass criteria
The user is able to configure a UDP forward-protocol on an interface.
#### Test fail criteria
The user is unable to configure a UDP forward-protocol on an interface.

### Verify UDP forward-protocol on an interface when IP routing is disabled
### Description
Disable the IP routing on the interface of dut01 when the UDP forward-protocol is enabled.
### Test result criteria
#### Test pass criteria
The user verifies the UDP forward-protcol configuration on the interface is present.
#### Test fail criteria
The user verifies the UDP forward-protocol configuration on the interface is not present.

### Verify UDP forward-protocol on multiple interfaces
### Description
Verify the UDP forward-protocol on multiple interfaces by configuring the same server IP on the same UDP port.
### Test result criteria
#### Test pass criteria
The user is able to configure UDP forward-protocol on multiple interfaces with the same server IP on the same UDP port.
#### Test fail criteria
The user is unable to configure UDP forward-protocol on multiple interfaces with the same server IP on the same UDP port.

### Verify UDP forward-protocol for multiple server IPs on a single UDP port on an interface
### Description
Verify UDP forward-protocol on dut01 for multiple server IPs on a single UDP port.
### Test result criteria
#### Test pass criteria
The user is able to configure the UDP forward-protocol for multiple server IPs on a single UDP port on an interface.
#### Test fail criteria
The user is unable to configure the UDP forward-protocol for multiple server IPs on a single UDP port on an interface.

### Verify UDP forward-protocol for a single server IP on multiple UDP ports on an interface
### Description
Verify the UDP forward-protocol on dut01 for a single server IP but on different UDP ports.
### Test result criteria
#### Test pass criteria
The user is able to configure the UDP forward-protocol for a single server IP on multiple UDP ports on same interface.
#### Test fail criteria
The user is unable to configure the UDP forward-protocol for a single server IP on multiple UDP ports on same interface.

### Verify UDP forward-protocol for a single server IP on multiple UDP ports on multiple interfaces
### Description
Verify the UDP forward-protocol on dut01 for a single server IP but on different UDP ports on multiple interfaces.
### Test result criteria
#### Test pass criteria
The user is able to configure the UDP forward-protocol for a single server IP but on different UDP ports on multiple interfaces.
#### Test fail criteria
The user is unable to configure the UDP forward-protocol for a single server IP but on different UDP ports on multiple interfaces.

## Verify negative test cases
### Objective
Verify different negative scenarios.
### Requirements
The requirements for this test case are:
 - Docker version 1.7 or above
 - Action switch docker instance/physical hardware switch
 - A topology with a link between dut01 and switch01, dut01 and switch02 and are configured with IPv4 addresses

### Setup
#### Topology diagram
```ditaa

                          +----------------+
                          |                |
                          |     dut01      |
                          |                |
                          |                |
                          +----------------+
```

### Verify UDP forward-protocol record duplication on an interface
### Description
Verify the UDP forward-protocol record duplication by configuring the same server IP multiple times on a UDP port.
### Test result criteria
#### Test pass criteria
The user is unable to configure the UDP forward-protocol multiple times.
#### Test fail criteria
The user is able to configure the UDP forward-protocol multiple times.

### Verify UDP forward-protocol configuration for invalid server IP on an interface
### Description
Verify the UDP forward-protocol configuration with server IP as 0.0.0.0 on an interface.
### Test result criteria
#### Test pass criteria
The user is unable to configure a UDP forward-protocol and an error is displayed on the console.
#### Test fail criteria
The user is able to configure a UDP forward-protocol.

### Verify UDP forward-protocol configuration for multicast server IP on an interface
### Description
Verify the UDP forward-protocol configuration with a multicast server IP on an interface.
### Test result criteria
#### Test pass criteria
The user is unable to configure a UDP forward-protocol and an error is displayed on the console.
#### Test fail criteria
The user is able to configure a UDP forward-protocol.

### Verify UDP forward-protocol configuration for anynet broadcast server IP on an interface
### Description
Verify the UDP forward-protocol configuration with anynet broadcast server IP as 255.255.255.255 on an interface.
### Test result criteria
#### Test pass criteria
The user is unable to configure a UDP forward-protocol and an error is displayed on the console.
#### Test fail criteria
The user is able to configure a UDP forward-protocol.

### Verify UDP forward-protocol configuration for loopback server IP on an interface
### Description
Verify the UDP forward-protocol configuration with loopback server IP on an interface.
### Test result criteria
#### Test pass criteria
The user is unable to configure a UDP forward-protocol and an error is displayed on the console.
#### Test fail criteria
The user is able to configure a UDP forward-protocol.
