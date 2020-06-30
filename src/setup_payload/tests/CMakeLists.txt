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
#      This file is the CMake template for the CHIP SDK QR code library tests.
#

add_library(SetupPayloadTests
    TestManualCode.cpp
    TestQRCode.cpp
    TestQRCodeTLV.cpp
)

target_include_directories(SetupPayloadTests PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/system
    ${PROJECT_SOURCE_DIR}/src/setup_payload
    ${NLUNIT_TEST_INCLUDES}
)

target_link_libraries(SetupPayloadTests PUBLIC
    ChipCore
    SystemLayer
    SupportLayer
    nlfaultinjection
    nlunit-test
    ${CHIP_PLATFORM_LIBS}
    chip-config
)

target_link_directories(SetupPayloadTests PUBLIC
    ${NLUNIT_TEST_LDPATH}
    ${NLFAULT_INJECTION_LDPATH}
)

#### TestManualCode
add_executable(TestManualCode TestManualCodeDriver.cpp)
target_link_libraries(TestManualCode PRIVATE SetupPayloadTests)
add_test(NAME TestManualCode COMMAND TestManualCode)
add_dependencies(check TestManualCode)

#### TestQRCode
add_executable(TestQRCode TestQRCodeDriver.cpp)
target_link_libraries(TestQRCode PRIVATE SetupPayloadTests)
add_test(NAME TestQRCode COMMAND TestQRCode)
add_dependencies(check TestQRCode)

#### TestQRCodeTLV
add_executable(TestQRCodeTLV TestQRCodeTLVDriver.cpp)
target_link_libraries(TestQRCodeTLV PRIVATE SetupPayloadTests)
add_test(NAME TestQRCodeTLV COMMAND TestQRCodeTLV)
add_dependencies(check TestQRCodeTLV)
