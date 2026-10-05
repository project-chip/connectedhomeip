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

#include "LevelControlClusterWidget.h"
#include "CommandStatusLog.h"
#include "DisplayNotificationHub.h"

#include <algorithm>
#include <cstdio>
#include <platform/CHIPDeviceLayer.h>

namespace chip::app {

namespace {

void UpdateLevelLabel(lv_obj_t * valueLabel, uint8_t level, uint8_t minLevel, uint8_t maxLevel)
{
    unsigned int pct = 0;
    if (maxLevel > minLevel)
    {
        // A cluster may report a level outside the range it advertises; clamping keeps the
        // subtraction below from wrapping.
        const uint8_t clamped = std::clamp(level, minLevel, maxLevel);

        pct = static_cast<unsigned int>((static_cast<uint32_t>(clamped - minLevel) * 100) / (maxLevel - minLevel));
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "Brightness: %u%% (%u)", pct, static_cast<unsigned int>(level));
    lv_label_set_text(valueLabel, buf);
}

} // namespace

lv_obj_t * CreateLevelControlClusterWidget(lv_obj_t * parent, Clusters::LevelControlCluster & cluster)
{
    uint8_t minLevel = cluster.GetMinLevel();
    uint8_t maxLevel = cluster.GetMaxLevel();
    uint8_t curLevel = cluster.GetCurrentLevel().ValueOr(maxLevel);

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
    lv_slider_set_range(slider, minLevel, maxLevel);
    lv_slider_set_value(slider, curLevel, LV_ANIM_OFF);

    // Initial label render
    UpdateLevelLabel(valueLabel, curLevel, minLevel, maxLevel);

    // Local touch: the label follows the knob, while the command is sent once the finger lifts.
    // Commanding on every value change turns a single drag into a burst of MoveToLevel commands.
    lv_obj_add_event_cb(
        slider,
        [](lv_event_t * event) {
            auto * sliderObj = static_cast<lv_obj_t *>(lv_event_get_target(event));
            auto * labelObj  = lv_obj_get_child(lv_obj_get_parent(sliderObj), 0);

            UpdateLevelLabel(labelObj, static_cast<uint8_t>(lv_slider_get_value(sliderObj)),
                             static_cast<uint8_t>(lv_slider_get_min_value(sliderObj)),
                             static_cast<uint8_t>(lv_slider_get_max_value(sliderObj)));
        },
        LV_EVENT_VALUE_CHANGED, nullptr);

    lv_obj_add_event_cb(
        slider,
        [](lv_event_t * event) {
            auto * clusterPtr = static_cast<Clusters::LevelControlCluster *>(lv_event_get_user_data(event));
            auto * sliderObj  = static_cast<lv_obj_t *>(lv_event_get_target(event));
            uint8_t level     = static_cast<uint8_t>(lv_slider_get_value(sliderObj));

            DeviceLayer::SystemLayer().ScheduleLambda([clusterPtr, level]() {
                LogCommandFailure("MoveToLevelWithOnOff",
                                  clusterPtr->MoveToLevelWithOnOff(level, DataModel::Nullable<uint16_t>(), {}, {}));
            });
        },
        LV_EVENT_RELEASED, &cluster);

    // Data model notifications (from local touch or network): updates slider & label when CurrentLevel changes.
    // Subscribing with 'card' automatically unregisters when 'card' is deleted.
    DisplayNotificationHub::Instance().Subscribe(
        card, cluster.GetPaths()[0].mEndpointId, Clusters::LevelControl::Id,
        [slider, valueLabel, &cluster, minLevel, maxLevel](const ConcreteAttributePath & path) {
            if (path.mAttributeId == Clusters::LevelControl::Attributes::CurrentLevel::Id)
            {
                uint8_t level = cluster.GetCurrentLevel().ValueOr(maxLevel);
                if (!lv_slider_is_dragged(slider))
                {
                    lv_slider_set_value(slider, level, LV_ANIM_OFF);
                    UpdateLevelLabel(valueLabel, level, minLevel, maxLevel);
                }
            }
        });

    return card;
}

} // namespace chip::app
