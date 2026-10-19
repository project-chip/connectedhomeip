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

#include "AggregatorScreen.h"
#include "DeviceHeader.h"
#include "DeviceScreenRegistry.h"
#include "clusters/IdentifyClusterWidget.h"

namespace chip::app {

namespace {

/// Direct children of an aggregator are bridged nodes. Counted from the screen registry
/// rather than the data model: the LVGL task must not touch CHIP state.
size_t CountBridgedNodes(EndpointId aggregatorEndpoint)
{
    size_t count = 0;
    DeviceScreenRegistry::Instance().WithEntries([&](const auto & entries) {
        for (const auto & entry : entries)
        {
            if (entry.parentEndpointId == aggregatorEndpoint)
            {
                count++;
            }
        }
    });
    return count;
}

} // namespace

void ShowAggregatorScreen(lv_obj_t * parent, Aggregator & device)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_hor(parent, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(parent, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_row(parent, 4, LV_PART_MAIN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    CreateDeviceHeader(parent, "Aggregator", device.GetEndpointId());

    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_height(card, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_hor(card, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(card, 6, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * countLabel = lv_label_create(card);
    lv_label_set_text_fmt(countLabel, "Bridged endpoints: %u",
                          static_cast<unsigned int>(CountBridgedNodes(device.GetEndpointId())));

    CreateIdentifyClusterWidget(parent, device.IdentifyCluster());
}

} // namespace chip::app
