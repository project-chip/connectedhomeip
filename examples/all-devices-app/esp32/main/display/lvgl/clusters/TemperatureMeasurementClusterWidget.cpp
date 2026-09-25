/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include "TemperatureMeasurementClusterWidget.h"
#include "DisplayNotificationHub.h"

#include <clusters/TemperatureMeasurement/Attributes.h>
#include <clusters/TemperatureMeasurement/Ids.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <platform/CHIPDeviceLayer.h>

namespace chip::app {

namespace {

// Slider operates in 0.1 °C steps (e.g. 225 = 22.5 °C; Matter attribute is 2250 = 0.01 °C).
constexpr int32_t kDefaultMinTenthsC = -100; // -10.0 °C
constexpr int32_t kDefaultMaxTenthsC = 500;  // +50.0 °C

void UpdateTemperatureLabel(lv_obj_t * valueLabel, int32_t tenthsC)
{
    char buf[32];
    const char * sign = (tenthsC < 0) ? "-" : "";
    int32_t absTenths = std::abs(tenthsC);
    snprintf(buf, sizeof(buf), "Temp: %s%d.%d C", sign, static_cast<int>(absTenths / 10), static_cast<int>(absTenths % 10));
    lv_label_set_text(valueLabel, buf);
}

} // namespace

lv_obj_t * CreateTemperatureMeasurementClusterWidget(lv_obj_t * parent, Clusters::TemperatureMeasurementCluster & cluster)
{
    int32_t minTenths = cluster.GetMinMeasuredValue().IsNull() ? kDefaultMinTenthsC : (cluster.GetMinMeasuredValue().Value() / 10);
    int32_t maxTenths = cluster.GetMaxMeasuredValue().IsNull() ? kDefaultMaxTenthsC : (cluster.GetMaxMeasuredValue().Value() / 10);
    int32_t curTenths = cluster.GetMeasuredValue().IsNull() ? 220 : (cluster.GetMeasuredValue().Value() / 10);

    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(card, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(card, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_row(card, 4, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * valueLabel = lv_label_create(card);
    lv_obj_set_style_text_align(valueLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_t * slider = lv_slider_create(card);
    lv_obj_set_width(slider, LV_PCT(96));
    lv_slider_set_range(slider, minTenths, maxTenths);
    lv_slider_set_value(slider, curTenths, LV_ANIM_OFF);

    UpdateTemperatureLabel(valueLabel, curTenths);

    // Local touch: the label follows the knob, while the attribute is written once the finger
    // lifts. Writing on every value change turns a single drag into a burst of reports.
    lv_obj_add_event_cb(
        slider,
        [](lv_event_t * event) {
            auto * sliderObj = static_cast<lv_obj_t *>(lv_event_get_target(event));
            auto * labelObj  = lv_obj_get_child(lv_obj_get_parent(sliderObj), 0);

            UpdateTemperatureLabel(labelObj, lv_slider_get_value(sliderObj));
        },
        LV_EVENT_VALUE_CHANGED, nullptr);

    lv_obj_add_event_cb(
        slider,
        [](lv_event_t * event) {
            auto * clusterPtr = static_cast<Clusters::TemperatureMeasurementCluster *>(lv_event_get_user_data(event));
            auto * sliderObj  = static_cast<lv_obj_t *>(lv_event_get_target(event));

            int16_t measuredVal = static_cast<int16_t>(lv_slider_get_value(sliderObj) * 10);
            DeviceLayer::SystemLayer().ScheduleLambda(
                [clusterPtr, measuredVal]() { LogErrorOnFailure(clusterPtr->SetMeasuredValue(measuredVal)); });
        },
        LV_EVENT_RELEASED, &cluster);

    DisplayNotificationHub::Instance().Subscribe(card, cluster.GetPaths()[0].mEndpointId, Clusters::TemperatureMeasurement::Id,
                                                 [slider, valueLabel, &cluster](const ConcreteAttributePath & path) {
                                                     if (path.mAttributeId ==
                                                         Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Id)
                                                     {
                                                         if (!cluster.GetMeasuredValue().IsNull() && !lv_slider_is_dragged(slider))
                                                         {
                                                             int32_t tenthsC = cluster.GetMeasuredValue().Value() / 10;
                                                             lv_slider_set_value(slider, tenthsC, LV_ANIM_OFF);
                                                             UpdateTemperatureLabel(valueLabel, tenthsC);
                                                         }
                                                     }
                                                 });

    return card;
}

} // namespace chip::app
