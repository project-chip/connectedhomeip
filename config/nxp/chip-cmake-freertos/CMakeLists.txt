#
# Copyright (c) 2024-2025 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

include(ExternalProject)

if(NOT CHIP_ROOT)
    get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
endif()

if(NOT GN_ROOT_TARGET)
    get_filename_component(GN_ROOT_TARGET ${CHIP_ROOT}/config/nxp/chip-gn-freertos REALPATH)
endif()

if(NOT OT_NXP_ROOT)
    get_filename_component(OT_NXP_ROOT ${CHIP_ROOT}/third_party/openthread/ot-nxp REALPATH)
endif()

# Include common NXP helpers for build & pre/post-build process
include(${CHIP_ROOT}/third_party/nxp/nxp_matter_support/cmake/build_helpers.cmake)

# Include SDK hooks required to build Matter libs
include(${CHIP_ROOT}/third_party/nxp/nxp_matter_support/cmake/sdk_hook.cmake)

include(${CHIP_ROOT}/config/nxp/cmake/common.cmake)
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

# Prepare compiler flags
matter_add_cflags(${CMAKE_C_FLAGS})
matter_add_cxxflags(${CMAKE_CXX_FLAGS})

matter_get_compiler_flags_from_targets("${CONFIG_CHIP_EXTERNAL_TARGETS}")

# ==============================================================================
# Generate configuration for CHIP GN build system
# ==============================================================================
matter_add_gn_arg_string("nxp_platform" ${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME})
matter_add_gn_arg_string("nxp_device_layer" ${CONFIG_CHIP_NXP_DEVICE_LAYER})
matter_add_gn_arg_bool("nxp_use_factory_data" ${CONFIG_CHIP_FACTORY_DATA})
matter_add_gn_arg_bool("nxp_enable_secure_whole_factory_data" ${CONFIG_CHIP_ENABLE_SECURE_WHOLE_FACTORY_DATA})
matter_add_gn_arg_bool("nxp_enable_secure_EL2GO_factory_data" ${CONFIG_CHIP_ENABLE_EL2GO_FACTORY_DATA})
matter_add_gn_arg_bool("nxp_enable_matter_cli" CONFIG_CHIP_LIB_SHELL)
matter_add_gn_arg_bool("nxp_enable_pairing_autostart" CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART)
matter_add_gn_arg_bool("nxp_use_low_power" CONFIG_NXP_USE_LOW_POWER)
matter_add_gn_arg_bool("nxp_use_plain_dac_key" CONFIG_NXP_USE_PLAIN_DAC_KEY)
matter_add_gn_arg_bool("nxp_use_smu2_dynamic" CONFIG_NXP_USE_SMU2_DYNAMIC)
matter_add_gn_arg_bool("nxp_use_smu2_static" CONFIG_NXP_USE_SMU2_STATIC)

# Remove default spec config from the build
matter_add_gn_arg_list("default_configs_specs" " ")

# Allows to set chip_stack_lock_tracking level
# Required for freeRTOS build in case default value needs to be set to "none" for optimization
matter_add_gn_arg_string("chip_stack_lock_tracking" "${CONFIG_CHIP_STACK_LOCK_TRACKING}")

# OT specific gn arguments
if(CONFIG_NET_L2_OPENTHREAD)
    matter_add_gn_arg_string("openthread_root" "${CHIP_ROOT}/${CONFIG_OT_ROOT}")
    matter_add_gn_arg_string("openthread_external_mbedtls" "${CHIP_ROOT}/third_party/nxp/nxp_matter_support/gn_build:${CONFIG_OT_EXTERNAL_MBEDTLS}")
    matter_add_gn_arg_string("openthread_project_core_config_file" "${CONFIG_OT_PROJECT_CORE_CONFIG_FILE}")
    matter_add_gn_arg_string("openthread_core_config_platform_check_file" ${CONFIG_OT_CORE_CONFIG_PLATFORM_CHECK_FILE})
    matter_add_gn_arg_string("openthread_external_platform" "${CHIP_ROOT}/third_party/openthread/platforms/nxp/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}:${CONFIG_OT_EXTERNAL_PLATFORM}")
    matter_add_gn_arg_list("openthread_core_config_deps" "\"${CHIP_ROOT}/${CONFIG_OT_CORE_CONFIG_DEPS}\"")
    matter_add_gn_arg("chip_device_config_thread_network_endpoint_id" ${CONFIG_CHIP_THREAD_NETWORK_ENDPOINT_ID})
    matter_add_gn_arg_string("nxp_ot_spinel_interface_link" ${CONFIG_OT_NXP_SPINEL_INTERFACE_LINK})
     matter_add_gn_arg_string("nxp_ot_lib_config_file" "${CONFIG_OT_PROJECT_LIB_CONFIG_FILE}")
endif()

if(CONFIG_CHIP_SECONDARY_NETWORK_IF)
    matter_add_gn_arg_bool("chip_enable_secondary_nwk_if" CONFIG_CHIP_SECONDARY_NETWORK_IF)
endif()

if(CONFIG_CHIP_NVM_COMPONENT_NVS)
    matter_add_gn_arg_string("nxp_nvm_component" "nvs")
elseif(CONFIG_CHIP_NVM_COMPONENT_LITTLEFS)
    matter_add_gn_arg_string("nxp_nvm_component" "littlefs")
elseif(CONFIG_CHIP_NVM_COMPONENT_KEYSTORAGE)
    matter_add_gn_arg_string("nxp_nvm_component" "key_storage")
elseif(CONFIG_CHIP_NVM_COMPONENT_NVMFWK)
    matter_add_gn_arg_string("nxp_nvm_component" "nvm_fwk")
endif()

if(CONFIG_CHIP_OTA_ENCRYPTION)
    matter_add_gn_arg_bool("chip_with_ota_encryption" CONFIG_CHIP_OTA_ENCRYPTION)
    matter_add_gn_arg_string("chip_with_ota_key" ${CONFIG_CHIP_OTA_ENCRYPTION_KEY})
endif()

# Enable map file generation by default
matter_add_gn_arg_bool("chip_generate_link_map_file" true)

matter_add_gn_arg_string("nxp_ar" ${CMAKE_AR})
matter_add_gn_arg_string("nxp_cc" ${CMAKE_C_COMPILER})
matter_add_gn_arg_string("nxp_cxx" ${CMAKE_CXX_COMPILER})

matter_common_gn_args(
    DEBUG CONFIG_DEBUG
    LIB_SHELL CONFIG_CHIP_LIB_SHELL
    LIB_TESTS CONFIG_CHIP_BUILD_TESTS
    PROJECT_CONFIG ${CONFIG_CHIP_PROJECT_CONFIG}
    PROJECT_CONFIG_INC_DIR ${CONFIG_CHIP_PROJECT_CONFIG_INCLUDE_DIRS}
    DEVICE_INFO_EXAMPLE_PROVIDER CONFIG_CHIP_EXAMPLE_DEVICE_INFO_PROVIDER
)

matter_generate_args_tmp_file()

# ==============================================================================
# Build chip library
# ==============================================================================

matter_build(chip
    LIB_SHELL ${CONFIG_CHIP_LIB_SHELL}
    LIB_TESTS ${CONFIG_CHIP_BUILD_TESTS}
    DEVICE_INFO_EXAMPLE_PROVIDER ${CONFIG_CHIP_EXAMPLE_DEVICE_INFO_PROVIDER}
    GN_DEPENDENCIES ${CONFIG_GN_DEPENDENCIES}
)

if(CONFIG_BT)
    if(CONFIG_CHIP_DEVICE_USE_ZEPHYR_BLE)
        target_compile_definitions(chip INTERFACE
            EXTERNAL_BLEMANAGERIMPL_HEADER=\"platform/nxp/common/ble_zephyr/BLEManagerImpl.h\"
        )
    else()
        target_compile_definitions(chip INTERFACE
            EXTERNAL_BLEMANAGERIMPL_HEADER=\"platform/nxp/${CONFIG_CHIP_NXP_PLATFORM_FOLDER_NAME}/BLEManagerImpl.h\"
        )
    endif()
endif()

# Add OT include directories to be visible from the application
if(CONFIG_NET_L2_OPENTHREAD)
    target_include_directories(chip INTERFACE
        ${CHIP_ROOT}/${CONFIG_OT_ROOT}/include
        ${CHIP_ROOT}/${CONFIG_OT_ROOT}/examples/platforms
    )
endif()

if(CONFIG_NET_L2_OPENTHREAD AND CONFIG_CHIP_WIFI)
    target_include_directories(chip INTERFACE
        ${OT_NXP_ROOT}/src/common/br
    )

    if(CONFIG_CHIP_LIB_SHELL)
        # Expose addons_cli.h for the application
        target_include_directories(chip INTERFACE
            ${OT_NXP_ROOT}/examples/utils/cli_addons
        )
    endif()
endif()

# **************************************************************************************
# Pre-build instructions
# **************************************************************************************
nxp_pre_build_process()

# **************************************************************************************
# Post-build instructions
# **************************************************************************************
if(CONFIG_CHIP_OTA_IMAGE_BUILD)
    if(CONFIG_BOOTLOADER_MCUBOOT)
        # Build mcuboot application
        nxp_generate_mcuboot()

        # Sign application binaries with mcuboot imgtool
        nxp_sign_app_imgtool("${CONFIG_REMOVE_SECTIONS_FROM_BIN}")
    endif()

    # Generate .ota file
    nxp_generate_ota_file()
endif()

if(CONFIG_CHIP_FACTORY_DATA_BUILD)
    set(OUTPUT_FILE_PATH ${APP_OUTPUT_DIR}/factory_data)

    # Generate factory data binaries
    nxp_generate_factory_data()
endif()