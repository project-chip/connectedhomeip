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
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

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
        cluster.SetSpeedSetting(DataModel::MakeNullable(newSpeed));

        if (currentSpeed == 0 && newSpeed > 0)
        {
            cluster.SetOnOffState(true);
        }
        else if (newSpeed == 0)
        {
            cluster.SetOnOffState(false);
        }
    }

    ChipLogProgress(DeviceLayer, "LoggingFanDevice::HandleStep() -> Speed changed from %u to %u", currentSpeed, newSpeed);

    return Protocols::InteractionModel::Status::Success;
}

void LoggingFanDevice::OnFanStateChanged(bool isOn)
{
    ChipLogProgress(DeviceLayer, "LoggingFanDevice::OnFanStateChanged() -> %s", isOn ? "ON" : "OFF");
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
    ChipLogProgress(DeviceLayer, "LoggingFanDevice::OnOffStartup() -> %s", on ? "ON" : "OFF");
    FanControlCluster().SetOnOffState(on);
}

void LoggingFanDevice::OnOnOffChanged(bool on)
{
    ChipLogProgress(DeviceLayer, "LoggingFanDevice::OnOffChanged() -> %s", on ? "ON" : "OFF");
    FanControlCluster().SetOnOffState(on);
}

} // namespace app
} // namespace chip
