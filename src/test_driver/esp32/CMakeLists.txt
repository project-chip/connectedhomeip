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


esp32_unit_test(NAME testAccess LIBRARY accesstest)
esp32_unit_test(NAME testAddressResolve LIBRARY AddressResolveTests)
esp32_unit_test(NAME testAppClusterBuildingBlock LIBRARY AppClusterBuildingBlockTests)
esp32_unit_test(NAME testAppDataModel LIBRARY AppDataModelTests)
esp32_unit_test(NAME testASN1 LIBRARY ASN1Tests)
esp32_unit_test(NAME testBDX LIBRARY BDXTests)
esp32_unit_test(NAME testChipCrypto LIBRARY ChipCryptoTests EXTRA_LIBRARIES -lCertTestVectors)
esp32_unit_test(NAME testCodegenDataModelProvider LIBRARY CodegenDataModelProviderTests)
esp32_unit_test(NAME testCore LIBRARY CoreTests)
esp32_unit_test(NAME testFormat LIBRARY FormatTests)
esp32_unit_test(NAME testIMInterface LIBRARY IMInterfaceTests)
esp32_unit_test(NAME testInetLayer LIBRARY InetLayerTests)
esp32_unit_test(NAME testInteractionModel LIBRARY InteractionModelTests)
esp32_unit_test(NAME testMinimalMdnsCore LIBRARY MinimalMdnsCoreTests)
esp32_unit_test(NAME testMinimalMdnsRecords LIBRARY MinimalMdnsRecordsTests)
esp32_unit_test(NAME testMinimalMdnsResponders LIBRARY MinimalMdnsRespondersTests)
esp32_unit_test(NAME testMdns LIBRARY MdnsTests)
esp32_unit_test(NAME testPlatform LIBRARY PlatformTests)
esp32_unit_test(NAME testRetransmit LIBRARY RetransmitTests)
esp32_unit_test(NAME testSetupPayload LIBRARY SetupPayloadTests)
esp32_unit_test(NAME testSystemLayer LIBRARY SystemLayerTests)
esp32_unit_test(NAME testUserDirectedCommissioning LIBRARY UserDirectedCommissioningTests)

# TODO: libAppTests depends on MessagingTestHelpers, which depends on
# NetworkTestHelpers.  That sort of depends on InetTestHelpers or
# equivalent (to provide gSystemLayer, gInet, InitNetwork(),
# ShutdownNetwork()) but there's only a POSIX implementation of that
# last, which does not compile on ESP32.
# This affects AppTests, MessagingLayerTests, MinimalMdnstests, ControllerTests
# ControllerDataModelTests, ICDServerTests, SecureChannelTests.


# allow other tools to discover what images are available without grepping for '.img'
string (REPLACE ";" "\n" BUILT_IMAGES "${ESP32_TEST_IMAGES}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/test_images.txt" "${BUILT_IMAGES}")
