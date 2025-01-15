#
#   Copyright (c) 2024 Project CHIP Authors
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

cmake_minimum_required(VERSION 3.13.1)

project(chip-rw61x-all-cluster-example)

if (NOT CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../../.. REALPATH)
endif()

# Define the GN root target for the application
get_filename_component(GN_ROOT_TARGET ${CMAKE_CURRENT_LIST_DIR} REALPATH)

# Include platform specific configs / variables
include(${CHIP_ROOT}/examples/platform/nxp/rt/rw61x/platform_config.cmake)

add_subdirectory(${CHIP_ROOT}/config/nxp/chip-cmake-freertos ./chip)

#**************************************************************************************
# Build example application
#**************************************************************************************
if(CONFIG_CHIP_BUILD_APP_WITH_GN)
    nxp_build_app_with_gn()
endif()
