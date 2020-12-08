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

# Define Zephyr C/C++ compiler flags which should not be forwarded to CHIP
# build system (e.g. because CHIP configures them on its own).
set(CHIP_CFLAG_EXCLUDES 
    "-fno-asynchronous-unwind-tables"
    "-fno-common"
    "-fno-defer-pop"
    "-fno-reorder-functions"
    "-ffunction-sections"
    "-fdata-sections"
    "-g*"
    "-O*"
    "-W*"
)

macro(chip_gn_arg_bool_if CONDITION ARGSTRING GN_VARNAME)
    if (${CONDITION})
        string(APPEND ${ARGSTRING} "${GN_VARNAME} = true\n")
    else ()
        string(APPEND ${ARGSTRING} "${GN_VARNAME} = false\n")
    endif()
endmacro()

macro(chip_gn_arg_string ARGSTRING GN_STRING)
    string(APPEND ${ARGSTRING} "${GN_STRING}\n")
endmacro()

macro(chip_gn_arg_cflags ARGSTRING ARG CFLAGS)
    set(CFLAG_EXCLUDES "[")
    foreach(cflag ${CHIP_CFLAG_EXCLUDES})
        string(APPEND CFLAG_EXCLUDES "\"${cflag}\", ")
    endforeach()
    string(APPEND CFLAG_EXCLUDES "]")
    string(APPEND ${ARGSTRING} "${ARG} = filter_exclude(string_split(\"${CFLAGS}\"), ${CFLAG_EXCLUDES})\n")
endmacro()

# Function to retrieve Zephyr compilation flags for the given language (C or CXX)
function(zephyr_get_compile_flags VAR LANG)
    # We want to treat all zephyr-provided headers as system headers, so
    # warnings in them don't trigger -Werror.  That means that for the headers
    # zephyr returns as "non-system" headers (the ones from
    # zephyr_get_include_directories_for_lang) we need to manually replace "-I"
    # with "-isystem" before adding them to INCLUDES.
    set(temp_includes "")
    zephyr_get_include_directories_for_lang(${LANG} temp_includes)
    foreach(include ${temp_includes})
      string(REPLACE "-I" "-isystem" include ${include})
      list(APPEND INCLUDES ${include})
    endforeach()

    zephyr_get_system_include_directories_for_lang(${LANG} SYSTEM_INCLUDES)
    zephyr_get_compile_definitions_for_lang(${LANG} DEFINES)
    zephyr_get_compile_options_for_lang(${LANG} FLAGS)
    set(${VAR} ${INCLUDES} ${SYSTEM_INCLUDES} ${DEFINES} ${FLAGS} ${${VAR}} PARENT_SCOPE)
endfunction()

function(chip_select_cpp_standard VAR)
    if (CONFIG_STD_CPP11)
        set(${VAR} -std=gnu++11 PARENT_SCOPE)
    elseif (CONFIG_STD_CPP14)
        set(${VAR} -std=gnu++14 PARENT_SCOPE)
    elseif (CONFIG_STD_CPP17)
        set(${VAR} -std=gnu++17 PARENT_SCOPE)
    elseif (CONFIG_STD_CPP2A)
        set(${VAR} -std=gnu++20 PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Building with unsupported C++ standard")
    endif()
endfunction()

# ==================================================
# Define chip configuration target
# ==================================================

function(chip_configure TARGET_NAME)
    cmake_parse_arguments(CHIP
                          "BUILD_TESTS"
                          "PROJECT_CONFIG"
                          "CFLAGS;CXXFLAGS"
                          ${ARGN})

    # Prepare CFLAGS & CXXFLAGS
    zephyr_get_compile_flags(CHIP_CFLAGS C)
    convert_list_of_flags_to_string_of_flags(CHIP_CFLAGS CHIP_CFLAGS)

    zephyr_get_compile_flags(CHIP_CXXFLAGS CXX)
    list(FILTER CHIP_CXXFLAGS EXCLUDE REGEX -std.*)
    chip_select_cpp_standard(CHIP_CXX_STANDARD)
    list(APPEND CHIP_CXXFLAGS ${CHIP_CXX_STANDARD})
    convert_list_of_flags_to_string_of_flags(CHIP_CXXFLAGS CHIP_CXXFLAGS)

    set(GN_ARGS "")
    chip_gn_arg_cflags(GN_ARGS "target_cflags_c" ${CHIP_CFLAGS})
    chip_gn_arg_cflags(GN_ARGS "target_cflags_cc" ${CHIP_CXXFLAGS})
    chip_gn_arg_string(GN_ARGS "zephyr_ar = \"${CMAKE_AR}\"")
    chip_gn_arg_string(GN_ARGS "zephyr_cc = \"${CMAKE_C_COMPILER}\"")
    chip_gn_arg_string(GN_ARGS "zephyr_cxx = \"${CMAKE_CXX_COMPILER}\"")

    if (CHIP_PROJECT_CONFIG)
        chip_gn_arg_string(GN_ARGS "chip_project_config_include = \"<${CHIP_PROJECT_CONFIG}>\"")
        chip_gn_arg_string(GN_ARGS "chip_system_project_config_include = \"<${CHIP_PROJECT_CONFIG}>\"")
    endif ()

    if (BOARD STREQUAL "native_posix")
        chip_gn_arg_string(GN_ARGS "target_cpu = \"x86\"")
    elseif (BOARD STREQUAL "native_posix_64")
        chip_gn_arg_string(GN_ARGS "target_cpu = \"x64\"")
    endif ()

    chip_gn_arg_bool_if(CONFIG_NET_L2_OPENTHREAD GN_ARGS "chip_enable_openthread")
    chip_gn_arg_bool_if(CONFIG_NET_IPV4          GN_ARGS "chip_inet_config_enable_ipv4")
    chip_gn_arg_bool_if(CHIP_BUILD_TESTS         GN_ARGS "chip_build_tests")
    chip_gn_arg_bool_if(CONFIG_DEBUG             GN_ARGS "is_debug")
    chip_gn_arg_bool_if(CHIP_BUILD_TESTS         GN_ARGS "chip_inet_config_enable_raw_endpoint")
    chip_gn_arg_bool_if(CHIP_BUILD_TESTS         GN_ARGS "chip_inet_config_enable_tcp_endpoint")
    chip_gn_arg_bool_if(CHIP_BUILD_TESTS         GN_ARGS "chip_inet_config_enable_dns_resolver")
    chip_gn_arg_bool_if(CONFIG_CHIP_LIB_SHELL    GN_ARGS "chip_build_libshell")
    chip_gn_arg_bool_if(CONFIG_CHIP_PW_RPC       GN_ARGS "chip_build_pw_rpc_lib")

    file(GENERATE OUTPUT ${CHIP_OUTPUT_DIR}/args.gn CONTENT "${GN_ARGS}")

    # Define target
    ExternalProject_Add(
        ${TARGET_NAME}
        PREFIX                  ${CHIP_OUTPUT_DIR}
        SOURCE_DIR              ${CHIP_ROOT}
        BINARY_DIR              ${CHIP_OUTPUT_DIR}
        CONFIGURE_COMMAND       gn --root=${CHIP_ROOT}/config/nrfconnect gen --check --fail-on-unused-args ${CHIP_OUTPUT_DIR}
        BUILD_COMMAND           ""
        INSTALL_COMMAND         ""
        BUILD_ALWAYS            TRUE
        USES_TERMINAL_CONFIGURE TRUE
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
        USES_TERMINAL_BUILD TRUE
    )

    # Define interface library containing desired CHIP byproducts
    add_library(${TARGET_NAME} INTERFACE)
    target_include_directories(${TARGET_NAME} INTERFACE
        ${CHIP_ROOT}/src
        ${CHIP_ROOT}/src/accessory
        ${CHIP_ROOT}/src/app/util
        ${CHIP_ROOT}/src/lib
        ${CHIP_ROOT}/src/lib/core
        ${CHIP_ROOT}/src/include
        ${CHIP_ROOT}/third_party/nlassert/repo/include
        ${CHIP_OUTPUT_DIR}/gen/include
        ${CHIP_OUTPUT_DIR}/gen/third_party/connectedhomeip/src/lib/support/include
        ${CHIP_OUTPUT_DIR}/gen/third_party/connectedhomeip/src/app/include
    )
    target_link_directories(${TARGET_NAME} INTERFACE ${CHIP_OUTPUT_DIR}/lib)
    target_link_libraries(${TARGET_NAME} INTERFACE -Wl,--start-group ${CHIP_BUILD_ARTIFACTS} -Wl,--end-group)

    add_dependencies(${TARGET_NAME}Build ${BASE_TARGET_NAME})
    add_dependencies(${TARGET_NAME} ${TARGET_NAME}Build)
    add_dependencies(${TARGET_NAME}Build kernel)
endfunction()
