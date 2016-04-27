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

set (RELAY ops-udpfwd)
project (${RELAY})
set (COMMON_SRC_DIR common)
set (UDPFWD_SRC_DIR udpfwd/src)
set (DHCPV6R_SRC_DIR dhcpv6r/src)
set (INCL_DIR include)

# Rules to locate needed libraries
include(FindPkgConfig)
pkg_check_modules(OVSCOMMON REQUIRED libovscommon)
pkg_check_modules(OVSDB REQUIRED libovsdb)

include_directories (${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/${INCL_DIR}
                     ${PROJECT_SOURCE_DIR}/${INCL_DIR}/udpfwd
                     ${PROJECT_SOURCE_DIR}/${INCL_DIR}/dhcpv6r
                     ${OVSCOMMON_INCLUDE_DIRS}
)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -D FTR_UDP_BCAST_FWD=1 -D FTR_DHCP_RELAY=1")

# Source files to build ops-udpfwd
set (SOURCES ${COMMON_SRC_DIR}/relay_main.c
             ${UDPFWD_SRC_DIR}/udpfwd.c
             ${UDPFWD_SRC_DIR}/udpfwd_config.c
             ${UDPFWD_SRC_DIR}/udpfwd_util.c
             ${UDPFWD_SRC_DIR}/udpfwd_xmit.c
             ${UDPFWD_SRC_DIR}/udpfwd_recv.c
             ${UDPFWD_SRC_DIR}/dhcp_options.c
             ${DHCPV6R_SRC_DIR}/dhcpv6_relay.c)

# Rules to build ops-udpfwd
add_executable (${RELAY} ${SOURCES})
target_link_libraries (${RELAY} ${OVSCOMMON_LIBRARIES}
                       ${OVSDB_LIBRARIES}
                       -lpthread -lrt)

# Build ops-udpfwd cli shared libraries.
add_subdirectory(${UDPFWD_SRC_DIR}/cli)

# Rules to install ops-udpfwd binary in rootfs
install(TARGETS ${RELAY}
    RUNTIME DESTINATION bin)
