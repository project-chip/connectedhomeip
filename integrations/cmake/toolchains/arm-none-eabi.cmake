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
#      This file is the CHIP SDK CMake template for defining the arm-none-eabi toolchain.
#

include (CMakeForceCompiler)

# We are cross compiling so we don't want compiler tests to run, as they will fail
set(CMAKE_CROSSCOMPILING    1)

set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_PROCESSOR  ARM)

set(ARM_GCC_INSTALL_ROOT    $ENV{ARM_GCC_INSTALL_ROOT})
set(CROSSCOMPILE_PREFIX     "${ARM_GCC_INSTALL_ROOT}/arm-none-eabi-")

set(CMAKE_C_COMPILER        "${CROSSCOMPILE_PREFIX}gcc")
set(CMAKE_CXX_COMPILER      "${CROSSCOMPILE_PREFIX}g++")
set(CMAKE_ASM_COMPILER      "${CROSSCOMPILE_PREFIX}gcc")   # Pass: -x assembler-with-cpp
set(CMAKE_LINKER            "${CROSSCOMPILE_PREFIX}g++")   # Always link C++ with g++ rather than gcc or ld
