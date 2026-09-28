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

#include <device/capabilities/fan-load/impl/LoggingFanDelegate.h>

#include <app/clusters/fan-control-server/FanControlCluster.h>
#include <app/clusters/on-off-server/OnOffCluster.h>
#include <device/capabilities/fan-load/FanLoad.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

bool IsFanSetForOn(const FanControl::FanDriveState & state)
{
    using FanControl::FanModeEnum;

    if (state.mode != FanModeEnum::kOff)
    {
        return true;
    }

    if (!state.percentSetting.IsNull() && state.percentSetting.Value() > 0)
    {
        return true;
    }

    if (!state.speedSetting.IsNull() && state.speedSetting.Value() > 0)
    {
        return true;
    }

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

// FanControl::Delegate

Protocols::InteractionModel::Status LoggingFanDelegate::HandleStep(FanControl::StepDirectionEnum aDirection, bool aWrap,
                                                                   bool aLowestOff)
{
    ChipLogProgress(DeviceLayer, "LoggingFanDelegate::HandleStep() -> direction=%u wrap=%d lowestOff=%d",
                    static_cast<unsigned>(to_underlying(aDirection)), aWrap, aLowestOff);

    VerifyOrReturnError(aDirection != FanControl::StepDirectionEnum::kUnknownEnumValue,
                        Protocols::InteractionModel::Status::InvalidCommand);

    VerifyOrReturnError(mFanLoad != nullptr, Protocols::InteractionModel::Status::Failure);
    auto & cluster = mFanLoad->FanControlCluster();
    VerifyOrReturnError(cluster.GetFeatureMap().Has(FanControl::Feature::kMultiSpeed),
                        Protocols::InteractionModel::Status::Failure);

    const uint8_t speedMax  = cluster.GetSpeedMax();
    const auto speedSetting = cluster.GetSpeedSetting();
    uint8_t newSpeedSetting = speedSetting.ValueOr(0);

    if (aDirection == FanControl::StepDirectionEnum::kIncrease)
    {
        if (speedSetting.IsNull())
        {
            newSpeedSetting = 1;
        }
        else if (speedSetting.Value() < speedMax)
        {
            newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() + 1);
        }
        else if (speedSetting.Value() == speedMax)
        {
            if (aWrap)
            {
                newSpeedSetting = aLowestOff ? 0 : 1;
            }
        }
    }
    else if (aDirection == FanControl::StepDirectionEnum::kDecrease)
    {
        if (speedSetting.IsNull())
        {
            newSpeedSetting = aLowestOff ? 0 : 1;
        }
        else if ((speedSetting.Value() > 1) && (speedSetting.Value() <= speedMax))
        {
            newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() - 1);
        }
        else if (speedSetting.Value() == 1)
        {
            if (aLowestOff)
            {
                newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() - 1);
            }
            else if (aWrap)
            {
                newSpeedSetting = speedMax;
            }
        }
        else if (speedSetting.Value() == 0)
        {
            if (aWrap)
            {
                newSpeedSetting = speedMax;
            }
            else if (!aLowestOff)
            {
                newSpeedSetting = 1;
            }
        }
    }

    if (!speedSetting.IsNull() && newSpeedSetting != speedSetting.Value())
    {
        ChipLogProgress(DeviceLayer, "LoggingFanDelegate::HandleStep() -> Speed changed from %u to %u", speedSetting.Value(),
                        newSpeedSetting);
    }
    else if (speedSetting.IsNull())
    {
        ChipLogProgress(DeviceLayer, "LoggingFanDelegate::HandleStep() -> Speed changed from NULL to %u", newSpeedSetting);
    }

    return cluster.SetSpeedSetting(DataModel::MakeNullable(newSpeedSetting));
}

void LoggingFanDelegate::OnFanDriveStateChanged(const FanControl::FanDriveState & newState)
{
    [[maybe_unused]] const unsigned mode           = static_cast<unsigned>(to_underlying(newState.mode));
    [[maybe_unused]] const unsigned percentCurrent = static_cast<unsigned>(newState.percentCurrent);
    [[maybe_unused]] const unsigned speedCurrent   = static_cast<unsigned>(newState.speedCurrent);
    const bool percentSettingIsNull                = newState.percentSetting.IsNull();
    const bool speedSettingIsNull                  = newState.speedSetting.IsNull();

    if (!percentSettingIsNull && !speedSettingIsNull)
    {
        ChipLogProgress(
            DeviceLayer,
            "LoggingFanDelegate::OnFanDriveStateChanged() -> mode=%u percentSetting=%u percentCurrent=%u speedSetting=%u "
            "speedCurrent=%u",
            mode, static_cast<unsigned>(newState.percentSetting.Value()), percentCurrent,
            static_cast<unsigned>(newState.speedSetting.Value()), speedCurrent);
    }
    else if (!percentSettingIsNull)
    {
        ChipLogProgress(
            DeviceLayer,
            "LoggingFanDelegate::OnFanDriveStateChanged() -> mode=%u percentSetting=%u percentCurrent=%u speedSetting=NULL "
            "speedCurrent=%u",
            mode, static_cast<unsigned>(newState.percentSetting.Value()), percentCurrent, speedCurrent);
    }
    else if (!speedSettingIsNull)
    {
        ChipLogProgress(
            DeviceLayer,
            "LoggingFanDelegate::OnFanDriveStateChanged() -> mode=%u percentSetting=NULL percentCurrent=%u speedSetting=%u "
            "speedCurrent=%u",
            mode, percentCurrent, static_cast<unsigned>(newState.speedSetting.Value()), speedCurrent);
    }
    else
    {
        ChipLogProgress(
            DeviceLayer,
            "LoggingFanDelegate::OnFanDriveStateChanged() -> mode=%u percentSetting=NULL percentCurrent=%u speedSetting=NULL "
            "speedCurrent=%u",
            mode, percentCurrent, speedCurrent);
    }

    VerifyOrReturn(mFanLoad != nullptr);
    auto & fan = mFanLoad->FanControlCluster();

    Clusters::OnOffCluster * onOff = mFanLoad->OnOffCluster();
    if (onOff == nullptr)
    {
        ApplyOnOffToFan(fan, IsFanSetForOn(newState));
        return;
    }

    if (!onOff->GetOnOff())
    {
        ApplyOnOffToFan(fan, false);
        return;
    }

    ApplyOnOffToFan(fan, IsFanSetForOn(newState));
}

void LoggingFanDelegate::OnRockSettingChanged(BitMask<FanControl::RockBitmap> newValue)
{
    ChipLogProgress(DeviceLayer, "LoggingFanDelegate::OnRockSettingChanged() -> 0x%02X", newValue.Raw());
}

void LoggingFanDelegate::OnWindSettingChanged(BitMask<FanControl::WindBitmap> newValue)
{
    ChipLogProgress(DeviceLayer, "LoggingFanDelegate::OnWindSettingChanged() -> 0x%02X", newValue.Raw());
}

void LoggingFanDelegate::OnAirflowDirectionChanged(FanControl::AirflowDirectionEnum newValue)
{
    ChipLogProgress(DeviceLayer, "LoggingFanDelegate::OnAirflowDirectionChanged() -> %u",
                    static_cast<unsigned>(to_underlying(newValue)));
}

// OnOffDelegate

void LoggingFanDelegate::OnOffStartup(bool on)
{
    if (mFanLoad != nullptr)
    {
        ApplyOnOffToFan(mFanLoad->FanControlCluster(), on);
    }
    ChipLogProgress(DeviceLayer, "LoggingFanDelegate::OnOffStartup() -> %s", on ? "ON" : "OFF");
}

void LoggingFanDelegate::OnOnOffChanged(bool on)
{
    if (mFanLoad != nullptr)
    {
        ApplyOnOffToFan(mFanLoad->FanControlCluster(), on);
    }
    ChipLogProgress(DeviceLayer, "LoggingFanDelegate::OnOnOffChanged() -> %s", on ? "ON" : "OFF");
}

} // namespace app
} // namespace chip
