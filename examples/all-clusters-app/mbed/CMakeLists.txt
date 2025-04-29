# Copyright (c) 2021 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
get_filename_component(MBED_COMMON ${CHIP_ROOT}/examples/platform/mbed REALPATH)
get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)
get_filename_component(ALL_CLUSTERS_COMMON ${CHIP_ROOT}/examples/all-clusters-app/all-clusters-common REALPATH)
get_filename_component(ENERGY_MANAGEMENT_COMMON ${CHIP_ROOT}/examples/energy-management-app/energy-management-common/ REALPATH)
get_filename_component(NLIO_ROOT ${CHIP_ROOT}/third_party/nlio/repo/include REALPATH)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.in
    ${CMAKE_CURRENT_BINARY_DIR}/chip_build/config
    @ONLY
)

set(MBED_PATH ${MBED_OS_PATH} CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(MCUBOOT_PATH ${MBED_MCU_BOOT_PATH} CACHE INTERNAL "")
set(APP_PATH ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")
set(APP_TYPE ${MBED_APP_TYPE} CACHE INTERNAL "")
set(BOOT_ENABLED FALSE)
set(APP_TARGET chip-mbed-all-clusters-app-example)

if(APP_TYPE STREQUAL "boot" OR APP_TYPE STREQUAL "upgrade")
    set(BOOT_ENABLED TRUE)
endif()

include(${MBED_PATH}/tools/cmake/app.cmake)
if(MBED_TARGET STREQUAL "CY8CPROTO_062_4343W" AND BOOT_ENABLED)
    list(REMOVE_ITEM MBED_TARGET_LABELS CM0P_SLEEP)
    list(REMOVE_ITEM MBED_TARGET_DEFINITIONS COMPONENT_CM0P_SLEEP=1)
endif()
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

project(${APP_TARGET})

add_subdirectory(${MBED_PATH} ./mbed_build)
add_subdirectory(${MBED_OS_POSIX_SOCKET_PATH} ./mbed_os_posix_socket_build)

add_executable(${APP_TARGET})

add_subdirectory(${CHIP_ROOT}/config/mbed ./chip_build)

mbed_configure_app_target(${APP_TARGET})

target_include_directories(${APP_TARGET} PRIVATE
                           main/include/
                           ${MBED_COMMON}/util/include
                           ${ALL_CLUSTERS_COMMON}/include
                           ${ENERGY_MANAGEMENT_COMMON}/common/include
                           ${ENERGY_MANAGEMENT_COMMON}/device-energy-management/include
                           ${ENERGY_MANAGEMENT_COMMON}/energy-evse/include
                           ${ENERGY_MANAGEMENT_COMMON}/energy-reporting/include
                           ${GEN_DIR}/app-common
                           ${GEN_DIR}/all-clusters-app
                           ${NLIO_ROOT}
)

target_sources(${APP_TARGET} PRIVATE
               main/main.cpp
               main/AppTask.cpp
               ${MBED_COMMON}/util/LEDWidget.cpp
               ${MBED_COMMON}/util/DFUManager.cpp
               ${ALL_CLUSTERS_COMMON}/src/bridged-actions-stub.cpp
               ${ALL_CLUSTERS_COMMON}/src/air-quality-instance.cpp
               ${ALL_CLUSTERS_COMMON}/src/chime-instance.cpp
               ${ALL_CLUSTERS_COMMON}/src/concentration-measurement-instances.cpp
               ${ALL_CLUSTERS_COMMON}/src/fan-stub.cpp
               ${ALL_CLUSTERS_COMMON}/src/device-energy-management-stub.cpp
               ${ALL_CLUSTERS_COMMON}/src/energy-evse-stub.cpp
               ${ALL_CLUSTERS_COMMON}/src/oven-modes.cpp
               ${ALL_CLUSTERS_COMMON}/src/resource-monitoring-delegates.cpp
               ${ALL_CLUSTERS_COMMON}/src/smco-stub.cpp
               ${ALL_CLUSTERS_COMMON}/src/static-supported-modes-manager.cpp
               ${ALL_CLUSTERS_COMMON}/src/static-supported-temperature-levels.cpp
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

chip_configure_data_model(${APP_TARGET}
    ZAP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../all-clusters-common/all-clusters-app.zap
)

target_link_libraries(${APP_TARGET} mbed-os-posix-socket mbed-os mbed-ble mbed-events mbed-netsocket mbed-storage mbed-storage-kv-global-api mbed-mbedtls mbed-emac chip)

if(MBED_TARGET STREQUAL "CY8CPROTO_062_4343W")
    target_link_libraries(${APP_TARGET} mbed-cy-psoc6-common-network)
endif()

mbed_set_post_build(${APP_TARGET})

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
