#
#   Copyright (c) 2020 Project CHIP Authors
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

#
#   @file
#         CMake module for incorporating CHIP into an nRF Connect SDK
#         application as an out-of-tree Zephyr module.
#

# Use Device Tree overlay if exists in the application directory

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dts.overlay")
    set(DTC_OVERLAY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/dts.overlay")
endif()

# Add CHIP to the list of Zephyr modules

list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_LIST_DIR}/../chip-module)

# Load NCS/Zephyr build system

find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

# Configure the application

target_link_libraries(app PUBLIC chip)
