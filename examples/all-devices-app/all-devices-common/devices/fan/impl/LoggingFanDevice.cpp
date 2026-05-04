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

#include <devices/fan/impl/LoggingFanDevice.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {
namespace {

bool IsFanSetForOn(const FanControl::FanDriveState & state)
{
    using FanControl::FanModeEnum;

    // NOTE: Automatically syncing the FanControl cluster's state (FanMode, PercentSetting,
    // SpeedSetting) to the OnOff cluster is a product/manufacturer choice.
    // The Matter SDK does not mandate that changing a fan speed setting implicitly
    // turns the device on. In this example application, we have chosen to implement
    // a behavior where any non-zero/active setting turns the OnOff cluster ON,
    // and turning all settings to zero/Off turns the OnOff cluster OFF.

    // 1. Check FanMode: If the mode is not Off, the fan should be considered ON.
    if (state.mode != FanModeEnum::kOff)
    {
        return true;
    }

    // 2. Check PercentSetting: If a percentage is set and it's greater than 0,
    // the manufacturer logic dictates the device should turn ON.
    if (!state.percentSetting.IsNull() && state.percentSetting.Value() > 0)
    {
        return true;
    }

    // 3. Check SpeedSetting: If a specific speed is set and it's greater than 0,
    // the manufacturer logic dictates the device should turn ON.
    if (!state.speedSetting.IsNull() && state.speedSetting.Value() > 0)
    {
        return true;
    }

    // If we reach this point, all relevant settings indicate the fan is off
    // (mode is kOff, and settings are 0 or Null).
    // Return false to explicitly turn the OnOff cluster OFF.
    return false;
}

void ApplyOnOffToFan(FanControlCluster & fan, bool on)
{
    if (!on)
    {
        fan.SetPercentCurrent(0);
        if (fan.GetFeatureMap().Has(FanControl::Feature::kMultiSpeed))
        {
            fan.SetSpeedCurrent(0);
        }
        return;
    }

    if (fan.GetFanMode() == FanControl::FanModeEnum::kOff)
    {
        return;
    }

    const auto percentSetting = fan.GetPercentSetting();
    if (!percentSetting.IsNull())
    {
        fan.SetPercentCurrent(percentSetting.Value());
    }
    if (fan.GetFeatureMap().Has(FanControl::Feature::kMultiSpeed))
    {
        const auto speedSetting = fan.GetSpeedSetting();
        if (!speedSetting.IsNull())
        {
            fan.SetSpeedCurrent(speedSetting.Value());
        }
    }
}

} // namespace

LoggingFanDevice::LoggingFanDevice(const Context & context) : FanDevice(*this, *this, context) {}

LoggingFanDevice::~LoggingFanDevice() {}

// FanControlDelegate

Protocols::InteractionModel::Status LoggingFanDevice::HandleStep(FanControl::StepDirectionEnum aDirection, bool aWrap,
                                                                 bool aLowestOff)
{
    ChipLogProgress(DeviceLayer, "LoggingFanDevice::HandleStep() -> direction=%u wrap=%d lowestOff=%d",
                    static_cast<unsigned>(to_underlying(aDirection)), aWrap, aLowestOff);

    auto & cluster = FanControlCluster();

    auto currentSpeedNullable = cluster.GetSpeedSetting();
    uint8_t currentSpeed      = currentSpeedNullable.IsNull() ? 0 : currentSpeedNullable.Value();
    uint8_t maxSpeed          = cluster.GetSpeedMax();
    uint8_t newSpeed          = currentSpeed;

    if (aDirection == FanControl::StepDirectionEnum::kIncrease)
    {
        if (currentSpeed < maxSpeed)
        {
            newSpeed++;
        }
        else if (aWrap)
        {
            newSpeed = aLowestOff ? 0 : 1;
        }
    }
    else if (aDirection == FanControl::StepDirectionEnum::kDecrease)
    {
        uint8_t lowestSpeed = aLowestOff ? 0 : 1;
        if (currentSpeed > lowestSpeed)
        {
            newSpeed--;
        }
        else if (aWrap)
        {
            newSpeed = maxSpeed;
        }
    }

    if (newSpeed != currentSpeed)
    {
        ChipLogProgress(DeviceLayer, "LoggingFanDevice::HandleStep() -> Speed changed from %u to %u", currentSpeed, newSpeed);
    }

    return cluster.SetSpeedSetting(DataModel::MakeNullable(newSpeed)).GetStatusCode().GetStatus();
}

void LoggingFanDevice::OnFanDriveStateChanged(const FanControl::FanDriveState & newState)
{
    const unsigned mode             = static_cast<unsigned>(to_underlying(newState.mode));
    const unsigned percentCurrent   = static_cast<unsigned>(newState.percentCurrent);
    const unsigned speedCurrent     = static_cast<unsigned>(newState.speedCurrent);
    const bool percentSettingIsNull = newState.percentSetting.IsNull();
    const bool speedSettingIsNull   = newState.speedSetting.IsNull();

    if (!percentSettingIsNull && !speedSettingIsNull)
    {
        ChipLogProgress(DeviceLayer,
                        "LoggingFanDevice::OnFanDriveStateChanged() -> mode=%u percentSetting=%u percentCurrent=%u speedSetting=%u "
                        "speedCurrent=%u",
                        mode, static_cast<unsigned>(newState.percentSetting.Value()), percentCurrent,
                        static_cast<unsigned>(newState.speedSetting.Value()), speedCurrent);
    }
    else if (!percentSettingIsNull)
    {
        ChipLogProgress(
            DeviceLayer,
            "LoggingFanDevice::OnFanDriveStateChanged() -> mode=%u percentSetting=%u percentCurrent=%u speedSetting=NULL "
            "speedCurrent=%u",
            mode, static_cast<unsigned>(newState.percentSetting.Value()), percentCurrent, speedCurrent);
    }
    else if (!speedSettingIsNull)
    {
        ChipLogProgress(
            DeviceLayer,
            "LoggingFanDevice::OnFanDriveStateChanged() -> mode=%u percentSetting=NULL percentCurrent=%u speedSetting=%u "
            "speedCurrent=%u",
            mode, percentCurrent, static_cast<unsigned>(newState.speedSetting.Value()), speedCurrent);
    }
    else
    {
        ChipLogProgress(
            DeviceLayer,
            "LoggingFanDevice::OnFanDriveStateChanged() -> mode=%u percentSetting=NULL percentCurrent=%u speedSetting=NULL "
            "speedCurrent=%u",
            mode, percentCurrent, speedCurrent);
    }

    auto & onOff         = OnOffCluster();
    const bool powered   = onOff.GetOnOff();
    if (!powered)
    {
        ApplyOnOffToFan(FanControlCluster(), false);
    }
    const bool setForOn = IsFanSetForOn(newState);
    if (setForOn && !powered)
    {
        return;
    }
    LogErrorOnFailure(onOff.SetOnOff(setForOn));
}

void LoggingFanDevice::OnRockSettingChanged(BitMask<FanControl::RockBitmap> newValue)
{
    ChipLogProgress(DeviceLayer, "LoggingFanDevice::OnRockSettingChanged() -> 0x%02X", newValue.Raw());
}

void LoggingFanDevice::OnWindSettingChanged(BitMask<FanControl::WindBitmap> newValue)
{
    ChipLogProgress(DeviceLayer, "LoggingFanDevice::OnWindSettingChanged() -> 0x%02X", newValue.Raw());
}

void LoggingFanDevice::OnAirflowDirectionChanged(FanControl::AirflowDirectionEnum newValue)
{
    ChipLogProgress(DeviceLayer, "LoggingFanDevice::OnAirflowDirectionChanged() -> %u",
                    static_cast<unsigned>(to_underlying(newValue)));
}

// OnOffDelegate

void LoggingFanDevice::OnOffStartup(bool on)
{
    ApplyOnOffToFan(FanControlCluster(), on);
    ChipLogProgress(DeviceLayer, "LoggingFanDevice::OnOffStartup() -> %s", on ? "ON" : "OFF");
}

void LoggingFanDevice::OnOnOffChanged(bool on)
{
    ApplyOnOffToFan(FanControlCluster(), on);
    ChipLogProgress(DeviceLayer, "LoggingFanDevice::OnOffChanged() -> %s", on ? "ON" : "OFF");
}

} // namespace app
} // namespace chip
