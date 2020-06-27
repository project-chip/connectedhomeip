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
#      This file is the CHIP SDK CMake template for defining toolchain-specific build parameters
#      and activating modular build system extension modules.
#

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
include("${PROJECT_SOURCE_DIR}/integrations/cmake/platforms.cmake")
endif ()

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

#
# Enable modular feature extensions to the cmake build system
#
include("${PROJECT_SOURCE_DIR}/integrations/cmake/features/coverage.cmake")
include("${PROJECT_SOURCE_DIR}/integrations/cmake/features/doxygen.cmake")
include("${PROJECT_SOURCE_DIR}/integrations/cmake/features/pretty.cmake")
