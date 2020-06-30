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
#      This file is the CMake template for the CHIP SDK shell library tests.
#

add_library(ChipShellTests
    TestStreamerStdio.cpp
    TestShell.cpp
)

target_include_directories(ChipShellTests PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/crypto
    ${NLUNIT_TEST_INCLUDES}
)

target_link_libraries(ChipShellTests PUBLIC
    ${PLATFORM_LINK_START}
    ChipShell
    ChipCore
    SupportLayer
    nlunit-test
    ${CHIP_PLATFORM_LIBS}
    ${PLATFORM_LINK_END}
    chip-config
)

target_link_directories(ChipShellTests PUBLIC
    ${NLUNIT_TEST_LDPATH}
)

add_executable(TestShellStreamerStdio TestStreamerStdioDriver.cpp)
target_link_libraries(TestShellStreamerStdio PRIVATE ChipShellTests)
add_test(NAME TestShellStreamerStdio COMMAND TestShellStreamerStdio)
add_dependencies(check TestShellStreamerStdio)

add_executable(TestShell TestShellDriver.cpp)
target_link_libraries(TestShell PRIVATE ChipShellTests)
add_test(NAME TestShell COMMAND TestShell)
add_dependencies(check TestShell)
