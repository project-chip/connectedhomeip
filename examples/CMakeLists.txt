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
#      This file is the CMake template for building CHIP SDK examples from the top-level.
#

if(CHIP_TARGET_STYLE_UNIX)
add_subdirectory(shell)
endif(CHIP_TARGET_STYLE_UNIX)

if(${CHIP_PLATFORM} STREQUAL "nrf52840")
add_subdirectory(lock-app/nrf5)
endif()