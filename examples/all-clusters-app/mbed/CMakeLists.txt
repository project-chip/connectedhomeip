# Copyright (c) 2021 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
get_filename_component(APP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/.. REALPATH)
get_filename_component(MBED_COMMON ${CHIP_ROOT}/examples/platform/mbed REALPATH)
get_filename_component(LOCK_COMMON ${CHIP_ROOT}/examples/all-clusters-app/all-clusters-common REALPATH)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.in
    ${CMAKE_CURRENT_BINARY_DIR}/chip_build/config
    @ONLY
)

set(MBED_PATH $ENV{MBED_OS_PATH} CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(APP_TARGET chip-mbed-all-clusters-app-example)

include(${MBED_PATH}/tools/cmake/app.cmake)
add_subdirectory(${MBED_PATH} ./mbed_build)

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
                           
)

target_sources(${APP_TARGET} PRIVATE
               ${LOCK_COMMON}/gen/attribute-size.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/main/main.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/main/AppTask.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/main/ZclCallbacks.cpp
               ${APP_ROOT}/all-clusters-common/gen/call-command-handler.cpp
               ${APP_ROOT}/all-clusters-common/gen/callback-stub.cpp
               ${MBED_COMMON}/util/LEDWidget.cpp
               ${CHIP_ROOT}/src/app/util/DataModelHandler.cpp
               ${CHIP_ROOT}/src/app/reporting/reporting-default-configuration.cpp
               ${CHIP_ROOT}/src/app/reporting/reporting.cpp
               ${CHIP_ROOT}/src/app/util/af-event.cpp
               ${CHIP_ROOT}/src/app/util/af-main-common.cpp
               ${CHIP_ROOT}/src/app/util/attribute-size.cpp
               ${CHIP_ROOT}/src/app/util/attribute-storage.cpp
               ${CHIP_ROOT}/src/app/util/attribute-table.cpp
               ${CHIP_ROOT}/src/app/util/binding-table.cpp
               ${CHIP_ROOT}/src/app/util/chip-message-send.cpp
               ${CHIP_ROOT}/src/app/util/client-api.cpp
               ${CHIP_ROOT}/src/app/util/ember-print.cpp
               ${CHIP_ROOT}/src/app/util/message.cpp
               ${CHIP_ROOT}/src/app/util/process-cluster-message.cpp
               ${CHIP_ROOT}/src/app/util/process-global-message.cpp
               ${CHIP_ROOT}/src/app/util/util.cpp
               ${CHIP_ROOT}/src/app/clusters/basic/basic.cpp
               ${CHIP_ROOT}/src/app/clusters/bindings/bindings.cpp
               ${CHIP_ROOT}/src/app/clusters/on-off-server/on-off-server.cpp
               ${CHIP_ROOT}/src/app/clusters/account-login-server/account-login-server.cpp
               ${CHIP_ROOT}/src/app/clusters/application-launcher-server/application-launcher-server.cpp
               ${CHIP_ROOT}/src/app/clusters/audio-output-server/audio-output-server.cpp   
               ${CHIP_ROOT}/src/app/clusters/barrier-control-server/barrier-control-server.cpp
               ${CHIP_ROOT}/src/app/clusters/basic/basic.cpp
               ${CHIP_ROOT}/src/app/clusters/bindings/bindings.cpp
               ${CHIP_ROOT}/src/app/clusters/color-control-server/color-control-server.cpp
               ${CHIP_ROOT}/src/app/clusters/content-launch-server/content-launch-server.cpp
               ${CHIP_ROOT}/src/app/clusters/descriptor/descriptor.cpp
               ${CHIP_ROOT}/src/app/clusters/door-lock-server/door-lock-server-core.cpp
               ${CHIP_ROOT}/src/app/clusters/door-lock-server/door-lock-server-logging.cpp
               ${CHIP_ROOT}/src/app/clusters/door-lock-server/door-lock-server-schedule.cpp
               ${CHIP_ROOT}/src/app/clusters/door-lock-server/door-lock-server-user.cpp
               ${CHIP_ROOT}/src/app/clusters/general-commissioning-server/general-commissioning-server.cpp     
               ${CHIP_ROOT}/src/app/clusters/groups-server/groups-server.cpp      
               ${CHIP_ROOT}/src/app/clusters/ias-zone-server/ias-zone-server.cpp
               ${CHIP_ROOT}/src/app/clusters/identify/identify.cpp
               ${CHIP_ROOT}/src/app/clusters/keypad-input-server/keypad-input-server.cpp
               ${CHIP_ROOT}/src/app/clusters/level-control/level-control.cpp
               ${CHIP_ROOT}/src/app/clusters/low-power-server/low-power-server.cpp
               ${CHIP_ROOT}/src/app/clusters/media-input-server/media-input-server.cpp
               ${CHIP_ROOT}/src/app/clusters/media-playback-server/media-playback-server.cpp
               ${CHIP_ROOT}/src/app/clusters/network-commissioning/network-commissioning-ember.cpp
               ${CHIP_ROOT}/src/app/clusters/network-commissioning/network-commissioning.cpp
               ${CHIP_ROOT}/src/app/clusters/on-off-server/on-off-server.cpp
               ${CHIP_ROOT}/src/app/clusters/ota-server/ota-server.cpp
               ${CHIP_ROOT}/src/app/clusters/scenes/scenes.cpp
               ${CHIP_ROOT}/src/app/clusters/target-navigator-server/target-navigator-server.cpp
               ${CHIP_ROOT}/src/app/clusters/temperature-measurement-server/temperature-measurement-server.cpp
               ${CHIP_ROOT}/src/app/clusters/tv-channel-server/tv-channel-server.cpp
)

add_subdirectory(${CHIP_ROOT}/config/mbed ./chip_build)

mbed_configure_app_target(${APP_TARGET})

mbed_set_mbed_target_linker_script(${APP_TARGET})

project(${APP_TARGET})

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

target_link_libraries(${APP_TARGET} mbed-os mbed-ble mbed-events mbed-netsocket mbed-storage mbed-storage-kv-global-api mbed-mbedtls mbed-mbedtls-cryptocell310 chip)

mbed_set_post_build(${APP_TARGET})

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
