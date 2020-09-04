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
cmake_minimum_required(VERSION 3.13.1)

get_filename_component(APP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/.. REALPATH)
get_filename_component(CHIP_ROOT ${APP_ROOT}/third_party/connectedhomeip REALPATH)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CHIP_ROOT}/config/nrfconnect/)
include(nrfconnect-app)

project(chip-nrf52840-shell-example)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(app PRIVATE
    ${APP_ROOT}/cmd_base64.cpp
    ${APP_ROOT}/cmd_device.cpp
    ${APP_ROOT}/cmd_misc.cpp
    ${APP_ROOT}/cmd_otcli.cpp
    ${APP_ROOT}/main.cpp
)

# 1) The example needs be linked with openthread command line interpreter (required by cmd_otcli.cpp)
# 2) Zephyr shell must be disabled not to interfere with the example
# We can't achieve 1 and 2 with Kconfig since CONFIG_OPENTHREAD_SHELL implies CONFIG_SHELL.
# Hence the workaround below.
if(CONFIG_OPENTHREAD_FTD)
    target_link_libraries(app INTERFACE openthread-cli-ftd)
elseif(CONFIG_OPENTHREAD_MTD)
    target_link_libraries(app INTERFACE openthread-cli-mtd)
endif()