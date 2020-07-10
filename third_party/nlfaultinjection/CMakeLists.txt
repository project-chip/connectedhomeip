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
#      This file is the CMake template for the nlfaultinjection library.
#

if (BUILD_EXTERNALS_WITH_CMAKE)

#
# Build external project with cmake
#
add_library(nlfaultinjection
    repo/src/nlfaultinjection.cpp
)

target_include_directories(nlfaultinjection
  PUBLIC
    repo/include
    ${PROJECT_SOURCE_DIR}/third_party/nlassert/repo/include
)

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
add_dependencies(check TestFaultInjection)

else (BUILD_EXTERNALS_WITH_CMAKE)

#
# Build external project with autotools
#
project(nlfaultinjection)
ExternalProject_Add(nlfaultinjection
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/repo
    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/repo/configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND make
)
add_test(NAME nlfaultinjection-check
    COMMAND make check
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/third_party/nlfaultinjection/nlfaultinjection-prefix/src/nlfaultinjection-build
)

set(NLFAULT_INJECTION_INCLUDES "${CMAKE_BINARY_DIR}/third_party/nlfaultinjection/nlfaultinjection-prefix/include" 
    CACHE PATH "Module nlfaultinjection include paths")
set(NLFAULT_INJECTION_LDPATH "${CMAKE_BINARY_DIR}/third_party/nlfaultinjection/nlfaultinjection-prefix/lib" 
    CACHE PATH "Module nlfaultinjection library paths")

endif (BUILD_EXTERNALS_WITH_CMAKE)
