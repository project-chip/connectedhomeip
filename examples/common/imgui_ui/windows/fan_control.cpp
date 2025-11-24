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
#include "accessor_updaters.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>

#include <imgui.h>

#include <lib/support/TypeTraits.h>

#include <math.h>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace example {
namespace Ui {
namespace Windows {

template <typename T>
static const std::string GetBitmapValueString(const T maxValue, chip::BitMask<T> bitmask,
                                              const std::vector<std::pair<T, std::string_view>> & valuesToNames)

{
    // Check if any new value was added and communicate to user that the code needs updating.
    if (chip::to_underlying(maxValue) << 1 <= bitmask.Raw())
    {
        return "UI code needs uptating to support new value";
    }

    std::string result;
    for (const auto & entry : valuesToNames)
    {
        if (bitmask.Has(entry.first))
        {
            if (!result.empty())
            {
                result += " | ";
            }
            result += entry.second;
        }
    }

    return result.empty() ? "Unknown" : result;
}

static const std::string GetRockBitmapValueString(chip::BitMask<chip::app::Clusters::FanControl::RockBitmap> bitmask)
{
    return GetBitmapValueString(chip::app::Clusters::FanControl::RockBitmap::kRockRound, bitmask,
                                { { chip::app::Clusters::FanControl::RockBitmap::kRockLeftRight, "Left-right" },
                                  { chip::app::Clusters::FanControl::RockBitmap::kRockUpDown, "Up-down" },
                                  { chip::app::Clusters::FanControl::RockBitmap::kRockRound, "Round" } });
}

static const std::string GetWindBitmapValueString(chip::BitMask<chip::app::Clusters::FanControl::WindBitmap> bitmask)
{
    return GetBitmapValueString(chip::app::Clusters::FanControl::WindBitmap::kNaturalWind, bitmask,
                                { { chip::app::Clusters::FanControl::WindBitmap::kSleepWind, "Sleep" },
                                  { chip::app::Clusters::FanControl::WindBitmap::kNaturalWind, "Natural" } });
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

    UpdateStateEnum(mEndpointId, mTargetRockSupport, mRockSupport, &chip::app::Clusters::FanControl::Attributes::RockSupport::Set,
                    &chip::app::Clusters::FanControl::Attributes::RockSupport::Get);

    UpdateStateEnum(mEndpointId, mTargetRockSetting, mRockSetting, &chip::app::Clusters::FanControl::Attributes::RockSetting::Set,
                    &chip::app::Clusters::FanControl::Attributes::RockSetting::Get);

    UpdateStateEnum(mEndpointId, mTargetWindSupport, mWindSupport, &chip::app::Clusters::FanControl::Attributes::WindSupport::Set,
                    &chip::app::Clusters::FanControl::Attributes::WindSupport::Get);

    UpdateStateEnum(mEndpointId, mTargetWindSetting, mWindSetting, &chip::app::Clusters::FanControl::Attributes::WindSetting::Set,
                    &chip::app::Clusters::FanControl::Attributes::WindSetting::Get);
}

void FanControl::Render()
{
    ImGui::Begin(mTitle.c_str());
    ImGui::Text("On Endpoint %d", mEndpointId);

    int uiFanMode                                                            = static_cast<int>(mFanMode);
    int uiPercentSetting                                                     = static_cast<int>(mPercentSetting);
    int uiSpeedSetting                                                       = static_cast<int>(mSpeedSetting);
    int uiAirflowDirection                                                   = static_cast<int>(mAirflowDirection);
    int uiFanModeSequence                                                    = static_cast<int>(mFanModeSequence);
    int uiPercent                                                            = static_cast<int>(mPercent);
    int uiSpeedCurrent                                                       = static_cast<int>(mSpeedCurrent);
    int uiSpeedMax                                                           = static_cast<int>(mSpeedMax);
    int uiFeatureMap                                                         = static_cast<int>(mFeatureMap);
    chip::BitMask<chip::app::Clusters::FanControl::RockBitmap> uiRockSetting = mRockSetting;
    chip::BitMask<chip::app::Clusters::FanControl::RockBitmap> uiRockSupport = mRockSupport;
    chip::BitMask<chip::app::Clusters::FanControl::WindBitmap> uiWindSetting = mWindSetting;
    chip::BitMask<chip::app::Clusters::FanControl::WindBitmap> uiWindSupport = mWindSupport;

    ImGui::SliderInt("Mode value", &uiFanMode, static_cast<int>(chip::app::Clusters::FanControl::FanModeEnum::kOff),
                     static_cast<int>(chip::app::Clusters::FanControl::FanModeEnum::kSmart));
    if (static_cast<int>(mFanMode) != uiFanMode)
    {
        mTargetFanMode = static_cast<chip::app::Clusters::FanControl::FanModeEnum>(uiFanMode);
    }

    ImGui::LabelText("Mode sequence", "%d", uiFanModeSequence);
    if (static_cast<int>(mFanModeSequence) != uiFanModeSequence)
    {
        mTargetFanModeSequence = static_cast<chip::app::Clusters::FanControl::FanModeSequenceEnum>(uiFanModeSequence);
    }

    ImGui::SliderInt("Percent setting", &uiPercentSetting, 0, 100);
    if (static_cast<int>(mPercentSetting) != uiPercentSetting)
    {
        mTargetPercentSetting.Update(uiPercentSetting);
    }

    ImGui::LabelText("Percent", "%d", uiPercent);
    if (static_cast<int>(mPercent) != uiPercent)
    {
        mTargetPercent = static_cast<chip::Percent>(uiPercent);
    }

    ImGui::SliderInt("Speed setting", &uiSpeedSetting, 0, 10);
    if (mSpeedSetting != static_cast<uint8_t>(uiSpeedSetting))
    {
        mTargetSpeedSetting.Update(uiSpeedSetting);
    }

    ImGui::LabelText("Speed current", "%d", uiSpeedCurrent);
    if (static_cast<int>(mSpeedCurrent) != uiSpeedCurrent)
    {
        mTargetSpeedCurrent = static_cast<uint8_t>(uiSpeedCurrent);
    }

    ImGui::LabelText("Speed Max", "%d", uiSpeedMax);
    if (static_cast<int>(mSpeedMax) != uiSpeedMax)
    {
        mTargetSpeedMax = static_cast<uint8_t>(uiSpeedMax);
    }

    ImGui::SliderInt("Airflow direction", &uiAirflowDirection, 1, 0,
                     mAirflowDirection == chip::app::Clusters::FanControl::AirflowDirectionEnum::kForward ? "Forward" : "Reverse");
    if (static_cast<int>(mAirflowDirection) != uiAirflowDirection)
    {
        mTargetAirflowDirection = static_cast<chip::app::Clusters::FanControl::AirflowDirectionEnum>(uiAirflowDirection);
    }

    if (!uiRockSetting.Has(mRockSetting))
    {
        mTargetRockSetting = uiRockSetting;
    }

    if (mRockSupport != uiRockSupport)
    {
        mTargetRockSupport = uiRockSupport;
    }

    if (!uiWindSetting.Has(mWindSetting))
    {
        mTargetWindSetting = uiWindSetting;
    }

    if (mWindSupport != uiWindSupport)
    {
        mTargetWindSupport = uiWindSupport;
    }

    ImGui::LabelText("Feature map", "%d", uiFeatureMap);
    if (static_cast<int>(mFeatureMap) != uiFeatureMap)
    {
        mTargetFeatureMap = static_cast<uint32_t>(uiFeatureMap);
    }

    const auto flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp;
    if (ImGui::BeginTable("Wind and Rock", 2, flags))
    {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None, 1.0f);
        ImGui::TableSetupColumn("Flags", ImGuiTableColumnFlags_None, 3.0f);
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Rock support");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", GetRockBitmapValueString(uiRockSupport).c_str());
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Rock setting");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", GetRockBitmapValueString(uiRockSetting).c_str());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Wind support");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", GetWindBitmapValueString(uiWindSupport).c_str());
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Wind setting");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", GetWindBitmapValueString(uiWindSetting).c_str());

        ImGui::EndTable();
    }

    ImGui::End();
}

} // namespace Windows
} // namespace Ui
} // namespace example
