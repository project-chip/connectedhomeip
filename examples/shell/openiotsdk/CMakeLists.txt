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

# # Fetch toolchain files before the first call to project
include(FetchContent)
FetchContent_Declare(iotsdk-toolchains
    GIT_REPOSITORY  https://git.gitlab.arm.com/iot/open-iot-sdk/toolchain.git
    GIT_TAG         v2022.05
    SOURCE_DIR      ${CMAKE_BINARY_DIR}/toolchains
)
FetchContent_MakeAvailable(iotsdk-toolchains)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../.. REALPATH)
get_filename_component(SHELL_COMMON ${CHIP_ROOT}/examples/shell/shell_common REALPATH)
get_filename_component(OPEN_IOT_SDK_SOURCE ${CHIP_ROOT}/third_party/open-iot-sdk/sdk REALPATH)
get_filename_component(OPEN_IOT_SDK_STORAGE_SOURCE ${CHIP_ROOT}/third_party/open-iot-sdk/storage REALPATH)
get_filename_component(OPEN_IOT_SDK_TOOLCHAIN ${CHIP_ROOT}/third_party/open-iot-sdk/toolchain REALPATH)
get_filename_component(OPEN_IOT_SDK_CONFIG ${CHIP_ROOT}/config/openiotsdk REALPATH)

set(APP_TARGET chip-openiotsdk-shell-example)
set(CMAKE_EXECUTABLE_SUFFIX_CXX .elf)

project(${APP_TARGET} LANGUAGES C CXX ASM)
add_executable(${APP_TARGET})

# Open IoT SDK configuration
set(IOTSDK_MDH_ARM ON)
set(MDH_PLATFORM "ARM_AN552_MPS3")
set(IOTSDK_CMSIS_RTOS_API ON)
set(IOTSDK_FREERTOS ON)
set(IOTSDK_MBEDTLS ON)
set(IOTSDK_LWIP ON)
set(FETCHCONTENT_QUIET OFF)
set(IOTSDK_EXAMPLES OFF)
set(VARIANT "FVP")

# Add Open IoT SDK source
add_subdirectory(${OPEN_IOT_SDK_SOURCE} ./sdk_build)
add_subdirectory(${OPEN_IOT_SDK_SOURCE}/utils ./sdk_utils_build)

# CMSIS-RTOS configuration

# CMSIS 5 require projects to provide configuration macros via RTE_Components.h
# and CMSIS_device_header. The macro CMSIS_device_header is not automatically set
# based on CMAKE_SYSTEM_PROCESSOR in the place where cmsis-core is first defined,
# because a project may want to provide its own device header.
target_include_directories(cmsis-rtos-api
        PUBLIC
            cmsis-config
)
target_compile_definitions(cmsis-core
    INTERFACE
        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},cortex-m55>:CMSIS_device_header="ARMCM55.h">
)

# LwIP configuration

# lwipcore requires the config defined by lwip-cmsis-port
target_link_libraries(lwipcore
    PUBLIC
        lwip-cmsis-port
)

# lwip requires user_lwipopts.h, we use the template provided by the lwip-cmsis-port
get_target_property(lwip-cmsis-port_SOURCE_DIR lwip-cmsis-port SOURCE_DIR)
target_include_directories(lwipopts
    INTERFACE
        lwip-config
)

# provide method to use for tracing by the lwip port (optional)
target_compile_definitions(lwipopts
    INTERFACE
        DEBUG_PRINT=printf
)

target_compile_definitions(mdh-arm-an552-mps3
    INTERFACE
        LAN91C111_RFS_MULTICAST_SUPPORT
)

# Link the emac factory to LwIP port
target_link_libraries(lwip-cmsis-port PUBLIC iotsdk-emac-factory)

# Mbedtls config

target_include_directories(mbedtls-config
    INTERFACE
        mbedtls-config
)

target_compile_definitions(mbedtls-config
    INTERFACE
        MBEDTLS_CONFIG_FILE="mbedtls_config.h"
)

# Add Open IoT SDK storage source
add_subdirectory(${OPEN_IOT_SDK_STORAGE_SOURCE} ./sdk_storage_build)

# CHIP configuration
set(CONFIG_CHIP_PROJECT_CONFIG main/include/CHIPProjectConfig.h)
set(CONFIG_CHIP_BUILD_TESTS NO)
set(CONFIG_CHIP_LIB_SHELL NO)

add_subdirectory(${OPEN_IOT_SDK_CONFIG} ./chip_build)

target_include_directories(${APP_TARGET} PRIVATE
                           main/include
                           ${MBED_COMMON}/util/include
                           ${SHELL_COMMON}/include
                           freertos-config
)

target_sources(${APP_TARGET} PRIVATE
                main/main.cpp
)

target_link_libraries(${APP_TARGET}
    mcu-driver-hal
    mcu-driver-bootstrap
    mdh-reference-platforms-for-arm

    iotsdk-serial-retarget
    freertos-cmsis-rtos
    freertos-kernel-heap-3

    iotsdk-ip-network-api
    lwipcore
    mbedtls
    
    chip

    $<$<TARGET_EXISTS:mdh-arm-an552-mps3>:mdh-arm-corstone-300-startup>
    $<$<TARGET_EXISTS:mdh-arm-an552-mps3>:mdh-arm-an552-mps3-linker>
)

target_link_options(${APP_TARGET}
    PRIVATE
        "-Wl,-Map=${APP_TARGET}.map"
)
