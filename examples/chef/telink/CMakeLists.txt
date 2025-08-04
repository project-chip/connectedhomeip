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
get_filename_component(CHEF ${CMAKE_CURRENT_SOURCE_DIR}/../ REALPATH)

include(${CHEF}/project_include.cmake)

get_filename_component(GEN_DIR ${CHEF}/out/${SAMPLE_NAME}/zap-generated REALPATH)

include(${TELINK_COMMON}/common.cmake)
include(${CHIP_ROOT}/config/telink/app/enable-gnu-std.cmake)
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

project(chip-telink-chef-example)

target_include_directories(app PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${GEN_DIR}/app-common
                           ${GEN_DIR}
                           ${CHEF}
                           ${GEN_DIR}/../
                           ${CHIP_ROOT}/src
                           ${CHEF}/shell_common/include
                           ${CHEF}/telink/include
                           ${TELINK_COMMON}/common/include
                           ${TELINK_COMMON}/util/include
                           ${TELINK_COMMON}/app/include
                           ${TELINK_COMMON}/zephyr_ext
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

target_sources(app PRIVATE
    src/AppTask.cpp
    ${TELINK_COMMON}/common/src/mainCommon.cpp
    ${TELINK_COMMON}/common/src/AppTaskCommon.cpp
    ${TELINK_COMMON}/util/src/LEDManager.cpp
    ${TELINK_COMMON}/util/src/ButtonManager.cpp
    ${TELINK_COMMON}/util/src/ThreadUtil.cpp
    ${TELINK_COMMON}/util/src/PWMManager.cpp
    ${TELINK_COMMON}/zephyr_ext/zephyr_key_matrix.c
    ${TELINK_COMMON}/zephyr_ext/zephyr_key_pool.c
    ${TELINK_COMMON}/zephyr_ext/zephyr_led_pool.c
    ${TELINK_COMMON}/zephyr_ext/zephyr_pwm_pool.c
    ${TELINK_COMMON}/zephyr_ext/zephyr_ws2812.c
    ${CHEF}/common/stubs.cpp
)

message(STATUS ${CHEF}/devices/${SAMPLE_NAME}.zap)

chip_configure_data_model(app
    ZAP_FILE ${CHEF}/devices/${SAMPLE_NAME}.zap
)

if(CONFIG_BOOTLOADER_MCUBOOT)
  target_sources(app PRIVATE ${TELINK_COMMON}/util/src/OTAUtil.cpp)
endif()

if (CONFIG_CHIP_PW_RPC)
  include(${TELINK_COMMON}/Rpc.cmake)
endif(CONFIG_CHIP_PW_RPC)
