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
#     CMake for CHIP library configuration
#

get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)

# Default CHIP build configuration
set(CONFIG_CHIP_PROJECT_CONFIG "main/include/CHIPProjectConfig.h" CACHE STRING "")
set(CONFIG_CHIP_PROJECT_CONFIG_INCLUDE_DIRS "" CACHE STRING "")
set(CONFIG_CHIP_LIB_TESTS NO CACHE BOOL "")
set(CONFIG_CHIP_LIB_SHELL NO CACHE BOOL "")

set(CONFIG_CHIP_DETAIL_LOGGING YES CACHE BOOL "Enable logging at detail level")
set(CONFIG_CHIP_PROGRESS_LOGGING YES CACHE BOOL "Enable logging at progress level")
set(CONFIG_CHIP_AUTOMATION_LOGGING YES CACHE BOOL "Enable logging at automation level")
set(CONFIG_CHIP_ERROR_LOGGING YES CACHE BOOL "Enable logging at error level")

set(CONFIG_CHIP_CRYPTO "mbedtls" CACHE STRING "Matter crypto backend. Mbedtls as default")
set(CONFIG_CHIP_OPEN_IOT_SDK_SOFTWARE_VERSION "0" CACHE STRING "Software version number")
set(CONFIG_CHIP_OPEN_IOT_SDK_SOFTWARE_VERSION_STRING ${TFM_NS_APP_VERSION} CACHE STRING "Software version in string format x.x.x")
set(CONFIG_CHIP_OPEN_IOT_SDK_OTA_ENABLE NO CACHE BOOL "Enable OTA support")
set(CONFIG_GN_DEPENDENCIES "")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CONFIG_CHIP_DEBUG YES)
else()
    set(CONFIG_CHIP_DEBUG NO)
endif()

# TF-M support requires the right order of generating targets
list(APPEND CONFIG_GN_DEPENDENCIES tfm-ns-interface)

# Add CHIP sources
add_subdirectory(${OPEN_IOT_SDK_CONFIG} ./chip_build)

# Additional chip target configuration

if ("${CONFIG_CHIP_CRYPTO}" STREQUAL "psa")
    target_compile_definitions(chip
        INTERFACE
            CONFIG_CHIP_CRYPTO_PSA)
endif()

function(chip_add_data_model target scope model_name)
    target_include_directories(${target}
        PUBLIC
            ${GEN_DIR}/app-common
            ${GEN_DIR}/${model_name}-app
    )

    target_compile_definitions(${target}
        PUBLIC
            USE_CHIP_DATA_MODEL
    )

    include(${CHIP_ROOT}/src/app/chip_data_model.cmake)
    chip_configure_data_model(${target}
        SCOPE ${scope}
        INCLUDE_SERVER
        ZAP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../${model_name}-common/${model_name}-app.zap
    )
endfunction()
