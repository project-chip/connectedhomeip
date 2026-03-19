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
#include <app/clusters/fan-control-server/fan-control-server.h>

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
using Protocols::InteractionModel::Status;
static Status GetFanModeAdapter(EndpointId ep, FanModeEnum * value)
{
    return FanControl::GetFanMode(ep, *value);
}
Status SetFanModeUI(EndpointId ep, FanModeEnum value)
{
    return FanControl::SetFanMode(ep, value);
}
 
 Status GetAirflowDirection(EndpointId ep, AirflowDirectionEnum * value)
 {
     if (auto * c = FindClusterOnEndpoint(ep))
     {
         *value = c->GetAirflowDirection();
         return Status::Success;
     }
     return Status::UnsupportedEndpoint;
 }
Status SetAirflowDirectionUI(EndpointId ep, AirflowDirectionEnum value)
{
    return FanControl::SetAirflowDirection(ep, value);
}
 
 Status GetFanModeSequence(EndpointId ep, FanModeSequenceEnum * value)
 {
     if (auto * c = FindClusterOnEndpoint(ep))
     {
         *value = c->GetFanModeSequence();
         return Status::Success;
     }
     return Status::UnsupportedEndpoint;
 }
 Status SetFanModeSequenceUI(EndpointId ep, FanModeSequenceEnum value)
 {
     return Status::UnsupportedWrite;
 }
 
 Status GetPercentCurrent(EndpointId ep, Percent * value)
 {
     if (auto * c = FindClusterOnEndpoint(ep))
     {
         *value = c->GetPercentCurrent();
         return Status::Success;
     }
     return Status::UnsupportedEndpoint;
 }
 
static Status GetSpeedMaxAdapter(EndpointId ep, uint8_t * value)
{
    return FanControl::GetSpeedMax(ep, *value);
}
 
 Status GetSpeedCurrent(EndpointId ep, uint8_t * value)
 {
     if (auto * c = FindClusterOnEndpoint(ep))
     {
         *value = c->GetSpeedCurrent();
         return Status::Success;
     }
     return Status::UnsupportedEndpoint;
 }
 
 Status GetFeatureMap(EndpointId ep, uint32_t * value)
 {
     if (auto * c = FindClusterOnEndpoint(ep))
     {
         *value = c->GetFeatureMap();
         return Status::Success;
     }
     return Status::UnsupportedEndpoint;
 }
 
Status SetPercentSettingUI(EndpointId ep, const app::DataModel::Nullable<Percent> & value)
{
    return FanControl::SetPercentSetting(ep, value);
}

Status SetSpeedSettingUI(EndpointId ep, const app::DataModel::Nullable<uint8_t> & value)
{
    return FanControl::SetSpeedSetting(ep, value);
}
 
 Status GetRockSupport(EndpointId ep, BitMask<RockBitmap> * value)
 {
     if (auto * c = FindClusterOnEndpoint(ep))
     {
         *value = c->GetRockSupport();
         return Status::Success;
     }
     return Status::UnsupportedEndpoint;
 }
 Status SetRockSupportUI(EndpointId ep, BitMask<RockBitmap> value)
 {
     return Status::UnsupportedWrite;
 }
 
 Status GetRockSetting(EndpointId ep, BitMask<RockBitmap> * value)
 {
     if (auto * c = FindClusterOnEndpoint(ep))
     {
         *value = c->GetRockSetting();
         return Status::Success;
     }
     return Status::UnsupportedEndpoint;
 }
Status SetRockSettingUI(EndpointId ep, BitMask<RockBitmap> value)
{
    return FanControl::SetRockSetting(ep, value);
}
 
 Status GetWindSupport(EndpointId ep, BitMask<WindBitmap> * value)
 {
     if (auto * c = FindClusterOnEndpoint(ep))
     {
         *value = c->GetWindSupport();
         return Status::Success;
     }
     return Status::UnsupportedEndpoint;
 }
 Status SetWindSupportUI(EndpointId ep, BitMask<WindBitmap> value)
 {
     return Status::UnsupportedWrite;
 }
 
 Status GetWindSetting(EndpointId ep, BitMask<WindBitmap> * value)
 {
     if (auto * c = FindClusterOnEndpoint(ep))
     {
         *value = c->GetWindSetting();
         return Status::Success;
     }
     return Status::UnsupportedEndpoint;
 }
Status SetWindSettingUI(EndpointId ep, BitMask<WindBitmap> value)
{
    return FanControl::SetWindSetting(ep, value);
}
 } // namespace
 // --- 2. UPDATE THIS FUNCTION ---
 void FanControl::UpdateState()
 {
     UpdateStateEnum(mEndpointId, mTargetFanMode, mFanMode, &SetFanModeUI, &GetFanModeAdapter);
     UpdateStateEnum(mEndpointId, mTargetAirflowDirection, mAirflowDirection, &SetAirflowDirectionUI, &GetAirflowDirection);
     UpdateStateEnum(mEndpointId, mTargetFanModeSequence, mFanModeSequence, &SetFanModeSequenceUI, &GetFanModeSequence);
 
     UpdateStateReadOnly(mEndpointId, mTargetPercent, mPercent, &GetPercentCurrent);
     UpdateStateReadOnly(mEndpointId, mTargetSpeedMax, mSpeedMax, &GetSpeedMaxAdapter);
     UpdateStateReadOnly(mEndpointId, mTargetSpeedCurrent, mSpeedCurrent, &GetSpeedCurrent);
    UpdateStateReadOnly(mEndpointId, mTargetFeatureMap, mFeatureMap, &GetFeatureMap);

    UpdateStateNullable(mEndpointId, mTargetPercentSetting, mPercentSetting, &SetPercentSettingUI, &FanControl::GetPercentSetting);
    UpdateStateNullable(mEndpointId, mTargetSpeedSetting, mSpeedSetting, &SetSpeedSettingUI, &FanControl::GetSpeedSetting);

    UpdateStateEnum(mEndpointId, mTargetRockSupport, mRockSupport, &SetRockSupportUI, &GetRockSupport);
     UpdateStateEnum(mEndpointId, mTargetRockSetting, mRockSetting, &SetRockSettingUI, &GetRockSetting);
     UpdateStateEnum(mEndpointId, mTargetWindSupport, mWindSupport, &SetWindSupportUI, &GetWindSupport);
     UpdateStateEnum(mEndpointId, mTargetWindSetting, mWindSetting, &SetWindSettingUI, &GetWindSetting);
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
 