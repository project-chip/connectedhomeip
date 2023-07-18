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

set(BOARD tlsr9518adk80d)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip REALPATH)
get_filename_component(TELINK_COMMON ${CHIP_ROOT}/examples/platform/telink REALPATH)
get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)
get_filename_component(APP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/.. REALPATH)

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

if(DTC_OVERLAY_FILE)
  set(DTC_OVERLAY_FILE
    "${DTC_OVERLAY_FILE} ${GLOBAL_DTC_OVERLAY_FILE} ${LOCAL_DTC_OVERLAY_FILE}"
    CACHE STRING "" FORCE
  )
else()
  set(DTC_OVERLAY_FILE ${GLOBAL_DTC_OVERLAY_FILE} ${LOCAL_DTC_OVERLAY_FILE})
endif()

set(CONF_FILE ${CHIP_ROOT}/config/telink/app/zephyr.conf prj.conf)

# Load NCS/Zephyr build system
list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/telink/chip-module)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

project(chip-telink-shell-example)

include(${CHIP_ROOT}/config/telink/app/enable-gnu-std.cmake)
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

target_compile_options(app PRIVATE -fpermissive)
#target_compile_options(app PRIVATE -Werror -Wno-error=maybe-uninitialized)

target_include_directories(app PRIVATE
                           include
                           ${GEN_DIR}/app-common
                           ${APP_ROOT}/shell_common/include
                           #${TELINK_COMMON}/common/include
                           #${TELINK_COMMON}/util/include
                           )

add_definitions(
    "-DCHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=<lib/address_resolve/AddressResolve_DefaultImpl.h>"
)

target_sources(app PRIVATE
               #src/AppTask.cpp
               #${TELINK_COMMON}/common/src/mainCommon.cpp
               #${TELINK_COMMON}/common/src/AppTaskCommon.cpp
               #${TELINK_COMMON}/util/src/LEDWidget.cpp
               #${TELINK_COMMON}/util/src/ButtonManager.cpp
               #${TELINK_COMMON}/util/src/ThreadUtil.cpp
               #${TELINK_COMMON}/util/src/PWMDevice.cpp
               ${APP_ROOT}/shell_common/globals.cpp
               ${APP_ROOT}/shell_common/cmd_misc.cpp
               ${APP_ROOT}/shell_common/cmd_otcli.cpp
               ${APP_ROOT}/standalone/main.cpp
               )

# chip_configure_data_model(app
#     INCLUDE_SERVER
#     ZAP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../smoke-co-alarm-common/smoke-co-alarm-app.zap
# )

if(CONFIG_CHIP_OTA_REQUESTOR)
    target_sources(app PRIVATE ${TELINK_COMMON}/util/src/OTAUtil.cpp)
endif()
