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
#      This file is the CMake template for the CHIP SDK QR code library.
#

add_subdirectory(tests)

add_library(SetupPayload)

set(COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/lib/core
    ${PROJECT_SOURCE_DIR}/src/system
)

set(COMMON_SOURCES
    ManualSetupPayloadGenerator.cpp
    ManualSetupPayloadParser.cpp
    QRCodeSetupPayloadGenerator.cpp
    SetupPayload.cpp
    Base41.cpp
    QRCodeSetupPayloadParser.cpp
    SetupPayloadHelper.cpp
)

target_compile_definitions(SetupPayload PRIVATE)
target_compile_options(SetupPayload PRIVATE)
target_include_directories(SetupPayload PRIVATE ${COMMON_INCLUDES})
target_sources(SetupPayload PRIVATE ${COMMON_SOURCES})

target_link_libraries(SetupPayload PRIVATE chip-config)
