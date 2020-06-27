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
#      This file is the CMake template for the CHIP SDK core layer code.
#

add_subdirectory(tests)

add_library(ChipCore)

target_compile_definitions(ChipCore PRIVATE
    ${CHIP_CORE_PRIVATE_DEFINES}
)

target_compile_options(ChipCore PRIVATE
    ${CHIP_CORE_CFLAGS}
)

set(COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/inet
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/lib/core
    ${PROJECT_SOURCE_DIR}/src/lib/support
    ${PROJECT_SOURCE_DIR}/src/system
)

set(COMMON_SOURCES
    CHIPCircularTLVBuffer.cpp
    CHIPError.cpp
    CHIPTLVDebug.cpp
    CHIPTLVReader.cpp
    CHIPTLVUtilities.cpp
    CHIPTLVWriter.cpp
    CHIPTLVUpdater.cpp
    CHIPKeyIds.cpp
)

target_include_directories(ChipCore PRIVATE ${COMMON_INCLUDES})

target_sources(ChipCore PRIVATE ${COMMON_SOURCES})

target_link_libraries(ChipCore PRIVATE
    ChipCrypto
    InetLayer
    SystemLayer
    SupportLayer
    ${CHIP_PLATFORM_LIBS}
    chip-config
)
