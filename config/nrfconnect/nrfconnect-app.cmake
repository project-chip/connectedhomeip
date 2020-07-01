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
#   set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CHIP_ROOT}/config/nrfconnect/)
#
#   include(nrfconnect-app)
#
#   project(chip-nrf52840-lock-example)
#   target_sources(app PRIVATE main/main.cpp ...)
#

# Set DTC overlay before finding the Zephyr package.
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dts.overlay")
    set(DTC_OVERLAY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/dts.overlay")
endif()

# ==================================================
# Load NCS/Zephyr build system
# ==================================================
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

# ==================================================
# General settings
# ==================================================

# Archtecture for which CHIP will be built.
set(CHIP_HOST_ARCH arm-none-eabi)

# Directory into which the CHIP build system will place its output.
set(CHIP_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/chip")

# An optional file containing application-specific configuration overrides.
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CHIPProjectConfig.h")
    set(CHIP_PROJECT_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/CHIPProjectConfig.h")
else()
    set(CHIP_PROJECT_CONFIG "")
endif()

zephyr_include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/main/include
    ${CHIP_ROOT}/src
    ${CHIP_ROOT}/src/lib
    ${CHIP_ROOT}/src/lib/core
    ${CHIP_ROOT}/src/include
    ${CHIP_OUTPUT_DIR}/include
    ${CHIP_OUTPUT_DIR}/src/include
)

set(CHIP_OUTPUT_LIBRARIES
    ${CHIP_OUTPUT_DIR}/lib/libCHIP.a
    ${CHIP_OUTPUT_DIR}/lib/libInetLayer.a
    ${CHIP_OUTPUT_DIR}/lib/libSystemLayer.a
    ${CHIP_OUTPUT_DIR}/lib/libSupportLayer.a
    ${CHIP_OUTPUT_DIR}/lib/libBleLayer.a
    ${CHIP_OUTPUT_DIR}/lib/libDeviceLayer.a
    ${CHIP_OUTPUT_DIR}/lib/libCHIPDataModel.a
    )

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
list(FILTER CHIP_CXXFLAGS EXCLUDE REGEX -std.*) # CHIP adds gnu++11 anyway...

set(CHIP_COMMON_FLAGS
    -D_SYS__PTHREADTYPES_H_
    -isystem${ZEPHYR_BASE}/include/posix
    -isystem${ZEPHYR_BASE}/../modules/crypto/mbedtls/configs)

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
    --with-device-layer=nrfconnect
    --with-network-layer=all
    --with-target-network=sockets
    --with-inet-endpoint=udp
    --with-logging-style=external
    --with-target-style=embedded
    --with-chip-project-includes=${CHIP_PROJECT_CONFIG}
    --with-chip-system-project-includes=${CHIP_PROJECT_CONFIG}
    --with-chip-inet-project-includes=${CHIP_PROJECT_CONFIG}
    --with-chip-ble-project-includes=${CHIP_PROJECT_CONFIG}
    --with-chip-warm-project-includes=${CHIP_PROJECT_CONFIG}
    --with-chip-device-project-includes=${CHIP_PROJECT_CONFIG}
    --enable-debug
    --enable-optimization=no
    --disable-ipv4
    --disable-tests
    --disable-tools
    --disable-docs
    --disable-java
    --disable-device-manager
    --with-mbedtls=${ZEPHYR_BASE}/../modules/crypto/mbedtls
    --with-crypto=mbedtls
    )

if (${CONFIG_NET_L2_OPENTHREAD})
    list(APPEND CHIP_CONFIGURE_ARGS --with-openthread=${ZEPHYR_BASE}/../modules/lib/openthread)
endif()

# Define CHIP as an external project (since CHIP doesn't support CMake natively)
include(ExternalProject)
ExternalProject_Add(
    chip_project
    PREFIX              ${CHIP_OUTPUT_DIR}
    SOURCE_DIR          ${CHIP_ROOT}
    BINARY_DIR          ${CHIP_OUTPUT_DIR}
    CONFIGURE_COMMAND   ${CHIP_ROOT}/configure ${CHIP_CONFIGURE_ARGS}
    BUILD_COMMAND       make --no-print-directory all install V=${CMAKE_AUTOGEN_VERBOSE}
    BUILD_BYPRODUCTS    ${CHIP_OUTPUT_LIBRARIES}
    BUILD_ALWAYS        TRUE
)

# ==================================================
# Configure application
# ==================================================
target_compile_definitions(app PRIVATE HAVE_CONFIG_H)
target_link_libraries(app PRIVATE -Wl,--start-group ${CHIP_OUTPUT_LIBRARIES} -Wl,--end-group)
add_dependencies(app chip_project)
