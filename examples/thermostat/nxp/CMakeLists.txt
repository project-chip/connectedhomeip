#
# Copyright (c) 2024-2025 Project CHIP Authors
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
#

# ****************************************************************
# Matter Thermostat application for NXP FreeRTOS platforms
# ****************************************************************

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

get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)
get_filename_component(NXP_EXAMPLE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common REALPATH)
get_filename_component(NXP_EXAMPLE_ZAP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zap REALPATH)
get_filename_component(ALL_CLUSTERS_COMMON_DIR ${CHIP_ROOT}/examples/all-clusters-app/all-clusters-common REALPATH)
get_filename_component(EXAMPLE_PLATFORM_NXP_COMMON_DIR ${CHIP_ROOT}/examples/platform/nxp/common REALPATH)
get_filename_component(NXP_MATTER_SUPPORT_DIR ${CHIP_ROOT}/third_party/nxp/nxp_matter_support REALPATH)

# Add the CHIP stack & NXP port libs as an external module
list(APPEND EXTRA_MCUX_MODULES ${CHIP_ROOT}/config/nxp/chip-cmake-freertos)

# If CONF_FILE_NAME is provided, add the full path in front of CONF_FILE
if (DEFINED CONF_FILE_NAME)
    set(CONF_FILE ${CHIP_ROOT}/examples/platform/nxp/config/${CONF_FILE_NAME} ${CONF_FILE})
endif()

# Add application prj.conf in front of CONF_FILE
set(CONF_FILE ${CMAKE_CURRENT_SOURCE_DIR}/prj.conf ${CONF_FILE})

set(KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Kconfig)

# Load the MCUX SDK package
find_package(McuxSDK 3.0.0)

# Get compiler flags and includes and pass them to CHIP target
list(APPEND CONFIG_CHIP_EXTERNAL_TARGETS McuxSDK)

if (NOT DEFINED NXP_SDK_RECONFIG_CMAKE_DIR)
    get_filename_component(NXP_SDK_RECONFIG_CMAKE_DIR ${NXP_MATTER_SUPPORT_DIR}/examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FAMILY} REALPATH)
endif()

# Include the nxp_sdk_reconfig.cmake to customize the SDK for Matter application
include(${NXP_SDK_RECONFIG_CMAKE_DIR}/nxp_sdk_reconfig.cmake)

# Define application project name
project(chip-nxp-thermostat-app-example)

# Add application definitions, include paths and source files
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

# Add NXP common application files
include(${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_common.cmake)

target_include_directories(app
    PRIVATE
    ${ALL_CLUSTERS_COMMON_DIR}/include
    ${NXP_EXAMPLE_DIR}/main/include
    ${GEN_DIR}/app-common
)

target_sources(app
    PRIVATE
    ${NXP_EXAMPLE_DIR}/main/main.cpp
    ${NXP_EXAMPLE_DIR}/main/AppTask.cpp
    ${NXP_EXAMPLE_DIR}/main/DeviceCallbacks.cpp
    ${NXP_EXAMPLE_DIR}/main/ZclCallbacks.cpp
    ${ALL_CLUSTERS_COMMON_DIR}/src/binding-handler.cpp
)

if(CONFIG_CHIP_WIFI AND CONFIG_NET_L2_OPENTHREAD)
    chip_configure_data_model(app
        INCLUDE_SERVER
        ZAP_FILE ${NXP_EXAMPLE_ZAP_DIR}/thermostat_matter_br.zap
    )
elseif(CONFIG_CHIP_WIFI AND NOT CONFIG_NET_L2_OPENTHREAD)
    chip_configure_data_model(app
        INCLUDE_SERVER
        ZAP_FILE ${NXP_EXAMPLE_ZAP_DIR}/thermostat_matter_wifi.zap
    )
elseif(CONFIG_NET_L2_OPENTHREAD AND NOT CONFIG_CHIP_WIFI)
    chip_configure_data_model(app
        INCLUDE_SERVER
        ZAP_FILE ${NXP_EXAMPLE_ZAP_DIR}/thermostat_matter_thread.zap
    )
elseif(CONFIG_CHIP_ETHERNET)
    chip_configure_data_model(app
        INCLUDE_SERVER
        ZAP_FILE ${NXP_EXAMPLE_ZAP_DIR}/thermostat_matter_eth.zap
    )
endif()

if(CONFIG_CHIP_WIFI AND CONFIG_NET_L2_OPENTHREAD)
    target_compile_definitions(app PRIVATE
        CHIP_DEVICE_CONFIG_ENABLE_TBR=1
    )
endif()
