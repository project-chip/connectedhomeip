# Copyright (c) 2025 Project CHIP Authors
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

# This is the equivalent to app_config_dependent_sources.gni
TARGET_SOURCES(
  ${APP_TARGET}
  PRIVATE
    "${CLUSTER_DIR}/icd-management-server.cpp"
    "${CLUSTER_DIR}/icd-management-server.h"
)

# TODO(#32321): Remove after issue is resolved
# Add ICDConfigurationData when ICD management server cluster is included,
# but ICD support is disabled, e.g. lock-app on some platforms
if(NOT CONFIG_CHIP_ENABLE_ICD_SUPPORT)
   target_sources(${APP_TARGET} PRIVATE ${CHIP_APP_BASE_DIR}/icd/server/ICDConfigurationData.cpp)
endif()
