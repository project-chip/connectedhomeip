# Copyright (c) 2021 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
get_filename_component(APP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/.. REALPATH)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.in
    ${CMAKE_CURRENT_BINARY_DIR}/chip_build/config
    @ONLY
)

set(MBED_PATH $ENV{MBED_OS_PATH} CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(APP_TARGET chip-mbed-shell-example)

include(${MBED_PATH}/tools/cmake/app.cmake)
add_subdirectory(${MBED_PATH} ./mbed_build)

if("wifi_ism43362" IN_LIST MBED_TARGET_LABELS)
    add_subdirectory(wifi-ism43362)
endif()

add_executable(${APP_TARGET})

target_include_directories(${APP_TARGET} PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${APP_ROOT}/shell_common/include)

target_sources(${APP_TARGET} PRIVATE
    ${APP_ROOT}/shell_common/cmd_misc.cpp
    ${APP_ROOT}/shell_common/cmd_otcli.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cmd_mbed_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
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
