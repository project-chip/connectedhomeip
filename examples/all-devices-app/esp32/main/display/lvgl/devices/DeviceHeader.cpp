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
#include "Ui.h"

#include <cstdio>

namespace chip::app {

lv_obj_t * CreateDeviceHeader(lv_obj_t * parent, const char * title, EndpointId endpointId)
{
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_height(card, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(card, Ui::Px(16, 8), LV_PART_MAIN);
    lv_obj_set_style_pad_ver(card, Ui::Px(10, 2), LV_PART_MAIN);
    lv_obj_set_style_bg_color(card, Ui::SurfaceRaised(), LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * titleLabel = Ui::CreateHeading(card, title);
    lv_label_set_long_mode(titleLabel, LV_LABEL_LONG_DOT);
    lv_obj_set_flex_grow(titleLabel, 1);

    char epBuf[16];
    snprintf(epBuf, sizeof(epBuf), "EP %u", static_cast<unsigned int>(endpointId));
    Ui::CreatePill(card, epBuf, Ui::Accent());
    return card;
}

} // namespace chip::app
