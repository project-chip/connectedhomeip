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
    "${CLUSTER_DIR}/CodegenAttributeValuePairValidator.cpp"
    "${CLUSTER_DIR}/CodegenAttributeValuePairValidator.h"
    "${CLUSTER_DIR}/CodegenEndpointToIndex.h"
    "${CLUSTER_DIR}/scenes-server.cpp"
    "${CLUSTER_DIR}/scenes-server.h"

    # these below would be pulled by GN (.gni file) dependency. CMAKE needs these directly
    "${CLUSTER_DIR}/AttributeValuePairValidator.h"
    "${CLUSTER_DIR}/ExtensionFieldSets.h"
    "${CLUSTER_DIR}/ExtensionFieldSetsImpl.cpp"
    "${CLUSTER_DIR}/ExtensionFieldSetsImpl.h"
    "${CLUSTER_DIR}/SceneHandlerImpl.cpp"
    "${CLUSTER_DIR}/SceneHandlerImpl.h"
    "${CLUSTER_DIR}/SceneTable.h"
    "${CLUSTER_DIR}/SceneTableImpl.cpp"
    "${CLUSTER_DIR}/SceneTableImpl.h"
)
