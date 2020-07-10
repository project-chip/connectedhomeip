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
#      CMake configuration for the Espressif ESP32 platform.
# 

#
# Define chip-config interface library to provide defines and include paths for platform
#

add_library(chip-config INTERFACE)

target_compile_definitions(chip-config INTERFACE
    "CHIP_SYSTEM_CONFIG_USE_SOCKETS=0"
    "CHIP_SYSTEM_CONFIG_USE_LWIP=1"
)
