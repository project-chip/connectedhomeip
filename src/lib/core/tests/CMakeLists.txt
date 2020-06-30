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
#      This file is the CMake template for the CHIP SDK core library tests.
#

add_library(ChipCoreTests
    TestCHIPErrorStr.cpp
    TestCHIPTLV.cpp
)

target_include_directories(ChipCoreTests PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/include
    ${NLUNIT_TEST_INCLUDES}
)

target_link_libraries(ChipCoreTests PUBLIC 
    SystemLayer
    SupportLayer
    ChipCore
    nlunit-test
    nlfaultinjection
    ${CHIP_PLATFORM_LIBS}
    chip-config
)

target_link_directories(ChipCoreTests PUBLIC
    ${NLUNIT_TEST_LDPATH}
    ${NLFAULT_INJECTION_LDPATH}
)

#### TestCHIPErrorStr
add_executable(TestCHIPErrorStr TestCHIPErrorStrDriver.cpp)
target_link_libraries(TestCHIPErrorStr PRIVATE ChipCoreTests)
add_test(NAME TestCHIPErrorStr COMMAND TestCHIPErrorStr)
add_dependencies(check TestCHIPErrorStr)

#### TestCHIPTLV
add_executable(TestCHIPTLV TestCHIPTLVDriver.cpp)
target_link_libraries(TestCHIPTLV PRIVATE ChipCoreTests)
add_test(NAME TestCHIPTLV COMMAND TestCHIPTLV)
add_dependencies(check TestCHIPTLV)
