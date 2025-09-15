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
get_filename_component(NXP_EXAMPLE_ZAP_LIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zap-lit REALPATH)
get_filename_component(NXP_EXAMPLE_ZAP_SIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zap-sit REALPATH)
get_filename_component(EXAMPLE_PLATFORM_NXP_COMMON_DIR ${CHIP_ROOT}/examples/platform/nxp/common REALPATH)
get_filename_component(NXP_MATTER_SUPPORT_DIR ${CHIP_ROOT}/third_party/nxp/nxp_matter_support REALPATH)

# Add the CHIP stack & NXP port libs as an external module
list(APPEND EXTRA_MCUX_MODULES ${CHIP_ROOT}/config/nxp/chip-cmake-freertos)

# If CONF_FILE_NAME is provided, append its full path to CONF_FILE
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

project(chip-nxp-contact-sensor-app-example)

#**********************************************************
# Add Application files to the build
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

# Add NXP common application files
include(${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_common.cmake)

target_include_directories(app
    PRIVATE
    ${NXP_EXAMPLE_DIR}
    ${NXP_EXAMPLE_DIR}/include
    ${GEN_DIR}/app-common
)

target_sources(app
    PRIVATE
    ${NXP_EXAMPLE_DIR}/AppTask.cpp
    ${NXP_EXAMPLE_DIR}/DeviceCallbacks.cpp
    ${NXP_EXAMPLE_DIR}/main.cpp
    ${NXP_EXAMPLE_DIR}/ZclCallbacks.cpp
)

# This is a temporary workaround to avoid a popular linker limitation.
# The linker is unable to replace weak symbols with their strong
# counterparts when both weak and strong symbols are hosted in a static
# library. A workaround for this issue is to add the object files which
# are defining the strong symbols directly to the linker command line.
if(CONFIG_CHIP_NXP_PLATFORM_MCXW71 OR CONFIG_CHIP_NXP_PLATFORM_MCXW72)
    target_sources(app
        PRIVATE
        ${SdkRootDirPath}/middleware/multicore/mcmgr/src/mcmgr_imu_internal.c
        ${SdkRootDirPath}/middleware/wireless/framework/services/LowPower/PWR_systicks.c
        ${SdkRootDirPath}/components/gpio/fsl_adapter_gpio.c
    )
endif()

if(CONFIG_CHIP_NXP_PLATFORM_MCXW72 AND CONFIG_NXP_USE_LOW_POWER)
    target_sources(app
        PRIVATE
        ${NXP_MATTER_SUPPORT_DIR}/examples/platform/common/low_power/freertos_lp_hooks.c
    )
endif()

if(CONFIG_CHIP_WIFI AND CONFIG_NET_L2_OPENTHREAD)
    message(FATAL_ERROR "WiFi + Thread (Border Router) configuration is not supported in this application.")
else()
    if(CONFIG_CHIP_ICD_LIT_SUPPORT)
        set(ZAP_FILE_PATH ${NXP_EXAMPLE_ZAP_LIT_DIR}/contact-sensor-app.zap)
    else()
        set(ZAP_FILE_PATH ${NXP_EXAMPLE_ZAP_SIT_DIR}/contact-sensor-app.zap)
    endif()

    chip_configure_data_model(app
        INCLUDE_SERVER
        ZAP_FILE ${ZAP_FILE_PATH}
    )
endif()
