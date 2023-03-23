#
#   Copyright (c) 2022 Project CHIP Authors
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
#     and other optional libraries like unit tests, built with 'open-iot-sdk'
#     platform.
#     Since CHIP doesn't provide native CMake support, ExternalProject
#     module is used to build the required artifacts with GN meta-build
#     system.
#

include(ExternalProject)
include(util.cmake)

# ==============================================================================
# Declare configuration variables and define constants
# ==============================================================================
# C/C++ compiler flags passed to CHIP build system
list(APPEND CHIP_CFLAGS \"-Wno-unused-function\")

# C compiler flags passed to CHIP build system
list(APPEND CHIP_CFLAGS_C ${CMAKE_C_FLAGS})

# C++ compiler flags passed to CHIP build system
list(APPEND CHIP_CFLAGS_CC ${CMAKE_CXX_FLAGS})

# GN meta-build system arguments in the form of 'key1 = value1\nkey2 = value2...' string
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

macro(chip_gn_arg_bool ARG)
    if (${ARGN})
        string(APPEND CHIP_GN_ARGS "--arg\n${ARG}\ntrue\n")
    else()
        string(APPEND CHIP_GN_ARGS "--arg\n${ARG}\nfalse\n")
    endif()
endmacro()

macro(chip_gn_arg_cflags ARG CFLAGS)
    string(APPEND CHIP_GN_ARGS "--arg-cflags\n${ARG}\n${CFLAGS}\n")
endmacro()

macro(chip_gn_arg_cflags_lang ARG CFLAGS)
    string(APPEND CHIP_GN_ARGS "--arg-cflags-lang\n${ARG}\n${CFLAGS}\n")
endmacro()

macro(chip_gn_arg ARG VALUE)
    string(APPEND CHIP_GN_ARGS "--arg\n${ARG}\n${VALUE}\n")
endmacro()

# Select gnu++<YY> standard based on project configuration
macro(get_gnu_cpp_standard VAR)
    if (CONFIG_STD_CPP11)
        list(APPEND ${VAR} -std=gnu++11)
    elseif (CONFIG_STD_CPP14)
        list(APPEND ${VAR} -std=gnu++14)
    elseif (CONFIG_STD_CPP17)
        list(APPEND ${VAR} -std=gnu++17)
    elseif (CONFIG_STD_CPP2A)
        list(APPEND ${VAR} -std=gnu++20)
    endif()
endmacro()

# ==============================================================================
# Prepare CHIP configuration based on the project configuration
# ==============================================================================
# Set paths
if (NOT CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../.. REALPATH)
endif()

set(GN_ROOT_TARGET ${CHIP_ROOT}/config/openiotsdk/chip-gn)

# Prepare compiler flags
# All Open IoT SDK targets
list(APPEND ALL_SDK_TARGETS)

foreach(source_dir ${SDK_SOURCES_BINARY_DIRS})
    get_all_cmake_targets(SDK_TARGETS ${source_dir})
    list(APPEND ALL_SDK_TARGETS ${SDK_TARGETS})
endforeach()

# Exclude unnecessary targets
list(FILTER ALL_SDK_TARGETS EXCLUDE REGEX "^.*linker.*|.*example.*|.*apps.*|.*doc.*|dist|lib$")

foreach(target ${ALL_SDK_TARGETS})
    get_target_common_compile_flags(SDK_TARGET_CFLAGS ${target})
    list(APPEND CHIP_CFLAGS ${SDK_TARGET_CFLAGS})
endforeach()

# Remove duplicated flags
list(REMOVE_DUPLICATES CHIP_CFLAGS)

get_gnu_cpp_standard(CHIP_CFLAGS_CC)

# CFLAGS are put in random order, sort them before converting them to a string
list(SORT CHIP_CFLAGS)
list(SORT CHIP_CFLAGS_C)
list(SORT CHIP_CFLAGS_CC)

set(SEPARATOR ",")
convert_list_of_flags_to_string_of_flags(CHIP_CFLAGS CHIP_CFLAGS ${SEPARATOR})
set(SEPARATOR " ")
convert_list_of_flags_to_string_of_flags(CHIP_CFLAGS_C CHIP_CFLAGS_C ${SEPARATOR})
convert_list_of_flags_to_string_of_flags(CHIP_CFLAGS_CC CHIP_CFLAGS_CC ${SEPARATOR})

# Prepare CHIP libraries that the application should be linked with
set(CHIP_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/lib/libCHIP.a")

if (CONFIG_CHIP_LIB_SHELL)
    list(APPEND CHIP_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/lib/libCHIPShell.a")
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

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CONFIG_DEBUG YES)
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
chip_gn_arg_string("target_cpu"                           "${CMAKE_SYSTEM_PROCESSOR}")
chip_gn_arg_cflags("target_cflags"                        ${CHIP_CFLAGS})
chip_gn_arg_cflags_lang("target_cflags_c"                 ${CHIP_CFLAGS_C})
chip_gn_arg_cflags_lang("target_cflags_cc"                ${CHIP_CFLAGS_CC})
chip_gn_arg_string("openiotsdk_ar"                        ${CMAKE_AR})
chip_gn_arg_string("openiotsdk_cc"                        ${CMAKE_C_COMPILER})
chip_gn_arg_string("openiotsdk_cxx"                       ${CMAKE_CXX_COMPILER})
chip_gn_arg_string("chip_project_config_include"          "${CHIP_PROJECT_CONFIG}")
chip_gn_arg_string("chip_system_project_config_include"   "${CHIP_PROJECT_CONFIG}")
chip_gn_arg_bool  ("is_debug"                             CONFIG_DEBUG)
chip_gn_arg_bool  ("chip_build_tests"                     CONFIG_CHIP_LIB_TESTS)
chip_gn_arg_bool  ("chip_monolithic_tests"                CONFIG_CHIP_LIB_TESTS)
chip_gn_arg_bool  ("chip_build_libshell"                  CONFIG_CHIP_LIB_SHELL)
chip_gn_arg_bool  ("chip_detail_logging"                  CONFIG_CHIP_DETAIL_LOGGING)
chip_gn_arg_bool  ("chip_progress_logging"                CONFIG_CHIP_PROGRESS_LOGGING)
chip_gn_arg_bool  ("chip_automation_logging"              CONFIG_CHIP_AUTOMATION_LOGGING)
chip_gn_arg_bool  ("chip_error_logging"                   CONFIG_CHIP_ERROR_LOGGING)
if (TARGET cmsis-rtos-api)
    chip_gn_arg_string("target_os"                        "cmsis-rtos")
endif()
chip_gn_arg_string("optimize_debug_level"                 "s")

file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/args.tmp" CONTENT ${CHIP_GN_ARGS})

# ==============================================================================
# Define 'chip-gn' target that builds CHIP library(ies) with GN build system
# ==============================================================================
ExternalProject_Add(
    chip-gn
    PREFIX                  ${CMAKE_CURRENT_BINARY_DIR}
    SOURCE_DIR              ${CHIP_ROOT}
    BINARY_DIR              ${CMAKE_CURRENT_BINARY_DIR}
    CONFIGURE_COMMAND       ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/make_gn_args.py
                                @args.tmp > args.gn &&
                            ${GN_EXECUTABLE}
                                --root=${CHIP_ROOT}
                                --root-target=${GN_ROOT_TARGET}
                                --dotfile=${GN_ROOT_TARGET}/.gn
                                --script-executable=${Python3_EXECUTABLE}
                                gen --check --fail-on-unused-args ${CMAKE_CURRENT_BINARY_DIR}
    BUILD_COMMAND           ninja
    INSTALL_COMMAND         ""
    BUILD_BYPRODUCTS        ${CHIP_LIBRARIES}
    BUILD_ALWAYS            TRUE
    USES_TERMINAL_CONFIGURE TRUE
    USES_TERMINAL_BUILD     TRUE
)

# ==============================================================================
# Define 'openiotsdk-chip' target that exposes CHIP and Open IoT SDK 
# headers & libraries to the application
# ==============================================================================
add_library(openiotsdk-chip INTERFACE)
target_compile_definitions(openiotsdk-chip INTERFACE CHIP_HAVE_CONFIG_H)
target_include_directories(openiotsdk-chip INTERFACE
    ${CHIP_ROOT}/src
    ${CHIP_ROOT}/src/include
    ${CHIP_ROOT}/src/lib
    ${CHIP_ROOT}/third_party/nlassert/repo/include
    ${CHIP_ROOT}/third_party/nlio/repo/include
    ${CHIP_ROOT}/zzz_generated/app-common
    ${CMAKE_CURRENT_BINARY_DIR}/gen/include
)
target_link_directories(openiotsdk-chip INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/lib)
target_link_libraries(openiotsdk-chip INTERFACE -Wl,--start-group ${CHIP_LIBRARIES} -Wl,--end-group)
target_link_libraries(openiotsdk-chip INTERFACE
    $<$<TARGET_EXISTS:mcu-driver-hal>:mcu-driver-hal>
    $<$<TARGET_EXISTS:mcu-driver-bootstrap>:mcu-driver-bootstrap>
    $<$<TARGET_EXISTS:mdh-reference-platforms-for-arm>:mdh-reference-platforms-for-arm>
    $<$<TARGET_EXISTS:mbedtls>:mbedtls>
    $<$<TARGET_EXISTS:lwipcore>:lwipcore>
    $<$<TARGET_EXISTS:iotsdk-ip-network-api>:iotsdk-ip-network-api>
    $<$<TARGET_EXISTS:iotsdk-tdbstore>:iotsdk-tdbstore>
    $<$<TARGET_EXISTS:iotsdk-blockdevice>:iotsdk-blockdevice>
    $<$<TARGET_EXISTS:iotsdk-serial-retarget>:$<TARGET_OBJECTS:iotsdk-serial-retarget>>
)
add_dependencies(openiotsdk-chip chip-gn)

target_include_directories(openiotsdk-chip INTERFACE
    ${CHIP_ROOT}/config/openiotsdk/mbedtls
)

if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_definitions(openiotsdk-chip INTERFACE
        NDEBUG
    )
endif()
