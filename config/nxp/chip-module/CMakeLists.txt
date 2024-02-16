#
#   Copyright (c) 2024 Project CHIP Authors
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
#     and other optional libraries like unit tests, built with 'nxp_zephyr'
#     platform.
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
# Prepare CHIP configuration based on the project Kconfig configuration
# ==============================================================================

# Set paths

if (NOT CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
endif()
get_filename_component(GN_ROOT_TARGET ${CHIP_ROOT}/config/nxp/chip-gn REALPATH)
get_filename_component(COMMON_CMAKE_SOURCE_DIR ${CHIP_ROOT}/config/common/cmake REALPATH)

# Get common Cmake sources

include(${COMMON_CMAKE_SOURCE_DIR}/chip_gn_args.cmake)
include(${COMMON_CMAKE_SOURCE_DIR}/chip_gn.cmake)

# Prepare compiler flags
if (CHIP_CFLAGS)
    matter_add_flags("${CHIP_CFLAGS}")
endif()

matter_add_cxxflags(-D_POSIX_C_SOURCE=200809)

if (CONFIG_ARM)
    matter_add_cflags(--specs=nosys.specs)
endif()

if(CONFIG_MBEDTLS)
    zephyr_include_directories($<TARGET_PROPERTY:mbedTLS,INTERFACE_INCLUDE_DIRECTORIES>)
    zephyr_compile_definitions($<TARGET_PROPERTY:mbedTLS,INTERFACE_COMPILE_DEFINITIONS>)
endif()

zephyr_include_directories(${CHIP_ROOT}/src/platform/nxp/zephyr)

zephyr_get_compile_flags(ZEPHYR_CFLAGS_C C)
matter_add_cflags("${ZEPHYR_CFLAGS_C}")
zephyr_get_compile_flags(ZEPHYR_CFLAGS_CC CXX)
matter_add_cxxflags("${ZEPHYR_CFLAGS_CC}")
zephyr_get_gnu_cpp_standard(ZEPHYR_GNU_CPP_STD)
matter_add_cxxflags(${ZEPHYR_GNU_CPP_STD})

# Set up custom OpenThread configuration

if (CONFIG_CHIP_OPENTHREAD_CONFIG)
    get_filename_component(CHIP_OPENTHREAD_CONFIG
        ${CONFIG_CHIP_OPENTHREAD_CONFIG}
        REALPATH
        BASE_DIR ${CMAKE_SOURCE_DIR}
    )
    zephyr_set_openthread_config(${CHIP_OPENTHREAD_CONFIG})
endif()

# Set up Zephyr symbol level

if (NOT CONFIG_CHIP_DEBUG_SYMBOLS)
    set_compiler_property(PROPERTY debug -g0)
endif()

# Determine if ccache should be used

get_property(CHIP_COMPILER_LAUNCHER GLOBAL PROPERTY RULE_LAUNCH_COMPILE)

# ==============================================================================
# Generate configuration for CHIP GN build system
# ==============================================================================

matter_common_gn_args(
    DEBUG           CONFIG_DEBUG
    LIB_SHELL       CONFIG_CHIP_LIB_SHELL
    LIB_TESTS       CONFIG_CHIP_BUILD_TESTS
    PROJECT_CONFIG  ${CONFIG_CHIP_PROJECT_CONFIG}
    DEVICE_INFO_EXAMPLE_PROVIDER CONFIG_CHIP_EXAMPLE_DEVICE_INFO_PROVIDER
)
matter_add_gn_arg_string("zephyr_ar"                              ${CMAKE_AR})
matter_add_gn_arg_string("zephyr_cc"                              ${CMAKE_C_COMPILER})
matter_add_gn_arg_string("zephyr_cxx"                             ${CMAKE_CXX_COMPILER})
matter_add_gn_arg_bool  ("chip_logging"                           CONFIG_LOG)
matter_add_gn_arg_bool  ("chip_enable_openthread"                 CONFIG_NET_L2_OPENTHREAD)
matter_add_gn_arg_bool  ("chip_openthread_ftd"                    CONFIG_OPENTHREAD_FTD)
matter_add_gn_arg_bool  ("chip_config_network_layer_ble"          CONFIG_BT)
matter_add_gn_arg_bool  ("chip_inet_config_enable_ipv4"           CONFIG_CHIP_IPV4)
matter_add_gn_arg_bool  ("chip_persist_subscriptions"             CONFIG_CHIP_PERSISTENT_SUBSCRIPTIONS)
matter_add_gn_arg_bool  ("chip_monolithic_tests"                  CONFIG_CHIP_BUILD_TESTS)
matter_add_gn_arg_bool  ("chip_inet_config_enable_tcp_endpoint"   CONFIG_CHIP_BUILD_TESTS)
matter_add_gn_arg_bool  ("chip_error_logging"                     CONFIG_MATTER_LOG_LEVEL GREATER_EQUAL 1)
matter_add_gn_arg_bool  ("chip_progress_logging"                  CONFIG_MATTER_LOG_LEVEL GREATER_EQUAL 3)
matter_add_gn_arg_bool  ("chip_detail_logging"                    CONFIG_MATTER_LOG_LEVEL GREATER_EQUAL 4)
matter_add_gn_arg_bool  ("chip_automation_logging"                FALSE)
matter_add_gn_arg_bool  ("chip_malloc_sys_heap"                   CONFIG_CHIP_MALLOC_SYS_HEAP)
matter_add_gn_arg_bool  ("chip_enable_wifi"                       CONFIG_CHIP_WIFI)
matter_add_gn_arg_bool  ("chip_system_config_provide_statistics"  CONFIG_CHIP_STATISTICS)
matter_add_gn_arg_bool  ("chip_enable_icd_server"                 CONFIG_CHIP_ENABLE_ICD_SUPPORT)
matter_add_gn_arg_bool  ("enable_eventlist_attribute"             TRUE)
matter_add_gn_arg_bool  ("chip_enable_ota_requestor"              CONFIG_CHIP_OTA_REQUESTOR)

if(CONFIG_DEBUG)
    matter_add_gn_arg_bool("optimize_debug" true)
    matter_add_gn_arg_string("optimize_debug_level" "0")
    matter_add_gn_arg_string("symbol_level" "2")
endif()

if (CONFIG_CHIP_FACTORY_DATA)
    matter_add_gn_arg_bool("chip_use_transitional_commissionable_data_provider" FALSE)
    matter_add_gn_arg_bool("chip_enable_factory_data"                           TRUE)
elseif (CONFIG_CHIP_FACTORY_DATA_CUSTOM_BACKEND)
    matter_add_gn_arg_bool("chip_use_transitional_commissionable_data_provider" FALSE)
endif()

if (CONFIG_CHIP_ROTATING_DEVICE_ID)
    matter_add_gn_arg_bool("chip_enable_rotating_device_id"          TRUE)
    matter_add_gn_arg_bool("chip_enable_additional_data_advertising" TRUE)
endif()

if (CONFIG_NET_L2_OPENTHREAD)
    matter_add_gn_arg_string("chip_mdns" "platform")
elseif(CONFIG_WIFI_NXP)
    matter_add_gn_arg_string("chip_mdns" "minimal")
else()
    matter_add_gn_arg_string("chip_mdns" "none")
endif()

if (CONFIG_CHIP_CRYPTO_PSA)
    matter_add_gn_arg_string("chip_crypto" "psa")
endif()

# if (BOARD STREQUAL "native_posix")
#     matter_add_gn_arg_string("target_cpu" "x86")
# elseif (BOARD STREQUAL "native_posix_64")
#     matter_add_gn_arg_string("target_cpu" "x64")
# endif()

if (NOT CONFIG_CHIP_DEBUG_SYMBOLS)
    matter_add_gn_arg_string("symbol_level" "0")
endif()

# if (CHIP_COMPILER_LAUNCHER)
#     matter_add_gn_arg_string("pw_command_launcher" ${CHIP_COMPILER_LAUNCHER})
# endif()

# if (CONFIG_CHIP_PW_RPC)
#     set(PIGWEED_DIR "//third_party/pigweed/repo")
#     matter_add_gn_arg_string("pw_assert_BACKEND" ${PIGWEED_DIR}/pw_assert_log:check_backend)
#     matter_add_gn_arg_string("pw_log_BACKEND" ${PIGWEED_DIR}/pw_log_basic)
#     matter_add_gn_arg("pw_build_LINK_DEPS" [\"${PIGWEED_DIR}/pw_assert:impl\",\ \"${PIGWEED_DIR}/pw_log:impl\"])
# endif()

matter_generate_args_tmp_file()

# ==============================================================================
# Build chip library
# ==============================================================================

matter_build(chip
    LIB_SHELL   ${CONFIG_CHIP_LIB_SHELL}
    LIB_TESTS   ${CONFIG_CHIP_BUILD_TESTS}
    DEVICE_INFO_EXAMPLE_PROVIDER ${CONFIG_CHIP_EXAMPLE_DEVICE_INFO_PROVIDER}
    GN_DEPENDENCIES kernel
)

set_property(GLOBAL APPEND PROPERTY ZEPHYR_INTERFACE_LIBS chip)
# Enable visibility of POSIX.1-2008 functions, such as strnlen
target_compile_definitions(chip INTERFACE _POSIX_C_SOURCE=200809)
# Make sure that kernel symbols that are only referenced by the Matter libraries are resolved.
target_link_libraries(chip INTERFACE $<TARGET_FILE:kernel>)

if (CONFIG_CHIP_MALLOC_SYS_HEAP_OVERRIDE)
    target_link_options(chip INTERFACE
        -Wl,--wrap=malloc
        -Wl,--wrap=calloc
        -Wl,--wrap=realloc
        -Wl,--wrap=free
        -Wl,--wrap=_malloc_r
        -Wl,--wrap=_calloc_r
        -Wl,--wrap=_realloc_r
        -Wl,--wrap=_free_r
    )
endif()

# ==============================================================================
# Define 'chip-ota-image' target for building CHIP OTA image
# ==============================================================================
if(EXISTS "${CHIP_ROOT}/src/platform/nxp/zephyr/boards/${BOARD}/${BOARD}.overlay")
  set(DTC_BOARD_OVERLAY "${CHIP_ROOT}/src/platform/nxp/zephyr/boards/${BOARD}/${BOARD}.overlay")
else()
  set(DTC_BOARD_OVERLAY "")
  message(STATUS "No board overlay found")
endif()

if (CONFIG_CHIP_OTA_REQUESTOR)
    if(CONFIG_MCUBOOT_SIGNATURE_KEY_FILE STREQUAL "")
        set(ZEPHYR_OUTPUT_NAME "zephyr")
    else()
        set(ZEPHYR_OUTPUT_NAME "zephyr.signed")
    endif()

    set(GLOBAL_BOOTLOADER_CONF_OVERLAY_FILE "${CHIP_ROOT}/config/nxp/app/bootloader.conf")

    add_custom_target(build_mcuboot ALL
        COMMAND
        west build -b ${BOARD} -d build_mcuboot ${ZEPHYR_BASE}/../bootloader/mcuboot/boot/zephyr
            -- -DOVERLAY_CONFIG=${GLOBAL_BOOTLOADER_CONF_OVERLAY_FILE} -DEXTRA_DTC_OVERLAY_FILE=${DTC_BOARD_OVERLAY}
        COMMAND
        cp ${PROJECT_BINARY_DIR}/../modules/connectedhomeip/build_mcuboot/zephyr/zephyr.bin ${PROJECT_BINARY_DIR}/zephyr.mcuboot.bin
    )

    set(BLOCK_SIZE "1024")
    dt_nodelabel(dts_partition_path NODELABEL "boot_partition")
    dt_reg_size(mcuboot_size PATH ${dts_partition_path})
    math(EXPR boot_blocks "${mcuboot_size} / ${BLOCK_SIZE}" OUTPUT_FORMAT DECIMAL)

    add_custom_target(merge_mcuboot ALL
        COMMAND
        dd if=${PROJECT_BINARY_DIR}/zephyr.mcuboot.bin of=${PROJECT_BINARY_DIR}/zephyr_full.bin
        COMMAND
        dd if=${PROJECT_BINARY_DIR}/${ZEPHYR_OUTPUT_NAME}.bin of=${PROJECT_BINARY_DIR}/zephyr_full.bin bs=${BLOCK_SIZE} seek=${boot_blocks}
    )
    add_dependencies(merge_mcuboot ${ZEPHYR_FINAL_EXECUTABLE})

    if (CONFIG_CHIP_OTA_IMAGE_BUILD)
        chip_ota_image(chip-ota-image
            INPUT_FILES ${PROJECT_BINARY_DIR}/${ZEPHYR_OUTPUT_NAME}.bin
            OUTPUT_FILE ${PROJECT_BINARY_DIR}/${CONFIG_CHIP_OTA_IMAGE_FILE_NAME}
        )
    endif()
endif()

# ==============================================================================
# Define 'factory_data' target for generating a factory data partition
# ==============================================================================

if (CONFIG_CHIP_FACTORY_DATA_BUILD)
    nxp_generate_factory_data()
endif()

endif() # CONFIG_CHIP
