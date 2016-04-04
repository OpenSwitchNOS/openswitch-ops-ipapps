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

project ("ops_diagtools")
set (NWDIAG nwdiag)

# Define compile flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Werror")

include_directories (${PROJECT_SOURCE_DIR})

# Source files
set (SOURCES ${PROJECT_SOURCE_DIR}/nwdiag.c)

# Rules to build nwdiag
add_executable (${NWDIAG} ${SOURCES})

# Rules to install nwdiag binary in rootfs
install(TARGETS ${NWDIAG}
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE SETUID
    RUNTIME DESTINATION bin)
