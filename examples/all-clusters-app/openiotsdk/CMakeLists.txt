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

cmake_minimum_required(VERSION 3.21)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
get_filename_component(OPEN_IOT_SDK_CONFIG ${CHIP_ROOT}/config/openiotsdk REALPATH)
get_filename_component(OPEN_IOT_SDK_EXAMPLE_COMMON ${CHIP_ROOT}/examples/platform/openiotsdk REALPATH)
get_filename_component(ALL_CLUSTERS_COMMON ${CHIP_ROOT}/examples/all-clusters-app/all-clusters-common REALPATH)
get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)

list(APPEND CMAKE_MODULE_PATH ${OPEN_IOT_SDK_CONFIG}/cmake)

set(APP_TARGET chip-openiotsdk-all-clusters-app-example_ns)

set(TFM_SUPPORT YES)
set(TFM_NS_APP_VERSION "0.0.1")

# Toolchain files need to exist before first call to project
include(toolchain)

project(${APP_TARGET} LANGUAGES C CXX ASM)

include(sdk)

# LwIP configuration
if(TARGET lwip-cmsis-port)
    # lwip requires user_lwipopts.h, we use the custom settings
    target_include_directories(lwipopts
        INTERFACE
            ${OPEN_IOT_SDK_CONFIG}/lwip
    )

    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        target_compile_definitions(lwipopts
            INTERFACE
                LWIP_DEBUG
        )
    endif()
endif()

add_executable(${APP_TARGET})

# Application CHIP build configuration
include(chip)

add_subdirectory(${OPEN_IOT_SDK_EXAMPLE_COMMON}/app ./app_build)
add_subdirectory(${OPEN_IOT_SDK_EXAMPLE_COMMON}/util ./util_build)

target_include_directories(${APP_TARGET}
    PRIVATE
        main/include
        ${ALL_CLUSTERS_COMMON}/include
        ${GEN_DIR}/app-common
        ${GEN_DIR}/all-clusters-app
)

target_sources(${APP_TARGET}
    PRIVATE
        main/main_ns.cpp
        main/AppTask.cpp
        ${GEN_DIR}/all-clusters-app/zap-generated/IMClusterCommandHandler.cpp
        ${ALL_CLUSTERS_COMMON}/src/bridged-actions-stub.cpp
        ${ALL_CLUSTERS_COMMON}/src/static-supported-modes-manager.cpp
)

target_link_libraries(${APP_TARGET}
    openiotsdk-app-cmdline
    openiotsdk-util
    mdh-reference-platforms-for-arm
)

include(${CHIP_ROOT}/src/app/chip_data_model.cmake)
chip_configure_data_model(${APP_TARGET}
    INCLUDE_SERVER
    ZAP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../all-clusters-common/all-clusters-app.zap
)

set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/cs300.ld)
include(linker)
set_target_link(${APP_TARGET})

tfm_post_build(${APP_TARGET})
