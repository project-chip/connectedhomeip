#
#   Copyright (c) 2022-2024 Project CHIP Authors
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
# Prepare CHIP configuration based on the project Kconfig configuration
# ==============================================================================

if (NOT CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
endif()
get_filename_component(GN_ROOT_TARGET ${CHIP_ROOT}/config/telink/chip-gn REALPATH)
get_filename_component(COMMON_CMAKE_SOURCE_DIR ${CHIP_ROOT}/config/common/cmake REALPATH)

# Get common Cmake sources

include(${COMMON_CMAKE_SOURCE_DIR}/chip_gn_args.cmake)
include(${COMMON_CMAKE_SOURCE_DIR}/chip_gn.cmake)

# Prepare compiler flags
matter_add_flags(-isystem${ZEPHYR_BASE}/../modules/crypto/mbedtls/include/)

if (CONFIG_POSIX_API)
    matter_add_flags(-D_DEFAULT_SOURCE)
    matter_add_flags(-isystem${ZEPHYR_BASE}/include/zephyr/posix)
endif()

matter_add_flags(-DNL_ASSERT_USE_FLAGS_DEFAULT=1)

zephyr_include_directories(${CHIP_ROOT}/src/platform/telink/)
zephyr_include_directories(${CHIP_ROOT}/src/platform/telink/wifi/${ZEPHYR_VERSION_STRING})

zephyr_get_compile_flags(ZEPHYR_CFLAGS_C C)
matter_add_cflags("${ZEPHYR_CFLAGS_C}")
zephyr_get_compile_flags(ZEPHYR_CFLAGS_CC CXX)
matter_add_cxxflags("${ZEPHYR_CFLAGS_CC}")
zephyr_get_gnu_cpp_standard(ZEPHYR_GNU_CPP_STD)
matter_add_cxxflags(${ZEPHYR_GNU_CPP_STD})

matter_add_flags(-DMBEDTLS_USER_CONFIG_FILE=<${CONFIG_MBEDTLS_CFG_FILE}>)

# Set up custom OpenThread configuration

if (CONFIG_CHIP_OPENTHREAD_CONFIG)
    get_filename_component(CHIP_OPENTHREAD_CONFIG
        ${CONFIG_CHIP_OPENTHREAD_CONFIG}
        REALPATH
        BASE_DIR ${CMAKE_SOURCE_DIR}
    )
    zephyr_set_openthread_config(${CHIP_OPENTHREAD_CONFIG})
endif()

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
matter_add_gn_arg_bool  ("chip_enable_nfc_onboarding_payload"     CONFIG_CHIP_NFC_ONBOARDING_PAYLOAD)
matter_add_gn_arg_bool  ("chip_enable_ota_requestor"              CONFIG_CHIP_OTA_REQUESTOR)
matter_add_gn_arg_bool  ("chip_inet_config_enable_tcp_endpoint"   FALSE)
matter_add_gn_arg_bool  ("chip_error_logging"                     CONFIG_MATTER_LOG_LEVEL GREATER_EQUAL 1)
matter_add_gn_arg_bool  ("chip_progress_logging"                  CONFIG_MATTER_LOG_LEVEL GREATER_EQUAL 3)
matter_add_gn_arg_bool  ("chip_detail_logging"                    CONFIG_MATTER_LOG_LEVEL GREATER_EQUAL 4)
matter_add_gn_arg_bool  ("chip_automation_logging"                FALSE)
matter_add_gn_arg_bool  ("chip_enable_wifi"                       CONFIG_WIFI_W91)
matter_add_gn_arg_bool  ("chip_enable_icd_server"                 CONFIG_CHIP_ENABLE_ICD_SUPPORT)
matter_add_gn_arg_bool  ("chip_enable_factory_data"               CONFIG_CHIP_FACTORY_DATA)
matter_add_gn_arg_bool  ("chip_mdns_minimal"                      CONFIG_WIFI_W91)
matter_add_gn_arg_bool  ("chip_mdns_platform"                     CONFIG_NET_L2_OPENTHREAD)

if (CONFIG_CHIP_ENABLE_ICD_SUPPORT)
    matter_add_gn_arg_bool  ("chip_enable_icd_lit"                       CONFIG_CHIP_ICD_LIT_SUPPORT)
    matter_add_gn_arg_bool  ("chip_enable_icd_checkin"                   CONFIG_CHIP_ICD_CHECK_IN_SUPPORT)
    matter_add_gn_arg_bool  ("chip_enable_icd_user_active_mode_trigger"  CONFIG_CHIP_ICD_UAT_SUPPORT)
endif()

if (CONFIG_CHIP_FACTORY_DATA OR CONFIG_CHIP_FACTORY_DATA_CUSTOM_BACKEND)
    matter_add_gn_arg_bool("chip_use_transitional_commissionable_data_provider"  FALSE)
    matter_add_gn_arg_bool("chip_use_transitional_device_instance_info_provider" FALSE)
endif()

if (CONFIG_CHIP_ROTATING_DEVICE_ID)
    matter_add_gn_arg_bool("chip_enable_rotating_device_id"          TRUE)
    matter_add_gn_arg_bool("chip_enable_additional_data_advertising" TRUE)
endif()

if (CONFIG_WIFI_W91)
    matter_add_gn_arg_string("chip_mdns" "minimal")
elseif (CONFIG_NET_L2_OPENTHREAD)
    matter_add_gn_arg_string("chip_mdns" "platform")
else()
    matter_add_gn_arg_string("chip_mdns" "none")
endif()

if (CONFIG_CHIP_PW_RPC)
    set(PIGWEED_DIR "//third_party/pigweed/repo")
    matter_add_gn_arg_string("pw_assert_BACKEND" ${PIGWEED_DIR}/pw_assert_log:check_backend)
    matter_add_gn_arg_string("pw_log_BACKEND" ${PIGWEED_DIR}/pw_log_basic)
    matter_add_gn_arg("pw_build_LINK_DEPS" [\"${PIGWEED_DIR}/pw_assert:impl\",\ \"${PIGWEED_DIR}/pw_log:impl\"])
endif()

matter_generate_args_tmp_file()

# ==============================================================================
# Build chip library
# ==============================================================================

matter_build(chip
    LIB_SHELL   ${CONFIG_CHIP_LIB_SHELL}
    DEVICE_INFO_EXAMPLE_PROVIDER ${CONFIG_CHIP_EXAMPLE_DEVICE_INFO_PROVIDER}
    GN_DEPENDENCIES kernel
)
set_property(GLOBAL APPEND PROPERTY ZEPHYR_INTERFACE_LIBS chip)

include(${TELINK_COMMON}/common.cmake)

if (CONFIG_CHIP_APP_LOG_LEVEL GREATER_EQUAL 4)
    include(${TELINK_COMMON}/build_info.cmake)
endif()

# ==============================================================================
# Define 'process_binaries' target for collecting final binary to flash
# ==============================================================================

add_custom_target(
    process_binaries ALL
    COMMAND ${Python3_EXECUTABLE} ${CHIP_ROOT}/scripts/tools/telink/process_binaries.py
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)
add_dependencies(process_binaries ${ZEPHYR_FINAL_EXECUTABLE})

# ==============================================================================
# Define 'build_mcuboot' target for building the MCUBoot bootloader
# ==============================================================================

if (CONFIG_BOOTLOADER_MCUBOOT AND (CONFIG_SOC_RISCV_TELINK_B9X OR CONFIG_SOC_RISCV_TELINK_TLX))
    add_custom_target(build_mcuboot ALL
        COMMAND
        west build -b ${BASE_BOARD} -d build_mcuboot ${ZEPHYR_BASE}/../bootloader/mcuboot/boot/zephyr
            -- -DOVERLAY_CONFIG=${GLOBAL_BOOT_CONF_OVERLAY_FILE} -DDTC_OVERLAY_FILE="${GLOBAL_BOOT_DTC_OVERLAY_FILE};${FLASH_DTC_OVERLAY_FILE};${USB_BOOT_DTC_OVERLAY_FILE};${MARS_BOOT_DTC_OVERLAY_FILE}"
        COMMAND
        cp ${PROJECT_BINARY_DIR}/../modules/chip-module/build_mcuboot/zephyr/zephyr.bin ${PROJECT_BINARY_DIR}/mcuboot.bin
    )
endif()

# ==============================================================================
# Define 'chip-ota-image' target for building CHIP OTA image
# ==============================================================================

if (CONFIG_CHIP_OTA_IMAGE_BUILD)
    if (CONFIG_COMPRESS_LZMA)
        add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/${CONFIG_SIGNED_OTA_IMAGE_FILE_NAME}
            COMMAND ${CMAKE_COMMAND} -E true
            COMMENT "Fake command, ${CONFIG_SIGNED_OTA_IMAGE_FILE_NAME} file created in process_binaries target"
            DEPENDS ${ZEPHYR_FINAL_EXECUTABLE}
        )
    endif()

    chip_ota_image(chip-ota-image
        INPUT_FILES ${PROJECT_BINARY_DIR}/${CONFIG_SIGNED_OTA_IMAGE_FILE_NAME}
        OUTPUT_FILE ${PROJECT_BINARY_DIR}/${CONFIG_CHIP_OTA_IMAGE_FILE_NAME}
    )
    add_dependencies(chip-ota-image process_binaries)
endif()

# ==============================================================================
# Define 'factory_data' target for generating a factory data partition
# ==============================================================================

if (CONFIG_CHIP_FACTORY_DATA_BUILD)
    telink_generate_factory_data()
endif()

endif() # CONFIG_CHIP
