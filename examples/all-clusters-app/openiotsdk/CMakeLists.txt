#
#   Copyright (c) 2023 Project CHIP Authors
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

cmake_minimum_required(VERSION 3.21)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
get_filename_component(OPEN_IOT_SDK_CONFIG ${CHIP_ROOT}/config/openiotsdk REALPATH)
get_filename_component(OPEN_IOT_SDK_EXAMPLE_COMMON ${CHIP_ROOT}/examples/platform/openiotsdk REALPATH)
get_filename_component(ALL_CLUSTERS_COMMON ${CHIP_ROOT}/examples/all-clusters-app/all-clusters-common REALPATH)
get_filename_component(ENERGY_MANAGEMENT_COMMON ${CHIP_ROOT}/examples/energy-management-app/energy-management-common/ REALPATH)

list(APPEND CMAKE_MODULE_PATH ${OPEN_IOT_SDK_CONFIG}/cmake)

set(APP_TARGET chip-openiotsdk-all-clusters-app-example_ns)

set(TFM_PROJECT_CONFIG_HEADER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/tf-m-config/TfmProjectConfig.h")

# Toolchain files need to exist before first call to project
include(toolchain)

project(${APP_TARGET} LANGUAGES C CXX ASM)

include(sdk)

add_executable(${APP_TARGET})

# Application CHIP build configuration
include(chip)

add_subdirectory(${OPEN_IOT_SDK_EXAMPLE_COMMON}/app ./app_build)

chip_add_data_model(openiotsdk-app PUBLIC all-clusters)

target_include_directories(${APP_TARGET}
    PRIVATE
        main/include
        ${ALL_CLUSTERS_COMMON}/include
        ${ENERGY_MANAGEMENT_COMMON}/common/include
        ${ENERGY_MANAGEMENT_COMMON}/device-energy-management/include
        ${ENERGY_MANAGEMENT_COMMON}/energy-evse/include
        ${ENERGY_MANAGEMENT_COMMON}/energy-reporting/include
)

target_sources(${APP_TARGET}
    PRIVATE
        main/main_ns.cpp
        ${ALL_CLUSTERS_COMMON}/src/smco-stub.cpp
        ${ALL_CLUSTERS_COMMON}/src/bridged-actions-stub.cpp
        ${ALL_CLUSTERS_COMMON}/src/air-quality-instance.cpp
        ${ALL_CLUSTERS_COMMON}/src/chime-instance.cpp
        ${ALL_CLUSTERS_COMMON}/src/concentration-measurement-instances.cpp
        ${ALL_CLUSTERS_COMMON}/src/fan-stub.cpp
        ${ALL_CLUSTERS_COMMON}/src/oven-modes.cpp
        ${ALL_CLUSTERS_COMMON}/src/device-energy-management-stub.cpp
        ${ALL_CLUSTERS_COMMON}/src/energy-evse-stub.cpp
        ${ALL_CLUSTERS_COMMON}/src/resource-monitoring-delegates.cpp
        ${ALL_CLUSTERS_COMMON}/src/static-supported-modes-manager.cpp
        ${ALL_CLUSTERS_COMMON}/src/binding-handler.cpp
        ${ENERGY_MANAGEMENT_COMMON}/common/src/EnergyTimeUtils.cpp        
        ${ENERGY_MANAGEMENT_COMMON}/device-energy-management/src/DeviceEnergyManagementDelegateImpl.cpp
        ${ENERGY_MANAGEMENT_COMMON}/device-energy-management/src/DeviceEnergyManagementManager.cpp
        ${ENERGY_MANAGEMENT_COMMON}/energy-evse/src/ChargingTargetsMemMgr.cpp
        ${ENERGY_MANAGEMENT_COMMON}/energy-evse/src/EVSEManufacturerImpl.cpp
        ${ENERGY_MANAGEMENT_COMMON}/energy-evse/src/EnergyEvseDelegateImpl.cpp
        ${ENERGY_MANAGEMENT_COMMON}/energy-evse/src/EnergyEvseManager.cpp
        ${ENERGY_MANAGEMENT_COMMON}/energy-evse/src/EnergyEvseTargetsStore.cpp
        ${ENERGY_MANAGEMENT_COMMON}/energy-reporting/src/ElectricalPowerMeasurementDelegate.cpp
)

target_link_libraries(${APP_TARGET}
    openiotsdk-startup
    openiotsdk-app
)

include(linker)
set_target_link(${APP_TARGET})

sdk_post_build(${APP_TARGET})
