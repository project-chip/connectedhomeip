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
#      This file is the CMake template for the CHIP SDK shell library code.
#

add_library(ChipShell)

set(COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_BINARY_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/lib/shell
    ${PROJECT_SOURCE_DIR}/third_party/nlassert/repo/include
)

set(COMMON_SOURCES
    commands.cpp
    streamer.cpp
    shell.cpp
)

add_dependencies(ChipShell
    chip-includes
)

if(CHIP_TARGET_STYLE_UNIX)
add_subdirectory(tests)

set(COMMON_SOURCES ${COMMON_SOURCES} 
    streamer_stdio.cpp
)
endif(CHIP_TARGET_STYLE_UNIX)

target_compile_definitions(ChipShell PRIVATE)
target_compile_options(ChipShell PRIVATE)
target_include_directories(ChipShell PRIVATE ${COMMON_INCLUDES})
target_sources(ChipShell PRIVATE ${COMMON_SOURCES})

target_link_libraries(ChipShell PRIVATE chip-config)
