#
#   Copyright (c) 2022-2023 Project CHIP Authors
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

if (CONFIG_POSIX_API)
    matter_add_flags(-D_DEFAULT_SOURCE)
    matter_add_flags(-isystem${ZEPHYR_BASE}/include/zephyr/posix)
endif()

matter_add_flags(-DNL_ASSERT_USE_FLAGS_DEFAULT=1)

zephyr_include_directories(${CHIP_ROOT}/src/platform/telink/)

zephyr_get_compile_flags(ZEPHYR_CFLAGS_C C)
matter_add_cflags("${ZEPHYR_CFLAGS_C}")
zephyr_get_compile_flags(ZEPHYR_CFLAGS_CC CXX)
matter_add_cxxflags("${ZEPHYR_CFLAGS_CC}")
zephyr_get_gnu_cpp_standard(ZEPHYR_GNU_CPP_STD)
matter_add_cxxflags(${ZEPHYR_GNU_CPP_STD})

matter_add_flags(-DMBEDTLS_USER_CONFIG_FILE=<telink-mbedtls-config.h>)

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
matter_add_gn_arg_bool  ("chip_inet_config_enable_ipv4"           CONFIG_NET_IPV4)
matter_add_gn_arg_bool  ("chip_enable_nfc"                        CONFIG_CHIP_NFC_COMMISSIONING)
matter_add_gn_arg_bool  ("chip_enable_ota_requestor"              CONFIG_CHIP_OTA_REQUESTOR)
matter_add_gn_arg_bool  ("chip_inet_config_enable_tcp_endpoint"   CONFIG_CHIP_BUILD_TESTS)
matter_add_gn_arg_bool  ("chip_error_logging"                     CONFIG_MATTER_LOG_LEVEL GREATER_EQUAL 1)
matter_add_gn_arg_bool  ("chip_progress_logging"                  CONFIG_MATTER_LOG_LEVEL GREATER_EQUAL 3)
matter_add_gn_arg_bool  ("chip_detail_logging"                    CONFIG_MATTER_LOG_LEVEL GREATER_EQUAL 4)
matter_add_gn_arg_bool  ("chip_automation_logging"                FALSE)
matter_add_gn_arg_bool  ("chip_enable_icd_server"                 CONFIG_CHIP_ENABLE_ICD_SUPPORT)

if (CONFIG_CHIP_ENABLE_ICD_SUPPORT)
    matter_add_gn_arg_bool  ("chip_enable_icd_lit"                      CONFIG_CHIP_ICD_LIT_SUPPORT)
    matter_add_gn_arg_bool  ("chip_enable_icd_checkin"                  CONFIG_CHIP_ICD_CHECK_IN_SUPPORT)
    matter_add_gn_arg_bool  ("chip_enable_icd_user_active_mode_trigger" CONFIG_CHIP_ICD_UAT_SUPPORT)
endif()

if (CONFIG_CHIP_FACTORY_DATA)
    matter_add_gn_arg_bool  ("chip_use_transitional_commissionable_data_provider"  "false")
    matter_add_gn_arg_bool  ("chip_enable_factory_data"                            "true")
elseif (CONFIG_CHIP_FACTORY_DATA_CUSTOM_BACKEND)
    matter_add_gn_arg_bool  ("chip_use_transitional_commissionable_data_provider"  "false")
endif()

if (CONFIG_CHIP_ROTATING_DEVICE_ID)
    matter_add_gn_arg_bool("chip_enable_rotating_device_id"          "true")
    matter_add_gn_arg_bool("chip_enable_additional_data_advertising" "true")
endif()

if (CONFIG_CHIP_ENABLE_DNSSD_SRP)
    matter_add_gn_arg_string("chip_mdns" "platform")
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

# ==============================================================================
# Define 'chip-ota-image' target for building CHIP OTA image
# ==============================================================================

string(REPLACE "_retention" "" BASE_BOARD ${BOARD})

if(${TLNK_USB_DONGLE} MATCHES y)
  if(EXISTS "${CHIP_ROOT}/src/platform/telink/${BASE_BOARD}_usb_boot.overlay")
    set(USB_BOOT_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BASE_BOARD}_usb_boot.overlay")
  else()
    unset(USB_BOOT_DTC_OVERLAY_FILE)
  endif()
else()
  unset(USB_BOOT_DTC_OVERLAY_FILE)
endif()

if(${TLNK_MARS_BOARD} MATCHES y)
  if(EXISTS "${CHIP_ROOT}/src/platform/telink/${BASE_BOARD}_mars_boot.overlay")
    set(MARS_BOOT_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BASE_BOARD}_mars_boot.overlay")
  else()
    unset(MARS_BOOT_DTC_OVERLAY_FILE)
  endif()
else()
  unset(MARS_CONF_OVERLAY_FILE)
endif()

if(EXISTS "${CHIP_ROOT}/src/platform/telink/${BASE_BOARD}.overlay")
    set(GLOBAL_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BASE_BOARD}.overlay")
else()
    unset(GLOBAL_DTC_OVERLAY_FILE)
endif()

if(EXISTS "${CHIP_ROOT}/src/platform/telink/${FLASH_SIZE}_flash.overlay")
    set(FLASH_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${FLASH_SIZE}_flash.overlay")
    message(STATUS "Flash memory size is set to: " ${FLASH_SIZE} "b")
else()
    set(FLASH_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/2m_flash.overlay")
    message(STATUS "Flash memory size is set to: 2mb")
endif()

if(EXISTS "${CHIP_ROOT}/config/telink/app/bootloader.conf")
    set(GLOBAL_BOOTLOADER_CONF_OVERLAY_FILE "${CHIP_ROOT}/config/telink/app/bootloader.conf")
else()
    unset(GLOBAL_BOOTLOADER_CONF_OVERLAY_FILE)
endif()

set(BLOCK_SIZE "1024")

# get code-partition mcuboot_partition size
dt_nodelabel(dts_partition_path NODELABEL "boot_partition")
dt_reg_size(mcuboot_size PATH ${dts_partition_path})
math(EXPR boot_blocks "${mcuboot_size} / ${BLOCK_SIZE}" OUTPUT_FORMAT DECIMAL)

if (CONFIG_BOOTLOADER_MCUBOOT)
    add_custom_target(build_mcuboot ALL
        COMMAND
        west build -b ${BASE_BOARD} -d build_mcuboot ${ZEPHYR_BASE}/../bootloader/mcuboot/boot/zephyr
            -- -DOVERLAY_CONFIG=${GLOBAL_BOOTLOADER_CONF_OVERLAY_FILE} -DDTC_OVERLAY_FILE="${GLOBAL_DTC_OVERLAY_FILE};${FLASH_DTC_OVERLAY_FILE};${USB_BOOT_DTC_OVERLAY_FILE};${MARS_BOOT_DTC_OVERLAY_FILE}"
        COMMAND
        cp ${PROJECT_BINARY_DIR}/../modules/chip-module/build_mcuboot/zephyr/zephyr.bin ${PROJECT_BINARY_DIR}/zephyr.mcuboot.bin
    )

    add_custom_target(merge_mcuboot ALL
        COMMAND
        dd if=${PROJECT_BINARY_DIR}/zephyr.mcuboot.bin of=${PROJECT_BINARY_DIR}/zephyr.bin
        COMMAND
        dd if=${PROJECT_BINARY_DIR}/zephyr.signed.bin of=${PROJECT_BINARY_DIR}/zephyr.bin bs=${BLOCK_SIZE} seek=${boot_blocks}
    )

    add_dependencies(merge_mcuboot ${ZEPHYR_FINAL_EXECUTABLE})

if (CONFIG_CHIP_OTA_IMAGE_BUILD)
    chip_ota_image(chip-ota-image
        INPUT_FILES ${PROJECT_BINARY_DIR}/zephyr.signed.bin
        OUTPUT_FILE ${PROJECT_BINARY_DIR}/zephyr-ota.bin
    )

    add_dependencies(chip-ota-image ${ZEPHYR_FINAL_EXECUTABLE})
endif()
endif()

# get code-partition factory_partition address
dt_nodelabel(dts_partition_path NODELABEL "factory_partition")
dt_reg_addr(factory_size PATH ${dts_partition_path})
math(EXPR factory_blocks "${factory_size} / ${BLOCK_SIZE}" OUTPUT_FORMAT DECIMAL)

if (CONFIG_CHIP_FACTORY_DATA_MERGE_WITH_FIRMWARE)
  add_custom_target(merge_factory_data ALL
    COMMAND
    dd if=${PROJECT_BINARY_DIR}/factory/factory_data.bin of=${PROJECT_BINARY_DIR}/zephyr.bin bs=${BLOCK_SIZE} seek=${factory_blocks}
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
