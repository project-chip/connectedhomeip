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

#
#    Description:
#      This file is the CMake template for the CHIP SDK network library tests.
#

add_library(InetLayerTests
    TestInetAddress.cpp
    TestInetCommon.cpp
    TestInetCommonOptions.cpp
    TestInetErrorStr.cpp
    TestInetLayerCommon.cpp
)

target_include_directories(InetLayerTests
  PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_BINARY_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/lib/core
    ${PROJECT_SOURCE_DIR}/src/system
    ${NLFAULT_INJECTION_INCLUDES}
    ${NLUNIT_TEST_INCLUDES}
)

target_link_libraries(InetLayerTests 
  PUBLIC
    InetLayer
    SystemLayer
    SupportLayer
    nlunit-test
    nlfaultinjection
    ${CHIP_PLATFORM_LIBS}
    chip-config
)

target_link_directories(InetLayerTests PUBLIC
    ${NLUNIT_TEST_LDPATH}
    ${NLFAULT_INJECTION_LDPATH}
)

#### TestInetAddress
add_executable(TestInetAddress TestInetAddressDriver.cpp)
target_link_libraries(TestInetAddress PRIVATE InetLayerTests)
add_test(NAME TestInetAddress COMMAND TestInetAddress)
add_dependencies(check TestInetAddress)

#### TestInetEndPoint
#add_executable(TestInetEndPoint TestInetEndPoint.cpp)
#target_link_libraries(TestInetEndPoint PRIVATE InetLayerTests)
# Disable based on #500 -- ipv6 support not working in CircleCI Docker
# add_test(NAME TestInetEndPoint COMMAND TestInetEndPoint)
# add_dependencies(check TestInetEndPoint)

#### TestInetErrorStr
add_executable(TestInetErrorStr TestInetErrorStrDriver.cpp)
target_link_libraries(TestInetErrorStr PRIVATE InetLayerTests)
add_test(NAME TestInetErrorStr COMMAND TestInetErrorStr)
add_dependencies(check TestInetErrorStr)

if(NOT CONFIG_DEVICE_LAYER)

#### TestInetLayerDNS
add_executable(TestInetLayerDNS TestInetLayerDNS.cpp)
target_link_libraries(TestInetLayerDNS PRIVATE InetLayerTests)
# Disable based on #500 -- ipv6 support not working in CircleCI Docker
# add_test(NAME TestInetLayerDNS COMMAND TestInetLayerDNS)
# add_dependencies(check TestInetLayerDNS)

endif(NOT CONFIG_DEVICE_LAYER)
