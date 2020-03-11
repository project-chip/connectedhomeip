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

cmake_minimum_required(VERSION 3.17.2)

project(ProjectChip C CXX ASM)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CHIP_BUILD_EXECUTABLES "Build executables" ON)

include("${PROJECT_SOURCE_DIR}/integrations/cmake/checks.cmake")
include("${PROJECT_SOURCE_DIR}/integrations/cmake/options.cmake")
include("${PROJECT_SOURCE_DIR}/integrations/cmake/coverage.cmake")
include("${PROJECT_SOURCE_DIR}/integrations/cmake/pretty.cmake")
include("${PROJECT_SOURCE_DIR}/integrations/cmake/platforms.cmake")


if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)

   ## TODO: Make toolchain selection configurable
   #set(CMAKE_C_COMPILER /usr/bin/gcc)
   #set(CMAKE_CXX_COMPILER /usr/bin/g++)

   #set(CMAKE_C_COMPILER clang)
   #set(CMAKE_CXX_COMPILER clang++)
   #set(CMAKE_C_COMPILER_TARGET ${triple})
   #set(CMAKE_CXX_COMPILER_TARGET ${triple})

   ## TODO: Make platform selection configurable

endif ()


## TODO: Enable testing only on simulation platform
include(CTest)
enable_testing()


if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
    option(CHIP_COMPILE_WARNING_AS_ERROR "whether to include -Werror with gcc-compatible compilers")
    if (CHIP_COMPILE_WARNING_AS_ERROR)
        set(CHIP_CFLAGS -Werror)
    endif()

    set(CHIP_CFLAGS
        $<$<COMPILE_LANGUAGE:C>:${CHIP_CFLAGS} -Wall -Wextra -Wshadow -Werror -pedantic-errors>
        $<$<COMPILE_LANGUAGE:CXX>:${CHIP_FLAGS} -Wall -Wextra -Wshadow -Wno-c++14-compat -fno-exceptions>
    )
endif()

file(READ .default-version CHIP_DEFAULT_VERSION)
execute_process(
    COMMAND bash "-c" "third_party/nlbuild-autotools/repo/scripts/mkversion -b ${CHIP_DEFAULT_VERSION}"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE CHIP_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Version: ${CHIP_VERSION}")

list(APPEND CHIP_PRIVATE_DEFINES
    "PACKAGE_NAME=\"ProjectChip\""
    "PACKAGE_VERSION=\"${CHIP_VERSION}\""
)

# CHIP_CONFIG allows users to specify the path to ProjectChip project core
# config header file. The default value of this parameter is empty string.
# When not specified by user (value is ""), a platform cmake file may
# choose to change this variable to provide its own core config header
# file instead.

set(CHIP_CONFIG "" CACHE STRING "CHIP project-specific config header file chosen by user at configure time")

list(APPEND CHIP_PUBLIC_INCLUDES ${PROJECT_BINARY_DIR}/integrations/cmake)
list(APPEND CHIP_PUBLIC_INCLUDES ${PROJECT_SOURCE_DIR}/integrations/cmake)
list(APPEND CHIP_PUBLIC_INCLUDES ${PROJECT_SOURCE_DIR}/src/include)

if(CHIP_CONFIG)
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_PROJECT_CORE_CONFIG_FILE=\"${CHIP_CONFIG}\"")
    message(STATUS "Project core config: \"${CHIP_CONFIG}\"")
endif()

list(APPEND CHIP_PRIVATE_DEFINES ${CHIP_PLATFORM_DEFINES})

add_subdirectory(examples)
add_subdirectory(src)
add_subdirectory(third_party)
