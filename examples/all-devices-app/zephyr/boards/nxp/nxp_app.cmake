#
#    Copyright (c) 2026 Project CHIP Authors
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

# NXP application integration shared by all NXP boards: NXP AppTask and the
# NXP factory data providers.

set(NXP_EXAMPLE_COMMON_DIR ${CHIP_ROOT}/examples/platform/nxp/common)
set(NXP_EXAMPLE_ZEPHYR_DIR ${CHIP_ROOT}/examples/platform/nxp/zephyr)

# Use the NXP AppTask unless the user provides their own.
if(APP_TASK_IMPL_HEADER STREQUAL "")
    set(APP_TASK_IMPL_HEADER "NxpAppTask.h")
endif()

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${NXP_EXAMPLE_COMMON_DIR}/factory_data/include
)

target_sources(app PRIVATE
    ${NXP_EXAMPLE_ZEPHYR_DIR}/factory_data/source/AppFactoryDataExample.cpp
)

if(CONFIG_CHIP_FACTORY_DATA)
    target_sources(app PRIVATE
        ${CHIP_ROOT}/src/platform/nxp/zephyr/FactoryDataProviderImpl.cpp
        ${CHIP_ROOT}/src/platform/nxp/common/factory_data/legacy/FactoryDataProvider.cpp
    )
endif()
