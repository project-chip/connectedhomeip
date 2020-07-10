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
#      CMake configuration for the Silicon Labs EFR32MG12 platform.
# 

include("${PROJECT_SOURCE_DIR}/integrations/cmake/toolchains/arm-none-eabi.cmake")

#
# Define chip-config interface library to provide defines and include paths for platform
#

add_library(chip-config INTERFACE)

target_compile_definitions(chip-config INTERFACE
    "CHIP_SYSTEM_CONFIG_USE_SOCKETS=0"
    "CHIP_SYSTEM_CONFIG_USE_LWIP=1"
)

if(NOT DEFINED LINKER_SCRIPT)
    message(FATAL_ERROR "No linker script defined")
endif(NOT DEFINED LINKER_SCRIPT)
message("Linker script: ${LINKER_SCRIPT}")

set(PLATFORM_MACH_FLAGS "-mcpu=cortex-m4 -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv5-sp-d16")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${PLATFORM_MACH_FLAGS}" CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PLATFORM_MACH_FLAGS}" CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${PLATFORM_MACH_FLAGS}" CACHE INTERNAL "ASM Compiler options")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -T${LINKER_SCRIPT}" CACHE INTERNAL "Linker options")
