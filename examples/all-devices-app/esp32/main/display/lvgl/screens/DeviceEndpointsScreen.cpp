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

#include <device-factory/DeviceFactory.h>

#include <string>

void ShowDeviceEndpoints(lv_obj_t * parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_row(parent, 6, LV_PART_MAIN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    const std::string & activeDev = GetActiveDeviceType();
    bool isAllBridged             = (activeDev == "*" || activeDev == "aggregator");

    if (isAllBridged)
    {
        // Info banner
        lv_obj_t * header = lv_label_create(parent);
        lv_label_set_text_static(header, "Active Bridged Endpoints");
        lv_obj_set_style_text_color(header, lv_palette_lighten(LV_PALETTE_BLUE, 2), LV_PART_MAIN);

        auto & deviceFactory = chip::app::NoHooksDeviceFactory::GetInstance();
        int epNum            = 2; // Aggregator is on ep 1, bridged devices start on ep 2
        for (const auto & deviceType : deviceFactory.SupportedDeviceTypes())
        {
            if (deviceType == "aggregator" || deviceType == "bridged-node")
            {
                continue;
            }

            lv_obj_t * card = lv_obj_create(parent);
            lv_obj_set_width(card, LV_PCT(100));
            lv_obj_set_height(card, 38);
            lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(card, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_set_style_pad_all(card, 6, LV_PART_MAIN);
            lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

            lv_obj_t * nameLabel = lv_label_create(card);
            lv_label_set_text(nameLabel, deviceType.c_str());

            lv_obj_t * epLabel = lv_label_create(card);
            char epBuf[16];
            snprintf(epBuf, sizeof(epBuf), "Ep %d", epNum);
            lv_label_set_text(epLabel, epBuf);
            lv_obj_set_style_text_color(epLabel, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);

            epNum += 2; // Bridged node is on odd, device is on even
        }
    }
    else
    {
        // Single device mode
        lv_obj_t * card = lv_obj_create(parent);
        lv_obj_set_width(card, LV_PCT(100));
        lv_obj_set_height(card, 90);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(card, 10, LV_PART_MAIN);
        lv_obj_set_style_pad_row(card, 6, LV_PART_MAIN);

        lv_obj_t * nameLabel = lv_label_create(card);
        std::string nameText = "Device: " + activeDev;
        lv_label_set_text(nameLabel, nameText.c_str());
        lv_obj_set_style_text_color(nameLabel, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);

        lv_obj_t * epLabel = lv_label_create(card);
        lv_label_set_text_static(epLabel, "Endpoint 1 (Direct)");
        lv_obj_set_style_text_color(epLabel, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);

        lv_obj_t * note = lv_label_create(parent);
        lv_label_set_text_static(note, "Interactive controls will be attached here in Phase 4.");
        lv_obj_set_style_text_color(note, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
        lv_obj_set_style_text_align(note, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    }
}
