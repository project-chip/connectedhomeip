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
#     Since CHIP doesn't provide native CMake support, ExternalProject
#     module is used to build the required artifacts with GN meta-build
#     system. It is assumed that find_package(Zephyr) has been called before
#     including this file.
#

if (CONFIG_CHIP)

include(ExternalProject)
include(../../zephyr/ota-image.cmake)
include(../../zephyr/zephyr-util.cmake)
include(generate_factory_data.cmake)

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

macro(chip_gn_arg ARG VALUE)
    string(APPEND CHIP_GN_ARGS "--arg\n${ARG}\n${VALUE}\n")
endmacro()

# ==============================================================================
# Prepare CHIP configuration based on the project Kconfig configuration
# ==============================================================================

if (NOT CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
endif()

set(GN_ROOT_TARGET ${CHIP_ROOT}/config/telink/chip-gn)

if (CONFIG_POSIX_API)
    list(APPEND CHIP_CFLAGS
        -D_SYS__PTHREADTYPES_H_
        -isystem${ZEPHYR_BASE}/include/zephyr/posix
    )
endif()

zephyr_include_directories(${CHIP_ROOT}/src/platform/telink/)

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

if (CONFIG_CHIP_LIB_SHELL)
    list(APPEND CHIP_LIBRARIES -lCHIPShell)
endif()

if (CONFIG_CHIP_PW_RPC)
    list(APPEND CHIP_LIBRARIES -lPwRpc)
endif()

if (CONFIG_TELINK_BLE_LIB)
    list(APPEND CHIP_LIBRARIES -lB91_ble_lib)
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

chip_gn_arg_cflags("target_cflags"                          ${CHIP_CFLAGS})
chip_gn_arg_cflags("target_cflags_c"                        ${CHIP_CFLAGS_C})
chip_gn_arg_cflags("target_cflags_cc"                       ${CHIP_CFLAGS_CC})
chip_gn_arg_string("zephyr_ar"                              ${CMAKE_AR})
chip_gn_arg_string("zephyr_cc"                              ${CMAKE_C_COMPILER})
chip_gn_arg_string("zephyr_cxx"                             ${CMAKE_CXX_COMPILER})
chip_gn_arg_bool  ("is_debug"                               CONFIG_DEBUG)
chip_gn_arg_bool  ("chip_enable_openthread"                 CONFIG_NET_L2_OPENTHREAD)
chip_gn_arg_bool  ("chip_openthread_ftd"                    CONFIG_OPENTHREAD_FTD)
chip_gn_arg_bool  ("chip_inet_config_enable_ipv4"           CONFIG_NET_IPV4)
chip_gn_arg_bool  ("chip_enable_ota_requestor"              CONFIG_CHIP_OTA_REQUESTOR)
chip_gn_arg_bool  ("chip_build_tests"                       CONFIG_CHIP_BUILD_TESTS)
chip_gn_arg_bool  ("chip_inet_config_enable_tcp_endpoint"   CONFIG_CHIP_BUILD_TESTS)
chip_gn_arg_bool  ("chip_build_libshell"                    CONFIG_CHIP_LIB_SHELL)

if (CONFIG_CHIP_FACTORY_DATA)
    chip_gn_arg_bool  ("chip_use_transitional_commissionable_data_provider"  "false")
    chip_gn_arg_bool  ("chip_enable_factory_data"                            "true")
elseif (CONFIG_CHIP_FACTORY_DATA_CUSTOM_BACKEND)
    chip_gn_arg_bool  ("chip_use_transitional_commissionable_data_provider"  "false")
endif()

if (CONFIG_CHIP_ROTATING_DEVICE_ID)
    chip_gn_arg_bool("chip_enable_rotating_device_id"          "true")
    chip_gn_arg_bool("chip_enable_additional_data_advertising" "true")
endif()

if (CONFIG_CHIP_ENABLE_DNSSD_SRP)
    chip_gn_arg_string("chip_mdns" "platform")
endif()

if (CHIP_PROJECT_CONFIG)
    chip_gn_arg_string("chip_project_config_include"        ${CHIP_PROJECT_CONFIG})
    chip_gn_arg_string("chip_system_project_config_include" ${CHIP_PROJECT_CONFIG})
endif()
if (CONFIG_CHIP_EXAMPLE_DEVICE_INFO_PROVIDER)
    chip_gn_arg_bool("chip_build_example_providers" "true")
    list(APPEND CHIP_LIBRARIES -lMatterDeviceInfoProviderExample)
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
    ${CHIP_ROOT}/third_party/telink_sdk
    ${CMAKE_CURRENT_BINARY_DIR}/gen/include
)
target_link_directories(chip INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/lib)
if (CONFIG_CHIP_LIB_SHELL)
    target_link_options(chip INTERFACE -Wl,--whole-archive -lCHIPShell -Wl,--no-whole-archive)
endif()

if (CONFIG_CHIP_EXAMPLE_DEVICE_INFO_PROVIDER)
    target_include_directories(chip INTERFACE ${CHIP_ROOT}/examples/providers)
endif()

if (CONFIG_TELINK_BLE_LIB)
    target_link_directories(chip INTERFACE ${CHIP_ROOT}/third_party/telink_sdk/repo/eagle_ble_sdk/proj_lib)
endif()

target_link_libraries(chip INTERFACE -Wl,--start-group ${CHIP_LIBRARIES} -Wl,--end-group)

add_dependencies(chip chip-gn)

# ==============================================================================
# Define 'chip-ota-image' target for building CHIP OTA image
# ==============================================================================

if (CONFIG_CHIP_OTA_IMAGE_BUILD)
    add_custom_target(build_mcuboot ALL
        COMMAND
        west build -b tlsr9518adk80d -d build_mcuboot ${ZEPHYR_BASE}/../bootloader/mcuboot/boot/zephyr
    )

    add_custom_target(west_sign ALL
        COMMAND
        west sign -t imgtool -p ${ZEPHYR_BASE}/../bootloader/mcuboot/scripts/imgtool.py -d ${PROJECT_BINARY_DIR}/.. -- --key ${ZEPHYR_BASE}/../bootloader/mcuboot/root-rsa-2048.pem
    )

    add_custom_target(merge_mcuboot ALL
        COMMAND
        dd if=${PROJECT_BINARY_DIR}/../modules/chip-module/build_mcuboot/zephyr/zephyr.bin of=${PROJECT_BINARY_DIR}/zephyr.bin
        COMMAND
        dd if=${PROJECT_BINARY_DIR}/zephyr.signed.bin of=${PROJECT_BINARY_DIR}/zephyr.bin bs=1024 seek=64
    )

    chip_ota_image(chip-ota-image
        INPUT_FILES ${PROJECT_BINARY_DIR}/zephyr.signed.bin
        OUTPUT_FILE ${PROJECT_BINARY_DIR}/zephyr-ota.bin
    )

    add_dependencies(west_sign ${ZEPHYR_FINAL_EXECUTABLE})
    add_dependencies(merge_mcuboot west_sign)
    add_dependencies(chip-ota-image west_sign)
endif()

if (CONFIG_CHIP_FACTORY_DATA_MERGE_WITH_FIRMWARE)
    add_custom_target(merge_factory_data ALL
        COMMAND
        dd if=${PROJECT_BINARY_DIR}/factory/factory_data.bin of=${PROJECT_BINARY_DIR}/zephyr.bin bs=1024 seek=976
    )
if (CONFIG_CHIP_OTA_IMAGE_BUILD)
    add_dependencies(merge_factory_data merge_mcuboot)
else()
    add_dependencies(merge_factory_data ${ZEPHYR_FINAL_EXECUTABLE})
endif()
endif()

# ==============================================================================
# Define 'factory_data' target for generating a factory data partition
# ==============================================================================

if (CONFIG_CHIP_FACTORY_DATA_BUILD)
    telink_generate_factory_data()
endif()

endif() # CONFIG_CHIP
