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

#include "DeviceHeader.h"

#include <cstdio>

namespace chip::app {

lv_obj_t * CreateDeviceHeader(lv_obj_t * parent, const char * title, EndpointId endpointId)
{
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_height(card, 26);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(card, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(card, 2, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * titleLabel = lv_label_create(card);
    lv_label_set_text(titleLabel, title);
    lv_obj_set_style_text_color(titleLabel, lv_palette_lighten(LV_PALETTE_BLUE, 2), LV_PART_MAIN);

    char epBuf[16];
    snprintf(epBuf, sizeof(epBuf), "Ep %u", static_cast<unsigned int>(endpointId));
    lv_obj_t * epLabel = lv_label_create(card);
    lv_label_set_text(epLabel, epBuf);
    lv_obj_set_style_text_color(epLabel, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);

    return card;
}

} // namespace chip::app
