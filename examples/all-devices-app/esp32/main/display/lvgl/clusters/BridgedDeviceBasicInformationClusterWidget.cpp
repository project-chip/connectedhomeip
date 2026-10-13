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

#include "BridgedDeviceBasicInformationClusterWidget.h"
#include "DisplayNotificationHub.h"

#include <platform/CHIPDeviceLayer.h>

namespace chip::app {

namespace {

void UpdateReachableDisplay(lv_obj_t * stateLabel, lv_obj_t * toggleBtn, lv_obj_t * btnLabel, bool reachable)
{
    if (reachable)
    {
        lv_label_set_text_static(stateLabel, "Reachable");
        lv_obj_set_style_text_color(stateLabel, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
        lv_label_set_text_static(btnLabel, "Set away");
        lv_obj_set_style_bg_color(toggleBtn, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    }
    else
    {
        lv_label_set_text_static(stateLabel, "Not reachable");
        lv_obj_set_style_text_color(stateLabel, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
        lv_label_set_text_static(btnLabel, "Set reachable");
        lv_obj_set_style_bg_color(toggleBtn, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
    }
}

} // namespace

lv_obj_t * CreateBridgedDeviceBasicInformationClusterWidget(lv_obj_t * parent,
                                                            Clusters::BridgedDeviceBasicInformationCluster & cluster)
{
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_hor(card, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(card, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_row(card, 4, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * labelText = lv_label_create(card);
    lv_label_set_text(labelText, cluster.GetNodeLabel().c_str());

    lv_obj_t * uniqueIdText = lv_label_create(card);
    lv_label_set_text(uniqueIdText, cluster.GetUniqueId().c_str());
    lv_obj_set_style_text_color(uniqueIdText, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);

    lv_obj_t * reachableRow = lv_obj_create(card);
    lv_obj_set_width(reachableRow, LV_PCT(100));
    lv_obj_set_height(reachableRow, 44);
    lv_obj_set_flex_flow(reachableRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(reachableRow, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(reachableRow, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(reachableRow, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(reachableRow, 0, LV_PART_MAIN);
    lv_obj_clear_flag(reachableRow, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * stateLabel = lv_label_create(reachableRow);

    lv_obj_t * toggleBtn = lv_button_create(reachableRow);
    lv_obj_set_width(toggleBtn, 120);
    lv_obj_set_height(toggleBtn, 34);
    lv_obj_set_flex_flow(toggleBtn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(toggleBtn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_ver(toggleBtn, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(toggleBtn, 6, LV_PART_MAIN);

    lv_obj_t * btnLabel = lv_label_create(toggleBtn);
    lv_obj_set_style_text_color(btnLabel, lv_color_white(), LV_PART_MAIN);

    UpdateReachableDisplay(stateLabel, toggleBtn, btnLabel, cluster.GetReachable());

    // Toggling Reachable emits ReachableChanged, which is what a controller watches a bridge for.
    lv_obj_add_event_cb(
        toggleBtn,
        [](lv_event_t * event) {
            auto * clusterPtr = static_cast<Clusters::BridgedDeviceBasicInformationCluster *>(lv_event_get_user_data(event));
            DeviceLayer::SystemLayer().ScheduleLambda([clusterPtr]() { clusterPtr->SetReachable(!clusterPtr->GetReachable()); });
        },
        LV_EVENT_CLICKED, &cluster);

    // Subscribing with 'card' automatically unregisters when 'card' is deleted.
    DisplayNotificationHub::Instance().Subscribe(
        card, cluster.GetPaths()[0].mEndpointId, Clusters::BridgedDeviceBasicInformation::Id,
        [labelText, stateLabel, toggleBtn, btnLabel, &cluster](const ConcreteAttributePath & path) {
            if (path.mAttributeId == Clusters::BridgedDeviceBasicInformation::Attributes::Reachable::Id)
            {
                UpdateReachableDisplay(stateLabel, toggleBtn, btnLabel, cluster.GetReachable());
            }
            else if (path.mAttributeId == Clusters::BridgedDeviceBasicInformation::Attributes::NodeLabel::Id)
            {
                lv_label_set_text(labelText, cluster.GetNodeLabel().c_str());
            }
        });

    return card;
}

} // namespace chip::app
