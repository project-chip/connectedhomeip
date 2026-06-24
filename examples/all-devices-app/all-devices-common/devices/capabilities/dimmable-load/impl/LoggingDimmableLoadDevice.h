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
#include <app/clusters/level-control/LevelControlDelegate.h>
#include <app/clusters/on-off-server/OnOffDelegate.h>
#include <app/clusters/on-off-server/OnOffEffectDelegate.h>
#include <devices/capabilities/dimmable-load/DimmableLoadDevice.h>

namespace chip {
namespace app {

/**
 * A simulator-friendly implementation of a DimmableLoadDevice that provides
 * automatic terminal logging for all cluster state transitions and operations.
 *
 * Beyond logging, this class implements the "Delegate Injection" pattern. It allows
 * the simulator or test harness to intercept and customize specific cluster behaviors
 * by passing custom delegate pointers in the constructor. If a custom delegate is
 * provided, cluster actions are forwarded to it; otherwise, they fall back to the
 * internal logging implementations.
 *
 * This enables:
 *   1. Out-of-the-box visibility into device state changes via terminal logs.
 *   2. Easy integration with hardware drivers or automated test mocks by injecting
 *      custom delegates for only the clusters of interest, without having to re-implement
 *      the entire device.
 *
 * It is intended as a base class for logging leaf simulator devices like
 * LoggingDimmableLightDevice, DimmablePlugInUnitDevice, and MountedDimmableLoadControlDevice.
 */
class LoggingDimmableLoadDevice : public DimmableLoadDevice,
                                  public Clusters::OnOffDelegate,
                                  public Clusters::LevelControlDelegate,
                                  public Clusters::OnOffEffectDelegate,
                                  public Clusters::IdentifyDelegate
{
public:
    LoggingDimmableLoadDevice(Span<const DataModel::DeviceTypeEntry> deviceTypes, const Context & context,
                              Clusters::OnOffDelegate * customOnOff        = nullptr,
                              Clusters::LevelControlDelegate * customLevel = nullptr,
                              Clusters::OnOffEffectDelegate * customEffect = nullptr,
                              Clusters::IdentifyDelegate * customIdentify  = nullptr);
    ~LoggingDimmableLoadDevice() override = default;

protected:
    // OnOffDelegate
    void OnOffStartup(bool on) override;
    void OnOnOffChanged(bool on) override;

    // LevelControlDelegate
    void OnLevelChanged(uint8_t level) override;
    void OnOptionsChanged(BitMask<Clusters::LevelControl::OptionsBitmap> options) override;
    void OnOnLevelChanged(DataModel::Nullable<uint8_t> onLevel) override;
    void OnDefaultMoveRateChanged(DataModel::Nullable<uint8_t> defaultMoveRate) override;

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
