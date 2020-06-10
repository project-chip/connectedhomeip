#
#   Copyright (c) 2020 Project CHIP Authors
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

#
#   @file
#         CMake module for incorporating CHIP into an nRF Connect SDK
#         application for Nordic platforms.
#

#
#   The top-level CMakeLists.txt of an application should define CHIP_ROOT variable,
#   include this module and configure `app` build target properly. E.g.:
#
#   cmake_minimum_required(VERSION 3.13.1)
#   
#   set(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip)
#   get_filename_component(CHIP_ROOT ${CHIP_ROOT} REALPATH)
#   set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CHIP_ROOT}/config/nrf5_ncs/)
#   
#   include(nrf5-app)
#   
#   project(chip-nrf52840-lock-example)
#   target_sources(app PRIVATE main/main.cpp ...)
#   target_link_libraries(app PRIVATE ${CHIP_OUTPUT_DIR}/lib/libCHIP.a ...)
# 

# ==================================================
# Load NCS/Zephyr build system
# ==================================================
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

# ==================================================
# General settings
# ==================================================

# Archtecture for which CHIP will be built.
set(CHIP_HOST_ARCH armv7-unknown-linux-gnu)

# Directory into which the CHIP build system will place its output.
set(CHIP_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/chip")

# An optional file containing application-specific configuration overrides.
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CHIPProjectConfig.h")
    set(CHIP_PROJECT_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/CHIPProjectConfig.h")
else()
    set(CHIP_PROJECT_CONFIG "")
endif()

set(CHIP_INCLUDE_DIRS
    -I${CMAKE_CURRENT_SOURCE_DIR}/main/include
    -I${CHIP_ROOT}/src/include
    -I${CHIP_ROOT}/third_party/lwip/repo/lwip/src/include
    -I${CHIP_ROOT}/src/lwip
    -I${CHIP_ROOT}/src/lwip/nrf5
    -I${CHIP_ROOT}/src/lwip/freertos)

# TODO: get rid of NRF5_SDK dependency when NCS platform layer is finished
set(NRF5_SDK_INCLUDE_DIRS
    -I$ENV{NRF5_SDK_ROOT}/components
    -I$ENV{NRF5_SDK_ROOT}/components/boards
    -I$ENV{NRF5_SDK_ROOT}/components/ble/ble_advertising
    -I$ENV{NRF5_SDK_ROOT}/components/ble/common
    -I$ENV{NRF5_SDK_ROOT}/components/ble/nrf_ble_gatt
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/atomic
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/atomic_fifo
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/balloc
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/bsp
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/button
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/crc16
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/delay
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/experimental_section_vars
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/fds
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/fstorage
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/log
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/log/src
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/memobj
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/mem_manager
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/mutex
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/queue
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/ringbuf
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/stack_info
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/strerror
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/timer
    -I$ENV{NRF5_SDK_ROOT}/components/libraries/util
    -I$ENV{NRF5_SDK_ROOT}/components/softdevice/common
    -I$ENV{NRF5_SDK_ROOT}/components/softdevice/s140/headers
    -I$ENV{NRF5_SDK_ROOT}/components/softdevice/s140/headers/nrf52
    -I$ENV{NRF5_SDK_ROOT}/components/softdevice/mbr/nrf52840/headers
    -I$ENV{NRF5_SDK_ROOT}/components/thread/freertos_mbedtls_mutex
    -I$ENV{NRF5_SDK_ROOT}/components/toolchain/cmsis/include
    -I$ENV{NRF5_SDK_ROOT}/external/openthread/nrf_security/config
    -I$ENV{NRF5_SDK_ROOT}/external/openthread/nrf_security/mbedtls_plat_config
    -I$ENV{NRF5_SDK_ROOT}/external/openthread/nrf_security/nrf_cc310_plat/include
    -I$ENV{NRF5_SDK_ROOT}/external/openthread/project/config
    -I$ENV{NRF5_SDK_ROOT}/external/openthread/project/nrf52840
    -I$ENV{NRF5_SDK_ROOT}/config/nrf52840/config
    -I$ENV{NRF5_SDK_ROOT}/external/fprintf
    -I$ENV{NRF5_SDK_ROOT}/external/freertos/config
    -I$ENV{NRF5_SDK_ROOT}/external/freertos/portable/CMSIS/nrf52
    -I$ENV{NRF5_SDK_ROOT}/external/freertos/portable/GCC/nrf52
    -I$ENV{NRF5_SDK_ROOT}/external/freertos/source/include
    -I$ENV{NRF5_SDK_ROOT}/external/openthread/include
    -I$ENV{NRF5_SDK_ROOT}/external/segger_rtt
    -I$ENV{NRF5_SDK_ROOT}/integration/nrfx
    -I$ENV{NRF5_SDK_ROOT}/integration/nrfx/legacy
    -I$ENV{NRF5_SDK_ROOT}/modules/nrfx
    -I$ENV{NRF5_SDK_ROOT}/modules/nrfx/drivers/include
    -I$ENV{NRF5_SDK_ROOT}/modules/nrfx/hal
    -I$ENV{NRF5_SDK_ROOT}/modules/nrfx/mdk)

set(CHIP_OUTPUT_LIBRARIES 
    ${CHIP_OUTPUT_DIR}/lib/libDeviceLayer.a
    ${CHIP_OUTPUT_DIR}/lib/libCHIP.a
    ${CHIP_OUTPUT_DIR}/lib/libInetLayer.a
    ${CHIP_OUTPUT_DIR}/lib/libSystemLayer.a
    ${CHIP_OUTPUT_DIR}/lib/liblwip.a)

# ==================================================
# Setup CHIP build
# ==================================================

# Function to retrieve Zephyr compilation flags for the given language (C or CXX)
function(get_zephyr_compilation_flags var lang)
    zephyr_get_include_directories_for_lang(${lang} INCLUDES)
    zephyr_get_system_include_directories_for_lang(${lang} SYSTEM_INCLUDES)
    zephyr_get_compile_definitions_for_lang(${lang} DEFINES)
    zephyr_get_compile_options_for_lang(${lang} FLAGS)
    set(${var} ${INCLUDES} ${SYSTEM_INCLUDES} ${DEFINES} ${FLAGS} PARENT_SCOPE)
endfunction(get_zephyr_compilation_flags)

# Inherit Zephyr build settings and add some extra ones
get_zephyr_compilation_flags(CHIP_CFLAGS C)
get_zephyr_compilation_flags(CHIP_CXXFLAGS CXX)

set(CHIP_COMMON_FLAGS
    ${CHIP_INCLUDE_DIRS}
    ${NRF5_SDK_INCLUDE_DIRS}
    -DUSE_APP_CONFIG)

set(CHIP_CFLAGS ${CHIP_CFLAGS} ${CHIP_COMMON_FLAGS} --specs=nosys.specs)
set(CHIP_CXXFLAGS ${CHIP_CXXFLAGS} ${CHIP_COMMON_FLAGS})

convert_list_of_flags_to_string_of_flags(CHIP_CFLAGS CHIP_CFLAGS)
convert_list_of_flags_to_string_of_flags(CHIP_CXXFLAGS CHIP_CXXFLAGS)

# Prepare command line arguments passed to the CHIP's ./configure script
set(CHIP_CONFIGURE_ARGS
    AR=${CMAKE_AR}
    AS=${CMAKE_AS}
    CC=${CMAKE_C_COMPILER}
    CXX=${CMAKE_CXX_COMPILER}
    LD=${CMAKE_LINKER}
    OBJCOPY=${CMAKE_OBJCOPY}
    RANLIB=${CMAKE_RANLIB}
    CFLAGS=${CHIP_CFLAGS}
    CXXFLAGS=${CHIP_CXXFLAGS}
    --prefix=${CHIP_OUTPUT_DIR}
    --exec-prefix=${CHIP_OUTPUT_DIR}
    --host=${CHIP_HOST_ARCH}
    --with-device-layer=nrf5    # TODO: change to NCS
    --with-network-layer=all
    --with-target-network=lwip  # TODO: use zephyr network stack
    --with-lwip=internal       
    --with-lwip-target=nrf5
    --with-inet-endpoint=tcp,udp
    --with-logging-style=external
    --with-chip-project-includes=${CHIP_PROJECT_CONFIG}
    --with-chip-system-project-includes=${CHIP_PROJECT_CONFIG}
    --with-chip-inet-project-includes=${CHIP_PROJECT_CONFIG}
    --with-chip-ble-project-includes=${CHIP_PROJECT_CONFIG}
    --with-chip-warm-project-includes=${CHIP_PROJECT_CONFIG}
    --with-chip-device-project-includes=${CHIP_PROJECT_CONFIG}
    --disable-ipv4
    --disable-tests
    --disable-tools
    --disable-docs
    --disable-java
    --disable-device-manager
    --with-mbedtls=${ZEPHYR_BASE}/../mbedtls
    --with-crypto=mbedtls
    )

# Define CHIP as an external project (since CHIP doesn't support CMake natively)
include(ExternalProject)
ExternalProject_Add(
    chip_project
    PREFIX              ${CHIP_OUTPUT_DIR}
    SOURCE_DIR          ${CHIP_ROOT}
    BINARY_DIR          ${CHIP_OUTPUT_DIR}
    CONFIGURE_COMMAND   ${CHIP_ROOT}/configure ${CHIP_CONFIGURE_ARGS}
    BUILD_COMMAND       make --no-print-directory all V=${CMAKE_AUTOGEN_VERBOSE}
    INSTALL_COMMAND     make --no-print-directory install V=${CMAKE_AUTOGEN_VERBOSE}
    BUILD_BYPRODUCTS    ${CHIP_OUTPUT_LIBRARIES}
)