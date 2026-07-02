/*
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

#include <app/clusters/fan-control-server/fan-control-delegate.h>
#include <app/clusters/on-off-server/OnOffDelegate.h>
#include <devices/capabilities/fan-load/FanLoadDevice.h>

namespace chip {
namespace app {

/**
 * Concrete implementation of FanLoadDevice that logs all fan and power state
 * transitions, with support for custom delegate injection.
 */
class LoggingFanLoadDevice : public FanLoadDevice, public Clusters::OnOffDelegate, public Clusters::FanControl::Delegate
{
public:
    LoggingFanLoadDevice(Span<const DataModel::DeviceTypeEntry> deviceTypes, const Context & context,
                         Clusters::FanControl::Delegate * customFan = nullptr, Clusters::OnOffDelegate * customOnOff = nullptr);

    ~LoggingFanLoadDevice() override = default;

protected:
    // FanControl::Delegate
    Protocols::InteractionModel::Status HandleStep(Clusters::FanControl::StepDirectionEnum aDirection, bool aWrap,
                                                   bool aLowestOff) override;
    void OnFanDriveStateChanged(const Clusters::FanControl::FanDriveState & newState) override;
    void OnRockSettingChanged(BitMask<Clusters::FanControl::RockBitmap> newValue) override;
    void OnWindSettingChanged(BitMask<Clusters::FanControl::WindBitmap> newValue) override;
    void OnAirflowDirectionChanged(Clusters::FanControl::AirflowDirectionEnum newValue) override;

    // OnOffDelegate
    void OnOffStartup(bool on) override;
    void OnOnOffChanged(bool on) override;
};

} // namespace app
} // namespace chip
