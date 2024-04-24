#
#    Copyright (c) 2023-2024 Project CHIP Authors
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
get_filename_component(APP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/.. REALPATH)

include(${TELINK_COMMON}/common.cmake)
include(${CHIP_ROOT}/config/telink/app/enable-gnu-std.cmake)
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

project(chip-telink-shell-example)

target_include_directories(app PRIVATE
                           include
                           ${GEN_DIR}/app-common
                           ${APP_ROOT}/shell_common/include)

target_sources(app PRIVATE
               ${APP_ROOT}/shell_common/globals.cpp
               ${APP_ROOT}/shell_common/cmd_misc.cpp
               ${APP_ROOT}/shell_common/cmd_otcli.cpp
               ${APP_ROOT}/standalone/main.cpp)

if(CONFIG_BOOTLOADER_MCUBOOT)
  target_sources(app PRIVATE ${TELINK_COMMON}/util/src/OTAUtil.cpp)
endif()
