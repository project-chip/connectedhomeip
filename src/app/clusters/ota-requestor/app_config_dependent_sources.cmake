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
    "${CLUSTER_DIR}/BDXDownloader.cpp"
    "${CLUSTER_DIR}/BDXDownloader.h"
    "${CLUSTER_DIR}/DefaultOTARequestor.cpp"
    "${CLUSTER_DIR}/DefaultOTARequestor.h"
    "${CLUSTER_DIR}/DefaultOTARequestorDriver.cpp"
    "${CLUSTER_DIR}/DefaultOTARequestorDriver.h"
    "${CLUSTER_DIR}/DefaultOTARequestorStorage.cpp"
    "${CLUSTER_DIR}/DefaultOTARequestorStorage.h"
    "${CLUSTER_DIR}/DefaultOTARequestorUserConsent.h"
    "${CLUSTER_DIR}/ExtendedOTARequestorDriver.cpp"
    "${CLUSTER_DIR}/ExtendedOTARequestorDriver.h"
    "${CLUSTER_DIR}/OTARequestorDriver.h"
    "${CLUSTER_DIR}/OTARequestorStorage.h"
    "${CLUSTER_DIR}/OTATestEventTriggerHandler.cpp"
    "${CLUSTER_DIR}/OTATestEventTriggerHandler.h"
    "${CLUSTER_DIR}/ota-requestor-server.cpp"
    "${CLUSTER_DIR}/ota-requestor-server.h"
)