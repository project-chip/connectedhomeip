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
#      This file is the top-level CMake template for the CHIP package.
#

set(CHIP_PROJECT_SOURCE_DIR ${Chip_DIR})
set(CHIP_PROJECT_BINARY_DIR ${PROJECT_BINARY_DIR}/third_party/connectedhomeip)

include("${CHIP_PROJECT_SOURCE_DIR}/integrations/cmake/options.cmake")
include("${CHIP_PROJECT_SOURCE_DIR}/integrations/cmake/toolchain.cmake")

set(Chip_INCLUDE_DIRS
    ${CHIP_PROJECT_SOURCE_DIR}/src
    ${CHIP_PROJECT_SOURCE_DIR}/src/include
    ${CHIP_PROJECT_SOURCE_DIR}/src/lib
    ${CHIP_PROJECT_SOURCE_DIR}/src/crypto
    ${CHIP_PROJECT_SOURCE_DIR}/third_party/nlassert/repo/include
)

set(Chip_LIBRARIES
    -L${CHIP_PROJECT_BINARY_DIR}/src/app
    -L${CHIP_PROJECT_BINARY_DIR}/src/ble
    -L${CHIP_PROJECT_BINARY_DIR}/src/crypto
    -L${CHIP_PROJECT_BINARY_DIR}/src/lwip
    -L${CHIP_PROJECT_BINARY_DIR}/src/inet
    -L${CHIP_PROJECT_BINARY_DIR}/src/platform
    -L${CHIP_PROJECT_BINARY_DIR}/src/system
    -L${CHIP_PROJECT_BINARY_DIR}/src/transport
    -L${CHIP_PROJECT_BINARY_DIR}/src/lib/core
    -L${CHIP_PROJECT_BINARY_DIR}/src/lib/shell
    -L${CHIP_PROJECT_BINARY_DIR}/src/lib/support
    -L${CHIP_PROJECT_BINARY_DIR}/third_party/lwip
    -L${CHIP_PROJECT_BINARY_DIR}/third_party/mbedtls
    -L${CHIP_PROJECT_BINARY_DIR}/third_party/nlfaultinjection
    -L${CHIP_PROJECT_BINARY_DIR}/third_party/nlunit-test
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
    nlfaultinjection
    nlunit-test
)
