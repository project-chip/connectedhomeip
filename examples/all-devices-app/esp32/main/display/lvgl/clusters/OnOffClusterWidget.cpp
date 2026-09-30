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

#include "OnOffClusterWidget.h"
#include "DisplayNotificationHub.h"

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip::app {

namespace {

void UpdateOnOffDisplay(lv_obj_t * stateLabel, lv_obj_t * toggleBtn, lv_obj_t * btnLabel, bool isOn)
{
    if (isOn)
    {
        lv_label_set_text_static(stateLabel, "Power: ON");
        lv_obj_set_style_text_color(stateLabel, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
        lv_label_set_text_static(btnLabel, "Turn OFF");
        lv_obj_set_style_bg_color(toggleBtn, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    }
    else
    {
        lv_label_set_text_static(stateLabel, "Power: OFF");
        lv_obj_set_style_text_color(stateLabel, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
        lv_label_set_text_static(btnLabel, "Turn ON");
        lv_obj_set_style_bg_color(toggleBtn, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
    }
}

} // namespace

lv_obj_t * CreateOnOffClusterWidget(lv_obj_t * parent, Clusters::OnOffCluster & cluster)
{
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_height(card, 44);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(card, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(card, 4, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * stateLabel = lv_label_create(card);

    lv_obj_t * toggleBtn = lv_button_create(card);
    lv_obj_set_width(toggleBtn, 100);
    lv_obj_set_height(toggleBtn, 34);
    lv_obj_set_flex_flow(toggleBtn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(toggleBtn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_ver(toggleBtn, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(toggleBtn, 6, LV_PART_MAIN);

    lv_obj_t * btnLabel = lv_label_create(toggleBtn);
    lv_obj_set_style_text_color(btnLabel, lv_color_white(), LV_PART_MAIN);

    // Initial render
    UpdateOnOffDisplay(stateLabel, toggleBtn, btnLabel, cluster.GetOnOff());

    // Local touch: schedule cluster toggle on Matter event loop asynchronously
    lv_obj_add_event_cb(
        toggleBtn,
        [](lv_event_t * event) {
            auto * clusterPtr = static_cast<Clusters::OnOffCluster *>(lv_event_get_user_data(event));
            DeviceLayer::SystemLayer().ScheduleLambda(
                [clusterPtr]() { LogErrorOnFailure(clusterPtr->SetOnOff(!clusterPtr->GetOnOff())); });
        },
        LV_EVENT_CLICKED, &cluster);

    // Data model notifications (from local touch or network): updates UI when OnOff attribute changes.
    // Subscribing with 'card' automatically unregisters when 'card' is deleted.
    DisplayNotificationHub::Instance().Subscribe(card, cluster.GetPaths()[0].mEndpointId, Clusters::OnOff::Id,
                                                 [stateLabel, toggleBtn, btnLabel, &cluster](const ConcreteAttributePath & path) {
                                                     if (path.mAttributeId == Clusters::OnOff::Attributes::OnOff::Id)
                                                     {
                                                         UpdateOnOffDisplay(stateLabel, toggleBtn, btnLabel, cluster.GetOnOff());
                                                     }
                                                 });

    return card;
}

} // namespace chip::app
