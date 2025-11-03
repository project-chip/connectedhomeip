#
#   Copyright (c) 2025 Project CHIP Authors
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
# CMakeLists.txt for building uriparser as a library
# This can be used with ESP-IDF via add_subdirectory
#

if(NOT TARGET uriparser::uriparser)
    # Collect all source files
    file(GLOB URIPARSER_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/repo/src/*.c"
    )

    # Create static library
    add_library(uriparser STATIC ${URIPARSER_SOURCES})
    add_library(uriparser::uriparser ALIAS uriparser)

    # Set include directories
    target_include_directories(uriparser PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/repo/include"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/repo/include>"
    )

    target_include_directories(uriparser PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/repo/src"
        "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    # Add compile definitions matching BUILD.gn
    target_compile_definitions(uriparser PRIVATE
        URI_LIBRARY_BUILD
        URI_STATIC_BUILD
    )

    target_compile_definitions(uriparser PUBLIC
        URI_ENABLE_ANSI=1
        URI_ENABLE_UNICODE=1
    )

    # Disable warnings for uriparser source files
    target_compile_options(uriparser PRIVATE
        -Wno-unused-parameter
        -Wno-shadow
        -Wno-sign-conversion
    )

    set_target_properties(uriparser PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
    )
endif()
