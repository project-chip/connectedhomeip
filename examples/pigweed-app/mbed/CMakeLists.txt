# Copyright (c) 2021 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
get_filename_component(MBED_COMMON ${CHIP_ROOT}/examples/platform/mbed REALPATH)
get_filename_component(APP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/.. REALPATH)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.in
    ${CMAKE_CURRENT_BINARY_DIR}/chip_build/config
    @ONLY
)

set(MBED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(APP_TARGET chip-mbed-pigweed-app-example)

include(${MBED_PATH}/tools/cmake/app.cmake)
add_subdirectory(${MBED_PATH} ./mbed_build)

add_subdirectory(mbed-os-posix-socket)

if(MBED_TARGET STREQUAL "DISCO_L475VG_IOT01A")
    add_subdirectory(wifi-ism43362)
endif()

add_executable(${APP_TARGET})

target_include_directories(${APP_TARGET} PRIVATE
                        ${CMAKE_CURRENT_SOURCE_DIR}
                        ${MBED_COMMON}/util/include
                        ${CHIP_ROOT}/src/lib/support
                        ${CMAKE_CURRENT_SOURCE_DIR}/main/include/
)

target_sources(${APP_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main/main.cpp
    ${MBED_COMMON}/util/LEDWidget.cpp
)

add_subdirectory(${CHIP_ROOT}/config/mbed ./chip_build)

mbed_configure_app_target(${APP_TARGET})

mbed_set_mbed_target_linker_script(${APP_TARGET})

project(${APP_TARGET})

target_link_libraries(${APP_TARGET} 
    mbed-os-posix-socket
    mbed-os 
    mbed-ble 
    mbed-events 
    mbed-netsocket 
    mbed-storage 
    mbed-storage-kv-global-api 
    mbed-mbedtls 
    mbed-mbedtls-cryptocell310 
    chip
)

if(MBED_TARGET STREQUAL "DISCO_L475VG_IOT01A")
    target_link_libraries(${APP_TARGET} wifi-ism43362 mbed-ble-blue_nrg)
endif()

if(MBED_TARGET STREQUAL "CY8CPROTO_062_4343W")
    target_link_libraries(${APP_TARGET} mbed-cy_psoc6_common_network mbed-emac mbed-cy_psoc6_whd)
endif()



mbed_set_post_build(${APP_TARGET})

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
