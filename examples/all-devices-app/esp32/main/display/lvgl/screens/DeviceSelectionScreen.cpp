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
#include "AppDeviceFactory.h"
#include "DeviceTypeSelection.h"
#include "Ui.h"

#include <esp_log.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <string>
#include <vector>

namespace {

const char TAG[] = "DeviceSelect";

std::string sPendingDeviceType;
lv_obj_t * sSwitchModal = nullptr;

struct DeviceOption
{
    std::string key;   // Device type argument understood by the factory.
    std::string label; // What the button shows.
    bool isActive;
};

// Backs the user_data of every button on this screen, so it has to outlive them. The
// screen is only rebuilt after its old widgets have been deleted, so rewriting it then
// is safe.
std::vector<DeviceOption> sOptions;

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
    const auto * option = static_cast<const DeviceOption *>(lv_event_get_user_data(event));
    VerifyOrReturn(option != nullptr);

    sPendingDeviceType = option->key;

    CloseModal();

    sSwitchModal = lv_msgbox_create(nullptr);
    lv_msgbox_add_title(sSwitchModal, "Update Device Type");

    std::string prompt = "Switch to '" + option->label + "' and restart?";
    lv_msgbox_add_text(sSwitchModal, prompt.c_str());

    lv_obj_t * cancelBtn = lv_msgbox_add_footer_button(sSwitchModal, "Cancel");
    lv_obj_add_event_cb(cancelBtn, OnCancelClicked, LV_EVENT_CLICKED, nullptr);

    lv_obj_t * switchBtn = lv_msgbox_add_footer_button(sSwitchModal, "Switch & Restart");
    lv_obj_add_event_cb(switchBtn, OnConfirmSwitchClicked, LV_EVENT_CLICKED, nullptr);
}

} // namespace

void ShowDeviceSelection(lv_obj_t * parent)
{
    Ui::LayoutColumn(parent);
    lv_obj_set_style_pad_row(parent, Ui::Px(8, 6), LV_PART_MAIN);

    const std::string activeDev = GetActiveDeviceType();
    bool isAllBridged           = (activeDev == "*" || activeDev == "aggregator");

    sOptions.clear();
    sOptions.push_back({ "*", "All Bridged (*)", isAllBridged });

    // The application creates devices through AppDeviceFactory, so list that instance:
    // any other instantiation of the factory template is a separate registry and would
    // miss the creators added by RegisterDeviceFactoryOverrides().
    auto & deviceFactory = chip::app::AppDeviceFactory::GetInstance();
    for (const auto & deviceType : deviceFactory.SupportedDeviceTypes())
    {
        if (deviceType == "aggregator" || deviceType == "bridged-node")
        {
            continue;
        }
        bool isActive = (!isAllBridged && deviceType == activeDev);
        sOptions.push_back({ deviceType, deviceType, isActive });
    }

    for (auto & opt : sOptions)
    {
        lv_obj_t * row = lv_button_create(parent);
        lv_obj_set_width(row, LV_PCT(100));
        lv_obj_set_height(row, Ui::Px(56, 36));
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(row, Ui::Px(14, 8), LV_PART_MAIN);
        lv_obj_set_style_radius(row, Ui::Px(16, 8), LV_PART_MAIN);
        lv_obj_set_style_bg_color(row, opt.isActive ? Ui::Success() : Ui::Surface(), LV_PART_MAIN);
        lv_obj_set_style_border_width(row, 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(row, opt.isActive ? Ui::Success() : Ui::Outline(), LV_PART_MAIN);

        lv_obj_t * mark = lv_label_create(row);
        lv_label_set_text_static(mark, opt.isActive ? LV_SYMBOL_OK : LV_SYMBOL_DUMMY);
        lv_obj_set_width(mark, Ui::Px(24, 16));
        lv_obj_set_style_text_color(mark, lv_color_white(), LV_PART_MAIN);

        lv_obj_t * label = lv_label_create(row);
        lv_label_set_text(label, opt.label.c_str());
        lv_obj_set_flex_grow(label, 1);
        lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
        lv_obj_set_style_text_color(label, opt.isActive ? lv_color_white() : Ui::Text(), LV_PART_MAIN);

        if (!opt.isActive)
        {
            lv_obj_t * chevron = lv_label_create(row);
            lv_label_set_text_static(chevron, LV_SYMBOL_RIGHT);
            lv_obj_set_style_text_color(chevron, Ui::TextMuted(), LV_PART_MAIN);
            lv_obj_add_event_cb(row, OnDeviceButtonClicked, LV_EVENT_CLICKED, &opt);
        }
    }
}
