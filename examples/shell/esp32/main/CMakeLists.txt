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
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

set(CHIP_SHELL_DIR "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/shell")


idf_component_register(SRCS main.cpp
                      "${CHIP_SHELL_DIR}/shell_common/globals.cpp"
                      "${CHIP_SHELL_DIR}/shell_common/cmd_ping.cpp"
                      "${CHIP_SHELL_DIR}/shell_common/cmd_send.cpp"                      
                      PRIV_INCLUDE_DIRS
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/src"
                      "${CHIP_SHELL_DIR}/shell_common/include"
                      PRIV_REQUIRES chip nvs_flash bt console esp32_mbedtls)
