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
#      This file is the CMake template for the CHIP SDK BLE library tests.
#

set(COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/third_party/nlunit-test/repo/src
)

set(COMMON_LIBS
    BleLayer
    SupportLayer
    nlunit-test
    ${CHIP_PLATFORM_LIBS}
    chip-config
)

set(COMMON_LDPATH)


#### TestBleErrorStr

add_executable(TestBleErrorStr
    TestBleErrorStr.cpp
    TestBleErrorStrDriver.cpp
)

target_include_directories(TestBleErrorStr PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestBleErrorStr PRIVATE ${COMMON_LIBS})
target_link_directories(TestBleErrorStr PRIVATE ${COMMON_LDPATH})

add_test(NAME TestBleErrorStr COMMAND TestBleErrorStr)
