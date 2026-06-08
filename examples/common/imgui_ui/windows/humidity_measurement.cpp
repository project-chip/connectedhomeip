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
#include "humidity_measurement.h"
#include "accessor_updaters.h"

#include <imgui.h>

#include <math.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>

namespace example {
namespace Ui {
namespace Windows {

void HumidityMeasurement::UpdateState()
{
    UpdateStateNullable(mEndpointId, mTargetRelativeHumidityValue, mRelativeHumidityValue,
                        &chip::app::Clusters::RelativeHumidityMeasurement::Attributes::MeasuredValue::Get);

    UpdateStateNullable(mEndpointId, mTargetRelativeHumidityMin, mRelativeHumidityMin,
                        &chip::app::Clusters::RelativeHumidityMeasurement::Attributes::MinMeasuredValue::Get);

    UpdateStateNullable(mEndpointId, mTargetRelativeHumidityMax, mRelativeHumidityMax,
                        &chip::app::Clusters::RelativeHumidityMeasurement::Attributes::MaxMeasuredValue::Get);

    mRelativeHumidityTolerance = mTargetRelativeHumidityTolerance;
}

void HumidityMeasurement::Render()
{
    ImGui::Begin(mTitle.c_str());
    ImGui::Text("On Endpoint %d", mEndpointId);

    uint16_t hValue   = mRelativeHumidityValue;
    uint16_t minValue = mRelativeHumidityMin;
    uint16_t maxValue = mRelativeHumidityMax;
    uint16_t tValue   = mRelativeHumidityTolerance;

    int uiValue     = hValue;
    int uiMin       = minValue;
    int uiMax       = maxValue;
    int uiTolerance = tValue;

    ImGui::LabelText("Relative Humidity Value", "%d", uiValue);
    ImGui::LabelText("Relative Humidity Min", "%d", uiMin);
    ImGui::LabelText("Relative Humidity Max", "%d", uiMax);
    ImGui::LabelText("Relative Humidity Tolerance", "%d", uiTolerance);

    ImGui::End();
}

} // namespace Windows
} // namespace Ui
} // namespace example
