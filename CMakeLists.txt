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
#      This file is the top-level CMake template for the CHIP SDK.
#

#
# Declare cmake version requirements
#
cmake_minimum_required(VERSION 3.17.2)

#
# Specify the top-level project name and the languages used.
#
project(Chip 
    LANGUAGES C CXX ASM
)

#
# Specifies the exact version of the C and C++ standard assumed by the codebase.
#
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CHIP_PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Root of CHIP SDK source")
set(CHIP_PROJECT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Root of CHIP SDK build output")

include("${CHIP_PROJECT_SOURCE_DIR}/integrations/cmake/options.cmake")
include("${CHIP_PROJECT_SOURCE_DIR}/integrations/cmake/toolchain.cmake")

add_subdirectory(src)
add_subdirectory(third_party)

#
# Compose list of all libraries produced by the CHIP SDK package.
#
set(CHIP_LIBRARIES
    ChipCore 
    ChipCrypto
    ChipDataModel
    ChipShell
    BleLayer
    InetLayer
    SystemLayer
    SupportLayer
    TransportLayer
    ${CHIP_PLATFORM_LIBS_PACKAGE}
    nlassert
    nlfaultinjection
    nlio
    nlunit-test
)

#
# Install defines which artifacts to include in CPack package.
#
install(
  TARGETS ${CHIP_LIBRARIES} 
  EXPORT Chip
  DESTINATION "${CHIP_PLATFORM}_${CMAKE_BUILD_TYPE}/lib"
)

set(CPACK_PACKAGE_NAME "Chip")
include(CPack)
