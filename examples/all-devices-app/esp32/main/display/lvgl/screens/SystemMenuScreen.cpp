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

#include "SystemMenuScreen.h"
#include "CommissioningCodesScreen.h"
#include "DeviceInfoScreen.h"
#include "DeviceOperationsScreen.h"
#include "NavigationStack.h"

namespace {

void OnQRClicked(lv_event_t * event)
{
    NavigationStack::Push("QR Code", ShowCommissioningCodesScreen);
}

void OnStatusClicked(lv_event_t * event)
{
    NavigationStack::Push("Status", ShowDeviceInfo);
}

void OnOperationsClicked(lv_event_t * event)
{
    NavigationStack::Push("Operations", ShowDeviceOperations);
}

lv_obj_t * CreateMenuItem(lv_obj_t * parent, const char * title, const char * subtitle, lv_event_cb_t callback)
{
    lv_obj_t * btn = lv_button_create(parent);
    lv_obj_set_width(btn, LV_PCT(100));
    lv_obj_set_height(btn, 50);
    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(btn, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_row(btn, 2, LV_PART_MAIN);

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, title);

    if (subtitle != nullptr)
    {
        lv_obj_t * sub = lv_label_create(btn);
        lv_label_set_text(sub, subtitle);
        lv_obj_set_style_text_color(sub, lv_color_black(), LV_PART_MAIN);
    }

    lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, nullptr);
    return btn;
}

} // namespace

void ShowSystemMenu(lv_obj_t * parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_row(parent, 8, LV_PART_MAIN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    CreateMenuItem(parent, "Commissioning QR Code >", "Onboarding setup payload", OnQRClicked);
    CreateMenuItem(parent, "Status & Diagnostics >", "Fabrics, network IP, heap", OnStatusClicked);
    CreateMenuItem(parent, "Device Operations >", "Restart, factory reset", OnOperationsClicked);
}
