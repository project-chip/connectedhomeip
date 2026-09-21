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

#include "HomeScreen.h"
#include "DeviceEndpointsScreen.h"
#include "DeviceSelectionScreen.h"
#include "DeviceTypeSelection.h"
#include "NavigationStack.h"
#include "SystemMenuScreen.h"

namespace {

void OnDevicesClicked(lv_event_t * event)
{
    NavigationStack::Push("Devices", ShowDeviceEndpoints);
}

void OnSelectDeviceClicked(lv_event_t * event)
{
    NavigationStack::Push("Select Device", ShowDeviceSelection);
}

void OnSystemClicked(lv_event_t * event)
{
    NavigationStack::Push("System", ShowSystemMenu);
}

lv_obj_t * CreateHomeButton(lv_obj_t * parent, const char * title, const char * subtitle, lv_event_cb_t callback)
{
    lv_obj_t * btn = lv_button_create(parent);
    lv_obj_set_width(btn, LV_PCT(100));
    lv_obj_set_height(btn, 52);
    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(btn, 8, LV_PART_MAIN);
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

void ShowHome(lv_obj_t * parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_row(parent, 8, LV_PART_MAIN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    const std::string activeDev = GetActiveDeviceType();
    bool isAllBridged           = (activeDev == "*" || activeDev == "aggregator");

    CreateHomeButton(parent, "Devices >", "View & Control Endpoints", OnDevicesClicked);

    std::string selectSub = "Current: " + (isAllBridged ? "All Bridged (*)" : activeDev);
    CreateHomeButton(parent, "Switch Device Type >", selectSub.c_str(), OnSelectDeviceClicked);

    CreateHomeButton(parent, "System >", "QR Code, Status & Operations", OnSystemClicked);
}
