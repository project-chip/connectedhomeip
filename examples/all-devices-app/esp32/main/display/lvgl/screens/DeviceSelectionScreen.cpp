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

#include "DeviceSelectionScreen.h"
#include "DeviceTypeSelection.h"

#include <device-factory/DeviceFactory.h>
#include <esp_log.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cstring>
#include <string>
#include <vector>

namespace {

const char TAG[] = "DeviceSelect";

std::string sPendingDeviceType;
lv_obj_t * sSwitchModal = nullptr;

void CloseModal()
{
    if (sSwitchModal != nullptr)
    {
        lv_msgbox_close(sSwitchModal);
        sSwitchModal = nullptr;
    }
}

void OnCancelClicked(lv_event_t * event)
{
    CloseModal();
}

void OnConfirmSwitchClicked(lv_event_t * event)
{
    std::string target = sPendingDeviceType;
    CloseModal();
    ESP_LOGI(TAG, "User confirmed switch to '%s'", target.c_str());
    LogErrorOnFailure(SetDeviceTypeAndRestart(target));
}

void OnDeviceButtonClicked(lv_event_t * event)
{
    lv_obj_t * btn   = lv_event_get_target_obj(event);
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    if (label == nullptr)
    {
        return;
    }

    const char * txt = lv_label_get_text(label);
    if (txt == nullptr)
    {
        return;
    }

    // Skip leading indent spaces ("    ")
    while (*txt == ' ')
    {
        txt++;
    }

    if (std::strcmp(txt, "All Bridged (*)") == 0)
    {
        sPendingDeviceType = "*";
    }
    else
    {
        sPendingDeviceType = txt;
    }

    CloseModal();

    sSwitchModal = lv_msgbox_create(nullptr);
    lv_msgbox_add_title(sSwitchModal, "Switch Device");

    std::string prompt = "Switch to '" + sPendingDeviceType + "' and restart?";
    lv_msgbox_add_text(sSwitchModal, prompt.c_str());

    lv_obj_t * cancelBtn = lv_msgbox_add_footer_button(sSwitchModal, "Cancel");
    lv_obj_add_event_cb(cancelBtn, OnCancelClicked, LV_EVENT_CLICKED, nullptr);

    lv_obj_t * switchBtn = lv_msgbox_add_footer_button(sSwitchModal, "Switch & Restart");
    lv_obj_add_event_cb(switchBtn, OnConfirmSwitchClicked, LV_EVENT_CLICKED, nullptr);
}

} // namespace

void ShowDeviceSelection(lv_obj_t * parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_row(parent, 6, LV_PART_MAIN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    const std::string & activeDev = GetActiveDeviceType();
    bool isAllBridged             = (activeDev == "*" || activeDev == "aggregator");

    struct DeviceOption
    {
        std::string label;
        bool isActive;
    };

    std::vector<DeviceOption> options;
    options.push_back({ "All Bridged (*)", isAllBridged });

    auto & deviceFactory = chip::app::NoHooksDeviceFactory::GetInstance();
    for (const auto & deviceType : deviceFactory.SupportedDeviceTypes())
    {
        if (deviceType == "aggregator" || deviceType == "bridged-node")
        {
            continue;
        }
        bool isActive = (!isAllBridged && deviceType == activeDev);
        options.push_back({ deviceType, isActive });
    }

    for (const auto & opt : options)
    {
        lv_obj_t * btn = lv_button_create(parent);
        lv_obj_set_width(btn, LV_PCT(100));
        lv_obj_set_height(btn, 36);

        lv_obj_t * label = lv_label_create(btn);
        if (opt.isActive)
        {
            lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
            std::string activeLabel = LV_SYMBOL_OK "  " + opt.label;
            lv_label_set_text(label, activeLabel.c_str());
        }
        else
        {
            std::string inactiveLabel = "    " + opt.label;
            lv_label_set_text(label, inactiveLabel.c_str());
            lv_obj_add_event_cb(btn, OnDeviceButtonClicked, LV_EVENT_CLICKED, nullptr);
        }
        lv_obj_center(label);
    }
}
