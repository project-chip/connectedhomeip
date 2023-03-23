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
#     CMake for Open IoT SDK configuration
#

include(FetchContent)

get_filename_component(OPEN_IOT_SDK_SOURCE ${CHIP_ROOT}/third_party/open-iot-sdk/sdk REALPATH)
get_filename_component(OPEN_IOT_SDK_STORAGE_SOURCE ${CHIP_ROOT}/third_party/open-iot-sdk/storage REALPATH)

# List of binary directories to Open IoT SDK sources
list(APPEND SDK_SOURCES_BINARY_DIRS)

# Open IoT SDK configuration
set(IOTSDK_MDH_ARM ON)
set(MDH_PLATFORM "ARM_AN552_MPS3")
set(MDH_ARM_BUILD_EXAMPLES OFF)
set(IOTSDK_CMSIS_RTOS_API ON)
set(IOTSDK_FREERTOS ON)
set(IOTSDK_MBEDTLS ON)
set(IOTSDK_LWIP ON)
set(FETCHCONTENT_QUIET OFF)
set(IOTSDK_EXAMPLES OFF)
set(BUILD_TESTING NO)
set(VARIANT "FVP")

# Add Open IoT SDK source
add_subdirectory(${OPEN_IOT_SDK_SOURCE} ./sdk_build)
list(APPEND SDK_SOURCES_BINARY_DIRS ${CMAKE_CURRENT_BINARY_DIR}/sdk_build)

# Add Open IoT SDK modules to path
list(APPEND CMAKE_MODULE_PATH ${open-iot-sdk_SOURCE_DIR}/cmake)

# CMSIS-RTOS configuration
# CMSIS 5 require projects to provide configuration macros via RTE_Components.h
# and CMSIS_device_header. The macro CMSIS_device_header is not automatically set
# based on CMAKE_SYSTEM_PROCESSOR in the place where cmsis-core is first defined,
# because a project may want to provide its own device header.
if(TARGET cmsis-rtos-api)
    target_include_directories(cmsis-rtos-api
            PUBLIC
                cmsis-config
    )
endif()

if(TARGET cmsis-core)
    target_compile_definitions(cmsis-core
        INTERFACE
            $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},cortex-m55>:CMSIS_device_header="ARMCM55.h">
    )
endif()

# LwIP configuration
if(TARGET lwip-cmsis-port)
    # lwipcore requires the config defined by lwip-cmsis-port
    target_link_libraries(lwipcore
        PUBLIC
            lwip-cmsis-port
    )

    # provide method to use for tracing by the lwip port (optional)
    target_compile_definitions(lwipopts
        INTERFACE
            DEBUG_PRINT=printf
    )

    if(TARGET lwip-cmsis-port)
        # Link the emac factory to LwIP port
        target_link_libraries(lwip-cmsis-port PUBLIC iotsdk-emac-factory)
    endif()
endif()

# MDH configuration
if(TARGET ethernet-lan91c111)
    target_compile_definitions(ethernet-lan91c111
        INTERFACE
            LAN91C111_RFS_MULTICAST_SUPPORT
    )
endif()

# Mbedtls config
if(TARGET mbedtls-config)
    target_include_directories(mbedtls-config
        INTERFACE
            ${OPEN_IOT_SDK_CONFIG}/mbedtls
    )

    target_sources(mbedtls-config 
        INTERFACE
            ${OPEN_IOT_SDK_CONFIG}/mbedtls/platform_alt.cpp
    )

    target_compile_definitions(mbedtls-config
        INTERFACE
            MBEDTLS_CONFIG_FILE="mbedtls_config.h"
    )

    target_link_libraries(mbedtls-config
        INTERFACE
            mbedtls-threading-cmsis-rtos
    )
endif()

# Declare RTOS interface target
add_library(cmsis-rtos-implementation INTERFACE)

if(TARGET freertos-kernel)
    target_link_libraries(cmsis-rtos-implementation
        INTERFACE
            freertos-cmsis-rtos
            freertos-kernel-heap-3
    )
    target_include_directories(cmsis-rtos-implementation 
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/freertos-config
    )
elseif(TARGET cmsis-rtx)
    target_link_libraries(cmsis-rtos-implementation
        INTERFACE
            cmsis-rtx
            cmsis-rtos-api
            cmsis-rtx-freertos-alloc-wrapper
    )
endif()

# Add Open IoT SDK storage source
add_subdirectory(${OPEN_IOT_SDK_STORAGE_SOURCE} ./sdk_storage_build)
list(APPEND SDK_SOURCES_BINARY_DIRS ${CMAKE_CURRENT_BINARY_DIR}/sdk_storage_build)
