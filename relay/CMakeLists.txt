# Copyright (C) 2016 Hewlett Packard Enterprise Development LP
# All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.

cmake_minimum_required (VERSION 2.8)

project ("ops-relay")

set (RELAY ops-relay)
project (${RELAY})
set (SRC_DIR_UDPFWD udpfwd/src)
set (INCL_DIR_UDPFWD udpfwd/include)
set (SRC_DIR_DHCPV6 dhcpv6_relay/src)
set (INCL_DIR_DHCPV6 dhcpv6_relay/include)

# Rules to locate needed libraries
include(FindPkgConfig)
pkg_check_modules(OVSCOMMON REQUIRED libovscommon)
pkg_check_modules(OVSDB REQUIRED libovsdb)

include_directories(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/${INCL_DIR_DHCPV6}
  ${PROJECT_SOURCE_DIR}/${INCL_DIR_UDPFWD}
  ${PROJECT_SOURCE_DIR}
  ${OVSCOMMON_INCLUDE_DIRS})

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -D FTR_UDP_BCAST_FWD=1 -D FTR_DHCP_RELAY=1 -D FTR_DHCPV6_RELAY=1")

# Source files to build ops-relay
set (SOURCES ${PROJECT_SOURCE_DIR}/relay_main.c
             ${SRC_DIR_UDPFWD}/udpfwd.c
             ${SRC_DIR_UDPFWD}/udpfwd_config.c
             ${SRC_DIR_UDPFWD}/udpfwd_util.c
             ${SRC_DIR_UDPFWD}/udpfwd_xmit.c
             ${SRC_DIR_UDPFWD}/udpfwd_recv.c
             ${SRC_DIR_UDPFWD}/dhcp_options.c
             ${SRC_DIR_DHCPV6}/dhcpv6r.c
             ${SRC_DIR_DHCPV6}/dhcpv6r_config.c
             )

# Rules to build ops-relay
add_executable (${RELAY} ${SOURCES})
target_link_libraries (${RELAY} ${OVSCOMMON_LIBRARIES}
                       ${OVSDB_LIBRARIES}
                       -lpthread -lrt)

# Build udpfwd cli shared libraries.
add_subdirectory(udpfwd/src/cli)

# Rules to install ops-relay binary in rootfs
install(TARGETS ${RELAY}
    RUNTIME DESTINATION bin)
