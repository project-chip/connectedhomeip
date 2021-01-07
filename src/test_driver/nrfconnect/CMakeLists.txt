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
#     'native_posix[_64]' platforms. Note that certain design decisions behind
#     the native_posix platforms make them inapplicable for some unit tests,
#     hence only a subset of CHIP unit tests is listed in the project.
#     See: https://docs.zephyrproject.org/1.12.0/boards/posix/native_posix/doc/board.html
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
    -isystem${ZEPHYR_BASE}/../modules/crypto/mbedtls/configs
    -isystem${ZEPHYR_BASE}/../mbedtls/include
)

set(CHIP_TESTS
    # TODO: this list is hard to maintain manually (and currently incomplete).
    # An automated way that includes figuring out dependencies would be much better
    -lAppTests
    -lASN1Tests
    -lBleLayerTests
    -lChipCryptoTests
    -lCoreTests
    -lInetLayerTests
    -lMessagingLayerTests
    -lPlatformTests
    # TODO: raw transport tests fail by running out of TCP/UDP endpoints
    # -lRawTransportTests
    -lRetransmitTests
    -lSupportTests
    -lSystemLayerTests
    -lTransportLayerTests
)

set(CHIP_LIBRARIES
    -lCHIP
    -lInetTestHelpers
    -lNetworkTestHelpers
    -lnlunit-test
    ${CHIP_TESTS}
)

include(${CHIP_ROOT}/config/nrfconnect/app/app.cmake)

# ==================================================
# Build 'all tests' runner
# ==================================================

project(AllChipTests)
enable_testing()

target_sources(app PRIVATE main/runner.cpp)
target_link_options(app PUBLIC -Wl,--whole-archive ${CHIP_TESTS} -Wl,--no-whole-archive)
target_link_libraries(app PUBLIC chip $<TARGET_FILE:kernel>)
target_compile_definitions(app PUBLIC CHIP_HAVE_CONFIG_H)

add_test(AllChipTests zephyr/zephyr.exe)
