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
#     and other optional libraries like unit tests, built with 'mbed'
#     platform.
#     Since CHIP doesn't provide native CMake support, ExternalProject
#     module is used to build the required artifacts with GN meta-build
#     system.
#

include(ExternalProject)
include(mbed-util.cmake)

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

# GN meta-build system arguments in the form of 'key1 = value1\nkey2 = value2...' string
string(APPEND CHIP_GN_ARGS)

# C/C++ compiler flags which should not be forwarded to CHIP
# build system (e.g. because CHIP configures them on its own)
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

# ==============================================================================
# Helper macros
# ==============================================================================

macro(chip_gn_arg_string ARG STRING)
    string(APPEND CHIP_GN_ARGS "${ARG} = \"${STRING}\"\n")
endmacro()

macro(chip_gn_arg_bool ARG BOOLEAN)
    if (${BOOLEAN})
        string(APPEND CHIP_GN_ARGS "${ARG} = true\n")
    else()
        string(APPEND CHIP_GN_ARGS "${ARG} = false\n")
    endif()
endmacro()

macro(chip_gn_arg_flags ARG CFLAGS)
    string(APPEND CHIP_GN_ARGS "${ARG} = [${CFLAGS}]\n")
endmacro()

macro(chip_gn_arg_lang_flags ARG CFLAGS)
    set(CFLAG_EXCLUDES "[")
    foreach(cflag ${CHIP_CFLAG_EXCLUDES})
        string(APPEND CFLAG_EXCLUDES "\"${cflag}\", ")
    endforeach()
    string(APPEND CFLAG_EXCLUDES "]")
    string(APPEND CHIP_GN_ARGS "${ARG} = filter_exclude(string_split(\"${CFLAGS}\"), ${CFLAG_EXCLUDES})\n")
endmacro()

macro(mbed_interface_library_named name)
  add_library(${name} INTERFACE)
  set_property(GLOBAL APPEND PROPERTY ZEPHYR_INTERFACE_LIBS ${name})
endmacro()

# ==============================================================================
# Prepare CHIP configuration based on the project configuration
# ==============================================================================
# Read configuration file and parse it content to create cmake variable
file(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/config ConfigContents)
foreach(NameAndValue ${ConfigContents})
  # Strip leading spaces
  string(REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue})
  # Find variable name
  string(REGEX MATCH "^[^=]+" Name ${NameAndValue})
  # Find the value
  string(REPLACE "${Name}=" "" Value ${NameAndValue})
  # Set the variable
  set(${Name} "${Value}")
endforeach()

if (NOT CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../.. REALPATH)
endif()

# Prepare compiler flags

mbed_get_target_common_compile_flags(CHIP_CFLAGS mbed-core)
mbed_get_lang_compile_flags(CHIP_CFLAGS_C mbed-core C)
list(APPEND CHIP_CFLAGS_C ${CMAKE_C_FLAGS_INIT})
mbed_get_lang_compile_flags(CHIP_CFLAGS_CC mbed-core CXX)
list(APPEND CHIP_CFLAGS_CC ${CMAKE_CXX_FLAGS_INIT})

# Add support for Mbed BLE
mbed_get_target_common_compile_flags(CHIP_MBEDBLE_CFLAGS mbed-ble)
list(APPEND CHIP_CFLAGS ${CHIP_MBEDBLE_CFLAGS})

# Add support for Mbed event 
mbed_get_target_common_compile_flags(CHIP_MBEDEVENTS_CFLAGS mbed-events)
list(APPEND CHIP_CFLAGS ${CHIP_MBEDEVENTS_CFLAGS})

# Add support for Mbed rtos
mbed_get_target_common_compile_flags(CHIP_MBEDRTOS_CFLAGS mbed-rtos)
list(APPEND CHIP_CFLAGS ${CHIP_MBEDRTOS_CFLAGS})

# Add support for Mbed storage
mbed_get_target_common_compile_flags(CHIP_MBEDSTORAGE_CFLAGS mbed-storage-kv-global-api)
list(APPEND CHIP_CFLAGS ${CHIP_MBEDSTORAGE_CFLAGS})

# Add support for Mbed Socket 
mbed_get_target_common_compile_flags(CHIP_MBEDNETSOCKET_CFLAGS mbed-netsocket)
list(APPEND CHIP_CFLAGS ${CHIP_MBEDNETSOCKET_CFLAGS})

if (CONFIG_CHIP_WITH_EXTERNAL_MBEDTLS)
    mbed_get_target_common_compile_flags(CHIP_MBEDTLS_CFLAGS mbed-mbedtls)
    list(APPEND CHIP_CFLAGS ${CHIP_MBEDTLS_CFLAGS})
endif()

list(APPEND CHIP_CFLAGS 
    \"-D__LINUX_ERRNO_EXTENSIONS__=1\"
)

set(SEPARATOR ",")
convert_list_of_flags_to_string_of_flags(CHIP_CFLAGS CHIP_CFLAGS ${SEPARATOR})
set(SEPARATOR " ")
convert_list_of_flags_to_string_of_flags(CHIP_CFLAGS_C CHIP_CFLAGS_C ${SEPARATOR})
convert_list_of_flags_to_string_of_flags(CHIP_CFLAGS_CC CHIP_CFLAGS_CC ${SEPARATOR})

# Prepare CHIP libraries that the application should be linked with

if (NOT CHIP_LIBRARIES)
    set(CHIP_LIBRARIES -lCHIP)
endif()

if (CONFIG_CHIP_LIB_SHELL)
    list(APPEND CHIP_LIBRARIES -lCHIPShell)
endif()

# Set up CHIP project configuration file

set(CHIP_DEFAULT_CONFIG_FILE "<${CHIP_ROOT}/config/mbed/CHIPProjectConfig.h>")
set(CHIP_PROJECT_CONFIG         ${CHIP_DEFAULT_CONFIG_FILE})

if (CONFIG_CHIP_PROJECT_CONFIG)
    get_filename_component(CHIP_PROJECT_CONFIG 
        ${CONFIG_CHIP_PROJECT_CONFIG}
        REALPATH
        BASE_DIR ${CMAKE_SOURCE_DIR}
    )
    set(CHIP_PROJECT_CONFIG "<${CHIP_PROJECT_CONFIG}>")
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "debug")
    set(CONFIG_DEBUG "y")
endif()

# ==============================================================================
# Generate configuration for CHIP GN build system
# ==============================================================================

chip_gn_arg_flags("target_cflags"                         ${CHIP_CFLAGS})
chip_gn_arg_lang_flags("target_cflags_c"                  ${CHIP_CFLAGS_C})
chip_gn_arg_lang_flags("target_cflags_cc"                 ${CHIP_CFLAGS_CC})
chip_gn_arg_string("mbed_ar"                              ${CMAKE_AR})
chip_gn_arg_string("mbed_cc"                              ${CMAKE_C_COMPILER})
chip_gn_arg_string("mbed_cxx"                             ${CMAKE_CXX_COMPILER})
chip_gn_arg_string("chip_project_config_include"          "${CHIP_PROJECT_CONFIG}")
chip_gn_arg_bool  ("is_debug"                             CONFIG_DEBUG)
chip_gn_arg_bool  ("chip_build_tests"                     CONFIG_CHIP_BUILD_TESTS)
chip_gn_arg_bool  ("chip_build_libshell"                  CONFIG_CHIP_LIB_SHELL)
chip_gn_arg_bool  ("chip_with_platform_mbedtls"           CONFIG_CHIP_WITH_EXTERNAL_MBEDTLS)
chip_gn_arg_bool  ("chip_bypass_rendezvous"               CONFIG_CHIP_BYPASS_RENDEZVOUS)

if (BOARD STREQUAL "native_posix")
    chip_gn_arg_string("target_cpu" "x86")
elseif (BOARD STREQUAL "native_posix_64")
    chip_gn_arg_string("target_cpu" "x64")
endif()

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/args.gn CONTENT ${CHIP_GN_ARGS})

# ==============================================================================
# Define 'chip-gn' target that builds CHIP library(ies) with GN build system
# ==============================================================================
ExternalProject_Add(
    chip-gn
    PREFIX                  ${CMAKE_CURRENT_BINARY_DIR}
    SOURCE_DIR              ${CHIP_ROOT}
    BINARY_DIR              ${CMAKE_CURRENT_BINARY_DIR}
    CONFIGURE_COMMAND       gn --root=${CHIP_ROOT}/config/mbed/chip-gn gen --check --fail-on-unused-args ${CMAKE_CURRENT_BINARY_DIR}
    BUILD_COMMAND           ninja
    INSTALL_COMMAND         ""
    BUILD_BYPRODUCTS        ${CHIP_LIBRARIES}
    BUILD_ALWAYS            TRUE
    USES_TERMINAL_CONFIGURE TRUE
    USES_TERMINAL_BUILD     TRUE
)

# ==============================================================================
# Define 'chip' target that exposes CHIP headers & libraries to the application
# ==============================================================================
mbed_interface_library_named(chip)
target_compile_definitions(chip INTERFACE CHIP_HAVE_CONFIG_H)
target_include_directories(chip INTERFACE
    ${CHIP_ROOT}/src
    ${CHIP_ROOT}/src/app/server
    ${CHIP_ROOT}/src/app/util
    ${CHIP_ROOT}/src/include
    ${CHIP_ROOT}/src/lib
    ${CHIP_ROOT}/src/lib/core
    ${CHIP_ROOT}/third_party/nlassert/repo/include
    ${CMAKE_CURRENT_BINARY_DIR}/gen/include
    ${CMAKE_CURRENT_BINARY_DIR}/gen/third_party/connectedhomeip/src/app/include
)
target_link_directories(chip INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/lib)
target_link_libraries(chip INTERFACE -Wl,--start-group ${CHIP_LIBRARIES} -Wl,--end-group)
add_dependencies(chip chip-gn)

# ==============================================================================
# Define mbed target configuration according to CHIP component usage
# ==============================================================================
# CHIP includes path
list(APPEND CHIP_INCLUDES)

# CHIP defines
list(APPEND CHIP_DEFINES)


list(APPEND CHIP_INCLUDES 
    ${CHIP_ROOT}/config/mbed/mbedtls
    ${CHIP_ROOT}/src/platform/mbed/posix/include
    ${CHIP_ROOT}/src/platform/mbed/net/include
)

list(APPEND CHIP_DEFINES 
    __LINUX_ERRNO_EXTENSIONS__=1
)

target_include_directories(${APP_TARGET} PRIVATE 
                           ${CHIP_INCLUDES}
)

target_compile_definitions(${APP_TARGET} PRIVATE
                           ${CHIP_DEFINES}                      
)
