#
#   Copyright (c) 2025-2026 Project CHIP Authors
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

# ****************************************************************
# Matter all-devices-app (code-driven) for NXP FreeRTOS platforms
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
get_filename_component(ALL_DEVICES_COMMON_DIR ${CHIP_ROOT}/examples/all-devices-app/all-devices-common REALPATH)
get_filename_component(NXP_EXAMPLE_DIR ${CHIP_ROOT}/examples/all-devices-app/nxp/common REALPATH)
get_filename_component(EXAMPLE_PLATFORM_NXP_COMMON_DIR ${CHIP_ROOT}/examples/platform/nxp/common REALPATH)
get_filename_component(NXP_MATTER_SUPPORT_DIR ${CHIP_ROOT}/third_party/nxp/nxp_matter_support REALPATH)

# Enable the selected device types; an empty list enables all types.
include(${ALL_DEVICES_COMMON_DIR}/device-factory/enabled_devices.cmake)

# Add the CHIP stack & NXP port libs as an external module
list(APPEND EXTRA_MCUX_MODULES ${CHIP_ROOT}/config/nxp/chip-cmake-freertos)

# Add secure element lib as an external module
list(APPEND EXTRA_MCUX_MODULES ${CHIP_ROOT}/third_party/simw-top-mini/repo/matter)

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

include(${NXP_SDK_RECONFIG_CMAKE_DIR}/nxp_sdk_reconfig.cmake)
project(chip-nxp-all-devices-app-example)

if (NOT CONFIG_APP_FREERTOS_OS)
    message(FATAL_ERROR "This example requires FreeRTOS")
endif()

if (NOT CONFIG_CHIP_DATA_MODEL_CODEDRIVEN)
    message(FATAL_ERROR "all-devices-app requires CONFIG_CHIP_DATA_MODEL_CODEDRIVEN=y")
endif()
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

# Add NXP common application files
include(${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_common.cmake)

target_include_directories(app
    PRIVATE
    ${NXP_EXAMPLE_DIR}/main/include
    ${ALL_DEVICES_COMMON_DIR}
    ${GEN_DIR}/app-common
    ${CMAKE_CURRENT_BINARY_DIR}
)

# NXP application sources
target_sources(app
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/CHIPProjectConfig.h
    ${NXP_EXAMPLE_DIR}/main/main.cpp
    ${NXP_EXAMPLE_DIR}/main/AppTask.cpp
    ${NXP_EXAMPLE_DIR}/main/include/AppTask.h
    ${NXP_EXAMPLE_DIR}/main/include/AppEvent.h
    ${NXP_EXAMPLE_DIR}/main/DeviceCallbacks.cpp
    ${NXP_EXAMPLE_DIR}/main/include/DeviceCallbacks.h
    ${NXP_EXAMPLE_DIR}/main/DeviceShellCommands.cpp
    ${NXP_EXAMPLE_DIR}/main/include/DeviceShellCommands.h
)

# Code-driven data model and common device/cluster sources
target_sources(app
    PRIVATE
    ${CHIP_ROOT}/src/data-model-providers/codedriven/CodeDrivenDataModelProvider.cpp
    ${CHIP_ROOT}/src/data-model-providers/codedriven/endpoint/EndpointInterfaceRegistry.cpp
    ${CHIP_ROOT}/src/data-model-providers/codedriven/endpoint/SpanEndpoint.cpp
    ${CHIP_ROOT}/src/app/server-cluster/ServerClusterInterfaceRegistry.cpp
    ${CHIP_ROOT}/src/app/server-cluster/SingleEndpointServerClusterRegistry.cpp
    ${CHIP_ROOT}/src/app/persistence/DefaultAttributePersistenceProvider.cpp
    ${ALL_DEVICES_COMMON_DIR}/providers/AllDevicesExampleDeviceInfoProviderImpl.cpp
    ${ALL_DEVICES_DEVICE_SOURCES}
    ${ALL_DEVICES_CLUSTER_SOURCES}
)

if (CONFIG_NET_L2_OPENTHREAD)
    target_sources(app
        PRIVATE
        ${ALL_DEVICES_COMMON_DIR}/device/types/root-node/features/ThreadFeature.cpp
    )
elseif (CONFIG_CHIP_WIFI)
    target_sources(app
        PRIVATE
        ${ALL_DEVICES_COMMON_DIR}/device/types/root-node/features/WifiFeature.cpp
    )
endif()

# The code-driven model does not link the ZAP OTA integration or its requestor sources.
if (CONFIG_CHIP_OTA_REQUESTOR)
    target_sources(app
        PRIVATE
        ${ALL_DEVICES_COMMON_DIR}/device/types/root-node/features/OtaFeature.cpp
        ${CHIP_ROOT}/src/app/clusters/ota-requestor/BDXDownloader.cpp
        ${CHIP_ROOT}/src/app/clusters/ota-requestor/DefaultOTARequestor.cpp
        ${CHIP_ROOT}/src/app/clusters/ota-requestor/DefaultOTARequestorDriver.cpp
        ${CHIP_ROOT}/src/app/clusters/ota-requestor/DefaultOTARequestorStorage.cpp
        ${CHIP_ROOT}/src/app/clusters/ota-requestor/OTARequestorAttributes.cpp
        ${CHIP_ROOT}/src/app/clusters/ota-requestor/OTARequestorCluster.cpp
    )
endif()
