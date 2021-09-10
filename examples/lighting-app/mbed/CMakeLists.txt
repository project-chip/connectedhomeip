# Copyright (c) 2021 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
get_filename_component(APP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/.. REALPATH)
get_filename_component(MBED_COMMON ${CHIP_ROOT}/examples/platform/mbed REALPATH)
get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.in
    ${CMAKE_CURRENT_BINARY_DIR}/chip_build/config
    @ONLY
)

set(MBED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(APP_TARGET chip-mbed-lighting-app-example)

include(${MBED_PATH}/tools/cmake/app.cmake)
add_subdirectory(${MBED_PATH} ./mbed_build)

add_subdirectory(mbed-os-posix-socket)

if("wifi_ism43362" IN_LIST MBED_TARGET_LABELS)
    add_subdirectory(wifi-ism43362)
endif()

add_executable(${APP_TARGET})

target_include_directories(${APP_TARGET} PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_CURRENT_SOURCE_DIR}/main/include/
                           ${APP_ROOT}/lighting-common
                           ${MBED_COMMON}/util/include
                           ${GEN_DIR}/app-common
                           ${GEN_DIR}/lighting-app
                           ${CHIP_ROOT}/src/app
                           ${CHIP_ROOT}/third_party/nlio/repo/include
)

target_sources(${APP_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/main/AppTask.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/main/LightingManager.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/main/main.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/main/ZclCallbacks.cpp
               ${GEN_DIR}/lighting-app/zap-generated/attribute-size.cpp
               ${GEN_DIR}/lighting-app/zap-generated/callback-stub.cpp
               ${GEN_DIR}/lighting-app/zap-generated/IMClusterCommandHandler.cpp
               ${MBED_COMMON}/util/LEDWidget.cpp
               ${CHIP_ROOT}/zzz_generated/app-common/app-common/zap-generated/attributes/Accessors.cpp
               ${CHIP_ROOT}/src/app/util/DataModelHandler.cpp
               ${CHIP_ROOT}/src/app/reporting/reporting-default-configuration.cpp
               ${CHIP_ROOT}/src/app/reporting/reporting.cpp
               ${CHIP_ROOT}/src/app/util/af-event.cpp
               ${CHIP_ROOT}/src/app/util/af-main-common.cpp
               ${CHIP_ROOT}/src/app/util/attribute-list-byte-span.cpp
               ${CHIP_ROOT}/src/app/util/attribute-size-util.cpp
               ${CHIP_ROOT}/src/app/util/attribute-storage.cpp
               ${CHIP_ROOT}/src/app/util/attribute-table.cpp
               ${CHIP_ROOT}/src/app/util/binding-table.cpp
               ${CHIP_ROOT}/src/app/util/chip-message-send.cpp
               ${CHIP_ROOT}/src/app/util/client-api.cpp
               ${CHIP_ROOT}/src/app/util/ember-compatibility-functions.cpp
               ${CHIP_ROOT}/src/app/util/ember-print.cpp
               ${CHIP_ROOT}/src/app/util/error-mapping.cpp
               ${CHIP_ROOT}/src/app/util/message.cpp
               ${CHIP_ROOT}/src/app/util/process-cluster-message.cpp
               ${CHIP_ROOT}/src/app/util/process-global-message.cpp
               ${CHIP_ROOT}/src/app/util/util.cpp
               ${CHIP_ROOT}/src/app/server/EchoHandler.cpp
               ${CHIP_ROOT}/src/app/server/Mdns.cpp
               ${CHIP_ROOT}/src/app/server/OnboardingCodesUtil.cpp
               ${CHIP_ROOT}/src/app/server/RendezvousServer.cpp
               ${CHIP_ROOT}/src/app/server/Server.cpp
               ${CHIP_ROOT}/src/app/clusters/administrator-commissioning-server/administrator-commissioning-server.cpp
               ${CHIP_ROOT}/src/app/clusters/basic/basic.cpp
               ${CHIP_ROOT}/src/app/clusters/bindings/bindings.cpp
               ${CHIP_ROOT}/src/app/clusters/diagnostic-logs-server/diagnostic-logs-server.cpp
               ${CHIP_ROOT}/src/app/clusters/ethernet_network_diagnostics_server/ethernet_network_diagnostics_server.cpp
               ${CHIP_ROOT}/src/app/clusters/thread_network_diagnostics_server/thread_network_diagnostics_server.cpp
               ${CHIP_ROOT}/src/app/clusters/wifi_network_diagnostics_server/wifi_network_diagnostics_server.cpp
               ${CHIP_ROOT}/src/app/clusters/software_diagnostics_server/software_diagnostics_server.cpp	                                    
               ${CHIP_ROOT}/src/app/clusters/general-commissioning-server/general-commissioning-server.cpp
               ${CHIP_ROOT}/src/app/clusters/on-off-server/on-off-server.cpp
               ${CHIP_ROOT}/src/app/clusters/operational-credentials-server/operational-credentials-server.cpp
               ${CHIP_ROOT}/src/app/clusters/level-control/level-control.cpp
               ${CHIP_ROOT}/src/app/clusters/color-control-server/color-control-server.cpp
               ${CHIP_ROOT}/src/app/clusters/occupancy-sensor-server/occupancy-sensor-server.cpp
               ${CHIP_ROOT}/src/app/clusters/network-commissioning/network-commissioning-ember.cpp
               ${CHIP_ROOT}/src/app/clusters/network-commissioning/network-commissioning.cpp
)

add_subdirectory(${CHIP_ROOT}/config/mbed ./chip_build)

mbed_configure_app_target(${APP_TARGET})

mbed_set_mbed_target_linker_script(${APP_TARGET})

project(${APP_TARGET})

target_link_libraries(${APP_TARGET} mbed-os-posix-socket mbed-os mbed-ble mbed-events mbed-netsocket mbed-storage mbed-storage-kv-global-api mbed-mbedtls mbed-mbedtls-cryptocell310 chip)

if("wifi_ism43362" IN_LIST MBED_TARGET_LABELS)
    target_link_libraries(${APP_TARGET}
            wifi-ism43362
    )
endif()

if("BlueNRG_MS" IN_LIST MBED_TARGET_LABELS)
    target_link_libraries(${APP_TARGET}
            mbed-ble-blue_nrg
    )
endif()

if("WHD" IN_LIST MBED_TARGET_LABELS)
    target_link_libraries(${APP_TARGET}
        mbed-cy_psoc6_common_network mbed-emac mbed-cy_psoc6_whd
    )
endif()

mbed_set_post_build(${APP_TARGET})

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
