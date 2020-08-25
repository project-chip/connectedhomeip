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
#         CMake module for configuring and building CHIP libraries to be used
#         in Zephyr applications.
#
#   It is assumed that at this point:
#    - CHIP_ROOT is defined
#    - find_package(Zephyr) has been called
#

# ==================================================
# Helpers & settings
# ==================================================

include(ExternalProject)

# Directory for CHIP build artifacts
set(CHIP_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/chip")

# Helper macros to conditionally append a list
macro(append_if VAR CONDITION)
    if (${${CONDITION}})
        list(APPEND ${VAR} ${ARGN})
    endif()
endmacro()

macro(append_if_not VAR CONDITION)
    if (${${CONDITION}})
    else()
        list(APPEND ${VAR} ${ARGN})
    endif()
endmacro()

# Function to retrieve Zephyr compilation flags for the given language (C or CXX)
function(zephyr_get_compile_flags VAR LANG)
    zephyr_get_include_directories_for_lang(${LANG} INCLUDES)
    zephyr_get_system_include_directories_for_lang(${LANG} SYSTEM_INCLUDES)
    zephyr_get_compile_definitions_for_lang(${LANG} DEFINES)
    zephyr_get_compile_options_for_lang(${LANG} FLAGS)
    set(${VAR} ${INCLUDES} ${SYSTEM_INCLUDES} ${DEFINES} ${FLAGS} ${${VAR}} PARENT_SCOPE)
endfunction()

# ==================================================
# Define chip configuration target
# ==================================================

function(chip_configure TARGET_NAME)
    cmake_parse_arguments(CHIP 
                          "BUILD_TESTS"
                          "ARCH;PROJECT_CONFIG"
                          "CFLAGS;CXXFLAGS" 
                          ${ARGN})

    # Prepare CFLAGS & CXXFLAGS
    zephyr_get_compile_flags(CHIP_CFLAGS C)
    convert_list_of_flags_to_string_of_flags(CHIP_CFLAGS CHIP_CFLAGS)

    zephyr_get_compile_flags(CHIP_CXXFLAGS CXX)
    list(FILTER CHIP_CXXFLAGS EXCLUDE REGEX -std.*) # CHIP adds gnu++11 anyway...
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
        --host=${CHIP_ARCH}
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
        --disable-tools
        --disable-docs
        --disable-java
        --disable-device-manager
        --with-mbedtls=${ZEPHYR_BASE}/../modules/crypto/mbedtls
        --with-crypto=mbedtls
    )
    
    append_if(CHIP_CONFIGURE_ARGS CONFIG_NET_L2_OPENTHREAD --with-openthread=${ZEPHYR_BASE}/../modules/lib/openthread)
    append_if_not(CHIP_CONFIGURE_ARGS CHIP_BUILD_TESTS --disable-tests)
    append_if_not(CHIP_CONFIGURE_ARGS CONFIG_NET_IPV4 --disable-ipv4)

    # Define target
    ExternalProject_Add(
        ${TARGET_NAME}
        PREFIX              ${CHIP_OUTPUT_DIR}
        SOURCE_DIR          ${CHIP_ROOT}
        BINARY_DIR          ${CHIP_OUTPUT_DIR}
        CONFIGURE_COMMAND   ${CHIP_ROOT}/configure ${CHIP_CONFIGURE_ARGS}
        BUILD_COMMAND       ""
        INSTALL_COMMAND     ""
        BUILD_ALWAYS        TRUE
    )
endfunction()

# ==================================================
# Define chip build target
# ==================================================    

function(chip_build TARGET_NAME BASE_TARGET_NAME)
    cmake_parse_arguments(CHIP "" "" "BUILD_COMMAND;BUILD_ARTIFACTS" ${ARGN})
    
    # Define build target
    ExternalProject_Add(
        ${TARGET_NAME}Build
        PREFIX              ${CHIP_OUTPUT_DIR}
        SOURCE_DIR          ${CHIP_ROOT}
        BINARY_DIR          ${CHIP_OUTPUT_DIR}
        CONFIGURE_COMMAND   ""
        BUILD_COMMAND       ${CHIP_BUILD_COMMAND}
        INSTALL_COMMAND     ""
        BUILD_BYPRODUCTS    ${CHIP_BUILD_ARTIFACTS}
        BUILD_ALWAYS        TRUE
    )

    # Define interface library containing desired CHIP byproducts
    add_library(${TARGET_NAME} INTERFACE)
    target_include_directories(${TARGET_NAME} INTERFACE
        ${CHIP_ROOT}/src
        ${CHIP_ROOT}/src/lib
        ${CHIP_ROOT}/src/lib/core
        ${CHIP_ROOT}/src/include
        ${CHIP_OUTPUT_DIR}/include
        ${CHIP_OUTPUT_DIR}/src/include
    )
    target_link_directories(${TARGET_NAME} INTERFACE ${CHIP_OUTPUT_DIR}/lib)
    target_link_libraries(${TARGET_NAME} INTERFACE -Wl,--start-group ${CHIP_BUILD_ARTIFACTS} -Wl,--end-group)

    add_dependencies(${TARGET_NAME}Build ${BASE_TARGET_NAME})
    add_dependencies(${TARGET_NAME} ${TARGET_NAME}Build)
endfunction()