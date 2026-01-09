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

#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/general-diagnostics-server/GeneralDiagnosticsCluster.h>
#include <system/SystemClock.h>

namespace chip::app::Clusters::GeneralDiagnostics {
void GlobalNotifyDeviceReboot(GeneralDiagnostics::BootReasonEnum bootReason);

void GlobalNotifyHardwareFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & previous,
    const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & current);

void GlobalNotifyRadioFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & previous,
    const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & current);

void GlobalNotifyNetworkFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & previous,
    const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & current);

/// Set the TestEventTriggerDelegate for dependency injection.
/// This should be called before MatterGeneralDiagnosticsClusterInitCallback.
void SetTestEventTriggerDelegate(TestEventTriggerDelegate * delegate);

/// Get the TestEventTriggerDelegate that was set via SetTestEventTriggerDelegate.
/// Returns nullptr if not set.
TestEventTriggerDelegate * GetTestEventTriggerDelegate();

/// Set the initialization timestamp for dependency injection.
/// This should be called before MatterGeneralDiagnosticsClusterInitCallback.
void SetInitTimestamp(System::Clock::Microseconds64 initTimestamp);

/// Get the initialization timestamp that was set via SetInitTimestamp.
/// Returns 0 if not set.
System::Clock::Microseconds64 GetInitTimestamp();
} // namespace chip::app::Clusters::GeneralDiagnostics
