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

#include "OccupancySensingClusterWidget.h"
#include "DisplayNotificationHub.h"

#include <clusters/OccupancySensing/Attributes.h>
#include <clusters/OccupancySensing/Ids.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip::app {

namespace {

void UpdateOccupancyDisplay(lv_obj_t * stateLabel, lv_obj_t * toggleBtn, lv_obj_t * btnLabel, bool isOccupied)
{
    if (isOccupied)
    {
        lv_label_set_text_static(stateLabel, "State: OCCUPIED");
        lv_obj_set_style_text_color(stateLabel, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_MAIN);
        lv_label_set_text_static(btnLabel, "Clear Occupancy");
        lv_obj_set_style_bg_color(toggleBtn, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
    }
    else
    {
        lv_label_set_text_static(stateLabel, "State: UNOCCUPIED");
        lv_obj_set_style_text_color(stateLabel, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
        lv_label_set_text_static(btnLabel, "Set Occupied");
        lv_obj_set_style_bg_color(toggleBtn, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_MAIN);
    }
}

} // namespace

lv_obj_t * CreateOccupancySensingClusterWidget(lv_obj_t * parent, Clusters::OccupancySensingCluster & cluster)
{
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_height(card, 48);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(card, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(card, 4, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * stateLabel = lv_label_create(card);

    lv_obj_t * toggleBtn = lv_button_create(card);
    lv_obj_set_width(toggleBtn, 124);
    lv_obj_set_height(toggleBtn, 34);
    lv_obj_set_flex_flow(toggleBtn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(toggleBtn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_ver(toggleBtn, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(toggleBtn, 6, LV_PART_MAIN);

    lv_obj_t * btnLabel = lv_label_create(toggleBtn);
    lv_obj_set_style_text_color(btnLabel, lv_color_white(), LV_PART_MAIN);

    UpdateOccupancyDisplay(stateLabel, toggleBtn, btnLabel, cluster.IsOccupied());

    lv_obj_add_event_cb(
        toggleBtn,
        [](lv_event_t * event) {
            auto * clusterPtr = static_cast<Clusters::OccupancySensingCluster *>(lv_event_get_user_data(event));
            DeviceLayer::SystemLayer().ScheduleLambda([clusterPtr]() { clusterPtr->SetOccupancy(!clusterPtr->IsOccupied()); });
        },
        LV_EVENT_CLICKED, &cluster);

    DisplayNotificationHub::Instance().Subscribe(card, cluster.GetPaths()[0].mEndpointId, Clusters::OccupancySensing::Id,
                                                 [stateLabel, toggleBtn, btnLabel, &cluster](const ConcreteAttributePath & path) {
                                                     if (path.mAttributeId == Clusters::OccupancySensing::Attributes::Occupancy::Id)
                                                     {
                                                         UpdateOccupancyDisplay(stateLabel, toggleBtn, btnLabel,
                                                                                cluster.IsOccupied());
                                                     }
                                                 });

    return card;
}

} // namespace chip::app
