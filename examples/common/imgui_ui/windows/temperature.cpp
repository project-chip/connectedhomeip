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
#include "temperature.h"

#include <imgui.h>

#include <math.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <limits>
#include <cmath>

using namespace chip::app::Clusters::TemperatureMeasurement;

namespace example {
namespace Ui {
namespace Windows {
namespace {

typedef chip::app::DataModel::Nullable<int16_t> RawTemperature;

const float TEMPERATURE_SCALE_FACTOR = 0.01f;

float toDegc(RawTemperature raw) {
    if (raw.IsNull()) return std::numeric_limits<float>::quiet_NaN();
    return float(raw.Value()) * TEMPERATURE_SCALE_FACTOR;
}

RawTemperature fromDegc(float degc) {
    RawTemperature t;
    if (not isnan(degc)) {
        t.SetNonNull(degc / TEMPERATURE_SCALE_FACTOR);
    } else {
        t.SetNull();
    }
    return t;
}

}

Temperature::Temperature(chip::EndpointId endpointId, const char * title):
    mEndpointId(endpointId), 
    mTitle(title),
    mFirstUpdate(true) {
}

void Temperature::UpdateState()
{
    if (mFirstUpdate) {
        Attributes::MeasuredValue::Set(mEndpointId, fromDegc(10.0));
        Attributes::MinMeasuredValue::Set(mEndpointId, fromDegc(-30.0));
        Attributes::MaxMeasuredValue::Set(mEndpointId, fromDegc(+70.0));
    }
    mFirstUpdate = false;

    if (not mMeasuredValue.IsNull()) {
        Attributes::MeasuredValue::Set(mEndpointId, mMeasuredValue);
    }
    Attributes::MeasuredValue::Get(mEndpointId, mMeasuredValue);
    Attributes::MinMeasuredValue::Get(mEndpointId, mMinMeasuredValue);
    Attributes::MaxMeasuredValue::Get(mEndpointId, mMaxMeasuredValue);
}

void Temperature::Render()
{
    ImGui::Begin(mTitle.c_str());
    ImGui::Text("On Endpoint %d", mEndpointId);

    float temperature_degc = toDegc(mMeasuredValue);
    mMeasuredValue.SetNull();
 
    if (ImGui::SliderFloat("Measured Value", &temperature_degc, toDegc(mMinMeasuredValue), toDegc(mMaxMeasuredValue), "%.2fdegC")) {
        mMeasuredValue = fromDegc(temperature_degc);
    }

    ImGui::End();
}

} // namespace Windows
} // namespace Ui
} // namespace example
