High level design of DHCPv6-Relay
=================================================

DHCP for IPv6 (DHCPv6) is a client/server protocol that provides managed configuration of devices. DHCPv6 can provide a device with addresses assigned by a DHCPv6 server and other configuration information, which are carried in options.
A DHCPv6 relay agent is required to allow a DHCPv6 client to send a message to a DHCPv6 server that is not attached to the same link. The DHCPv6 relay agent on the client’s link will relay messages between the client and server.

## Contents
- [Responsibilities](#responsibilities)
- [Design choices](#design-choices)
- [Participating Modules](#participating-modules)
- [Sub-module interactions](#sub-module-interactions)
- [OVSDB-Schema](#ovsdb-schema)
- [Any other sections that are relevant for the module](#any-other-sections-that-are-relevant-for-the-module)
- [References](#references)


## Responsibilities
-------------------
DHCPv6-Relay enables support for the following features
1. IPv6 DHCPv6-Relay Agent
2. IPv6 DHCPv6-Relay options support

## Design choices
-----------------
DHCPv6 relay daemon functions with the help of following threads.

Main Thread : Schedule idl cache updations and if any configuration change is noticed, update the local database by acquiring a lock.
Packet Receiver Thread : This thread is used to receive DHCPv6 messages sent by clients as well as servers. Received packets are delegated to DHCPv6-Relay handler for further processing within the same thread context. The packet send routine will write to the same socket that blocks on the receive path. Depending on the type of helper address specified, unicast/multicast packet is sent by the relay agent to server.


Following sequence diagrams describe the packet handling high-level design.


DHCP-Relay :

```ditaa

DHCPv6-Client                Packet receiver                 DHCPv6-Relay              DHCP-Server
      |                           |                              |                          +
      |                           |                              |                          |
      | DHCP Packet to server     |                              |                          |
      +--------------------------->                              |                          |
      |                           |                              |                          |
      |                           |                              |                          |
      |                           | DHCP multicast pkt to relay  |                          |
      |                           +------------------------------>                          |
      |                           |                              |                          |
      |                           |                              | send unicast/Multicast   |
      |                           |                              | pkt to servers           |
      |                           |                              +-------------------------->
      |                           |                              |                          |
      |                           |                              |                          |
      |                           |                              |                          |
      |                           |                              |                          |
      |                           |                              | response from server     |
      |                           |                              <--------------------------+
      |                           |                              |                          |
      |             Server response to client                    |                          |
      <----------------------------------------------------------+                          |
      |                           |                              |                          |
      |                           |                              |                          |
      +                           +                              +                          +

```


## Participating modules
------------------------

```ditaa
                            +--------------------------+
                            |     DHCPv6-Relay UI      |
                            |   (CLI/REST/Ansible)     |
                            |                          |
                            +-----------^--------------+
                                        |
                                        |
 +-----------------------+              |
 |                       |        +-----v-------+      +---------------+
 |    DHCPv6-Relay       |        |             |      | DHCPv6-Server |
 |                       <-------->             <------>               |
 |                       |        |             |      |               |
 +---^---------------^---+        |             |      +---------------+
     |               |            |             |
     | Pkt Tx/Rx     |            |    OVSDB    |
     |               |            |             |
     |               |            |             |
     |               |            |             |
+----v---------------v---+        |             |      +--------------+
|                        |        |             |      |              |
|    Kernel Interfaces   |        |             <------>     port     |
|                        |        |             |      |              |
|                        |        |             |      |              |
+------------------------+        +-------------+      +--------------+
```

Detailed description of the relationships and interactions
1. DHCPv6-Relay UI : DHCPv6 global configurations and interface level configurations. Also this module takes care of the statistics display
2. DHCPv6-Relay : Listens to OVSDB notifications for config changes and interacts with kernel interfaces to receive and send DHCP packets to DHCPv6 client/server
3. DHCP-Server : DHCPv6-Server assigns IPv6 address and other network configuration parameters to the DHCPv6 client.
4. port : When a port is deleted, DHCPv6-Relay configuration on it shall be flushed.

## Sub-module interactions
--------------------------

```ditaa
     /-----------\               /----------\                               /-------\
     |packet     |               |DHCP relay|                               |Raw pkt|
     |receiver   |               \----------/                               |sender |
     \-----------/                                                          \-------/
           |                           |                                         |
           | DHCP multicast received   |                                         |
           | on dest port (547)        |                                         |
           |-------------------------->|                                         |
           |                           |Encapsulate the packet with relay info   |
           |                           |and send a new relay-forward packet to   |
           |                           |server                                   |
           |                           |---------------------------------------->|
           |                           |                                         |
           |                           |                                         |
```

##OVSDB-Schema
--------------

Following new columns are added to DHCP-Relay tables for DHCPv6-Relay functionality.

Tables :

DHCP_Relay : Place holder for server configuration for dhcpv6-relay feature.
DHCP_Relay_Statistics : Place holder for DHCPv6 relayed packets statistics.

```

Table descriptions:

  DHCP_Relay Table:
    ipv6_ucast_server : List of IPv6 server destinations
    ipv6_mcast_server : Key, value pairs of multicast or link local destination and the output port

  Dhcp_Relay_Statistics:
    ipv6_stats : IPv6 dhcp-relay packet statistics

```

##Any other sections that are relevant for the module
-----------------------------------------------------
N/A


##References
------------
Dynamic Host Configuration Protocol for IPv6(https://tools.ietf.org/html/rfc3315)
