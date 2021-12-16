#
#    Copyright (c) 2021 Project CHIP Authors
#    All rights reserved.
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

# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../../examples/common/cmake/idf_flashing.cmake)

set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/third_party/connectedhomeip/config/esp32/components"
)

project(test-driver)

# C++17 is required for RPC build.
idf_build_set_property(CXX_COMPILE_OPTIONS "-std=gnu++17;-Os;-DLWIP_IPV6_SCOPES=0;-DCHIP_HAVE_CONFIG_H" APPEND)
idf_build_set_property(C_COMPILE_OPTIONS "-Os;-DLWIP_IPV6_SCOPES=0" APPEND)

# FIXME: elfbuilder ?
#    mkdir -p build/chip/
#    echo "#!/bin/sh" > build/chip/esp32_elf_builder.sh
#    echo set -e >> build/chip/esp32_elf_builder.sh
#    echo set -x >> build/chip/esp32_elf_builder.sh
#    echo $(CXX) $(CXXFLAGS) $(CPPFLAGS) -L$(PROJECT_PATH)/build/chip/lib -Wl,--whole-archive '$$1' -Wl,--no-whole-archive \
#         -lnlunit-test $(LDFLAGS) -lnlfaultinjection '$$2' -o $(PROJECT_PATH)/build/chip-tests.elf -Wl,-Map=$(APP_MAP) >> build/chip/esp32_elf_builder.sh
#    echo $(ESPTOOLPY) elf2image $(ESPTOOL_FLASH_OPTIONS) $(ESPTOOL_ELF2IMAGE_OPTIONS) \
#         -o $(PROJECT_PATH)/build/chip/chip-tests.bin $(PROJECT_PATH)/build/chip-tests.elf >> build/chip/esp32_elf_builder.sh
#    ln -sf $(PROJECT_PATH)/build/partitions.bin $(PROJECT_PATH)/build/chip/partitions.bin
#    mkdir -p build/chip/bootloader
#    ln -sf $(PROJECT_PATH)/build/bootloader/bootloader.bin $(PROJECT_PATH)/build/chip/bootloader.bin
#    ln -sf $(PROJECT_PATH)/idf.sh $(PROJECT_PATH)/build/chip/env.sh

flashing_script()
