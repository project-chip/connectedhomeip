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
#      This file is the CMake template for the nlunit-test library.
#

if (BUILD_EXTERNALS_WITH_CMAKE)

#
# Build external project with cmake
#
add_library(nlunit-test
    repo/src/nlunit-test.c
)

target_include_directories(nlunit-test PUBLIC
    repo/src
)

target_link_libraries(nlunit-test PRIVATE chip-config)

else (BUILD_EXTERNALS_WITH_CMAKE)

#
# Build external project with autotools
#
project(nlunit-test)
ExternalProject_Add(nlunit-test
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/repo
    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/repo/configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND make
)
add_test(NAME nlunit-test-check
    COMMAND make check
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/third_party/nlunit-test/nlunit-test-prefix/src/nlunit-test-build
)

set(NLUNIT_TEST_INCLUDES "${CMAKE_BINARY_DIR}/third_party/nlunit-test/nlunit-test-prefix/include"
    CACHE PATH "Module nlunit-test include paths")
set(NLUNIT_TEST_LDPATH "${CMAKE_BINARY_DIR}/third_party/nlunit-test/nlunit-test-prefix/lib"
    CACHE PATH "Module nlunit-test library paths")

endif (BUILD_EXTERNALS_WITH_CMAKE)
