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

#include "DeviceEndpointsScreen.h"
#include "DeviceTypeSelection.h"

// TODO: Implement dynamic screen registration.
// Devices (including composed devices like refrigerator or oven, which span
// multiple endpoints) should dynamically register their UI screens/endpoints
// with the display subsystem when constructed via DeviceFactory hooks,
// rather than statically inspecting supported device types or assuming fixed
// endpoint structures.

void ShowDeviceEndpoints(lv_obj_t * parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_row(parent, 8, LV_PART_MAIN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    const std::string & activeDev = GetActiveDeviceType();
    bool isAllBridged             = (activeDev == "*" || activeDev == "aggregator");

    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(card, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_row(card, 6, LV_PART_MAIN);

    lv_obj_t * header = lv_label_create(card);
    lv_label_set_text_static(header, "Device Endpoints");
    lv_obj_set_style_text_color(header, lv_palette_lighten(LV_PALETTE_BLUE, 2), LV_PART_MAIN);

    lv_obj_t * configLabel = lv_label_create(card);
    std::string configText = "Configured: " + (isAllBridged ? std::string("All Bridged (*)") : activeDev);
    lv_label_set_text(configLabel, configText.c_str());

    lv_obj_t * todoNote = lv_label_create(parent);
    lv_obj_set_width(todoNote, LV_PCT(100));
    lv_label_set_text_static(
        todoNote,
        "Dynamic device endpoint registration is not yet implemented.\n\n"
        "Screens will be dynamically registered during device construction via DeviceFactory hooks.");
    lv_obj_set_style_text_color(todoNote, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
}
