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
#include <app/clusters/fan-control-server/CodegenIntegration.h>

#include <app/clusters/fan-control-server/FanControlCluster.h>

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
namespace {
using namespace chip;
using namespace chip::app::Clusters::FanControl;
using chip::app::Clusters::FanControlCluster;
using Protocols::InteractionModel::Status;
static Status GetFanModeAdapter(EndpointId ep, FanModeEnum * value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    *value = c->GetFanMode();
    return Status::Success;
}
Status SetFanModeUI(EndpointId ep, FanModeEnum value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return c->SetFanMode(value).GetStatusCode().GetStatus();
}

static Status GetAirflowDirectionAdapter(EndpointId ep, AirflowDirectionEnum * value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!c->GetFeatureMap().Has(FanControl::Feature::kAirflowDirection))
    {
        return Status::UnsupportedAttribute;
    }
    *value = c->GetAirflowDirection();
    return Status::Success;
}
Status SetAirflowDirectionUI(EndpointId ep, AirflowDirectionEnum value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return c->SetAirflowDirection(value).GetStatusCode().GetStatus();
}

static Status GetFanModeSequenceAdapter(EndpointId ep, FanModeSequenceEnum * value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    *value = c->GetFanModeSequence();
    return Status::Success;
}
Status SetFanModeSequenceUI(EndpointId ep, FanModeSequenceEnum value)
{
    return Status::UnsupportedWrite;
}

static Status GetPercentCurrentAdapter(EndpointId ep, Percent * value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    *value = c->GetPercentCurrent();
    return Status::Success;
}

static Status GetSpeedMaxAdapter(EndpointId ep, uint8_t * value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!c->GetFeatureMap().Has(FanControl::Feature::kMultiSpeed))
    {
        return Status::UnsupportedAttribute;
    }
    *value = c->GetSpeedMax();
    return Status::Success;
}

static Status GetSpeedCurrentAdapter(EndpointId ep, uint8_t * value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!c->GetFeatureMap().Has(FanControl::Feature::kMultiSpeed))
    {
        return Status::UnsupportedAttribute;
    }
    *value = c->GetSpeedCurrent();
    return Status::Success;
}

static Status GetFeatureMapAdapter(EndpointId ep, uint32_t * value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    *value = c->GetFeatureMap().Raw();
    return Status::Success;
}

Status SetPercentSettingUI(EndpointId ep, Percent value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return c->SetPercentSetting(app::DataModel::Nullable<Percent>(value)).GetStatusCode().GetStatus();
}

Status SetSpeedSettingUI(EndpointId ep, uint8_t value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return c->SetSpeedSetting(app::DataModel::Nullable<uint8_t>(value)).GetStatusCode().GetStatus();
}

static Status GetPercentSettingAdapter(EndpointId ep, app::DataModel::Nullable<Percent> & value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = c->GetPercentSetting();
    return Status::Success;
}

static Status GetSpeedSettingAdapter(EndpointId ep, app::DataModel::Nullable<uint8_t> & value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!c->GetFeatureMap().Has(FanControl::Feature::kMultiSpeed))
    {
        return Status::UnsupportedAttribute;
    }
    value = c->GetSpeedSetting();
    return Status::Success;
}

static Status GetRockSupportAdapter(EndpointId ep, BitMask<RockBitmap> * value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!c->GetFeatureMap().Has(FanControl::Feature::kRocking))
    {
        return Status::UnsupportedAttribute;
    }
    *value = c->GetRockSupport();
    return Status::Success;
}
Status SetRockSupportUI(EndpointId ep, BitMask<RockBitmap> value)
{
    return Status::UnsupportedWrite;
}

static Status GetRockSettingAdapter(EndpointId ep, BitMask<RockBitmap> * value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!c->GetFeatureMap().Has(FanControl::Feature::kRocking))
    {
        return Status::UnsupportedAttribute;
    }
    *value = c->GetRockSetting();
    return Status::Success;
}
Status SetRockSettingUI(EndpointId ep, BitMask<RockBitmap> value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return c->SetRockSetting(value).GetStatusCode().GetStatus();
}

static Status GetWindSupportAdapter(EndpointId ep, BitMask<WindBitmap> * value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!c->GetFeatureMap().Has(FanControl::Feature::kWind))
    {
        return Status::UnsupportedAttribute;
    }
    *value = c->GetWindSupport();
    return Status::Success;
}
Status SetWindSupportUI(EndpointId ep, BitMask<WindBitmap> value)
{
    return Status::UnsupportedWrite;
}

static Status GetWindSettingAdapter(EndpointId ep, BitMask<WindBitmap> * value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    if (!c->GetFeatureMap().Has(FanControl::Feature::kWind))
    {
        return Status::UnsupportedAttribute;
    }
    *value = c->GetWindSetting();
    return Status::Success;
}
Status SetWindSettingUI(EndpointId ep, BitMask<WindBitmap> value)
{
    FanControlCluster * c = FanControl::FindClusterOnEndpoint(ep);
    if (c == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return c->SetWindSetting(value).GetStatusCode().GetStatus();
}
} // namespace

void FanControl::UpdateState()
{
    UpdateStateEnum(mEndpointId, mTargetFanMode, mFanMode, &SetFanModeUI, &GetFanModeAdapter);
    UpdateStateEnum(mEndpointId, mTargetAirflowDirection, mAirflowDirection, &SetAirflowDirectionUI, &GetAirflowDirectionAdapter);
    UpdateStateEnum(mEndpointId, mTargetFanModeSequence, mFanModeSequence, &SetFanModeSequenceUI, &GetFanModeSequenceAdapter);

    UpdateStateReadOnly(mEndpointId, mTargetPercent, mPercent, &GetPercentCurrentAdapter);
    UpdateStateReadOnly(mEndpointId, mTargetSpeedMax, mSpeedMax, &GetSpeedMaxAdapter);
    UpdateStateReadOnly(mEndpointId, mTargetSpeedCurrent, mSpeedCurrent, &GetSpeedCurrentAdapter);
    UpdateStateReadOnly(mEndpointId, mTargetFeatureMap, mFeatureMap, &GetFeatureMapAdapter);

    UpdateStateNullable(mEndpointId, mTargetPercentSetting, mPercentSetting, &SetPercentSettingUI, &GetPercentSettingAdapter);
    UpdateStateNullable(mEndpointId, mTargetSpeedSetting, mSpeedSetting, &SetSpeedSettingUI, &GetSpeedSettingAdapter);

    UpdateStateEnum(mEndpointId, mTargetRockSupport, mRockSupport, &SetRockSupportUI, &GetRockSupportAdapter);
    UpdateStateEnum(mEndpointId, mTargetRockSetting, mRockSetting, &SetRockSettingUI, &GetRockSettingAdapter);
    UpdateStateEnum(mEndpointId, mTargetWindSupport, mWindSupport, &SetWindSupportUI, &GetWindSupportAdapter);
    UpdateStateEnum(mEndpointId, mTargetWindSetting, mWindSetting, &SetWindSettingUI, &GetWindSettingAdapter);
}

void FanControl::Render()
{
    ImGui::Begin(mTitle.c_str());
    ImGui::Text("On Endpoint %d", mEndpointId);

    int uiFanMode          = static_cast<int>(mFanMode);
    int uiPercentSetting   = static_cast<int>(mPercentSetting);
    int uiSpeedSetting     = static_cast<int>(mSpeedSetting);
    int uiAirflowDirection = static_cast<int>(mAirflowDirection);
    int uiFanModeSequence  = static_cast<int>(mFanModeSequence);
    int uiPercent          = static_cast<int>(mPercent);
    int uiSpeedCurrent     = static_cast<int>(mSpeedCurrent);
    int uiSpeedMax         = static_cast<int>(mSpeedMax);
    int uiFeatureMap       = static_cast<int>(mFeatureMap);
    int uiRockSetting      = mRockSetting.Raw();
    int uiRockSupport      = mRockSupport.Raw();
    int uiWindSetting      = mWindSetting.Raw();
    int uiWindSupport      = mWindSupport.Raw();

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

    ImGui::LabelText("Feature map", "%d", uiFeatureMap);
    if (static_cast<int>(mFeatureMap) != uiFeatureMap)
    {
        mTargetFeatureMap = static_cast<uint32_t>(uiFeatureMap);
    }

    ImGui::SliderInt("Airflow direction", &uiAirflowDirection, 1, 0,
                     mAirflowDirection == chip::app::Clusters::FanControl::AirflowDirectionEnum::kForward ? "Forward" : "Reverse");
    if (static_cast<int>(mAirflowDirection) != uiAirflowDirection)
    {
        mTargetAirflowDirection = static_cast<chip::app::Clusters::FanControl::AirflowDirectionEnum>(uiAirflowDirection);
    }

    ImGui::LabelText("Rock support", "%s", GetRockBitmapValueString(uiRockSupport).c_str());
    if (static_cast<int>(mRockSupport.Raw()) != uiRockSupport)
    {
        mTargetRockSupport = static_cast<chip::BitMask<chip::app::Clusters::FanControl::RockBitmap>>(uiRockSupport);
    }

    const auto rockSettingString = GetRockBitmapValueString(mRockSetting);
    ImGui::SliderInt("Rock setting", &uiRockSetting, 0, 1, rockSettingString.c_str());
    if (uiRockSetting != mRockSetting.Raw())
    {
        mTargetRockSetting = static_cast<chip::BitMask<chip::app::Clusters::FanControl::RockBitmap>>(uiRockSetting);
    }

    ImGui::LabelText("Wind support", "%s", GetWindBitmapValueString(uiWindSupport).c_str());
    if (static_cast<int>(mWindSupport.Raw()) != uiWindSupport)
    {
        mTargetWindSupport = static_cast<chip::BitMask<chip::app::Clusters::FanControl::WindBitmap>>(uiWindSupport);
    }

    const auto windSettingString = GetWindBitmapValueString(mWindSetting);
    ImGui::SliderInt("Wind setting", &uiWindSetting, 0, 2, windSettingString.c_str());
    if (uiWindSetting != mWindSetting.Raw())
    {
        mTargetWindSetting = static_cast<chip::BitMask<chip::app::Clusters::FanControl::WindBitmap>>(uiWindSetting);
    }

    ImGui::End();
}

} // namespace Windows
} // namespace Ui
} // namespace example
