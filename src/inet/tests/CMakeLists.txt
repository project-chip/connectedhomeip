#
#
#    Copyright (c) 2020 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file excegrep  in compliance with the License.
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

add_library(InetLayerTests
    TestInetAddress.cpp
    TestInetCommon.cpp
    TestInetCommonOptions.cpp
    TestInetErrorStr.cpp
    TestInetLayerCommon.cpp
)

set(COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_BINARY_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/lib/core
    ${PROJECT_SOURCE_DIR}/src/system
    ${PROJECT_SOURCE_DIR}/third_party/nlunit-test/repo/src
    ${PROJECT_SOURCE_DIR}/third_party/nlfaultinjection/repo/include
)

target_include_directories(InetLayerTests PRIVATE ${COMMON_INCLUDES})
target_link_libraries(InetLayerTests PRIVATE chip-config)

set(COMMON_LIBS
    InetLayerTests
    InetLayer
    SystemLayer
    SupportLayer
    nlunit-test
    nlfaultinjection
    ${CHIP_PLATFORM_LIBS}
    chip-config
)

set(COMMON_LDPATH)

#### TestInetAddress

add_executable(TestInetAddress
    TestInetAddress.cpp
    TestInetAddressDriver.cpp
)

target_include_directories(TestInetAddress PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestInetAddress PRIVATE ${COMMON_LIBS})
target_link_directories(TestInetAddress PRIVATE ${COMMON_LDPATH})
add_test(NAME TestInetAddress COMMAND TestInetAddress)


#### TestInetEndPoint

add_executable(TestInetEndPoint
    TestInetEndPoint.cpp
)

target_include_directories(TestInetEndPoint PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestInetEndPoint PRIVATE ${COMMON_LIBS})
target_link_directories(TestInetEndPoint PRIVATE ${COMMON_LDPATH})

# Disable based on #500 -- ipv6 support not working in CircleCI Docker
# add_test(NAME TestInetEndPoint COMMAND TestInetEndPoint)


#### TestInetErrorStr

add_executable(TestInetErrorStr
    TestInetErrorStr.cpp
    TestInetErrorStrDriver.cpp
)

target_include_directories(TestInetErrorStr PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestInetErrorStr PRIVATE ${COMMON_LIBS})
target_link_directories(TestInetErrorStr PRIVATE ${COMMON_LDPATH})
add_test(NAME TestInetErrorStr COMMAND TestInetErrorStr)


#### TestInetLayerDNS

if(NOT CONFIG_DEVICE_LAYER)
add_executable(TestInetLayerDNS
    TestInetLayerDNS.cpp
)

target_include_directories(TestInetLayerDNS PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestInetLayerDNS PRIVATE ${COMMON_LIBS})
target_link_directories(TestInetLayerDNS PRIVATE ${COMMON_LDPATH})

# Disable based on #500 -- ipv6 support not working in CircleCI Docker
# add_test(NAME TestInetLayerDNS COMMAND TestInetLayerDNS)
endif()