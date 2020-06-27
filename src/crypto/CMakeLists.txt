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
#      This file is the CMake template for the CHIP SDK Cryptography library.
#

add_subdirectory(tests)

add_library(ChipCrypto)

set(COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/system
    ${PROJECT_SOURCE_DIR}/third_party/mbedtls/repo/include
)

if (CHIP_CRYPTO_MBEDTLS)
set(COMMON_SOURCES CHIPCryptoPALmbedTLS.cpp)
endif ()

if (CHIP_CRYPTO_OPENSSL)
set(COMMON_SOURCES CHIPCryptoPALOpenSSL.cpp)
endif ()

target_compile_definitions(ChipCrypto PRIVATE)
target_compile_options(ChipCrypto PRIVATE)
target_include_directories(ChipCrypto PRIVATE ${COMMON_INCLUDES})
target_sources(ChipCrypto PRIVATE ${COMMON_SOURCES})

target_link_libraries(ChipCrypto PRIVATE chip-config)
