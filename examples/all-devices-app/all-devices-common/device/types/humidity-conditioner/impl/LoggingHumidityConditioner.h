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

#include <device/types/humidity-conditioner/HumidityConditioner.h>

namespace chip {
namespace app {

class LoggingHumidityConditioner : public HumidityConditioner, public Clusters::HumidistatDelegate
{
public:
    explicit LoggingHumidityConditioner(TimerDelegate & timerDelegate);
    ~LoggingHumidityConditioner() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;

    void OnModeChanged(Clusters::Humidistat::ModeEnum newMode) override;
    void OnSystemStateChanged(Clusters::Humidistat::SystemStateEnum newSystemState) override;
    void OnUserSetpointChanged(chip::Percent newUserSetpoint) override;
    void OnTargetSetpointChanged(chip::Percent newTargetSetpoint) override;
    void OnMistTypeChanged(chip::BitMask<Clusters::Humidistat::MistTypeBitmap> newMistType) override;
    void OnContinuousChanged(bool newContinuous) override;
    void OnSleepChanged(bool newSleep) override;
    void OnOptimalChanged(bool newOptimal) override;
    void OnCondPumpEnabledChanged(bool newCondPumpEnabled) override;
    void OnCondRunCountChanged(uint16_t newCondRunCount) override;
};

} // namespace app
} // namespace chip
