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

# Open IoT SDK targets passed to CHIP build
list(APPEND EXTERNAL_TARGETS)

# Overwrite versions of Open IoT SDK components

# Add a Matter specific version of Mbedtls
FetchContent_Declare(
    mbedtls
    GIT_REPOSITORY https://github.com/ARMmbed/mbedtls
    GIT_TAG        v3.2.1
    GIT_SHALLOW    ON
    GIT_PROGRESS   ON
)

# Open IoT SDK configuration
set(IOTSDK_FETCH_LIST
    mcu-driver-reference-platforms-for-arm
    cmsis-5
    cmsis-freertos
    mbedtls
    lwip
    cmsis-sockets-api
)

set(MDH_PLATFORM ARM_AN552_MPS3)
set(VARIANT "FVP")
set(FETCHCONTENT_QUIET OFF)

# Add Open IoT SDK source
add_subdirectory(${OPEN_IOT_SDK_SOURCE} ./sdk_build)

# Add Open IoT SDK modules to path
list(APPEND CMAKE_MODULE_PATH ${open-iot-sdk_SOURCE_DIR}/cmake)

# Configure component properties

# CMSIS 5 require projects to provide configuration macros via RTE_Components.h
# and CMSIS_device_header. The macro CMSIS_device_header is not automatically set
# based on CMAKE_SYSTEM_PROCESSOR in the place where cmsis-core is first defined,
# because a project may want to provide its own device header.
if(TARGET cmsis-core)
    target_compile_definitions(cmsis-core
        INTERFACE
            $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},cortex-m55>:CMSIS_device_header="ARMCM55.h">
    )
endif()

# Add RTOS configuration headers
# Link cmsis-rtos-api against a concrete implementation
if(TARGET cmsis-rtos-api)
    target_include_directories(cmsis-core 
        INTERFACE 
            cmsis-config
    )

    if(TARGET freertos-kernel)
        target_include_directories(freertos-kernel 
            PUBLIC 
                freertos-config
        )

        target_link_libraries(freertos-kernel 
            PUBLIC 
                cmsis-core
        )

        target_link_libraries(cmsis-rtos-api
            PUBLIC
                freertos-cmsis-rtos
                freertos-kernel-heap-3
        )
    elseif(TARGET cmsis-rtx)
        target_link_libraries(cmsis-rtos-api
            INTERFACE
                cmsis-rtx
        )
    endif()
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
            MBEDTLS_CONFIG_FILE="${OPEN_IOT_SDK_CONFIG}/mbedtls/mbedtls_config.h"
    )

    target_link_libraries(mbedtls-config
        INTERFACE
            mbedtls-threading-cmsis-rtos
    )
endif()

if("mcu-driver-reference-platforms-for-arm" IN_LIST IOTSDK_FETCH_LIST)
    list(APPEND EXTERNAL_TARGETS
        mcu-driver-bootstrap
        mcu-driver-hal
        mdh-arm-corstone-300-common
        target-interface
    )
endif()

if("cmsis-5" IN_LIST IOTSDK_FETCH_LIST)
    list(APPEND EXTERNAL_TARGETS
        cmsis-core
        cmsis-rtos-api
        iotsdk-ip-network-api
    )
endif()

if("cmsis-freertos" IN_LIST IOTSDK_FETCH_LIST)
    list(APPEND EXTERNAL_TARGETS
        freertos-cmsis-rtos
    )
endif()

if("mbedtls" IN_LIST IOTSDK_FETCH_LIST)
    list(APPEND EXTERNAL_TARGETS
        mbedtls
        mbedtls-config
        mbedtls-threading-cmsis-rtos 
    )
endif()

if("lwip" IN_LIST IOTSDK_FETCH_LIST)
    list(APPEND EXTERNAL_TARGETS
        lwipcore
        lwip-cmsis-port
        lwip-cmsis-sys
        lwip-cmsis-port-low-input-latency
        lwipopts
    )
endif()

if("cmsis-sockets-api" IN_LIST IOTSDK_FETCH_LIST)
    list(APPEND EXTERNAL_TARGETS
        cmsis-sockets-api
        lwip-sockets
    )
endif()

# Additional Open IoT SDK port components

# Add Open IoT SDK storage source
add_subdirectory(${OPEN_IOT_SDK_STORAGE_SOURCE} ./sdk_storage_build)
list(APPEND EXTERNAL_TARGETS
    iotsdk-blockdevice
    iotsdk-tdbstore
)

# Add custom storage library
add_subdirectory(${OPEN_IOT_SDK_CONFIG}/storage storage_build)
list(APPEND EXTERNAL_TARGETS
    openiotsdk-storage
)
