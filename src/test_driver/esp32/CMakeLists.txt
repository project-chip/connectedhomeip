#
#    Copyright (c) 2021 Project CHIP Authors
#    All rights reserved.
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

# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(PROJECT_VER "v1.0")
set(PROJECT_VER_NUMBER 1)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/esp32_unit_tests.cmake)

set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/third_party/connectedhomeip/config/esp32/components"
)

project(test-driver)

# C++17 is required for RPC build.
idf_build_set_property(CXX_COMPILE_OPTIONS "-std=gnu++17;-Os;-DLWIP_IPV6_SCOPES=0;-DCHIP_HAVE_CONFIG_H" APPEND)
idf_build_set_property(C_COMPILE_OPTIONS "-Os;-DLWIP_IPV6_SCOPES=0" APPEND)

# TODO: libAppTests depends on MessagingTestHelpers, which depends on
# NetworkTestHelpers.  That sort of depends on InetTestHelpers or
# equivalent (to provide gSystemLayer, gInet, InitNetwork(),
# ShutdownNetwork()) but there's only a POSIX implementation of that
# last, which does not compile on ESP32.  Need to figure out how to
# make that work.  See comments below for the transport layer tests,
# which have the same issue.
#
# libAppTests.a -lMessagingTestHelpers -lNetworkTestHelpers
#
# TODO: ble tests do not compile using CMake (library is not auto-built)
# libBleLayerTests.a

esp32_unit_test(NAME testASN1 LIBRARY ASN1Tests)
esp32_unit_test(NAME testChipCrypto LIBRARY ChipCryptoTests EXTRA_LIBRARIES -lCertTestVectors)
esp32_unit_test(NAME testCore LIBRARY CoreTests)
esp32_unit_test(NAME testInetLayer LIBRARY InetLayerTests)
esp32_unit_test(NAME testRetransmit LIBRARY RetransmitTests)
esp32_unit_test(NAME testSystemLayer LIBRARY SystemLayerTests)


# allow other tools to discover what images are available without grepping for '.img'
string (REPLACE ";" "\n" BUILT_IMAGES "${ESP32_TEST_IMAGES}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/test_images.txt" "${BUILT_IMAGES}")
