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
get_filename_component(CHEF ${CMAKE_CURRENT_SOURCE_DIR}/../ REALPATH)

include(${CHEF}/project_include.cmake)

get_filename_component(GEN_DIR ${CHEF}/out/${SAMPLE_NAME}/zap-generated REALPATH)

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

project(chip-telink-chef-example)

include(${CHIP_ROOT}/config/telink/app/enable-gnu-std.cmake)
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

target_compile_options(app PRIVATE -fpermissive)

target_include_directories(app PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip/zzz_generated/app-common
                           ${GEN_DIR}
                           ${CHEF}
                           ${GEN_DIR}/../
                           ${CHIP_ROOT}/src
                           ${CHIP_ROOT}/examples/shell/shell_common/include
                           ${TELINK_COMMON}/common/include
                           ${TELINK_COMMON}/util/include
)

if (CONFIG_CHIP_LIB_SHELL)
  target_sources(app PRIVATE
    ${CHEF}/shell_common/globals.cpp
    ${CHEF}/shell_common/cmd_misc.cpp
    ${CHEF}/shell_common/cmd_otcli.cpp
  )

  target_include_directories(app PRIVATE
    ${CHEF}/shell_common/include
  )
endif()

add_definitions(
    "-DCHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=<lib/address_resolve/AddressResolve_DefaultImpl.h>"
)

target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CHEF}/common/stubs.cpp
)

message(STATUS ${CHEF}/devices/${SAMPLE_NAME}.zap)

chip_configure_data_model(app
    INCLUDE_SERVER
    ZAP_FILE ${CHEF}/devices/${SAMPLE_NAME}.zap
)
           

if(CONFIG_CHIP_OTA_REQUESTOR)
    target_sources(app PRIVATE ${TELINK_COMMON}/util/src/OTAUtil.cpp)
endif()
