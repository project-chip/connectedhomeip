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
#      This file is the CMake template for the CHIP SDK transport layer library.
#

add_subdirectory(tests)

add_library(TransportLayer)

set(COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/lib/core
    ${PROJECT_SOURCE_DIR}/src/system
    ${PROJECT_SOURCE_DIR}/src/transport
    ${PROJECT_SOURCE_DIR}/third_party/nlassert/repo/include
    ${PROJECT_SOURCE_DIR}/third_party/mbedtls/repo/include
)

set(COMMON_SOURCES
    MessageHeader.cpp
    SecureSession.cpp
    SecureSessionMgr.cpp
    UDP.cpp
)

target_compile_definitions(TransportLayer PRIVATE)
target_compile_options(TransportLayer PRIVATE)
target_include_directories(TransportLayer PRIVATE ${COMMON_INCLUDES})
target_sources(TransportLayer PRIVATE ${COMMON_SOURCES})

target_link_libraries(TransportLayer PRIVATE chip-config)
