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
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/crypto
    ${PROJECT_SOURCE_DIR}/third_party/nlunit-test/repo/src
)

set(COMMON_LIBS
    ${PLATFORM_LINK_START}
    ChipShell
    ChipCore
    SupportLayer
    nlunit-test
    ${CHIP_PLATFORM_LIBS}
    ${PLATFORM_LINK_END}
    chip-config
)

set(COMMON_LDPATH)

add_executable(TestShellStreamerStdio
    TestStreamerStdio.cpp
    TestStreamerStdioDriver.cpp
)

target_include_directories(TestShellStreamerStdio PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestShellStreamerStdio PRIVATE ${COMMON_LIBS})
target_link_directories(TestShellStreamerStdio PRIVATE ${COMMON_LDPATH})

add_test(NAME TestShellStreamerStdio COMMAND TestShellStreamerStdio)


add_executable(TestShell
    TestShell.cpp
    TestShellDriver.cpp
)

target_include_directories(TestShell PRIVATE ${COMMON_INCLUDES})
target_link_libraries(TestShell PRIVATE ${COMMON_LIBS})
target_link_directories(TestShell PRIVATE ${COMMON_LDPATH})

add_test(NAME TestShell COMMAND TestShell)
