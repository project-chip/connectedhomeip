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
