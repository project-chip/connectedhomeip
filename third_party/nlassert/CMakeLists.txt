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
#      This file is the CMake template for the nlassert library.
#

add_library(nlassert INTERFACE)     # Tag header-only library as INTERFACE

target_include_directories(nlassert INTERFACE repo/include)

#### tests common

set(COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/third_party/nlunit-test/repo/src
    repo/tests
)

set(COMMON_LIBS
    nlassert
    nlunit-test
    ${CHIP_PLATFORM_LIBS}
    chip-config
)

#### nlassert-static-test-cxx
add_executable(nlassert-static-test-cxx repo/tests/nlassert-static-test-cxx.cpp)
target_include_directories(nlassert-static-test-cxx PRIVATE ${COMMON_INCLUDES})
target_link_libraries(nlassert-static-test-cxx PRIVATE ${COMMON_LIBS})
target_link_directories(nlassert-static-test-cxx PRIVATE ${COMMON_LDPATH})
add_test(NAME nlassert-static-test-cxx COMMAND nlassert-static-test-cxx)

#### nlassert-static-test
add_executable(nlassert-static-test repo/tests/nlassert-static-test.c)
target_include_directories(nlassert-static-test PRIVATE ${COMMON_INCLUDES})
target_link_libraries(nlassert-static-test PRIVATE ${COMMON_LIBS})
target_link_directories(nlassert-static-test PRIVATE ${COMMON_LDPATH})
add_test(NAME nlassert-static-test COMMAND nlassert-static-test)

#### nlassert-test
add_executable(nlassert-test repo/tests/nlassert-test.c)
target_include_directories(nlassert-test PRIVATE ${COMMON_INCLUDES})
target_link_libraries(nlassert-test PRIVATE ${COMMON_LIBS})
target_link_directories(nlassert-test PRIVATE ${COMMON_LDPATH})
add_test(NAME nlassert-test COMMAND nlassert-test)
