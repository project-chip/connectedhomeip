/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/on-off-server/OnOffDelegate.h>
#include <app/clusters/on-off-server/OnOffEffectDelegate.h>
#include <devices/capabilities/on-off-load/OnOffLoadDevice.h>

namespace chip {
namespace app {

/**
 * A simulator-friendly implementation of an OnOffLoadDevice that provides
 * automatic terminal logging for all cluster state transitions and operations.
 *
 * Supports the "Delegate Injection" pattern, allowing the simulator or test harness
 * to inject custom delegate pointers (OnOff, OnOffEffect, Identify) in the constructor
 * to override default behaviors, falling back to internal logging for any delegates
 * left as nullptr.
 *
 * This provides a balance between out-of-the-box observability (via logs) and
 * high customizability for integration testing and driver bindings.
 *
 * It is intended as a base class for logging leaf simulator devices like
 * LoggingOnOffLightDevice, OnOffPlugInUnitDevice, and MountedOnOffControlDevice.
 */
class LoggingOnOffLoadDevice : public OnOffLoadDevice,
                               public Clusters::OnOffDelegate,
                               public Clusters::OnOffEffectDelegate,
                               public Clusters::IdentifyDelegate
{
public:
    LoggingOnOffLoadDevice(Span<const DataModel::DeviceTypeEntry> deviceTypes, const Context & context,
                           Clusters::OnOffDelegate * customOnOff = nullptr, Clusters::OnOffEffectDelegate * customEffect = nullptr,
                           Clusters::IdentifyDelegate * customIdentify = nullptr);
    ~LoggingOnOffLoadDevice() override = default;

protected:
    // OnOffDelegate
    void OnOffStartup(bool on) override;
    void OnOnOffChanged(bool on) override;

    // OnOffEffectDelegate
    DataModel::ActionReturnStatus TriggerDelayedAllOff(Clusters::OnOff::DelayedAllOffEffectVariantEnum e) override;
    DataModel::ActionReturnStatus TriggerDyingLight(Clusters::OnOff::DyingLightEffectVariantEnum e) override;

    // IdentifyDelegate
    void OnIdentifyStart(Clusters::IdentifyCluster & cluster) override;
    void OnIdentifyStop(Clusters::IdentifyCluster & cluster) override;
    void OnTriggerEffect(Clusters::IdentifyCluster & cluster) override;
    bool IsTriggerEffectEnabled() const override;
};

} // namespace app
} // namespace chip
