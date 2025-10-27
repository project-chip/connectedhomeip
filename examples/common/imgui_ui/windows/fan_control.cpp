/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "fan_control.h"
#include "helpers.h"

#include <imgui.h>

#include <math.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>

namespace example {
namespace Ui {
namespace Windows {

template <typename T>
void collect(T value, chip::BitMask<T> mask, std::string & result, const std::string & tag)
{
    if (mask.Has(value))
    {
        if (!result.empty())
        {
            result += "|";
        }
        result += tag;
    }
}

static const std::string GetRockBitmapValueString(chip::BitMask<chip::app::Clusters::FanControl::RockBitmap> bitmask)
{
    std::string result;
    collect(chip::app::Clusters::FanControl::RockBitmap::kRockLeftRight, bitmask, result, "Left-right");
    collect(chip::app::Clusters::FanControl::RockBitmap::kRockUpDown, bitmask, result, "Up-down");
    collect(chip::app::Clusters::FanControl::RockBitmap::kRockRound, bitmask, result, "Round");
    return result.empty() ? "Unknown" : result;
}

static const std::string GetWindBitmapValueString(chip::BitMask<chip::app::Clusters::FanControl::WindBitmap> bitmask)
{
    std::string result;
    collect(chip::app::Clusters::FanControl::WindBitmap::kSleepWind, bitmask, result, "Sleep");
    collect(chip::app::Clusters::FanControl::WindBitmap::kNaturalWind, bitmask, result, "Natural");
    return result.empty() ? "Unknown" : result;
}

template <typename T>
static constexpr int GetBitIdx(const chip::BitMask<T> & bitmap, int tMin, int tMax)
{
    if (tMin > tMax)
    {
        return -1;
    }

    int arg = tMin;
    int r   = 0;

    while (!bitmap.HasOnly(static_cast<T>(arg)))
    {
        arg = arg << 1;
        r++;
    }
    return r;
}

template <typename T>
static constexpr int GetBits(const chip::BitMask<T> & bitmap, int tMin, int tMax)
{
    if (tMin > tMax)
    {
        return -1;
    }

    int arg = tMin;
    int r   = 0;

    while (arg <= tMax)
    {
        if (bitmap.Has(static_cast<T>(arg)))
        {
            r |= arg;
        }
        arg = arg << 1;
    }
    return r;
}

void FanControl::UpdateState()
{
    UpdateStateEnum(mEndpointId, mTargetFanMode, mFanMode, &chip::app::Clusters::FanControl::Attributes::FanMode::Set,
                    &chip::app::Clusters::FanControl::Attributes::FanMode::Get);

    UpdateStateEnum(mEndpointId, mTargetAirflowDirection, mAirflowDirection,
                    &chip::app::Clusters::FanControl::Attributes::AirflowDirection::Set,
                    &chip::app::Clusters::FanControl::Attributes::AirflowDirection::Get);

    UpdateStateEnum(mEndpointId, mTargetFanModeSequence, mFanModeSequence,
                    &chip::app::Clusters::FanControl::Attributes::FanModeSequence::Set,
                    &chip::app::Clusters::FanControl::Attributes::FanModeSequence::Get);

    UpdateStateReadOnly(mEndpointId, mTargetPercent, mPercent, &chip::app::Clusters::FanControl::Attributes::PercentCurrent::Get);

    UpdateStateReadOnly(mEndpointId, mTargetSpeedMax, mSpeedMax, &chip::app::Clusters::FanControl::Attributes::SpeedMax::Get);

    UpdateStateReadOnly(mEndpointId, mTargetSpeedCurrent, mSpeedCurrent,
                        &chip::app::Clusters::FanControl::Attributes::SpeedCurrent::Get);

    UpdateStateReadOnly(mEndpointId, mTargetFeatureMap, mFeatureMap, &chip::app::Clusters::FanControl::Attributes::FeatureMap::Get);

    UpdateStateNullable(mEndpointId, mTargetPercentSetting, mPercentSetting,
                        &chip::app::Clusters::FanControl::Attributes::PercentSetting::Set,
                        &chip::app::Clusters::FanControl::Attributes::PercentSetting::Get);

    UpdateStateNullable(mEndpointId, mTargetSpeedSetting, mSpeedSetting,
                        &chip::app::Clusters::FanControl::Attributes::SpeedSetting::Set,
                        &chip::app::Clusters::FanControl::Attributes::SpeedSetting::Get);

    UpdateStateOptional(mEndpointId, mTargetRockSupport, mRockSupport,
                        &chip::app::Clusters::FanControl::Attributes::RockSupport::Set,
                        &chip::app::Clusters::FanControl::Attributes::RockSupport::Get);

    UpdateStateOptional(mEndpointId, mTargetRockSetting, mRockSetting,
                        &chip::app::Clusters::FanControl::Attributes::RockSetting::Set,
                        &chip::app::Clusters::FanControl::Attributes::RockSetting::Get);

    UpdateStateOptional(mEndpointId, mTargetWindSupport, mWindSupport,
                        &chip::app::Clusters::FanControl::Attributes::WindSupport::Set,
                        &chip::app::Clusters::FanControl::Attributes::WindSupport::Get);

    UpdateStateOptional(mEndpointId, mTargetWindSetting, mWindSetting,
                        &chip::app::Clusters::FanControl::Attributes::WindSetting::Set,
                        &chip::app::Clusters::FanControl::Attributes::WindSetting::Get);
}

void FanControl::Render()
{
    ImGui::Begin(mTitle.c_str());
    ImGui::Text("On Endpoint %d", mEndpointId);

    auto fanMode          = mFanMode;
    auto percentSetting   = mPercentSetting;
    auto percent          = mPercent;
    auto airflowDirection = mAirflowDirection;
    auto fanModeSequence  = mFanModeSequence;
    auto rockSetting      = mRockSetting;
    auto windSetting      = mWindSetting;
    auto rockSupport      = mRockSupport;
    auto windSupport      = mWindSupport;
    uint8_t speedSetting  = mSpeedSetting;
    uint16_t speedCurrent = mSpeedCurrent;
    uint16_t speedMax     = mSpeedMax;
    uint32_t featureMap   = mFeatureMap;

    int uiFanMode          = static_cast<int>(fanMode);
    int uiPercentSetting   = static_cast<int>(percentSetting);
    int uiSpeedSetting     = static_cast<int>(speedSetting);
    int uiAirflowDirection = static_cast<int>(airflowDirection);

    int uiFanModeSequence                                                    = static_cast<int>(fanModeSequence);
    int uiPercent                                                            = percent;
    int uiSpeedCurrent                                                       = speedCurrent;
    int uiSpeedMax                                                           = speedMax;
    int uiFeatureMap                                                         = featureMap;
    chip::BitMask<chip::app::Clusters::FanControl::RockBitmap> uiRockSetting = rockSetting;
    chip::BitMask<chip::app::Clusters::FanControl::RockBitmap> uiRockSupport = rockSupport;
    chip::BitMask<chip::app::Clusters::FanControl::WindBitmap> uiWindSetting = windSetting;
    chip::BitMask<chip::app::Clusters::FanControl::WindBitmap> uiWindSupport = windSupport;

    ImGui::SliderInt("Mode value", &uiFanMode, static_cast<int>(chip::app::Clusters::FanControl::FanModeEnum::kOff),
                     static_cast<int>(chip::app::Clusters::FanControl::FanModeEnum::kSmart));
    if (static_cast<int>(fanMode) != uiFanMode)
    {
        mTargetFanMode = static_cast<chip::app::Clusters::FanControl::FanModeEnum>(uiFanMode);
    }

    ImGui::LabelText("Mode sequence", "%d", uiFanModeSequence);
    if (static_cast<int>(fanModeSequence) != uiFanModeSequence)
    {
        mTargetFanModeSequence = static_cast<chip::app::Clusters::FanControl::FanModeSequenceEnum>(uiFanModeSequence);
    }

    ImGui::SliderInt("Percent setting", &uiPercentSetting, 0, 100);
    if (static_cast<int>(percentSetting) != uiPercentSetting)
    {
        mTargetPercentSetting.Update(uiPercentSetting);
    }

    ImGui::LabelText("Percent", "%d", uiPercent);
    if (static_cast<int>(percent) != uiPercent)
    {
        mTargetPercent = uiPercent;
    }

    ImGui::SliderInt("Speed setting", &uiSpeedSetting, 0, 10);
    if (speedSetting != static_cast<uint8_t>(uiSpeedSetting))
    {
        mTargetSpeedSetting.Update(uiSpeedSetting);
    }

    ImGui::LabelText("Speed current", "%d", uiSpeedCurrent);
    if (static_cast<int>(speedCurrent) != uiSpeedCurrent)
    {
        mTargetSpeedCurrent = uiSpeedCurrent;
    }

    ImGui::LabelText("Speed Max", "%d", uiSpeedMax);
    if (static_cast<int>(speedMax) != uiSpeedMax)
    {
        mTargetSpeedMax = uiSpeedMax;
    }

    ImGui::SliderInt("Airflow direction", &uiAirflowDirection, 1, 0,
                     airflowDirection == chip::app::Clusters::FanControl::AirflowDirectionEnum::kForward ? "Forward" : "Reverse");
    if (static_cast<int>(airflowDirection) != uiAirflowDirection)
    {
        mTargetAirflowDirection = static_cast<chip::app::Clusters::FanControl::AirflowDirectionEnum>(uiAirflowDirection);
    }

    auto rockSettingMin = static_cast<int>(chip::app::Clusters::FanControl::RockBitmap::kRockLeftRight);
    auto rockSettingMax = static_cast<int>(chip::app::Clusters::FanControl::RockBitmap::kRockRound);
    auto rockSettingVal = GetBitIdx(uiRockSetting, rockSettingMin, rockSettingMax);
    ImGui::SliderInt(
        "Rock setting", &rockSettingVal, 0,
        GetBitIdx(chip::BitMask(chip::app::Clusters::FanControl::RockBitmap::kRockRound), rockSettingMin, rockSettingMax),
        GetRockBitmapValueString(rockSetting).c_str());
    if (!uiRockSetting.Has(rockSetting))
    {
        mTargetRockSetting.SetValue(uiRockSetting);
    }

    ImGui::LabelText("Rock support", "%d", GetBits(uiRockSupport, rockSettingMin, rockSettingMax));
    if (rockSupport != uiRockSupport)
    {
        mTargetRockSupport.SetValue(uiRockSupport);
    }

    auto windSettingMin = static_cast<int>(chip::app::Clusters::FanControl::WindBitmap::kSleepWind);
    auto windSettingMax = static_cast<int>(chip::app::Clusters::FanControl::WindBitmap::kNaturalWind);
    auto windSettingVal = GetBitIdx(uiWindSetting, windSettingMin, windSettingMax);
    ImGui::SliderInt(
        "Wind setting", &windSettingVal, 0,
        GetBitIdx(chip::BitMask(chip::app::Clusters::FanControl::WindBitmap::kNaturalWind), windSettingMin, windSettingMax),
        GetWindBitmapValueString(windSetting).c_str());
    if (!uiWindSetting.Has(windSetting))
    {
        mTargetWindSetting.SetValue(uiWindSetting);
    }

    ImGui::LabelText("Wind support", "%d", GetBits(uiWindSupport, windSettingMin, windSettingMax));
    if (windSupport != uiWindSupport)
    {
        mTargetWindSupport.SetValue(uiWindSupport);
    }

    ImGui::LabelText("Feature map", "%d", uiFeatureMap);
    if (static_cast<int>(featureMap) != uiFeatureMap)
    {
        mTargetFeatureMap = uiFeatureMap;
    }

    ImGui::End();
}

} // namespace Windows
} // namespace Ui
} // namespace example
