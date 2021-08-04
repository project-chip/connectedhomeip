# Copyright (c) 2021 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
get_filename_component(APP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/.. REALPATH)
get_filename_component(MBED_COMMON ${CHIP_ROOT}/examples/platform/mbed REALPATH)
get_filename_component(ALL_CLUSTERS_COMMON ${CHIP_ROOT}/examples/all-clusters-app/all-clusters-common REALPATH)
get_filename_component(APP_UTIL ${CHIP_ROOT}/src/app/util REALPATH)
get_filename_component(APP_CLUSTERS ${CHIP_ROOT}/src/app/clusters REALPATH)
get_filename_component(APP_COMMON_GEN ${CHIP_ROOT}/src/app/common/gen REALPATH)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.in
    ${CMAKE_CURRENT_BINARY_DIR}/chip_build/config
    @ONLY
)

set(MBED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(APP_TARGET chip-mbed-all-clusters-app-example)

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
                           ${APP_ROOT}/all-clusters-common
                           ${MBED_COMMON}/util/include
                           ${CHIP_ROOT}/src/app
                           ${CHIP_ROOT}/third_party/nlio/repo/include
                           ${APP_COMMON_GEN}
                           ${APP_COMMON_GEN}/attributes
)

target_sources(${APP_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/main/main.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/main/AppTask.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/main/ZclCallbacks.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/main/tv-callbacks.cpp
               ${ALL_CLUSTERS_COMMON}/gen/attribute-size.cpp
               ${ALL_CLUSTERS_COMMON}/gen/callback-stub.cpp
               ${ALL_CLUSTERS_COMMON}/gen/CHIPClientCallbacks.cpp
               ${ALL_CLUSTERS_COMMON}/gen/CHIPClusters.cpp
               ${ALL_CLUSTERS_COMMON}/gen/encoder.cpp
               ${ALL_CLUSTERS_COMMON}/gen/IMClusterCommandHandler.cpp
               ${MBED_COMMON}/util/LEDWidget.cpp
               ${CHIP_ROOT}/src/app/server/EchoHandler.cpp
               ${CHIP_ROOT}/src/app/server/Mdns.cpp
               ${CHIP_ROOT}/src/app/server/OnboardingCodesUtil.cpp
               ${CHIP_ROOT}/src/app/server/RendezvousServer.cpp
               ${CHIP_ROOT}/src/app/server/Server.cpp
               ${CHIP_ROOT}/src/app/server/StorablePeerConnection.cpp
               ${CHIP_ROOT}/src/app/reporting/reporting-default-configuration.cpp
               ${CHIP_ROOT}/src/app/reporting/reporting.cpp
               ${APP_UTIL}/DataModelHandler.cpp
               ${APP_UTIL}/af-event.cpp
               ${APP_UTIL}/af-main-common.cpp
               ${APP_UTIL}/attribute-storage.cpp
               ${APP_UTIL}/attribute-table.cpp
               ${APP_UTIL}/attribute-size-util.cpp
               ${APP_UTIL}/attribute-list-byte-span.cpp
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
               ${APP_CLUSTERS}/basic/basic.cpp
               ${APP_CLUSTERS}/bindings/bindings.cpp
               ${APP_CLUSTERS}/on-off-server/on-off-server.cpp
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
               ${APP_CLUSTERS}/identify/identify.cpp
               ${APP_CLUSTERS}/keypad-input-server/keypad-input-server.cpp
               ${APP_CLUSTERS}/level-control/level-control.cpp
               ${APP_CLUSTERS}/low-power-server/low-power-server.cpp
               ${APP_CLUSTERS}/media-input-server/media-input-server.cpp
               ${APP_CLUSTERS}/media-playback-server/media-playback-server.cpp
               ${APP_CLUSTERS}/network-commissioning/network-commissioning-ember.cpp
               ${APP_CLUSTERS}/network-commissioning/network-commissioning.cpp
               ${APP_CLUSTERS}/on-off-server/on-off-server.cpp
               ${APP_CLUSTERS}/ota-provider/ota-provider.cpp
               ${APP_CLUSTERS}/scenes/scenes.cpp
               ${APP_CLUSTERS}/target-navigator-server/target-navigator-server.cpp
               ${APP_CLUSTERS}/temperature-measurement-server/temperature-measurement-server.cpp
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
               ${APP_COMMON_GEN}/attributes/Accessors.cpp              
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
