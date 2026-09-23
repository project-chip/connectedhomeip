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

#include "IdentifyClusterWidget.h"
#include "DisplayNotificationHub.h"

#include <platform/CHIPDeviceLayer.h>

namespace chip::app {

namespace {

void UpdateIdentifyDisplay(lv_obj_t * stateLabel, lv_obj_t * stopBtn, uint16_t identifyTime)
{
    if (identifyTime > 0)
    {
        lv_label_set_text_fmt(stateLabel, "Identifying (%u s)", static_cast<unsigned int>(identifyTime));
        lv_obj_set_style_text_color(stateLabel, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_MAIN);
        lv_obj_clear_state(stopBtn, LV_STATE_DISABLED);
    }
    else
    {
        lv_label_set_text_static(stateLabel, "Idle");
        lv_obj_set_style_text_color(stateLabel, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
        lv_obj_add_state(stopBtn, LV_STATE_DISABLED);
    }
}

} // namespace

lv_obj_t * CreateIdentifyClusterWidget(lv_obj_t * parent, Clusters::IdentifyCluster & cluster)
{
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_height(card, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_hor(card, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(card, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_row(card, 4, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * title = lv_label_create(card);
    lv_label_set_text_static(title, "Identify");

    lv_obj_t * row = lv_obj_create(card);
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_height(row, 44);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(row, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * stateLabel = lv_label_create(row);

    lv_obj_t * stopBtn = lv_button_create(row);
    lv_obj_set_width(stopBtn, 120);
    lv_obj_set_height(stopBtn, 34);
    lv_obj_set_flex_flow(stopBtn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(stopBtn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_ver(stopBtn, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(stopBtn, 6, LV_PART_MAIN);

    lv_obj_t * stopLabel = lv_label_create(stopBtn);
    lv_label_set_text_static(stopLabel, "Stop");
    lv_obj_set_style_text_color(stopLabel, lv_color_white(), LV_PART_MAIN);

    UpdateIdentifyDisplay(stateLabel, stopBtn, cluster.GetIdentifyTime());

    lv_obj_add_event_cb(
        stopBtn,
        [](lv_event_t * event) {
            auto * clusterPtr = static_cast<Clusters::IdentifyCluster *>(lv_event_get_user_data(event));
            DeviceLayer::SystemLayer().ScheduleLambda([clusterPtr]() { clusterPtr->StopIdentifying(); });
        },
        LV_EVENT_CLICKED, &cluster);

    // Subscribing with 'card' automatically unregisters when 'card' is deleted.
    DisplayNotificationHub::Instance().Subscribe(card, cluster.GetPaths()[0].mEndpointId, Clusters::Identify::Id,
                                                 [stateLabel, stopBtn, &cluster](const ConcreteAttributePath & path) {
                                                     if (path.mAttributeId == Clusters::Identify::Attributes::IdentifyTime::Id)
                                                     {
                                                         UpdateIdentifyDisplay(stateLabel, stopBtn, cluster.GetIdentifyTime());
                                                     }
                                                 });

    return card;
}

} // namespace chip::app
