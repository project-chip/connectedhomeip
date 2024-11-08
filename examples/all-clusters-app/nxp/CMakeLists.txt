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

# ****************************************************************
# Matter All-clusters application for NXP FreeRTOS platforms
# ****************************************************************

cmake_minimum_required(VERSION 3.30)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../ REALPATH)
get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)
get_filename_component(ALL_CLUSTERS_COMMON_DIR ${CHIP_ROOT}/examples/all-clusters-app/all-clusters-common REALPATH)
get_filename_component(ALL_CLUSTERS_NXP_COMMON_DIR ${CHIP_ROOT}/examples/all-clusters-app/nxp/common REALPATH)
get_filename_component(EXAMPLE_PLATFORM_NXP_COMMON_DIR ${CHIP_ROOT}/examples/platform/nxp/common REALPATH)

# Add the CHIP stack & NXP port libs as an external module
list(APPEND EXTRA_MCUX_MODULES ${CHIP_ROOT}/config/nxp/chip-cmake-freertos)

# Add application prj.conf & Kconfig
set(CONF_FILE ${CONF_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/prj.conf)

set(KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Kconfig)

# Load the MCUX SDK package
find_package(McuxSDK 3.0.0)

# Get compiler flags and includes and pass them to CHIP target
list(APPEND CONFIG_CHIP_EXTERNAL_TARGETS McuxSDK)

# Include the nxp_sdk_reconfigure.cmake to customize the SDK for Matter application
include(${CHIP_ROOT}/third_party/nxp/nxp_matter_support/examples/platform/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/nxp_sdk_reconfig.cmake)

# Define application project name
project(chip-nxp-all-clusters-app-example)

# Add application definitions, include paths and source files
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

target_include_directories(app
    PRIVATE
    ${ALL_CLUSTERS_NXP_COMMON_DIR}/main/include
    ${ALL_CLUSTERS_COMMON_DIR}/include
    ${GEN_DIR}/app-common
    ${GEN_DIR}/all-clusters-app
    ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/device_manager/include
    ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/icd/include
    ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/device_callbacks/include
    ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/factory_data/include
    ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_task/include
    ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/matter_button/include
)

target_sources(app
    PRIVATE
    ${ALL_CLUSTERS_NXP_COMMON_DIR}/main/main.cpp
    ${ALL_CLUSTERS_NXP_COMMON_DIR}/main/AppTask.cpp
    ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/device_manager/source/CHIPDeviceManager.cpp
    ${ALL_CLUSTERS_NXP_COMMON_DIR}/main/DeviceCallbacks.cpp
    ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/device_callbacks/source/CommonDeviceCallbacks.cpp
    ${ALL_CLUSTERS_NXP_COMMON_DIR}/main/ZclCallbacks.cpp
    ${ALL_CLUSTERS_COMMON_DIR}/src/binding-handler.cpp
    ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_task/source/AppTaskBase.cpp
    ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_task/source/AppTaskFreeRTOS.cpp
    ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/matter_button/source/ButtonRegistrationEmpty.cpp
)

chip_configure_data_model(app
    INCLUDE_SERVER
    ZAP_FILE ${ALL_CLUSTERS_COMMON_DIR}/all-clusters-app.zap
)

target_sources(app
    PRIVATE
    ${ALL_CLUSTERS_COMMON_DIR}/src/smco-stub.cpp
    ${ALL_CLUSTERS_COMMON_DIR}/src/static-supported-modes-manager.cpp
    ${ALL_CLUSTERS_COMMON_DIR}/src/static-supported-temperature-levels.cpp
    ${ALL_CLUSTERS_COMMON_DIR}/src/bridged-actions-stub.cpp
    ${ALL_CLUSTERS_COMMON_DIR}/src/fan-stub.cpp
    ${ALL_CLUSTERS_COMMON_DIR}/src/air-quality-instance.cpp
    ${ALL_CLUSTERS_COMMON_DIR}/src/concentration-measurement-instances.cpp
    ${ALL_CLUSTERS_COMMON_DIR}/src/resource-monitoring-delegates.cpp
    ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/icd/source/ICDUtil.cpp
)

if (CONFIG_CHIP_ENABLE_SECURE_WHOLE_FACTORY_DATA OR CONFIG_CHIP_ENABLE_EL2GO_FACTORY_DATA)
    target_sources(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/factory_data/source/AppFactoryDataExample.cpp
    )
    if (CONFIG_CHIP_ENABLE_SECURE_WHOLE_FACTORY_DATA)
        target_compile_definitions(app PRIVATE
            ENABLE_SECURE_WHOLE_FACTORY_DATA
        )
        target_compile_definitions(app 
            PUBLIC
            EXTERNAL_FACTORY_DATA_PROVIDER_IMPL_HEADER=\"platform/nxp/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/FactoryDataProviderEncImpl.h\"
        )
    else()
        target_compile_definitions(app 
            PUBLIC
            EXTERNAL_FACTORY_DATA_PROVIDER_IMPL_HEADER=\"platform/nxp/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/FactoryDataProviderEl2GoImpl.h\"
        )
    endif()
else()
    target_sources(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/factory_data/source/AppFactoryDataDefaultImpl.cpp
    )
    target_compile_definitions(app 
        PUBLIC
        EXTERNAL_FACTORY_DATA_PROVIDER_IMPL_HEADER=\"platform/nxp/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/FactoryDataProviderImpl.h\"
    )
endif()

if (CONFIG_CHIP_LIB_SHELL)
    target_compile_definitions(app PRIVATE
        ENABLE_CHIP_SHELL
    )

    target_sources(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/matter_cli/source/AppCLIBase.cpp
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/matter_cli/source/AppCLIFreeRTOS.cpp
        ${CHIP_ROOT}/examples/shell/shell_common/cmd_misc.cpp
        ${CHIP_ROOT}/examples/shell/shell_common/cmd_otcli.cpp
        ${CHIP_ROOT}/examples/shell/shell_common/cmd_server.cpp
    )

    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/matter_cli/include
        ${CHIP_ROOT}/examples/shell/shell_common/include
    )
endif()

if (CONFIG_CHIP_OTA_REQUESTOR)
    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/ota_requestor/include
    )

    target_sources(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/ota_requestor/source/OTARequestorInitiator.cpp
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/ota_requestor/source/OTARequestorInitiatorCommon.cpp
    )
endif()