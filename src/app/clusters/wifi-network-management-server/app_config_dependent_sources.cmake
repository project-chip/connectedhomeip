# Copyright (c) 2025-2026 Project CHIP Authors
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

# This block adds the codegen integration sources
TARGET_SOURCES(
  ${APP_TARGET}
  PRIVATE
    "${CLUSTER_DIR}/CodegenIntegration.cpp"
    "${CLUSTER_DIR}/CodegenIntegration.h"
    "${CLUSTER_DIR}/wifi-network-management-server.h"
)

# These are the things that BUILD.gn dependencies would pull
TARGET_SOURCES(
  ${APP_TARGET}
  PRIVATE
    "${CLUSTER_DIR}/WiFiNetworkManagementCluster.cpp"
    "${CLUSTER_DIR}/WiFiNetworkManagementCluster.h"
)
