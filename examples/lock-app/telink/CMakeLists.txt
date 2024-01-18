#
#    Copyright (c) 2023 Project CHIP Authors
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
cmake_minimum_required(VERSION 3.13.1)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip REALPATH)
get_filename_component(TELINK_COMMON ${CHIP_ROOT}/examples/platform/telink REALPATH)
get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD}.overlay")
  set(LOCAL_DTC_OVERLAY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD}.overlay")
else()
  unset(LOCAL_DTC_OVERLAY_FILE)
endif()

if(EXISTS "${CHIP_ROOT}/src/platform/telink/${BOARD}.overlay")
  set(GLOBAL_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BOARD}.overlay")
else()
  unset(GLOBAL_DTC_OVERLAY_FILE)
endif()

if(EXISTS "${CHIP_ROOT}/src/platform/telink/${FLASH_SIZE}_flash.overlay")
  set(FLASH_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${FLASH_SIZE}_flash.overlay")
  message(STATUS "Flash memory size is set to: " ${FLASH_SIZE} "b")
else()
  set(FLASH_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/2m_flash.overlay")
  message(STATUS "Flash memory size is set to: 2mb")
endif()

if(DTC_OVERLAY_FILE)
  set(DTC_OVERLAY_FILE
    "${DTC_OVERLAY_FILE} ${GLOBAL_DTC_OVERLAY_FILE} ${FLASH_DTC_OVERLAY_FILE} ${LOCAL_DTC_OVERLAY_FILE}"
    CACHE STRING "" FORCE
  )
else()
  set(DTC_OVERLAY_FILE ${GLOBAL_DTC_OVERLAY_FILE} ${FLASH_DTC_OVERLAY_FILE} ${LOCAL_DTC_OVERLAY_FILE})
endif()

set(CONF_FILE prj.conf)

# Load NCS/Zephyr build system
list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/telink/chip-module)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

project(chip-telink-lock-example)

include(${CHIP_ROOT}/config/telink/app/enable-gnu-std.cmake)
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

target_compile_options(app PRIVATE -fpermissive)

target_include_directories(app PRIVATE
                           include
                           ${GEN_DIR}/app-common
                           ${GEN_DIR}/lock-app
                           ${TELINK_COMMON}/common/include
                           ${TELINK_COMMON}/util/include
                           ${TELINK_COMMON}/app/include)

target_sources(app PRIVATE
               src/AppTask.cpp
               src/ZclCallbacks.cpp
               src/LockManager.cpp
               src/LockSettingsStorage.cpp
               ${TELINK_COMMON}/common/src/mainCommon.cpp
               ${TELINK_COMMON}/common/src/AppTaskCommon.cpp
               ${TELINK_COMMON}/util/src/LEDWidget.cpp
               ${TELINK_COMMON}/util/src/ButtonManager.cpp
               ${TELINK_COMMON}/util/src/ThreadUtil.cpp
               ${TELINK_COMMON}/util/src/PWMDevice.cpp)

chip_configure_data_model(app
    INCLUDE_SERVER
    ZAP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../lock-common/lock-app.zap
)

if(CONFIG_BOOTLOADER_MCUBOOT)
  target_sources(app PRIVATE ${TELINK_COMMON}/util/src/OTAUtil.cpp)
endif()
