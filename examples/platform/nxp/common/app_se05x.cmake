#
# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# ****************************************************************
# SE05X Application Files
# ****************************************************************

if(CONFIG_CHIP_SE05X)
    list(FIND EXTRA_MCUX_MODULES "${CHIP_ROOT}/third_party/simw-top-mini/repo/matter" se_index)
    if(se_index EQUAL -1)
        message(FATAL_ERROR "MCUX_MODULES must include ${CHIP_ROOT}/third_party/simw-top-mini/repo/matter in the application when CONFIG_CHIP_SE05X is enabled")
    endif()

    target_include_directories(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_se05x/include
    )

    target_sources(app PRIVATE
        ${EXAMPLE_PLATFORM_NXP_COMMON_DIR}/app_se05x/source/AppSe05x.cpp
    )

endif()
