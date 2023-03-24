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

# Additional Open IoT SDK build configuration 
set(TFM_SUPPORT NO CACHE BOOL "Add Trusted Firmware-M (TF-M) support to application")
set(TFM_NS_APP_VERSION "0.0.0" CACHE STRING "TF-M non-secure application version (in the x.x.x format)")

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
if(TFM_SUPPORT)
    list(APPEND IOTSDK_FETCH_LIST trusted-firmware-m)
    set(TFM_PLATFORM ${OPEN_IOT_SDK_EXAMPLE_COMMON}/tf-m/targets/an552)
    set(TFM_PSA_FIRMWARE_UPDATE ON)
    set(MCUBOOT_IMAGE_VERSION_NS ${TFM_NS_APP_VERSION})
    set(TFM_CMAKE_ARGS "-DCONFIG_TFM_ENABLE_FP=ON;-DTFM_PROFILE=profile_medium")
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(TFM_CMAKE_ARGS "${TFM_CMAKE_ARGS};-DMCUBOOT_LOG_LEVEL=INFO;-DTFM_SPM_LOG_LEVEL=TFM_SPM_LOG_LEVEL_INFO;-DTFM_PARTITION_LOG_LEVEL=TFM_PARTITION_LOG_LEVEL_INFO")
    else()
        set(TFM_CMAKE_ARGS "${TFM_CMAKE_ARGS};-DMCUBOOT_LOG_LEVEL=ERROR;-DTFM_SPM_LOG_LEVEL=TFM_SPM_LOG_LEVEL_ERROR;-DTFM_PARTITION_LOG_LEVEL=TFM_PARTITION_LOG_LEVEL_ERROR")
    endif()
    if(TFM_PROJECT_CONFIG_HEADER_FILE)
        set(TFM_CMAKE_ARGS "${TFM_CMAKE_ARGS};-DPROJECT_CONFIG_HEADER_FILE=${TFM_PROJECT_CONFIG_HEADER_FILE}")
    endif()
    set(LINKER_SCRIPT ${OPEN_IOT_SDK_CONFIG}/ld/cs300_gcc_tfm.ld)
endif()

# Add Open IoT SDK source
add_subdirectory(${OPEN_IOT_SDK_SOURCE} ./sdk_build)

# Add Open IoT SDK modules to path
list(APPEND CMAKE_MODULE_PATH ${open-iot-sdk_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${open-iot-sdk_SOURCE_DIR}/components/trusted-firmware-m)

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
    
    target_compile_definitions(cmsis-rtos-api
        PUBLIC
            DOMAIN_NS=$<IF:$<BOOL:${TFM_SUPPORT}>,1,0>
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

        target_compile_definitions(cmsis-rtos-api
            INTERFACE
                CONFIG_RUN_FREERTOS_SECURE_ONLY=$<IF:$<BOOL:${TFM_SUPPORT}>,0,1>
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

if(TARGET mcu-driver-hal)
    target_compile_definitions(mcu-driver-hal
        INTERFACE
            DOMAIN_NS=$<IF:$<BOOL:${TFM_SUPPORT}>,1,0>
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

if("trusted-firmware-m" IN_LIST IOTSDK_FETCH_LIST)
    list(APPEND EXTERNAL_TARGETS
        tfm-ns-interface
        tfm-ns-interface-cmsis-rtos
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

function(sdk_post_build target)
    string(REPLACE "_ns" "" APP_NAME ${APP_TARGET})
if(TFM_SUPPORT)
    include(ConvertElfToBin)
    include(SignTfmImage)
    target_elf_to_bin(${APP_TARGET})
    iotsdk_tf_m_sign_image(${APP_TARGET})
    iotsdk_tf_m_merge_images(${APP_TARGET} 0x10000000 0x38000000 0x28060000)
    ExternalProject_Get_Property(trusted-firmware-m-build BINARY_DIR)
    # Cleanup
    add_custom_command(
        TARGET
            ${APP_TARGET}
        POST_BUILD
        DEPENDS
            $<TARGET_FILE_DIR:${APP_TARGET}>/tfm_s_signed.bin
            $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_TARGET}.bin
            $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_TARGET}_signed.bin
            $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_TARGET}_merged.hex
            $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_TARGET}_merged.elf
        COMMAND
            # Copy the TF-M secure elf image
            ${CMAKE_COMMAND} -E copy
                ${BINARY_DIR}/install/outputs/tfm_s.elf
                $<TARGET_FILE_DIR:${APP_TARGET}>/
        COMMAND
            # Rename output file
            ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_TARGET}_merged.elf
                $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_NAME}.elf
        COMMAND rm
        ARGS -Rf
            $<TARGET_FILE_DIR:${APP_TARGET}>/tfm_s_signed.bin 
            $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_TARGET}.bin
            $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_TARGET}_signed.bin 
            $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_TARGET}_merged.hex
            $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_TARGET}_merged.elf
        VERBATIM
    )
else()
    add_custom_command(
        TARGET
            ${APP_TARGET}
        POST_BUILD
        DEPENDS
            $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_TARGET}.elf
            $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_TARGET}.map
        COMMAND
            # Rename output elf file
            ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_TARGET}.elf
                $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_NAME}.elf
        COMMAND
            # Rename output map file
            ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_TARGET}.map
                $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_NAME}.map
        COMMAND rm
        ARGS -Rf
            $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_TARGET}.elf
            $<TARGET_FILE_DIR:${APP_TARGET}>/${APP_TARGET}.map
        VERBATIM
    )
endif() #TFM_SUPPORT
endfunction()
