# Copyright (c) 2021 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
get_filename_component(MBED_COMMON ${CHIP_ROOT}/examples/platform/mbed REALPATH)
get_filename_component(SHELL_COMMON ${CHIP_ROOT}/examples/shell/shell_common REALPATH)
get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)
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
set(APP_TARGET chip-mbed-shell-example)

if(APP_TYPE STREQUAL "boot" OR APP_TYPE STREQUAL "upgrade")
    set(BOOT_ENABLED TRUE)
endif()

include(${MBED_PATH}/tools/cmake/app.cmake)
if(MBED_TARGET STREQUAL "CY8CPROTO_062_4343W" AND BOOT_ENABLED)
    list(REMOVE_ITEM MBED_TARGET_LABELS CM0P_SLEEP)
    list(REMOVE_ITEM MBED_TARGET_DEFINITIONS COMPONENT_CM0P_SLEEP=1)
endif()

project(${APP_TARGET})


add_subdirectory(${MBED_PATH} ./mbed_build)
add_subdirectory(${MBED_OS_POSIX_SOCKET_PATH} ./mbed_os_posix_socket_build)

add_executable(${APP_TARGET})

add_subdirectory(${CHIP_ROOT}/config/mbed ./chip_build)

mbed_configure_app_target(${APP_TARGET})

target_include_directories(${APP_TARGET} PRIVATE
                           main/include
                           ${MBED_COMMON}/util/include
                           ${SHELL_COMMON}/include
                           ${GEN_DIR}/app-common
                           ${NLIO_ROOT}
)

target_sources(${APP_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/main/main.cpp
               ${SHELL_COMMON}/cmd_misc.cpp
               ${SHELL_COMMON}/cmd_otcli.cpp
               ${SHELL_COMMON}/cmd_server.cpp
               ${SHELL_COMMON}/globals.cpp
               ${MBED_COMMON}/util/DFUManager.cpp
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
