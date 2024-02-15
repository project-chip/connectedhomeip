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
#include "light.h"

#include <imgui.h>

#include <math.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>

#include <app/clusters/level-control/level-control.h>
#include <app/clusters/on-off-server/on-off-server.h>

namespace example {
namespace Ui {
namespace Windows {
namespace {

using namespace chip::app::Clusters;
using chip::app::DataModel::Nullable;

ImVec4 HueSaturationToColor(float hueDegrees, float saturationPercent)
{
    saturationPercent /= 100.0f;

    float x = saturationPercent * static_cast<float>(1 - fabs(fmod(hueDegrees / 60, 2) - 1));

    if (hueDegrees < 60)
    {
        return ImVec4(saturationPercent, x, 0, 1.0f);
    }
    if (hueDegrees < 120)
    {
        return ImVec4(x, saturationPercent, 0, 1.0f);
    }
    if (hueDegrees < 180)
    {
        return ImVec4(0, saturationPercent, x, 1.0f);
    }
    if (hueDegrees < 240)
    {
        return ImVec4(0, x, saturationPercent, 1.0f);
    }
    if (hueDegrees < 300)
    {
        return ImVec4(x, 0, saturationPercent, 1.0f);
    }

    return ImVec4(saturationPercent, 0, x, 1.0f);
}

} // namespace

void Light::UpdateState()
{
    if (mTargetLightIsOn.HasValue())
    {
        chip::Protocols::InteractionModel::Status status = OnOffServer::Instance().setOnOffValue(
            mEndpointId, mTargetLightIsOn.Value() ? OnOff::Commands::On::Id : OnOff::Commands::Off::Id,
            false /* initiatedByLevelChange */);

        if (status != chip::Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(AppServer, "Failed to set on/off value: %d", chip::to_underlying(status));
        }

        mTargetLightIsOn.ClearValue();
    }
    OnOff::Attributes::OnOff::Get(mEndpointId, &mLightIsOn);

    // Level Control
    if (mTargetLevel.HasValue())
    {
        LevelControl::Commands::MoveToLevel::DecodableType data;

        data.level = mTargetLevel.Value();
        data.optionsMask.Set(LevelControl::OptionsBitmap::kExecuteIfOff);
        data.optionsOverride.Set(LevelControl::OptionsBitmap::kExecuteIfOff);

        (void) LevelControlServer::MoveToLevel(mEndpointId, data);

        mTargetLevel.ClearValue();
    }

    LevelControl::Attributes::CurrentLevel::Get(mEndpointId, mCurrentLevel);
    LevelControl::Attributes::MinLevel::Get(mEndpointId, &mMinLevel);
    LevelControl::Attributes::MaxLevel::Get(mEndpointId, &mMaxLevel);
    LevelControl::Attributes::RemainingTime::Get(mEndpointId, &mLevelRemainingTime10sOfSec);

    // Color control
    ColorControl::Attributes::ColorMode::Get(mEndpointId, &mColorMode);

    ColorControl::Attributes::CurrentHue::Get(mEndpointId, &mColorHue);
    ColorControl::Attributes::CurrentSaturation::Get(mEndpointId, &mColorSaturation);
    ColorControl::Attributes::CurrentX::Get(mEndpointId, &mColorX);
    ColorControl::Attributes::CurrentY::Get(mEndpointId, &mColorY);
    ColorControl::Attributes::ColorTemperatureMireds::Get(mEndpointId, &mColorTemperatureMireds);
}

void Light::Render()
{
    ImGui::Begin("Light state");
    ImGui::Text("Light on endpoint %d", mEndpointId);

    ImGui::Indent();
    {
        bool uiValue = mLightIsOn;
        ImGui::Checkbox("Light is ON", &uiValue);
        if (uiValue != mLightIsOn)
        {
            mTargetLightIsOn.SetValue(uiValue); // schedule future update
        }
    }

    // bright yellow vs dark yellow on/off view
    ImGui::ColorButton("LightIsOn", mLightIsOn ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(0.3f, 0.3f, 0.0f, 1.0f),
                       0 /* ImGuiColorEditFlags_* */, ImVec2(80, 80));
    ImGui::Unindent();

    ImGui::Text("Level Control:");
    ImGui::Indent();
    ImGui::Text("Remaining Time (1/10s): %d", mLevelRemainingTime10sOfSec);
    ImGui::Text("MIN Level:              %d", mMinLevel);
    ImGui::Text("MAX Level:              %d", mMaxLevel);
    if (mCurrentLevel.IsNull())
    {
        ImGui::Text("Current Level: NULL");
    }
    else
    {
        int uiValue = mCurrentLevel.Value();
        ImGui::SliderInt("Current Level", &uiValue, mMinLevel, mMaxLevel);
        if (uiValue != mCurrentLevel.Value())
        {
            mTargetLevel.SetValue(uiValue); // schedule future update
        }
    }
    ImGui::Unindent();

    ImGui::Text("Color Control:");
    ImGui::Indent();
    const char * mode = // based on ColorMode attribute: spec 3.2.7.9
        (mColorMode == kColorModeCurrentHueAndCurrentSaturation) ? "Hue/Saturation"
        : (mColorMode == kColorModeCurrentXAndCurrentY)          ? "X/Y"
        : (mColorMode == kColorModeColorTemperature)             ? "Temperature/Mireds"
                                                                 : "UNKNOWN";

    ImGui::Text("Mode: %s", mode);

    if (mColorMode == kColorModeCurrentHueAndCurrentSaturation)
    {
        const float hueDegrees        = (mColorHue * 360.0f) / 254.0f;
        const float saturationPercent = 100.0f * (mColorSaturation / 254.0f);

        ImGui::Text("Current Hue:        %d (%f deg)", mColorHue, hueDegrees);
        ImGui::Text("Current Saturation: %d (%f %%)", mColorSaturation, saturationPercent);

        ImGui::ColorButton("LightColor", HueSaturationToColor(hueDegrees, saturationPercent), 0 /* ImGuiColorEditFlags_* */,
                           ImVec2(80, 80));
    }
    else if (mColorMode == kColorModeCurrentXAndCurrentY)
    {
        ImGui::Text("Current X: %d", mColorX);
        ImGui::Text("Current Y: %d", mColorY);
    }
    else if (mColorMode == kColorModeColorTemperature)
    {
        ImGui::Text("Color Temperature Mireds: %d", mColorTemperatureMireds);
    }
    ImGui::Unindent();

    ImGui::End();
}

} // namespace Windows
} // namespace Ui
} // namespace example
