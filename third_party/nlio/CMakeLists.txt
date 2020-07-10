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
#      This file is the CMake template for the nlio library.
#

if (BUILD_EXTERNALS_WITH_CMAKE)

#
# Build external project with cmake
#
add_library(nlio INTERFACE)     # Tag header-only library as INTERFACE

target_include_directories(nlio INTERFACE repo/include)

#### tests common

set(COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/third_party/nlunit-test/repo/src
)

set(COMMON_LIBS
    nlio
    nlunit-test
    ${CHIP_PLATFORM_LIBS}
    chip-config
)

#### nlbyteorder-test-cxx
add_executable(nlbyteorder-test-cxx repo/tests/nlbyteorder-test-cxx.cpp)
target_include_directories(nlbyteorder-test-cxx PRIVATE ${COMMON_INCLUDES})
target_link_libraries(nlbyteorder-test-cxx PRIVATE ${COMMON_LIBS})
target_link_directories(nlbyteorder-test-cxx PRIVATE ${COMMON_LDPATH})
add_test(NAME nlbyteorder-test-cxx COMMAND nlbyteorder-test-cxx)
add_dependencies(check nlbyteorder-test-cxx)

#### nlbyteorder-test
add_executable(nlbyteorder-test repo/tests/nlbyteorder-test.c)
target_include_directories(nlbyteorder-test PRIVATE ${COMMON_INCLUDES})
target_link_libraries(nlbyteorder-test PRIVATE ${COMMON_LIBS})
target_link_directories(nlbyteorder-test PRIVATE ${COMMON_LDPATH})
add_test(NAME nlbyteorder-test COMMAND nlbyteorder-test)
add_dependencies(check nlbyteorder-test)

#### nlio-byteorder-test-cxx
add_executable(nlio-byteorder-test-cxx repo/tests/nlio-byteorder-test-cxx.cpp)
target_include_directories(nlio-byteorder-test-cxx PRIVATE ${COMMON_INCLUDES})
target_link_libraries(nlio-byteorder-test-cxx PRIVATE ${COMMON_LIBS})
target_link_directories(nlio-byteorder-test-cxx PRIVATE ${COMMON_LDPATH})
add_test(NAME nlio-byteorder-test-cxx COMMAND nlio-byteorder-test-cxx)
add_dependencies(check nlio-byteorder-test-cxx)

#### nlio-byteorder-test
add_executable(nlio-byteorder-test repo/tests/nlio-byteorder-test.c)
target_include_directories(nlio-byteorder-test PRIVATE ${COMMON_INCLUDES})
target_link_libraries(nlio-byteorder-test PRIVATE ${COMMON_LIBS})
target_link_directories(nlio-byteorder-test PRIVATE ${COMMON_LDPATH})
add_test(NAME nlio-byteorder-test COMMAND nlio-byteorder-test)
add_dependencies(check nlio-byteorder-test)

#### nlio-test-cxx
add_executable(nlio-test-cxx repo/tests/nlio-test-cxx.cpp)
target_include_directories(nlio-test-cxx PRIVATE ${COMMON_INCLUDES})
target_link_libraries(nlio-test-cxx PRIVATE ${COMMON_LIBS})
target_link_directories(nlio-test-cxx PRIVATE ${COMMON_LDPATH})
add_test(NAME nlio-test-cxx COMMAND nlio-test-cxx)
add_dependencies(check nlio-test-cxx)

#### nlio-test
add_executable(nlio-test repo/tests/nlio-test.c)
target_include_directories(nlio-test PRIVATE ${COMMON_INCLUDES})
target_link_libraries(nlio-test PRIVATE ${COMMON_LIBS})
target_link_directories(nlio-test PRIVATE ${COMMON_LDPATH})
add_test(NAME nlio-test COMMAND nlio-test)
add_dependencies(check nlio-test)

else (BUILD_EXTERNALS_WITH_CMAKE)

#
# Build external project with autotools
#
project(nlio)
ExternalProject_Add(nlio
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/repo
    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/repo/configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND make
)
add_test(NAME nlio-check
    COMMAND make check
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/third_party/nlio/nlio-prefix/src/nlio-build
)

endif (BUILD_EXTERNALS_WITH_CMAKE)
