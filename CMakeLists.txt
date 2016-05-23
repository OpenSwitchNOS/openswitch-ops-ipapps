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

project ("ops-ipapps")

# Build ops-relay daemon.
add_subdirectory(relay)
# Build diagtools libraries.
add_subdirectory(diagtools/src/cli)
# Build nwdiag
add_subdirectory(diagtools)
# Build apps libraries
add_subdirectory(apps/sourceinterface/src/cli)
add_subdirectory(apps/sftp/src/cli)
