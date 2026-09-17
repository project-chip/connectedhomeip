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

namespace chip {
namespace app {

class FanLoad;

/**
 * Reusable logging delegate providing terminal logging and default state synchronization
 * for fan control and on/off power transitions.
 */
class LoggingFanDelegate : public Clusters::OnOffDelegate, public Clusters::FanControl::Delegate
{
public:
    explicit LoggingFanDelegate(FanLoad * fanLoad = nullptr) : mFanLoad(fanLoad) {}
    ~LoggingFanDelegate() override = default;

    void SetFanLoad(FanLoad * fanLoad) { mFanLoad = fanLoad; }

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

private:
    FanLoad * mFanLoad = nullptr;
};

} // namespace app
} // namespace chip
