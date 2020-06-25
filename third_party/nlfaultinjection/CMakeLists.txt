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

add_library(nlfaultinjection)

set(COMMON_SOURCES
    repo/src/nlfaultinjection.cpp
)

set(COMMON_INCLUDES
    repo/include
    ${PROJECT_SOURCE_DIR}/third_party/nlassert/repo/include
)

target_include_directories(nlfaultinjection PRIVATE ${COMMON_INCLUDES})
target_sources(nlfaultinjection PRIVATE ${COMMON_SOURCES})
target_compile_definitions(nlfaultinjection PRIVATE)
target_compile_options(nlfaultinjection PRIVATE)

target_link_libraries(nlfaultinjection PRIVATE chip-config)

#### tests common

set(TEST_COMMON_INCLUDES
    repo/include
    ${PROJECT_SOURCE_DIR}/third_party/nlunit-test/repo/src
)

set(TEST_COMMON_LIBS
    nlfaultinjection
    nlunit-test
    ${CHIP_PLATFORM_LIBS}
    chip-config
)

#### TestFaultInjection

add_executable(TestFaultInjection
    repo/tests/TestFaultInjection.cpp
)

target_include_directories(TestFaultInjection PRIVATE ${TEST_COMMON_INCLUDES})
target_link_libraries(TestFaultInjection PRIVATE ${TEST_COMMON_LIBS})
target_link_directories(TestFaultInjection PRIVATE ${COMMON_LDPATH})

add_test(NAME TestFaultInjection COMMAND TestFaultInjection)
