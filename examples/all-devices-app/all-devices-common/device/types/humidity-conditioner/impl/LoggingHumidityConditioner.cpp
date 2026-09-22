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

#include <device/types/humidity-conditioner/impl/LoggingHumidityConditioner.h>

#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip::app {

// LoggingHumidityConditioner

LoggingHumidityConditioner::LoggingHumidityConditioner(TimerDelegate & timerDelegate) :
    HumidityConditioner(HumidityConditioner::Config{
        .timerDelegate      = timerDelegate,
        .identifyDelegate   = *this,
        .onOffDelegate      = *this,
        .humidistatDelegate = *this,
    })
{}

void LoggingHumidityConditioner::OnModeChanged(Humidistat::ModeEnum newMode)
{
    ChipLogProgress(DeviceLayer, "HumidityConditioner: OnModeChanged -> %u", static_cast<unsigned>(newMode));
}

void LoggingHumidityConditioner::OnSystemStateChanged(Humidistat::SystemStateEnum newSystemState)
{
    ChipLogProgress(DeviceLayer, "HumidityConditioner: OnSystemStateChanged -> %u", static_cast<unsigned>(newSystemState));
}

void LoggingHumidityConditioner::OnUserSetpointChanged(chip::Percent newUserSetpoint)
{
    ChipLogProgress(DeviceLayer, "HumidityConditioner: OnUserSetpointChanged -> %u%%", newUserSetpoint);
}

void LoggingHumidityConditioner::OnTargetSetpointChanged(chip::Percent newTargetSetpoint)
{
    ChipLogProgress(DeviceLayer, "HumidityConditioner: OnTargetSetpointChanged -> %u%%", newTargetSetpoint);
}

void LoggingHumidityConditioner::OnMistTypeChanged(chip::BitMask<Humidistat::MistTypeBitmap> newMistType)
{
    ChipLogProgress(DeviceLayer, "HumidityConditioner: OnMistTypeChanged -> 0x%02x", newMistType.Raw());
}

void LoggingHumidityConditioner::OnContinuousChanged(bool newContinuous)
{
    ChipLogProgress(DeviceLayer, "HumidityConditioner: OnContinuousChanged -> %s", newContinuous ? "true" : "false");
}

void LoggingHumidityConditioner::OnSleepChanged(bool newSleep)
{
    ChipLogProgress(DeviceLayer, "HumidityConditioner: OnSleepChanged -> %s", newSleep ? "true" : "false");
}

void LoggingHumidityConditioner::OnOptimalChanged(bool newOptimal)
{
    ChipLogProgress(DeviceLayer, "HumidityConditioner: OnOptimalChanged -> %s", newOptimal ? "true" : "false");
}

void LoggingHumidityConditioner::OnCondPumpEnabledChanged(bool newCondPumpEnabled)
{
    ChipLogProgress(DeviceLayer, "HumidityConditioner: OnCondPumpEnabledChanged -> %s", newCondPumpEnabled ? "true" : "false");
}

void LoggingHumidityConditioner::OnCondRunCountChanged(uint16_t newCondRunCount)
{
    ChipLogProgress(DeviceLayer, "HumidityConditioner: OnCondRunCountChanged -> %u", newCondRunCount);
}

} // namespace chip::app
