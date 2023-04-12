#
#   Copyright (c) 2022 Project CHIP Authors
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
#     CMake for CHIP library configuration
#

# Default CHIP build configuration 
set(CONFIG_CHIP_PROJECT_CONFIG "main/include/CHIPProjectConfig.h" CACHE STRING "")
set(CONFIG_CHIP_LIB_TESTS NO CACHE BOOL "")
set(CONFIG_CHIP_LIB_SHELL NO CACHE BOOL "")

set(CONFIG_CHIP_DETAIL_LOGGING YES CACHE BOOL "Enable logging at detail level")
set(CONFIG_CHIP_PROGRESS_LOGGING YES CACHE BOOL "Enable logging at progress level")
set(CONFIG_CHIP_AUTOMATION_LOGGING YES CACHE BOOL "Enable logging at automation level")
set(CONFIG_CHIP_ERROR_LOGGING YES CACHE BOOL "Enable logging at error level")

# Add CHIP sources
add_subdirectory(${OPEN_IOT_SDK_CONFIG} ./chip_build)
