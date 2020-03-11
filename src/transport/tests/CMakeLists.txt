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

add_library(TransportLayerTests
    TestMessageHeader.cpp
    TestPeerConnections.cpp
    TestSecureSessionMgr.cpp
    TestSecureSession.cpp
    TestUDP.cpp
)

set(COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/lib/core
    ${PROJECT_SOURCE_DIR}/src/system
    ${PROJECT_SOURCE_DIR}/third_party/nlunit-test/repo/src
    ${PROJECT_SOURCE_DIR}/third_party/lwip/repo/lwip/src/include
)

set(COMMON_LIBS
    ${PLATFORM_LINK_START}
    TransportLayerTests
    TransportLayer
    BleLayer
    ChipCrypto
    InetLayer
    SystemLayer
    SupportLayer
    nlunit-test
    nlfaultinjection
    ${CHIP_PLATFORM_LIBS}
    ${PLATFORM_LINK_END}
    chip-config
)

set(COMMON_LDPATH)

target_include_directories(TransportLayerTests PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TransportLayerTests PRIVATE chip-config)

add_executable(TestTransportMessageHeader TestMessageHeaderDriver.cpp)
target_include_directories(TestTransportMessageHeader PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestTransportMessageHeader PRIVATE ${COMMON_LIBS})
target_link_directories(TestTransportMessageHeader PRIVATE ${COMMON_LDPATH})
add_test(NAME TestTransportMessageHeader COMMAND TestTransportMessageHeader)

add_executable(TestTransportPeerConnections TestPeerConnectionsDriver.cpp)
target_include_directories(TestTransportPeerConnections PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestTransportPeerConnections PRIVATE ${COMMON_LIBS})
target_link_directories(TestTransportPeerConnections PRIVATE ${COMMON_LDPATH})
add_test(NAME TestTransportPeerConnections COMMAND TestTransportPeerConnections)

if(CHIP_TARGET_STYLE_UNIX)
add_executable(TestTransportSecureSession TestSecureSessionDriver.cpp)
target_include_directories(TestTransportSecureSession PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestTransportSecureSession PRIVATE ${COMMON_LIBS})
target_link_directories(TestTransportSecureSession PRIVATE ${COMMON_LDPATH})
add_test(NAME TestTransportSecureSession COMMAND TestTransportSecureSession)

add_executable(TestTransportSecureSessionMgr TestSecureSessionMgrDriver.cpp)
target_include_directories(TestTransportSecureSessionMgr PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestTransportSecureSessionMgr PRIVATE ${COMMON_LIBS})
target_link_directories(TestTransportSecureSessionMgr PRIVATE ${COMMON_LDPATH})
add_test(NAME TestTransportSecureSessionMgr COMMAND TestTransportSecureSessionMgr)

add_executable(TestTransportUDP TestUDPDriver.cpp)
target_include_directories(TestTransportUDP PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestTransportUDP PRIVATE ${COMMON_LIBS})
target_link_directories(TestTransportUDP PRIVATE ${COMMON_LDPATH})
add_test(NAME TestTransportUDP COMMAND TestTransportUDP)
endif(CHIP_TARGET_STYLE_UNIX)
