#
# Copyright (c) 2024 Project CHIP Authors
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

if (NOT GN_ROOT_TARGET)
    get_filename_component(GN_ROOT_TARGET ${CHIP_ROOT}/config/nxp/chip-gn-freertos REALPATH)
endif()

# Include common NXP helpers for build & pre/post-build process
include(${CHIP_ROOT}/third_party/nxp/nxp_matter_support/cmake/build_helpers.cmake)

include(${CHIP_ROOT}/config/nxp/cmake/common.cmake)
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

if(NOT CONFIG_CHIP_BUILD_APP_WITH_GN)
# Prepare compiler flags
matter_add_cflags(${CMAKE_C_FLAGS})
matter_add_cxxflags(${CMAKE_CXX_FLAGS})

matter_get_compiler_flags_from_targets("${CONFIG_CHIP_EXTERNAL_TARGETS}")
endif()


# ==============================================================================
# Generate configuration for CHIP GN build system
# ==============================================================================
matter_add_gn_arg_string("nxp_nvm_component" ${CONFIG_CHIP_NVM_COMPONENT})
matter_add_gn_arg_string("nxp_platform" ${CONFIG_CHIP_NXP_PLATFORM})
matter_add_gn_arg("chip_with_factory_data" ${CONFIG_CHIP_FACTORY_DATA})
matter_add_gn_arg("chip_enable_secure_dac_private_key_storage" ${CONFIG_CHIP_SECURE_DAC_PRIVATE_KEY_STORAGE})
matter_add_gn_arg("chip_enable_secure_whole_factory_data" ${CONFIG_CHIP_ENABLE_SECURE_WHOLE_FACTORY_DATA})
matter_add_gn_arg_bool("chip_enable_matter_cli" CONFIG_CHIP_LIB_SHELL)
matter_add_gn_arg_bool("chip_enable_pairing_autostart" CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART)

if (CONFIG_USE_CHIP_DATA_MODEL_INTERFACE)
    set(DATA_MODEL_INTERFACE "enabled")
else()
    set(DATA_MODEL_INTERFACE "disabled")
endif()

if(CONFIG_BOOTLOADER_MCUBOOT)
    matter_add_gn_arg_bool("no_mcuboot" false)
endif(CONFIG_BOOTLOADER_MCUBOOT)

# Enable map file generation by default
matter_add_gn_arg_bool("chip_generate_link_map_file" true)

if(CONFIG_CHIP_BUILD_APP_WITH_GN)
    include(${CHIP_ROOT}/third_party/nxp/nxp_matter_support/cmake/nxp_sdk_gn_build.cmake)
else()
    matter_common_gn_args(
        DEBUG CONFIG_DEBUG
        LIB_SHELL CONFIG_CHIP_LIB_SHELL
        LIB_TESTS CONFIG_CHIP_BUILD_TESTS
        PROJECT_CONFIG ${CONFIG_CHIP_PROJECT_CONFIG}
        PROJECT_CONFIG_INC_DIR ${CONFIG_CHIP_PROJECT_CONFIG_INCLUDE_DIRS}
        DEVICE_INFO_EXAMPLE_PROVIDER CONFIG_CHIP_EXAMPLE_DEVICE_INFO_PROVIDER
        DATA_MODEL_INTERFACE ${DATA_MODEL_INTERFACE}
    )
endif()

matter_generate_args_tmp_file()

# ==============================================================================
# Build chip library
# ==============================================================================
if(NOT CONFIG_CHIP_BUILD_APP_WITH_GN)
matter_build(chip
    LIB_SHELL ${CONFIG_CHIP_LIB_SHELL}
    LIB_TESTS ${CONFIG_CHIP_BUILD_TESTS}
    DEVICE_INFO_EXAMPLE_PROVIDER ${CONFIG_CHIP_EXAMPLE_DEVICE_INFO_PROVIDER}
    GN_DEPENDENCIES ${CONFIG_GN_DEPENDENCIES}
)
endif()


#**************************************************************************************
# Pre-build instructions
#**************************************************************************************
nxp_pre_build_process()

#**************************************************************************************
# Post-build instructions
#**************************************************************************************
if(CONFIG_CHIP_OTA_REQUESTOR)

if (CONFIG_BOOTLOADER_MCUBOOT)
    # Build mcuboot application
    nxp_generate_mcuboot()

    # Sign application binaries with mcuboot imgtool
    nxp_sign_app_imgtool("${CONFIG_REMOVE_SECTIONS_FROM_BIN}")

endif()

    # Generate .ota file
    nxp_generate_ota_file()

endif(CONFIG_CHIP_OTA_REQUESTOR)

if (CONFIG_CHIP_FACTORY_DATA_BUILD)
    
    set(OUTPUT_FILE_PATH ${gn_build}/factory_data)
    
    # Generate factory data binaries
    nxp_generate_factory_data()

endif(CONFIG_CHIP_FACTORY_DATA_BUILD)
