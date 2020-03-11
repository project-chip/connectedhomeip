#
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

#
#    Description:
#      This file is the CMake template for the CHIP SDK transport layer library tests.
#

add_library(TransportLayerTests
    NetworkTestHelpers.cpp
    TestMessageHeader.cpp
    TestPeerConnections.cpp
    TestSecureSessionMgr.cpp
    TestSecureSession.cpp
    TestUDP.cpp
)

target_include_directories(TransportLayerTests PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/lib/core
    ${PROJECT_SOURCE_DIR}/src/system
    ${PROJECT_SOURCE_DIR}/src/inet/tests
    ${PROJECT_SOURCE_DIR}/src/transport/tests
    ${NLUNIT_TEST_INCLUDES}
    ${PROJECT_SOURCE_DIR}/third_party/lwip/repo/lwip/src/include
)

target_link_libraries(TransportLayerTests PUBLIC
    ${PLATFORM_LINK_START}
    InetLayerTests
    TransportLayer
    BleLayer
    ChipCrypto
    InetLayer
    SystemLayer
    SupportLayer
    nlfaultinjection
    nlunit-test
    ${CHIP_PLATFORM_LIBS}
    ${PLATFORM_LINK_END}
    chip-config
)

target_link_directories(TransportLayerTests PUBLIC
    ${NLUNIT_TEST_LDPATH}
    ${NLFAULT_INJECTION_LDPATH}
)

add_executable(TestTransportMessageHeader TestMessageHeaderDriver.cpp)
target_link_libraries(TestTransportMessageHeader PRIVATE TransportLayerTests)
add_test(NAME TestTransportMessageHeader COMMAND TestTransportMessageHeader)
add_dependencies(check TestTransportMessageHeader)

add_executable(TestTransportPeerConnections TestPeerConnectionsDriver.cpp)
target_link_libraries(TestTransportPeerConnections PRIVATE TransportLayerTests)
add_test(NAME TestTransportPeerConnections COMMAND TestTransportPeerConnections)
add_dependencies(check TestTransportPeerConnections)

if(CHIP_TARGET_STYLE_UNIX)

add_executable(TestTransportSecureSession TestSecureSessionDriver.cpp)
target_link_libraries(TestTransportSecureSession PRIVATE TransportLayerTests)
add_test(NAME TestTransportSecureSession COMMAND TestTransportSecureSession)
add_dependencies(check TestTransportSecureSession)

add_executable(TestTransportSecureSessionMgr TestSecureSessionMgrDriver.cpp)
target_link_libraries(TestTransportSecureSessionMgr PRIVATE TransportLayerTests)
add_test(NAME TestTransportSecureSessionMgr COMMAND TestTransportSecureSessionMgr)
add_dependencies(check TestTransportSecureSessionMgr)

add_executable(TestTransportUDP TestUDPDriver.cpp)
target_link_libraries(TestTransportUDP PRIVATE TransportLayerTests)
add_test(NAME TestTransportUDP COMMAND TestTransportUDP)
add_dependencies(check TestTransportUDP)

endif(CHIP_TARGET_STYLE_UNIX)
