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
#     CMake sub-project defining 'chip' target which represents CHIP library
#     and other optional libraries like unit tests, built with 'nrfconnect'
#     platform.
#     Since CHIP doesn't provide native CMake support, ExternalProject
#     module is used to build the required artifacts with GN meta-build
#     system. It is assumed that find_package(Zephyr) has been called before
#     including this file.
#

if (CONFIG_CHIP)

include(ExternalProject)
include(../../zephyr/zephyr-util.cmake)

# ==============================================================================
# Declare configuration variables and define constants
# ==============================================================================

# C/C++ compiler flags passed to CHIP build system
list(APPEND CHIP_CFLAGS)

# C compiler flags passed to CHIP build system
list(APPEND CHIP_CFLAGS_C)

# C++ compiler flags passed to CHIP build system
list(APPEND CHIP_CFLAGS_CC)

# CHIP libraries that the application should be linked with
list(APPEND CHIP_LIBRARIES)

# GN meta-build system arguments passed to the make_gn_args.py script
string(APPEND CHIP_GN_ARGS)

# ==============================================================================
# Helper macros
# ==============================================================================

macro(chip_gn_arg_import FILE)
    string(APPEND CHIP_GN_ARGS "--module\n${FILE}\n")
endmacro()

macro(chip_gn_arg_string ARG STRING)
    string(APPEND CHIP_GN_ARGS "--arg-string\n${ARG}\n${STRING}\n")
endmacro()

macro(chip_gn_arg_bool ARG BOOLEAN)
    if (${BOOLEAN})
        string(APPEND CHIP_GN_ARGS "--arg\n${ARG}\ntrue\n")
    else()
        string(APPEND CHIP_GN_ARGS "--arg\n${ARG}\nfalse\n")
    endif()
endmacro()

macro(chip_gn_arg_cflags ARG CFLAGS)
    string(APPEND CHIP_GN_ARGS "--arg-cflags\n${ARG}\n${CFLAGS}\n")
endmacro()

# ==============================================================================
# Prepare CHIP configuration based on the project Kconfig configuration
# ==============================================================================

if (NOT CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
endif()

set(GN_ROOT_TARGET ${CHIP_ROOT}/config/nrfconnect/chip-gn)

# Prepare compiler flags

if (CONFIG_ARM)
    list(APPEND CHIP_CFLAGS_C
        --specs=nosys.specs
    )
endif()

if (CONFIG_POSIX_API)
    list(APPEND CHIP_CFLAGS
        -D_SYS__PTHREADTYPES_H_
        -isystem${ZEPHYR_BASE}/include/posix
    )
endif()

if (CONFIG_NORDIC_SECURITY_BACKEND)
    # TODO: Remove when the missing dependency is fixed in nRF Connect SDK
    zephyr_include_directories(${ZEPHYR_BASE}/../nrfxlib/crypto/nrf_cc310_platform/include)

    list(APPEND CHIP_CFLAGS -DMBEDTLS_CONFIG_FILE=<nrf-config.h>)
endif()

zephyr_get_compile_flags(CHIP_CFLAGS_C C)
zephyr_get_compile_flags(CHIP_CFLAGS_CC CXX)
zephyr_get_gnu_cpp_standard(CHIP_CFLAGS_CC)

convert_list_of_flags_to_string_of_flags(CHIP_CFLAGS CHIP_CFLAGS)
convert_list_of_flags_to_string_of_flags(CHIP_CFLAGS_C CHIP_CFLAGS_C)
convert_list_of_flags_to_string_of_flags(CHIP_CFLAGS_CC CHIP_CFLAGS_CC)

# Prepare CHIP libraries that the application should be linked with

if (NOT CHIP_LIBRARIES)
    set(CHIP_LIBRARIES -lCHIP)
endif()

if (CONFIG_CHIP_PW_RPC)
    list(APPEND CHIP_LIBRARIES -lPwRpc)
endif()

# Set up CHIP project configuration file

if (CONFIG_CHIP_PROJECT_CONFIG)
    get_filename_component(CHIP_PROJECT_CONFIG 
        ${CONFIG_CHIP_PROJECT_CONFIG}
        REALPATH
        BASE_DIR ${CMAKE_SOURCE_DIR}
    )
    set(CHIP_PROJECT_CONFIG "<${CHIP_PROJECT_CONFIG}>")
else()
    set(CHIP_PROJECT_CONFIG "")
endif()

# Set up custom OpenThread configuration

if (CONFIG_CHIP_OPENTHREAD_CONFIG)
    get_filename_component(CHIP_OPENTHREAD_CONFIG 
        ${CONFIG_CHIP_OPENTHREAD_CONFIG}
        REALPATH
        BASE_DIR ${CMAKE_SOURCE_DIR}
    )
    zephyr_set_openthread_config(${CHIP_OPENTHREAD_CONFIG})
endif()

# Find required programs

find_program(GN_EXECUTABLE gn)
if (${GN_EXECUTABLE} STREQUAL GN_EXECUTABLE-NOTFOUND)
    message(FATAL_ERROR "The 'gn' command was not found. Make sure you have GN installed.")
else()
    # Parse the 'gn --version' output to find the installed version.
    set(MIN_GN_VERSION 1851)
    execute_process(
        COMMAND
        ${GN_EXECUTABLE} --version
        OUTPUT_VARIABLE gn_version_output
        ERROR_VARIABLE  gn_error_output
        RESULT_VARIABLE gn_status
    )

    if(${gn_status} EQUAL 0)
        if(gn_version_output VERSION_LESS ${MIN_GN_VERSION})
            message(FATAL_ERROR "Found unsuitable version of 'gn'. Required is at least ${MIN_GN_VERSION}")
        endif()
    else()
        message(FATAL_ERROR "Could NOT find working gn: Found gn (${GN_EXECUTABLE}), but failed to load with:\n ${gn_error_output}")
    endif()
endif()

find_package(Python3 REQUIRED)

# ==============================================================================
# Generate configuration for CHIP GN build system
# ==============================================================================

if (CONFIG_CHIP_PW_RPC)
    chip_gn_arg_import("${GN_ROOT_TARGET}/lib/pw_rpc/pw_rpc.gni")
endif()

chip_gn_arg_cflags("target_cflags"                          ${CHIP_CFLAGS})
chip_gn_arg_cflags("target_cflags_c"                        ${CHIP_CFLAGS_C})
chip_gn_arg_cflags("target_cflags_cc"                       ${CHIP_CFLAGS_CC})
chip_gn_arg_string("zephyr_ar"                              ${CMAKE_AR})
chip_gn_arg_string("zephyr_cc"                              ${CMAKE_C_COMPILER})
chip_gn_arg_string("zephyr_cxx"                             ${CMAKE_CXX_COMPILER})
chip_gn_arg_bool  ("is_debug"                               CONFIG_DEBUG)
chip_gn_arg_bool  ("chip_enable_openthread"                 CONFIG_NET_L2_OPENTHREAD)
chip_gn_arg_bool  ("chip_config_network_layer_ble"          CONFIG_BT)
chip_gn_arg_bool  ("chip_inet_config_enable_ipv4"           CONFIG_NET_IPV4)
chip_gn_arg_bool  ("chip_enable_nfc"                        CONFIG_CHIP_NFC_COMMISSIONING)
chip_gn_arg_bool  ("chip_build_tests"                       CONFIG_CHIP_BUILD_TESTS)
chip_gn_arg_bool  ("chip_monolithic_tests"                  CONFIG_CHIP_BUILD_TESTS)
chip_gn_arg_bool  ("chip_inet_config_enable_raw_endpoint"   CONFIG_CHIP_BUILD_TESTS)
chip_gn_arg_bool  ("chip_inet_config_enable_tcp_endpoint"   CONFIG_CHIP_BUILD_TESTS)
chip_gn_arg_bool  ("chip_inet_config_enable_dns_resolver"   CONFIG_CHIP_BUILD_TESTS)
chip_gn_arg_bool  ("chip_build_libshell"                    CONFIG_CHIP_LIB_SHELL)
chip_gn_arg_bool  ("chip_build_pw_rpc_lib"                  CONFIG_CHIP_PW_RPC)

if (CONFIG_CHIP_ENABLE_DNSSD_SRP)
    chip_gn_arg_string("chip_mdns" "platform")
endif()

if (CHIP_PROJECT_CONFIG)
    chip_gn_arg_string("chip_project_config_include"        ${CHIP_PROJECT_CONFIG})
    chip_gn_arg_string("chip_system_project_config_include" ${CHIP_PROJECT_CONFIG})
endif()

if (BOARD STREQUAL "native_posix")
    chip_gn_arg_string("target_cpu" "x86")
elseif (BOARD STREQUAL "native_posix_64")
    chip_gn_arg_string("target_cpu" "x64")
endif()

file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/args.tmp" CONTENT ${CHIP_GN_ARGS})

# ==============================================================================
# Define 'chip-gn' target that builds CHIP library(ies) with GN build system
# ==============================================================================
ExternalProject_Add(
    chip-gn
    PREFIX                  ${CMAKE_CURRENT_BINARY_DIR}
    SOURCE_DIR              ${CHIP_ROOT}
    BINARY_DIR              ${CMAKE_CURRENT_BINARY_DIR}
    CONFIGURE_COMMAND       ""
    BUILD_COMMAND           ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/make_gn_args.py
                                @args.tmp > args.gn &&
                            ${GN_EXECUTABLE}
                                --root=${CHIP_ROOT}
                                --root-target=${GN_ROOT_TARGET}
                                --dotfile=${GN_ROOT_TARGET}/.gn
                                --script-executable=${Python3_EXECUTABLE}
                                --export-compile-commands
                                gen --check --fail-on-unused-args . &&
                            ninja
    INSTALL_COMMAND         ""
    BUILD_BYPRODUCTS        ${CHIP_LIBRARIES}
    BUILD_ALWAYS            TRUE
    USES_TERMINAL_CONFIGURE TRUE
    USES_TERMINAL_BUILD     TRUE
)
add_dependencies(chip-gn kernel)

# ==============================================================================
# Define 'chip' target that exposes CHIP headers & libraries to the application
# ==============================================================================

zephyr_interface_library_named(chip)
target_compile_definitions(chip INTERFACE CHIP_HAVE_CONFIG_H)
target_include_directories(chip INTERFACE
    ${CHIP_ROOT}/src
    ${CHIP_ROOT}/src/include
    ${CHIP_ROOT}/src/lib
    ${CHIP_ROOT}/third_party/nlassert/repo/include
    ${CMAKE_CURRENT_BINARY_DIR}/gen/include
)
target_link_directories(chip INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/lib)
if (CONFIG_CHIP_LIB_SHELL)
    target_link_options(chip INTERFACE -Wl,--whole-archive -lCHIPShell -Wl,--no-whole-archive)
endif()

target_link_libraries(chip INTERFACE -Wl,--start-group ${CHIP_LIBRARIES} -Wl,--end-group)

add_dependencies(chip chip-gn)

endif() # CONFIG_CHIP
