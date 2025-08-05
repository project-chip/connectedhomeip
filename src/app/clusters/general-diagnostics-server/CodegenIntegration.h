/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <app/clusters/general-diagnostics-server/general-diagnostics-cluster.h>

namespace chip::app::Clusters::GeneralDiagnostics {
void GlobalNotifyDeviceReboot(GeneralDiagnostics::BootReasonEnum bootReason);

void GlobalNotifyHardwareFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & previous,
                                      const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & current);

void GlobalNotifyRadioFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & previous,
                                   const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & current);

void GlobalNotifyNetworkFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & previous,
                                     const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & current);
} // namespace chip::app::Clusters::GeneralDiagnostics
