#
#
#    Copyright (c) 2020 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

#
#    Description:
#      This file is the CMake template for the CHIP SDK Device Layer code.
#      The Device Layer defines and implements platform abstractions to port a platform to CHIP.
#

add_library(DeviceLayer)

set(COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/system
)

set(COMMON_SOURCES
    PersistedStorage.cpp
    SystemEventSupport.cpp
    SystemTimerSupport.cpp
    GeneralUtils.cpp
    Globals.cpp
)

set(COMMON_LIBS)

if(${CHIP_PLATFORM} STREQUAL "nrf52840")
set(COMMON_INCLUDES ${COMMON_INCLUDES}
    # DeviceLayer BSP
    ${NRF5_SDK_ROOT}/components/ble/common
    ${NRF5_SDK_ROOT}/components/ble/nrf_ble_gatt
    ${NRF5_SDK_ROOT}/components/libraries/fds
    ${NRF5_SDK_ROOT}/components/libraries/mem_manager
)

set(COMMON_SOURCES ${COMMON_SOURCES}
    nRF5/BLEManagerImpl.cpp
    nRF5/ConfigurationManagerImpl.cpp
    nRF5/ConnectivityManagerImpl.cpp
    nRF5/PlatformManagerImpl.cpp
    nRF5/nRF5Config.cpp
    nRF5/nRF5Utils.cpp
    nRF5/Logging.cpp
    nRF5/ThreadStackManagerImpl.cpp
    FreeRTOS/SystemTimeSupport.cpp
    OpenThread/OpenThreadUtils.cpp
)

set(COMMON_LIBS ${COMMON_LIBS}
    BleLayer
)
endif()

if(${CHIP_PLATFORM} STREQUAL "linux")
add_subdirectory(tests)

set(COMMON_SOURCES
    ${COMMON_SOURCES}
    Linux/BLEManagerImpl.cpp
    Linux/CHIPLinuxStorage.cpp
    Linux/CHIPLinuxStorageIni.cpp
    Linux/ConfigurationManagerImpl.cpp
    Linux/ConnectivityManagerImpl.cpp
    Linux/Logging.cpp
    Linux/PlatformManagerImpl.cpp
    Linux/PosixConfig.cpp
    Linux/SystemTimeSupport.cpp
)
endif()

target_include_directories(DeviceLayer PRIVATE ${COMMON_INCLUDES})

target_sources(DeviceLayer PRIVATE ${COMMON_SOURCES})

target_link_libraries(DeviceLayer PRIVATE ${COMMON_LIBS} chip-config)
