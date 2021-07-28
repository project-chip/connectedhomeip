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

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip REALPATH)
get_filename_component(APP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/.. REALPATH)

include(${CHIP_ROOT}/config/nrfconnect/app/check-nrfconnect-version.cmake)

# Load NCS/Zephyr build system
set(CONF_FILE ${CHIP_ROOT}/config/nrfconnect/app/sample-defaults.conf prj.conf)
list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/nrfconnect/chip-module)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

project(chip-nrfconnect-shell-example)

include(${CHIP_ROOT}/config/nrfconnect/app/enable-gnu-std.cmake)

target_compile_options(app PRIVATE -Werror)

target_include_directories(app PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${APP_ROOT}/shell_common/include)

target_sources(app PRIVATE
    ${APP_ROOT}/shell_common/globals.cpp
    ${APP_ROOT}/shell_common/cmd_misc.cpp
    ${APP_ROOT}/shell_common/cmd_otcli.cpp
    ${APP_ROOT}/shell_common/cmd_ping.cpp
    ${APP_ROOT}/shell_common/cmd_send.cpp     
    ${APP_ROOT}/standalone/main.cpp
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

include(${CHIP_ROOT}/config/nrfconnect/app/flashing.cmake)
