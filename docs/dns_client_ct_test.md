# DNS client Test Cases

## Contents
- [Verify DNS client configuration](#verify-dns-client-configuration)
    - [Enable DNS client](#enable-dns-client)
    - [Disable DNS client](#disable-dns-client)
    - [Verify DNS client status in show-running](#verify-dns-client-status-in-show-running)
	- [Verify DNS client domain-name configuration](#verify-dns-client-domain-name-configuration)
	- [Verify DNS client domain-name unconfiguration](#verify-dns-client-domain-name-unconfiguration)
	- [Verify multiple DNS client domain-name configuration](#verify-multiple-dns-client-domain-name-configuration)
	- [Verify DNS client domain-list configuration](#verify-dns-client-domain-list-configuration)
	- [Verify DNS client domain-list unconfiguration](#verify-dns-client-domain-list-unconfiguration)
	- [Verify multiple DNS client domain-list configuration](#verify-multiple-dns-client-domain-list-configuration)
	- [Verify DNS client IPv4 name-server configuration](#verify-dns-client-ipv4-name-server-configuration)
	- [Verify DNS client IPv4 name-server unconfiguration](#verify-dns-client-ipv4-name-server-unconfiguration)
	- [Verify multiple DNS client IPv4 name-server configuration](#verify-multiple-dns-client-ipv4-name-server-configuration)
	- [Verify DNS client IPv6 name-server configuration](#verify-dns-client-ipv6-name-server-configuration)
	- [Verify DNS client IPv6 name-server unconfiguration](#verify-dns-client-ipv6-name-server-unconfiguration)
	- [Verify multiple DNS client IPv6 name-server configuration](#verify-multiple-dns-client-ipv6-name-server-configuration)
	- [Verify DNS client IPv4 and IPv6 name-servers configuration](#verify-dns-client-ipv4-and-ipv6-name-servers-configuration)
	- [Verify DNS client host configuration](#verify-dns-client-host-configuration)
	- [Verify DNS client host unconfiguration](#verify-dns-client-host-unconfiguration)
	- [Verify multiple DNS client host configuration](#verify-multiple-dns-client-host-configuration)
    - [Verify maximum DNS client domain-list configuration](#verify-maximum-dns-client-domain-list-configuration)
    - [Verify maximum DNS client name-server configuration](#verify-maximum-dns-client-name-server-configuration)
    - [Verify maximum DNS client host configuration](#verify-maximum-dns-client-host-configuration)
    - [Verify invalid DNS client domain-name configuration](#verify-invalid-dns-client-domain-name-configuration)
    - [Verify invalid DNS client domain-list configuration](#verify-invalid-dns-client-domain-list-configuration)
    - [Verify invalid DNS client name-server configuration](#verify-invalid-dns-client-name-server-configuration)
    - [Verify invalid DNS client host configuration](#verify-invalid-dns-client-host-configuration)
    - [Verify unconfigure of a non-existing DNS client domain-name](#verify-unconfigure-of-a-non-existing-dns-client-domain-name)
    - [Verify unconfigure of a non-existing DNS client domain-list](#verify-unconfigure-of-a-non-existing-dns-client-domain-list)
    - [Verify unconfigure of a non-existing DNS client name-server](#verify-unconfigure-of-a-non-existing-dns-client-name-server)
    - [Verify unconfigure of a non-existing DNS client host](#verify-unconfigure-of-a-non-existing-dns-client-host)
    - [Verify a stress scenario for DNS client configuration](#verify-a-stress-scenario-for-dns-client-configuration)
    - [Verify DNS client configuration post reboot](#verify-dns-configuration-post-reboot)

## Verify DNS client configuration
### Objective
To verify enablement/disablement of the DNS client functionality.
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

### Enable DNS client
### Description
Enable the DNS client on dut01 using the `ip dns` command.
### Test result criteria
#### Test pass criteria
The user verifies that the DNS client enable state is reflected in the respective `show` command.
#### Test fail criteria
The user verifies that the DNS client enable state is not reflected in the respective `show` command.

### Disable DNS client
### Description
Disable the DNS client on dut01 using the `no ip dns` command.
### Test result criteria
#### Test pass criteria
The user verifies that the DNS client disable state is reflected in the respective `show` command.
#### Test fail criteria
The user verifies that the DNS client disable state is not reflected in the respective `show` command.

### Verify DNS client status in show-running
### Description
Verify the DNS client status using the `show running-configuration` command.
### Test result criteria
#### Test pass criteria
The user verifies that the current DNS client status is reflected in the `show` command.
#### Test fail criteria
The user verifies that the current DNS client status is not reflected in the `show` command.

### Verify DNS client domain-name configuration
### Description
Verify the DNS client domain-name configuration on dut01.
### Test result criteria
#### Test pass criteria
The user is able to configure DNS client domain-name on dut01.
#### Test fail criteria
The user is unable to configure DNS client domain-name on dut01.

### Verify DNS client domain-name unconfiguration
### Description
Verify the DNS client domain-name unconfiguration on dut01.
### Test result criteria
#### Test pass criteria
The user is able to unconfigure DNS client domain-name on dut01.
#### Test fail criteria
The user is unable to unconfigure DNS client domain-name on dut01.

### Verify multiple DNS client domain-name configuration
### Description
Configure the DNS client domain-name multiple times on dut01.
### Test result criteria
#### Test pass criteria
The user is able to configure DNS client domain-name multiple times on dut01.
#### Test fail criteria
The user is unable to configure DNS client domain-name multiple times on dut01.

### Verify DNS client domain-list configuration
### Description
Verify the DNS client domain-list configuration on dut01.
### Test result criteria
#### Test pass criteria
The user is able to configure DNS client domain-list on dut01.
#### Test fail criteria
The user is unable to configure DNS client domain-list on dut01.

### Verify DNS client domain-list unconfiguration
### Description
Verify the DNS client domain-list unconfiguration on dut01.
### Test result criteria
#### Test pass criteria
The user is able to unconfigure DNS client domain-list on dut01.
#### Test fail criteria
The user is unable to unconfigure DNS client domain-list on dut01.

### Verify multiple DNS client domain-list configuration
### Description
Configure multiple DNS client domain-list on dut01.
### Test result criteria
#### Test pass criteria
The user is able to configure multiple DNS client domain-list on dut01.
#### Test fail criteria
The user is unable to configure multiple DNS client domain-list on dut01.

## Verify DNS client IPv4 name-server configuration
### Description
Verify the DNS client IPv4 name-server configuration on dut01.
### Test result criteria
#### Test pass criteria
The user is able to configure DNS client IPv4 name-server on dut01.
#### Test fail criteria
The user is unable to configure DNS client IPv4 name-server on dut01.

### Verify DNS client IPv4 name-server unconfiguration
### Description
Verify the DNS client IPv4 name-server unconfiguration on dut01.
### Test result criteria
#### Test pass criteria
The user is able to unconfigure DNS client IPv4 name-server on dut01.
#### Test fail criteria
The user is unable to unconfigure DNS client IPv4 name-server on dut01.

### Verify multiple DNS client IPv4 name-server configuration
### Description
Configure multiple DNS client IPv4 name-server on dut01.
### Test result criteria
#### Test pass criteria
The user is able to configure multiple DNS client IPv4 name-server on dut01.
#### Test fail criteria
The user is unable to configure multiple DNS client IPv4 name-server on dut01.

## Verify DNS client IPv6 name-server configuration
### Description
Verify the DNS client IPv6 name-server configuration on dut01.
### Test result criteria
#### Test pass criteria
The user is able to configure DNS client IPv6 name-server on dut01.
#### Test fail criteria
The user is unable to configure DNS client IPv6 name-server on dut01.

### Verify DNS client IPv6 name-server unconfiguration
### Description
Verify the DNS client IPv6 name-server unconfiguration on dut01.
### Test result criteria
#### Test pass criteria
The user is able to unconfigure DNS client IPv6 name-server on dut01.
#### Test fail criteria
The user is unable to unconfigure DNS client IPv6 name-server on dut01.

### Verify multiple DNS client IPv6 name-server configuration
### Description
Configure multiple DNS client IPv6 name-server on dut01.
### Test result criteria
#### Test pass criteria
The user is able to configure multiple DNS client IPv6 name-server on dut01.
#### Test fail criteria
The user is unable to configure multiple DNS client IPv6 name-server on dut01.

## Verify DNS client IPv4 and IPv6 name-servers configuration
### Description
Verify the DNS client IPv4 and IPv6 name-servers configuration on dut01.
### Test result criteria
#### Test pass criteria
The user is able to configure DNS client IPv4 and IPv6 name-servers on dut01.
#### Test fail criteria
The user is unable to configure DNS client IPv4 and IPv6 name-servers on dut01.

### Verify DNS client host configuration
### Description
Verify the DNS client host configuration on dut01.
### Test result criteria
#### Test pass criteria
The user is able to configure DNS client host on dut01.
#### Test fail criteria
The user is unable to configure DNS client host on dut01.

### Verify DNS client host unconfiguration
### Description
Verify the DNS client host unconfiguration on dut01.
### Test result criteria
#### Test pass criteria
The user is able to unconfigure DNS client host on dut01.
#### Test fail criteria
The user is unable to unconfigure DNS client host on dut01.

### Verify multiple DNS client host configuration
### Description
Configure multiple DNS client host on dut01.
### Test result criteria
#### Test pass criteria
The user is able to configure multiple DNS client host on dut01.
#### Test fail criteria
The user is unable to configure multiple DNS client host on dut01.

### Verify maximum DNS client domain-list configuration
### Description
Verify the maximum(6) DNS client domain-list configuration on dut01.
### Test result criteria
#### Test pass criteria
The user is able to configure maximum(6) DNS client domain-list on dut01.
#### Test fail criteria
The user is unable to configure maximum(6) DNS client domain-list on dut01.

### Verify maximum DNS client name-server configuration
### Description
Verify the maximum(3) DNS client name-servers configuration on dut01.
### Test result criteria
#### Test pass criteria
The user is able to configure maximum(3) DNS client name-servers on dut01.
#### Test fail criteria
The user is unable to configure maximum(3) DNS client name-servers on dut01.

### Verify maximum DNS client host configuration
### Description
Verify the maximum(6) DNS client host configuration on dut01.
### Test result criteria
#### Test pass criteria
The user is able to configure maximum(6) DNS client host on dut01.
#### Test fail criteria
The user is unable to configure maximum(6) DNS client host on dut01.

### Verify invalid DNS client domain-name configuration
### Description
Verify configuration of an invalid DNS client domain-name on dut01.
### Test result criteria
#### Test pass criteria
The user is unable to configure an invalid DNS client domain-name on dut01.
#### Test fail criteria
The user is able to configure an invalid DNS client domain-name on dut01.

### Verify invalid DNS client domain-list configuration
### Description
Verify configuration of an invalid DNS client domain-list on dut01.
### Test result criteria
#### Test pass criteria
The user is unable to configure an invalid DNS client domain-list on dut01.
#### Test fail criteria
The user is able to configure an invalid DNS client domain-list on dut01.

### Verify invalid DNS client name-server configuration
### Description
Verify configuration of an invalid DNS client name-server on dut01.
### Test result criteria
#### Test pass criteria
The user is unable to configure an invalid DNS client name-server on dut01.
#### Test fail criteria
The user is able to configure an invalid DNS client name-server on dut01.

### Verify invalid DNS client host configuration
### Description
Verify configuration of an invalid DNS client host on dut01.
### Test result criteria
#### Test pass criteria
The user is unable to configure an invalid DNS client host on dut01.
#### Test fail criteria
The user is able to configure an invalid DNS client host on dut01.

### Verify unconfigure of a non-existing DNS client domain-name
### Description
Verify unconfiguration of a non-existing DNS client domain-name on dut01.
### Test result criteria
#### Test pass criteria
The user is unable to unconfigure a non-existing DNS client domain-name on dut01.
#### Test fail criteria
The user is able to unconfigure a non-existing DNS client domain-name on dut01.

### Verify unconfigure of a non-existing DNS client domain-list
### Description
Verify unconfiguration of a non-existing DNS client domain-list on dut01.
### Test result criteria
#### Test pass criteria
The user is unable to unconfigure a non-existing DNS client domain-list on dut01.
#### Test fail criteria
The user is able to unconfigure a non-existing DNS client domain-list on dut01.

### Verify unconfigure of a non-existing DNS client name-server
### Description
Verify unconfiguration of a non-existing DNS client name-server on dut01.
### Test result criteria
#### Test pass criteria
The user is unable to unconfigure a non-existing DNS client name-server on dut01.
#### Test fail criteria
The user is able to unconfigure a non-existing DNS client name-server on dut01.

### Verify unconfigure of a non-existing DNS client host
### Description
Verify unconfiguration of a non-existing DNS client host on dut01.
### Test result criteria
#### Test pass criteria
The user is unable to unconfigure a non-existing DNS client host on dut01.
#### Test fail criteria
The user is able to unconfigure a non-existing DNS client host on dut01.

### Verify a stress case scenario for DNS client configuration
### Description
Verify configuration of maximum DNS client domain-list, name-server, hosts and domain-name all together.
### Test Result Criteria
#### Test pass criteria
The user is able to configure maximum DNS client domain-list, name-server, hosts and domain-name all together.
#### Test fail criteria
The user is unable to configure maximum DNS client domain-list, name-server, hosts and domain-name all together.

### Verify DNS client configuration post reboot
### Description
Verify that the DNS client configuration is retained after the device reboots.
### Test Result Criteria
#### Test pass criteria
The DNS client configurations is retained post reboot.
#### Test fail criteria
The DNS client configurations is not retained post reboot.