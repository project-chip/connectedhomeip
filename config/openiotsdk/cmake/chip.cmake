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
set(CONFIG_CHIP_LIB_TESTS NO CACHE BOOL "")
set(CONFIG_CHIP_LIB_SHELL NO CACHE BOOL "")

set(CONFIG_CHIP_DETAIL_LOGGING YES CACHE BOOL "Enable logging at detail level")
set(CONFIG_CHIP_PROGRESS_LOGGING YES CACHE BOOL "Enable logging at progress level")
set(CONFIG_CHIP_AUTOMATION_LOGGING YES CACHE BOOL "Enable logging at automation level")
set(CONFIG_CHIP_ERROR_LOGGING YES CACHE BOOL "Enable logging at error level")

set(CONFIG_CHIP_OPEN_IOT_SDK_USE_PSA_PS NO CACHE BOOL "Enable using PSA Protected Storage")

if(CONFIG_CHIP_OPEN_IOT_SDK_USE_PSA_PS AND NOT TFM_SUPPORT)
    message( FATAL_ERROR "You can not use PSA Protected Storage without TF-M support" )
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CONFIG_CHIP_DEBUG YES)
else()
    set(CONFIG_CHIP_DEBUG NO)
endif()

# Add CHIP sources
add_subdirectory(${OPEN_IOT_SDK_CONFIG} ./chip_build)

# Additional chip target configuration

# TF-M support requires the right order of generating targets
if(TFM_SUPPORT)
    add_dependencies(chip-gn tfm-ns-interface)
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
