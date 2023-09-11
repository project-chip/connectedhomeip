#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#
cmake_minimum_required(VERSION 3.13.1)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip REALPATH)
get_filename_component(APP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/.. REALPATH)

include(${CHIP_ROOT}/config/nrfconnect/app/check-nrfconnect-version.cmake)

# Load NCS/Zephyr build system
set(CONF_FILE prj.conf)

list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/nrfconnect/chip-module)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD}.conf)
  list(APPEND CONF_FILE boards/${BOARD}.conf)
endif()

project(chip-nrfconnect-shell-example)

include(${CHIP_ROOT}/config/nrfconnect/app/enable-gnu-std.cmake)

# -Wmaybe-uninitialized has too many false positives, including on std::optional
# and chip::Optional.  Make it nonfatal.
#
# See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
target_compile_options(app PRIVATE -Werror -Wno-error=maybe-uninitialized)

target_include_directories(app PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip/zzz_generated/app-common
                           ${APP_ROOT}/shell_common/include)

target_sources(app PRIVATE
    ${APP_ROOT}/shell_common/globals.cpp
    ${APP_ROOT}/shell_common/cmd_misc.cpp
    ${APP_ROOT}/shell_common/cmd_otcli.cpp
    ${APP_ROOT}/standalone/main.cpp
)

include(${CHIP_ROOT}/config/nrfconnect/app/flashing.cmake)
