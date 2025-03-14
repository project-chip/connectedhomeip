#
#   Copyright (c) 2020 Project CHIP Authors
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

#
#   @file
#     CMake project for building and running selected CHIP unit tests using
#     'nrfconnect' platform integration layer in CHIP and Zephyr
#     'native_sim[/native/64]' platforms. Note that certain design decisions behind
#     the native_sim platforms make them inapplicable for some unit tests,
#     hence only a subset of CHIP unit tests is listed in the project.
#     See: https://docs.zephyrproject.org/4.0.0/boards/native/native_sim/doc/index.html
#     for more details.
#

cmake_minimum_required(VERSION 3.13.1)

get_filename_component(CHIP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/connectedhomeip REALPATH)

# Make sure all components can see project mbedTLS config
include_directories(AFTER ${CMAKE_CURRENT_SOURCE_DIR}/main/include)

# ==================================================
# Setup CHIP build
# ==================================================

set(CHIP_CFLAGS
    -DCHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS=1
    -DCHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS=0
    -DCHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF=0
    -DCHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS=1
    -I${CMAKE_CURRENT_SOURCE_DIR}/main/include
)

# Load NCS/Zephyr build system
list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/nrfconnect/chip-module)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

set(PIGWEED_ROOT "${CHIP_ROOT}/third_party/pigweed/repo")

# ==================================================
# Build 'all tests' runner
# ==================================================

project(AllChipTests)
enable_testing()

target_sources(app PRIVATE main/runner.cpp)
target_include_directories(app PUBLIC
  ${PIGWEED_ROOT}/pw_assert/public
  ${PIGWEED_ROOT}/pw_assert_zephyr/public
  ${PIGWEED_ROOT}/pw_assert_zephyr/public_overrides
  ${PIGWEED_ROOT}/pw_bytes/public
  ${PIGWEED_ROOT}/pw_preprocessor/public
  ${PIGWEED_ROOT}/pw_polyfill/public
  ${PIGWEED_ROOT}/pw_polyfill/standard_library_public
  ${PIGWEED_ROOT}/pw_polyfill/public_overrides
  ${PIGWEED_ROOT}/pw_result/public
  ${PIGWEED_ROOT}/pw_span/public
  ${PIGWEED_ROOT}/pw_span/public_overrides
  ${PIGWEED_ROOT}/pw_status/public
  ${PIGWEED_ROOT}/pw_string/public
  ${PIGWEED_ROOT}/pw_unit_test/public
  ${PIGWEED_ROOT}/pw_unit_test/light_public_overrides
  ${PIGWEED_ROOT}/third_party/fuchsia/repo/sdk/lib/stdcompat/include
)
target_link_libraries(app PUBLIC chip $<TARGET_FILE:kernel>)
target_compile_definitions(app PUBLIC CHIP_HAVE_CONFIG_H)

add_test(AllChipTests zephyr/zephyr.exe)
