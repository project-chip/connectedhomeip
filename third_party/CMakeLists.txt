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

add_subdirectory(lwip)
add_subdirectory(mbedtls)

if (NOT CHIP_BUILD_NLFRIENDS_WITH_AUTOTOOLS)

# Build nlfriends natively with cmake. This results in much faster builds, but 
# requires updates when upstream diverges. Luckily these libs don't change often.

add_subdirectory(nlassert)
add_subdirectory(nlio)
add_subdirectory(nlfaultinjection)
add_subdirectory(nlunit-test)

else (NOT CHIP_BUILD_NLFRIENDS_WITH_AUTOTOOLS)

include(ExternalProject)

project(nlassert)
ExternalProject_Add(nlassert
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nlassert/repo
    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/nlassert/repo/configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND make
)
add_test(NAME nlassert-check
    COMMAND make check
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/third_party/nlassert-prefix/src/nlassert-build
)

project(nlio)
ExternalProject_Add(nlio
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nlio/repo
    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/nlio/repo/configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND make
)
add_test(NAME nlio-check
    COMMAND make check
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/third_party/nlio-prefix/src/nlio-build
)

project(nlunit-test)
ExternalProject_Add(nlunit-test
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nlunit-test/repo
    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/nlunit-test/repo/configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND make
)
add_test(NAME nlunit-test-check
    COMMAND make check
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/third_party/nlunit-test-prefix/src/nlunit-test-build
)

project(nlfaultinjection)
ExternalProject_Add(nlfaultinjection
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nlfaultinjection/repo
    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/nlfaultinjection/repo/configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND make
)
add_test(NAME nlfaultinjection-check
    COMMAND make check
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/third_party/nlfaultinjection-prefix/src/nlfaultinjection-build
)

endif (NOT CHIP_BUILD_NLFRIENDS_WITH_AUTOTOOLS)
