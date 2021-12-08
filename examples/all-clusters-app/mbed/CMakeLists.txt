# Copyright (c) 2021 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
get_filename_component(APP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/.. REALPATH)
get_filename_component(MBED_COMMON ${CHIP_ROOT}/examples/platform/mbed REALPATH)
get_filename_component(APP_UTIL ${CHIP_ROOT}/src/app/util REALPATH)
get_filename_component(APP_CLUSTERS ${CHIP_ROOT}/src/app/clusters REALPATH)
get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.in
    ${CMAKE_CURRENT_BINARY_DIR}/chip_build/config
    @ONLY
)

set(MBED_PATH ${MBED_OS_PATH} CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(APP_TARGET chip-mbed-all-clusters-app-example)

include(${MBED_PATH}/tools/cmake/app.cmake)

project(${APP_TARGET})

add_subdirectory(${MBED_PATH} ./mbed_build)
add_subdirectory(${MBED_OS_POSIX_SOCKET_PATH} ./mbed_os_posix_socket_build)

add_executable(${APP_TARGET})

add_subdirectory(${CHIP_ROOT}/config/mbed ./chip_build)

mbed_configure_app_target(${APP_TARGET})

target_include_directories(${APP_TARGET} PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_CURRENT_SOURCE_DIR}/main/include/
                           ${APP_ROOT}/all-clusters-common
                           ${APP_ROOT}/all-clusters-common/include
                           ${APP_CLUSTERS}
                           ${MBED_COMMON}/util/include
                           ${CHIP_ROOT}/src/app
                           ${CHIP_ROOT}/third_party/nlio/repo/include
                           ${GEN_DIR}/all-clusters-app
                           ${GEN_DIR}/ota-provider-app
                           ${GEN_DIR}/app-common
)

target_sources(${APP_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/main/main.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/main/AppTask.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/main/LowPowerManager.cpp

               ${MBED_COMMON}/util/LEDWidget.cpp

               ${GEN_DIR}/all-clusters-app/zap-generated/callback-stub.cpp
               ${GEN_DIR}/all-clusters-app/zap-generated/CHIPClientCallbacks.cpp
               ${GEN_DIR}/all-clusters-app/zap-generated/CHIPClusters.cpp
               ${GEN_DIR}/all-clusters-app/zap-generated/IMClusterCommandHandler.cpp
               ${GEN_DIR}/app-common/app-common/zap-generated/attributes/Accessors.cpp
               ${GEN_DIR}/app-common/app-common/zap-generated/cluster-objects.cpp
               
               ${CHIP_ROOT}/src/app/server/EchoHandler.cpp
               ${CHIP_ROOT}/src/app/server/Dnssd.cpp
               ${CHIP_ROOT}/src/app/server/OnboardingCodesUtil.cpp
               ${CHIP_ROOT}/src/app/server/Server.cpp
               ${CHIP_ROOT}/src/app/server/CommissioningWindowManager.cpp

               ${CHIP_ROOT}/examples/all-clusters-app/all-clusters-common/src/bridged-actions-stub.cpp
               ${CHIP_ROOT}/examples/all-clusters-app/all-clusters-common/src/static-supported-modes-manager.cpp

               ${APP_UTIL}/DataModelHandler.cpp
               ${APP_UTIL}/af-event.cpp
               ${APP_UTIL}/af-main-common.cpp
               ${APP_UTIL}/attribute-storage.cpp
               ${APP_UTIL}/attribute-table.cpp
               ${APP_UTIL}/attribute-size-util.cpp
               ${APP_UTIL}/binding-table.cpp
               ${APP_UTIL}/chip-message-send.cpp
               ${APP_UTIL}/client-api.cpp
               ${APP_UTIL}/ember-print.cpp
               ${APP_UTIL}/ember-compatibility-functions.cpp
               ${APP_UTIL}/message.cpp
               ${APP_UTIL}/process-cluster-message.cpp
               ${APP_UTIL}/process-global-message.cpp
               ${APP_UTIL}/util.cpp
               ${APP_UTIL}/error-mapping.cpp

               ${APP_CLUSTERS}/application-basic-server/application-basic-server.cpp
               ${APP_CLUSTERS}/basic/basic.cpp
               ${APP_CLUSTERS}/bindings/bindings.cpp
               ${APP_CLUSTERS}/on-off-server/on-off-server.cpp
               ${APP_CLUSTERS}/access-control-server/access-control-server.cpp
               ${APP_CLUSTERS}/account-login-server/account-login-server.cpp
               ${APP_CLUSTERS}/application-launcher-server/application-launcher-server.cpp
               ${APP_CLUSTERS}/audio-output-server/audio-output-server.cpp   
               ${APP_CLUSTERS}/barrier-control-server/barrier-control-server.cpp
               ${APP_CLUSTERS}/basic/basic.cpp
               ${APP_CLUSTERS}/bindings/bindings.cpp
               ${APP_CLUSTERS}/color-control-server/color-control-server.cpp
               ${APP_CLUSTERS}/content-launch-server/content-launch-server.cpp
               ${APP_CLUSTERS}/descriptor/descriptor.cpp
               ${APP_CLUSTERS}/door-lock-server/door-lock-server-core.cpp
               ${APP_CLUSTERS}/door-lock-server/door-lock-server-logging.cpp
               ${APP_CLUSTERS}/door-lock-server/door-lock-server-schedule.cpp
               ${APP_CLUSTERS}/door-lock-server/door-lock-server-user.cpp
               ${APP_CLUSTERS}/general-commissioning-server/general-commissioning-server.cpp     
               ${APP_CLUSTERS}/groups-server/groups-server.cpp      
               ${APP_CLUSTERS}/ias-zone-server/ias-zone-server.cpp
               ${APP_CLUSTERS}/keypad-input-server/keypad-input-server.cpp
               ${APP_CLUSTERS}/level-control/level-control.cpp
               ${APP_CLUSTERS}/low-power-server/low-power-server.cpp
               ${APP_CLUSTERS}/media-input-server/media-input-server.cpp
               ${APP_CLUSTERS}/media-playback-server/media-playback-server.cpp
               ${APP_CLUSTERS}/mode-select-server/mode-select-server.cpp
               ${APP_CLUSTERS}/network-commissioning/network-commissioning-ember.cpp
               ${APP_CLUSTERS}/network-commissioning/network-commissioning.cpp
               ${APP_CLUSTERS}/on-off-server/on-off-server.cpp
               ${APP_CLUSTERS}/ota-provider/ota-provider.cpp
               ${APP_CLUSTERS}/scenes/scenes.cpp
               ${APP_CLUSTERS}/target-navigator-server/target-navigator-server.cpp
               ${APP_CLUSTERS}/thermostat-server/thermostat-server.cpp
               ${APP_CLUSTERS}/thermostat-user-interface-configuration-server/thermostat-user-interface-configuration-server.cpp
               ${APP_CLUSTERS}/tv-channel-server/tv-channel-server.cpp
               ${APP_CLUSTERS}/operational-credentials-server/operational-credentials-server.cpp
               ${APP_CLUSTERS}/test-cluster-server/test-cluster-server.cpp
               ${APP_CLUSTERS}/occupancy-sensor-server/occupancy-sensor-server.cpp
               ${APP_CLUSTERS}/diagnostic-logs-server/diagnostic-logs-server.cpp
               ${APP_CLUSTERS}/ethernet_network_diagnostics_server/ethernet_network_diagnostics_server.cpp
               ${APP_CLUSTERS}/software_diagnostics_server/software_diagnostics_server.cpp
               ${APP_CLUSTERS}/thread_network_diagnostics_server/thread_network_diagnostics_server.cpp
               ${APP_CLUSTERS}/wifi_network_diagnostics_server/wifi_network_diagnostics_server.cpp
               ${APP_CLUSTERS}/pump-configuration-and-control-server/pump-configuration-and-control-server.cpp
               ${APP_CLUSTERS}/administrator-commissioning-server/administrator-commissioning-server.cpp
               ${APP_CLUSTERS}/identify-server/identify-server.cpp
               ${APP_CLUSTERS}/window-covering-server/window-covering-server.cpp
               ${APP_CLUSTERS}/general_diagnostics_server/general_diagnostics_server.cpp
               ${APP_CLUSTERS}/power-source-configuration-server/power-source-configuration-server.cpp
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
