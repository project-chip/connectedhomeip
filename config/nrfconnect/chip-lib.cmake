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

macro(chip_gn_arg_bool_if CONDITION ARGSTRING GN_VARNAME)
    if (${${CONDITION}})
        string(APPEND ${ARGSTRING} "${GN_VARNAME} = true\n")
    else ()
        string(APPEND ${ARGSTRING} "${GN_VARNAME} = false\n")
    endif()
endmacro()

macro(chip_gn_arg_string ARGSTRING GN_STRING)
    string(APPEND ${ARGSTRING} "${GN_STRING}\n")
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
    list(APPEND CHIP_CFLAGS -I${ZEPHYR_BASE}/../mbedtls/include)
    list(APPEND CHIP_CFLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/main/include)
    convert_list_of_flags_to_string_of_flags(CHIP_CFLAGS CHIP_CFLAGS)

    zephyr_get_compile_flags(CHIP_CXXFLAGS CXX)
    list(FILTER CHIP_CXXFLAGS EXCLUDE REGEX -std.*) # CHIP adds gnu++11 anyway...
    list(FILTER CHIP_CXXFLAGS EXCLUDE REGEX -Wno-deprecated-copy)
    list(FILTER CHIP_CXXFLAGS EXCLUDE REGEX -Wno-cast-function-type)
    list(FILTER CHIP_CXXFLAGS EXCLUDE REGEX -Werror)
    list(APPEND CHIP_CXXFLAGS -I${ZEPHYR_BASE}/../mbedtls/include)
    list(APPEND CHIP_CXXFLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/main/include)
    convert_list_of_flags_to_string_of_flags(CHIP_CXXFLAGS CHIP_CXXFLAGS)

    set(GN_ARGS "")
    chip_gn_arg_string(GN_ARGS "target_cflags_c = string_split(\"${CHIP_CFLAGS}\")")
    chip_gn_arg_string(GN_ARGS "target_cflags_cc = string_split(\"${CHIP_CXXFLAGS}\")")
    chip_gn_arg_string(GN_ARGS "ar_ext = \"${CMAKE_AR}\"")
    chip_gn_arg_string(GN_ARGS "cc_ext = \"${CMAKE_C_COMPILER}\"")
    chip_gn_arg_string(GN_ARGS "cxx_ext = \"${CMAKE_CXX_COMPILER}\"")

    chip_gn_arg_bool_if(CONFIG_BT                GN_ARGS "chip_config_network_layer_ble")
    chip_gn_arg_bool_if(CONFIG_NET_L2_OPENTHREAD GN_ARGS "chip_enable_openthread")
    chip_gn_arg_bool_if(BUILD_TESTING            GN_ARGS "chip_build_tests")

    message("-- GN arguments:\n${GN_ARGS}")

    file(
        GENERATE OUTPUT ${CHIP_OUTPUT_DIR}/args.gn
        CONTENT "${GN_ARGS}"
    )
    # Define target
    ExternalProject_Add(
        ${TARGET_NAME}
        PREFIX              ${CHIP_OUTPUT_DIR}
        SOURCE_DIR          ${CHIP_ROOT}
        BINARY_DIR          ${CHIP_OUTPUT_DIR}
        CONFIGURE_COMMAND   gn --root=${CHIP_ROOT}/config/nrfconnect gen ${CHIP_OUTPUT_DIR}
        BUILD_COMMAND       ""
        INSTALL_COMMAND     ""
        BUILD_ALWAYS        TRUE
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
endfunction()
