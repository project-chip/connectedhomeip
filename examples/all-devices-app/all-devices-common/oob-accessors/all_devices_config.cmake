#
#    Copyright (c) 2026 Project CHIP Authors
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

if(NOT DEFINED ALL_DEVICES_APP_ENABLE_NAMED_PIPES)
    set(ALL_DEVICES_APP_ENABLE_NAMED_PIPES 0)
endif()

if(NOT DEFINED ALL_DEVICES_APP_ENABLE_OOB_ACCESSORS)
    set(ALL_DEVICES_APP_ENABLE_OOB_ACCESSORS 0)
endif()

if(NOT CMAKE_BUILD_EARLY_EXPANSION)
    configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/all_devices_config.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/app_config/all_devices_config.h"
    )
endif()
