#
#    Copyright (c) 2022 Project CHIP Authors
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
get_filename_component(NLIO_ROOT ${CHIP_ROOT}/third_party/nlio/repo/include REALPATH)
get_filename_component(TELINK_COMMON ${CHIP_ROOT}/examples/platform/telink REALPATH)
get_filename_component(GEN_DIR ${CHIP_ROOT}/zzz_generated/ REALPATH)

set(CONF_FILE ${CHIP_ROOT}/config/telink/app/zephyr.conf prj.conf)

# Load NCS/Zephyr build system
list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/telink/chip-module)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

project(chip-telink-ota-requestor-example)

include(${CHIP_ROOT}/config/telink/app/enable-gnu-std.cmake)
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

target_compile_options(app PRIVATE -fpermissive)

target_include_directories(app PRIVATE
                           include
                           ${GEN_DIR}/app-common
                           ${GEN_DIR}/ota-requestor-app
                           ${NLIO_ROOT}
                           ${TELINK_COMMON}/util/include
                           ${TELINK_COMMON}/app/include)

add_definitions(
    "-DCHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=<lib/address_resolve/AddressResolve_DefaultImpl.h>"
)

target_sources(app PRIVATE
               src/AppTask.cpp
               src/main.cpp
               src/ZclCallbacks.cpp
               ${GEN_DIR}/ota-requestor-app/zap-generated/callback-stub.cpp
               ${GEN_DIR}/ota-requestor-app/zap-generated/IMClusterCommandHandler.cpp
               ${TELINK_COMMON}/util/src/LEDWidget.cpp
               ${TELINK_COMMON}/util/src/ButtonManager.cpp
               ${TELINK_COMMON}/util/src/ThreadUtil.cpp)

chip_configure_data_model(app
    INCLUDE_SERVER
    ZAP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../ota-requestor-common/ota-requestor-app.zap
)

if(CONFIG_CHIP_OTA_REQUESTOR)
    target_sources(app PRIVATE ${TELINK_COMMON}/util/src/OTAUtil.cpp)
endif()
