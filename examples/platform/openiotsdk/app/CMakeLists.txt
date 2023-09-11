#
# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.21)

# Declare Open IoT SDK startup target
add_library(openiotsdk-startup
    OBJECT
        openiotsdk_startup_gcc.cpp
)

target_link_libraries(openiotsdk-startup
    PUBLIC
        # iotsdk-serial-retarget contains the UART but we don't need the retarget part
        $<FILTER:$<TARGET_OBJECTS:iotsdk-serial-retarget>,EXCLUDE,.*gcc_retarget.*>
        cmsis-rtos-api
        mcu-driver-hal
        tfm-ns-interface
)

target_link_options(openiotsdk-startup
    PUBLIC
        "-Wl,--wrap,__malloc_lock"
        "-Wl,--wrap,__malloc_unlock"
)

# Declare Open IoT SDK app interface target
add_library(openiotsdk-app
    openiotsdk_platform.cpp
    ${CHIP_ROOT}/examples/providers/DeviceInfoProviderImpl.cpp
)

target_include_directories(openiotsdk-app
    PUBLIC
        .
        ${CHIP_ROOT}/examples/providers
)

target_link_libraries(openiotsdk-app
    PUBLIC
        chip
)

if(CONFIG_CHIP_OPEN_IOT_SDK_OTA_ENABLE)
    target_include_directories(openiotsdk-app
        PUBLIC
            dfu
    )

    target_sources(openiotsdk-app
        PUBLIC
            dfu/openiotsdk_dfu_manager.cpp
    )

    target_compile_definitions(openiotsdk-app
        PUBLIC
            CHIP_OPEN_IOT_SDK_OTA_ENABLE
    )
endif()
