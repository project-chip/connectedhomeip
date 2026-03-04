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
    "${CLUSTER_DIR}/CodegenIntegration.cpp"
    "${CLUSTER_DIR}/PresetStructWithOwnedMembers.cpp"
    "${CLUSTER_DIR}/PresetStructWithOwnedMembers.h"
    "${CLUSTER_DIR}/ThermostatDelegate.h"
    "${CLUSTER_DIR}/ThermostatClusterAtomic.cpp"
    "${CLUSTER_DIR}/ThermostatClusterPresets.cpp"
    "${CLUSTER_DIR}/ThermostatClusterPresets.h"
    "${CLUSTER_DIR}/ThermostatClusterRead.cpp"
    "${CLUSTER_DIR}/ThermostatClusterSetpoints.cpp"
    "${CLUSTER_DIR}/ThermostatClusterSetpoints.h"
    "${CLUSTER_DIR}/ThermostatClusterSuggestions.cpp"
    "${CLUSTER_DIR}/ThermostatClusterWrite.cpp"
    "${CLUSTER_DIR}/ThermostatCluster.cpp"
    "${CLUSTER_DIR}/ThermostatCluster.h"
    "${CLUSTER_DIR}/ThermostatSuggestionStructWithOwnedMembers.cpp"
    "${CLUSTER_DIR}/ThermostatSuggestionStructWithOwnedMembers.h"
)