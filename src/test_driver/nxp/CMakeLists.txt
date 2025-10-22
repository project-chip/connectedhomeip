#
# Copyright (c) 2025 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


cmake_minimum_required(VERSION 3.30)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Retrieve CHIP_ROOT if provided
if (DEFINED CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CHIP_ROOT} ABSOLUTE)
else()
    get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../ REALPATH)
endif()

# Export CHIP_ROOT as an environment variable to be recognized by Kconfig
set(COMMON_KCONFIG_ENV_SETTINGS CHIP_ROOT=${CHIP_ROOT})

get_filename_component(EXAMPLE_PLATFORM_NXP_COMMON_DIR ${CHIP_ROOT}/examples/platform/nxp/common REALPATH)
get_filename_component(NXP_MATTER_SUPPORT_DIR ${CHIP_ROOT}/third_party/nxp/nxp_matter_support REALPATH)

# Add the CHIP stack & NXP port libs as an external module
list(APPEND EXTRA_MCUX_MODULES ${CHIP_ROOT}/config/nxp/chip-cmake-freertos)

# Add secure element lib as an external module
list(APPEND EXTRA_MCUX_MODULES ${CHIP_ROOT}/third_party/simw-top-mini/repo/matter)

# If CONF_FILE_NAME is provided, add the full path in front of CONF_FILE
if (DEFINED CONF_FILE_NAME)
    set(CONF_FILE ${CHIP_ROOT}/examples/platform/nxp/config/${CONF_FILE_NAME} ${CONF_FILE})
endif()

# Add application prj.conf & Kconfig
set(CONF_FILE ${CONF_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/prj.conf)

set(KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Kconfig)

# Load the MCUX SDK package
find_package(McuxSDK 3.0.0)

# Get SDK compiler flags and includes and pass them to CHIP target
list(APPEND CONFIG_CHIP_EXTERNAL_TARGETS McuxSDK)

if (NOT DEFINED NXP_SDK_RECONFIG_CMAKE_DIR)
    get_filename_component(NXP_SDK_RECONFIG_CMAKE_DIR ${NXP_MATTER_SUPPORT_DIR}/examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FAMILY} REALPATH)
endif()

# Include the nxp_sdk_reconfig.cmake to customize the SDK for Matter application
include(${NXP_SDK_RECONFIG_CMAKE_DIR}/nxp_sdk_reconfig.cmake)

get_filename_component(UNIT_TEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../ REALPATH)

# Define application project name
project(chip-nxp-unit-test-app)

target_sources(app
    PRIVATE
    ./src/main.cpp
)

# This is a temporary workaround to avoid a popular linker limitation.
# The linker is unable to replace weak symbols with their strong
# counterparts when both weak and strong symbols are hosted in a static
# library. A solution for this issue is to add the object files which
# are defining the strong symbols directly to the linker command line.
if(CONFIG_CHIP_NXP_PLATFORM_MCXW71 OR CONFIG_CHIP_NXP_PLATFORM_MCXW72)
    target_sources(app
        PRIVATE
        ${SdkRootDirPath}/middleware/multicore/mcmgr/src/mcmgr_imu_internal.c
        ${SdkRootDirPath}/components/gpio/fsl_adapter_gpio.c
    )
endif()

set(tests_list)

if(NOT "${CONFIG_NXP_CUSTOM_UNIT_TESTS_DIR}" STREQUAL "")
# read test_components.txt file which contain the list of NXP test modules to be built
file(STRINGS ${CONFIG_NXP_CUSTOM_UNIT_TESTS_DIR}/test_components.txt TEST_NAMES_FROM_FILE)
else()
# read test_components.txt file which contain the list of Matter test modules to be built
file(STRINGS test_components.txt TEST_NAMES_FROM_FILE)
endif()

foreach(TEST_NAME IN LISTS TEST_NAMES_FROM_FILE)
# add the the path of the library to be linked
list(APPEND tests_list "${CMAKE_CURRENT_BINARY_DIR}/modules/chip/lib/${TEST_NAME}.a")
endforeach()

# Link the *whole-archives* to keep the static test objects for each tests library previously selected.
target_link_options(app
PUBLIC
    -Wl,--whole-archive
    "${CMAKE_CURRENT_BINARY_DIR}/modules/chip/lib/libPWTestsWrapper.a"
    -Wl,--no-whole-archive
    -Wl,--start-group,--whole-archive
    ${tests_list}
    -Wl,--end-group,--no-whole-archive
    )