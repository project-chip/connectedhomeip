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
#      This file is the CMake template for the CHIP SDK system layer library.
#

add_subdirectory(tests)

add_library(SystemLayer)

set(COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/lib
    ${PROJECT_SOURCE_DIR}/src/lib/core
    ${PROJECT_SOURCE_DIR}/src/system
    ${PROJECT_SOURCE_DIR}/third_party/lwip/repo/lwip/src/include
    ${PROJECT_SOURCE_DIR}/third_party/nlfaultinjection/repo/include
)

set(COMMON_SOURCES
    SystemClock.cpp
    SystemError.cpp
    SystemLayer.cpp
    SystemMutex.cpp
    SystemObject.cpp
    SystemTimer.cpp
    SystemPacketBuffer.cpp
    SystemStats.cpp
    SystemFaultInjection.cpp
)

target_compile_definitions(SystemLayer PRIVATE)
target_compile_options(SystemLayer PRIVATE)
target_include_directories(SystemLayer PRIVATE ${COMMON_INCLUDES})
target_sources(SystemLayer PRIVATE ${COMMON_SOURCES})

target_link_libraries(SystemLayer PRIVATE chip-config)