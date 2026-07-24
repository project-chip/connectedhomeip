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
    "${CLUSTER_DIR}/CodegenIntegration.h"
    # Ember-free cross-cluster coupling implementations. They call CodegenDataModelProvider /
    # ScenesServer Instance()s, so they build with the app codegen model (here), not the core BUILD.gn.
    "${CLUSTER_DIR}/DefaultColorControlSceneHandler.h"
    "${CLUSTER_DIR}/LevelControlCoupling.cpp"
    "${CLUSTER_DIR}/SceneIntegration.cpp"
    "${CLUSTER_DIR}/SceneIntegration.h"
)