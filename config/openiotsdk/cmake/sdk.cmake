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
#     CMake for Open IoT SDK configuration
#

include(FetchContent)

get_filename_component(OPEN_IOT_SDK_SOURCE ${CHIP_ROOT}/third_party/open-iot-sdk/sdk REALPATH)

# Open IoT SDK targets passed to CHIP build
list(APPEND CONFIG_CHIP_EXTERNAL_TARGETS)

# Additional Open IoT SDK build configuration
set(TFM_NS_APP_VERSION "0.0.0" CACHE STRING "TF-M non-secure application version (in the x.x.x format)")
set(CONFIG_CHIP_OPEN_IOT_SDK_LWIP_DEBUG NO CACHE BOOL "Enable LwIP debug logs")

# Default LwIP options directory (should contain user_lwipopts.h file)
if (NOT LWIP_PROJECT_OPTS_DIR)  
    set(LWIP_PROJECT_OPTS_DIR ${OPEN_IOT_SDK_CONFIG}/lwip)
endif()

# Overwrite versions of Open IoT SDK components

# Add a Matter specific version of Mbedtls
FetchContent_Declare(
    mbedtls
    GIT_REPOSITORY https://github.com/ARMmbed/mbedtls
    GIT_TAG        v3.2.1
    GIT_SHALLOW    ON
    GIT_PROGRESS   ON
)

# Apply a patch to TF-M to support GCC 12
FetchContent_Declare(
    trusted-firmware-m
    GIT_REPOSITORY  https://git.trustedfirmware.org/TF-M/trusted-firmware-m.git
    GIT_TAG         d0c0a67f1b412e89d09b0987091c12998c4e4660
    GIT_SHALLOW     OFF
    GIT_PROGRESS    ON
    # Note: This prevents FetchContent_MakeAvailable() from calling
    # add_subdirectory() on the fetched repository. TF-M needs a
    # standalone build because it relies on functions defined in its
    # own toolchain files and contains paths that reference the
    # top-level project instead of its own project.
    SOURCE_SUBDIR   NONE
    PATCH_COMMAND   git reset --hard --quiet && git clean --force -dx --quiet && git apply ${CMAKE_CURRENT_LIST_DIR}/tf-m.patch
)

# Open IoT SDK configuration
set(IOTSDK_FETCH_LIST
    mcu-driver-reference-platforms-for-arm
    cmsis-5
    cmsis-freertos
    mbedtls
    lwip
    cmsis-sockets-api
    trusted-firmware-m
)

set(MDH_PLATFORM ARM_AN552_MPS3)
set(VARIANT "FVP")
set(FETCHCONTENT_QUIET OFF)
set(TFM_PLATFORM ${OPEN_IOT_SDK_EXAMPLE_COMMON}/tf-m/targets/an552)
set(TFM_PSA_FIRMWARE_UPDATE ON)
set(MCUBOOT_IMAGE_VERSION_NS ${TFM_NS_APP_VERSION})
set(TFM_CMAKE_ARGS "-DCONFIG_TFM_ENABLE_FP=ON;-DTFM_PROFILE=profile_medium;-DTFM_EXCEPTION_INFO_DUMP=ON;-DCONFIG_TFM_HALT_ON_CORE_PANIC=ON;-DTFM_ISOLATION_LEVEL=1;-DTFM_MBEDCRYPTO_PLATFORM_EXTRA_CONFIG_PATH=${OPEN_IOT_SDK_CONFIG}/mbedtls/mbedtls_config_psa.h;-DMBEDCRYPTO_BUILD_TYPE=${CMAKE_BUILD_TYPE};-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(TFM_CMAKE_ARGS "${TFM_CMAKE_ARGS};-DMCUBOOT_LOG_LEVEL=INFO;-DTFM_SPM_LOG_LEVEL=TFM_SPM_LOG_LEVEL_DEBUG;-DTFM_PARTITION_LOG_LEVEL=TFM_PARTITION_LOG_LEVEL_INFO")
else()
    set(TFM_CMAKE_ARGS "${TFM_CMAKE_ARGS};-DMCUBOOT_LOG_LEVEL=ERROR;-DTFM_SPM_LOG_LEVEL=TFM_SPM_LOG_LEVEL_DEBUG;-DTFM_PARTITION_LOG_LEVEL=TFM_PARTITION_LOG_LEVEL_ERROR")
endif()
if(TFM_PROJECT_CONFIG_HEADER_FILE)
    set(TFM_CMAKE_ARGS "${TFM_CMAKE_ARGS};-DPROJECT_CONFIG_HEADER_FILE=${TFM_PROJECT_CONFIG_HEADER_FILE}")
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
            DOMAIN_NS=1
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
            $<$<BOOL:${CONFIG_CHIP_OPEN_IOT_SDK_LWIP_DEBUG}>:LWIP_DEBUG>
            $<$<BOOL:${CONFIG_CHIP_LIB_TESTS}>:CHIP_LIB_TESTS>
    )

    target_include_directories(lwipopts
        INTERFACE
            ${LWIP_PROJECT_OPTS_DIR}
    )

    # Link the emac factory to LwIP port
    target_link_libraries(lwip-cmsis-port PUBLIC iotsdk-emac-factory)
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
            DOMAIN_NS=1
    )

    # Fixing the optimization issue for mcu-driver-hal target in the release build.
    # The default -Os optimization causes performance issues for the application.
    # We need to replace it with -O2 which is suitable for performance.
    # This fix can be removed in the future when the issue will be fixed in SDK directly.
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        target_compile_options(mcu-driver-hal INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-O2>)
        target_compile_options(mcu-driver-hal INTERFACE $<$<COMPILE_LANGUAGE:C>:-O2>)
    endif()
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
    list(APPEND CONFIG_CHIP_EXTERNAL_TARGETS
        mcu-driver-bootstrap
        mcu-driver-hal
        mdh-arm-corstone-300-common
        target-interface
    )
endif()

if("cmsis-5" IN_LIST IOTSDK_FETCH_LIST)
    list(APPEND CONFIG_CHIP_EXTERNAL_TARGETS
        cmsis-core
        cmsis-rtos-api
        iotsdk-ip-network-api
    )
endif()

if("cmsis-freertos" IN_LIST IOTSDK_FETCH_LIST)
    list(APPEND CONFIG_CHIP_EXTERNAL_TARGETS
        freertos-cmsis-rtos
    )
endif()

if("lwip" IN_LIST IOTSDK_FETCH_LIST)
    list(APPEND CONFIG_CHIP_EXTERNAL_TARGETS
        lwipcore
        lwip-cmsis-port
        lwip-cmsis-sys
        lwip-cmsis-port-low-input-latency
        lwipopts
    )
endif()

if("cmsis-sockets-api" IN_LIST IOTSDK_FETCH_LIST)
    list(APPEND CONFIG_CHIP_EXTERNAL_TARGETS
        cmsis-sockets-api
        lwip-sockets
    )
endif()

if("trusted-firmware-m" IN_LIST IOTSDK_FETCH_LIST)
    list(APPEND CONFIG_CHIP_EXTERNAL_TARGETS
        tfm-ns-interface
        tfm-ns-interface-cmsis-rtos
    )
endif()

# Note: Mbed TLS must appear after TF-M otherwise psa from mbed TLS is used
if("mbedtls" IN_LIST IOTSDK_FETCH_LIST)
    list(APPEND CONFIG_CHIP_EXTERNAL_TARGETS
        mbedtls
        mbedtls-config
        mbedtls-threading-cmsis-rtos
    )
endif()

function(sdk_post_build target)
    string(REPLACE "_ns" "" APP_NAME ${target})
    include(ConvertElfToBin)
    include(SignTfmImage)
    ExternalProject_Get_Property(trusted-firmware-m-build BINARY_DIR)
    target_elf_to_bin(${target})
    add_custom_command(
        TARGET
            ${target}
        POST_BUILD
        DEPENDS
            $<TARGET_FILE_DIR:${target}>/${target}.bin
        COMMAND
            # Sign the non-secure (application) image for TF-M bootloader (BL2)"
            python3 ${BINARY_DIR}/install/image_signing/scripts/wrapper/wrapper.py
                --layout ${BINARY_DIR}/install/image_signing/layout_files/signing_layout_ns.o
                -v ${MCUBOOT_IMAGE_VERSION_NS}
                -k ${BINARY_DIR}/install/image_signing/keys/root-RSA-3072_1.pem
                --public-key-format full
                --align 1 --pad --pad-header -H 0x400 -s auto -d "(0, 0.0.0+0)"
                $<TARGET_FILE_DIR:${target}>/${target}.bin
                --overwrite-only
                --measured-boot-record
                $<TARGET_FILE_DIR:${target}>/${target}_signed.bin
        VERBATIM
    )
    iotsdk_tf_m_merge_images(${target} 0x10000000 0x38000000 0x28060000)
if(CONFIG_CHIP_OPEN_IOT_SDK_OTA_ENABLE)
    add_custom_command(
        TARGET
            ${target}
        POST_BUILD
        DEPENDS
            $<TARGET_FILE_DIR:${target}>/${target}.bin
        COMMAND
            # Sign the update image
            python3 ${BINARY_DIR}/install/image_signing/scripts/wrapper/wrapper.py
                --layout ${BINARY_DIR}/install/image_signing/layout_files/signing_layout_ns.o
                -v ${MCUBOOT_IMAGE_VERSION_NS}
                -k ${BINARY_DIR}/install/image_signing/keys/root-RSA-3072_1.pem
                --public-key-format full
                --align 1 --pad-header -H 0x400 -s auto -d "(0, 0.0.0+0)"
                $<TARGET_FILE_DIR:${target}>/${target}.bin
                --overwrite-only
                --measured-boot-record
                $<TARGET_FILE_DIR:${target}>/${target}_signed.ota
        COMMAND
            # Create OTA udpate file
            ${CHIP_ROOT}/src/app/ota_image_tool.py
                create
                -v 0xfff1 -p 0x8001
                -vn ${CONFIG_CHIP_OPEN_IOT_SDK_SOFTWARE_VERSION}
                -vs "${CONFIG_CHIP_OPEN_IOT_SDK_SOFTWARE_VERSION_STRING}"
                -da sha256
                $<TARGET_FILE_DIR:${target}>/${target}_signed.ota
                $<TARGET_FILE_DIR:${target}>/${APP_NAME}.ota
        # Cleanup
        COMMAND rm
        ARGS -Rf
                $<TARGET_FILE_DIR:${target}>/${target}_signed.ota
        VERBATIM
    )
endif()
    # Cleanup
    add_custom_command(
        TARGET
            ${target}
        POST_BUILD
        DEPENDS
            $<TARGET_FILE_DIR:${target}>/${target}.bin
            $<TARGET_FILE_DIR:${target}>/${target}_signed.bin
            $<TARGET_FILE_DIR:${target}>/${target}_merged.hex
            $<TARGET_FILE_DIR:${target}>/${target}_merged.elf
        COMMAND
            # Copy the bootloader and TF-M secure image for debugging purposes
            ${CMAKE_COMMAND} -E copy
                ${BINARY_DIR}/install/outputs/bl2.elf
                ${BINARY_DIR}/install/outputs/tfm_s.elf
                $<TARGET_FILE_DIR:${target}>/
        COMMAND
            # Rename output file
            ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE_DIR:${target}>/${target}_merged.elf
                $<TARGET_FILE_DIR:${target}>/${APP_NAME}.elf
        COMMAND rm
        ARGS -Rf
            $<TARGET_FILE_DIR:${target}>/${target}.bin
            $<TARGET_FILE_DIR:${target}>/${target}_signed.bin 
            $<TARGET_FILE_DIR:${target}>/${target}_merged.hex
            $<TARGET_FILE_DIR:${target}>/${target}_merged.elf
        VERBATIM
    )
endfunction()
