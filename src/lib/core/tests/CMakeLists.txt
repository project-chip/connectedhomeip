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

set(COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/third_party/nlunit-test/repo/src
)

set(COMMON_LIBS
    SystemLayer
    SupportLayer
    ChipCore
    nlunit-test
    nlfaultinjection
    ${CHIP_PLATFORM_LIBS}
    chip-config
)

set(COMMON_LDPATH)


#### TestCHIPErrorStr

add_executable(TestCHIPErrorStr
    TestCHIPErrorStr.cpp
    TestCHIPErrorStrDriver.cpp
)

target_include_directories(TestCHIPErrorStr PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestCHIPErrorStr PRIVATE ${COMMON_LIBS})
target_link_directories(TestCHIPErrorStr PRIVATE ${COMMON_LDPATH})

add_test(NAME TestCHIPErrorStr COMMAND TestCHIPErrorStr)


#### TestCHIPTLV

add_executable(TestCHIPTLV
    TestCHIPTLV.cpp
    TestCHIPTLVDriver.cpp
)

target_include_directories(TestCHIPTLV PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestCHIPTLV PRIVATE ${COMMON_LIBS})
target_link_directories(TestCHIPTLV PRIVATE ${COMMON_LDPATH})

add_test(NAME TestCHIPTLV COMMAND TestCHIPTLV)
