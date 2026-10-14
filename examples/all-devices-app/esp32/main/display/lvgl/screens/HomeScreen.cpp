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
#include "DisplayBoard.h"
#include "NavigationStack.h"
#include "SystemMenuScreen.h"
#include "Ui.h"

namespace {

void OnDevicesClicked(lv_event_t *)
{
    NavigationStack::Push("Devices", ShowDeviceEndpoints);
}

void OnSelectDeviceClicked(lv_event_t *)
{
    NavigationStack::Push("Device Type", ShowDeviceSelection);
}

void OnSystemClicked(lv_event_t *)
{
    NavigationStack::Push("System", ShowSystemMenu);
}

std::string TitleCase(std::string text)
{
    bool capitalize = true;
    for (char & c : text)
    {
        if (c == '-')
        {
            c          = ' ';
            capitalize = true;
        }
        else if (capitalize)
        {
            c          = static_cast<char>(toupper(static_cast<unsigned char>(c)));
            capitalize = false;
        }
    }
    return text;
}

// Accent gradient card naming the node and what it currently is.
void CreateHero(lv_obj_t * parent, const std::string & deviceType, bool isAllBridged)
{
    lv_obj_t * hero = lv_obj_create(parent);
    lv_obj_set_width(hero, LV_PCT(100));
    lv_obj_set_height(hero, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(hero, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hero, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(hero, Ui::Px(18, 8), LV_PART_MAIN);
    lv_obj_set_style_pad_column(hero, Ui::Px(16, 8), LV_PART_MAIN);
    lv_obj_set_style_bg_color(hero, Ui::Accent(), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(hero, Ui::AccentAlt(), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(hero, LV_GRAD_DIR_HOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(hero, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(hero, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(hero, Ui::AccentAlt(), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(hero, LV_OPA_50, LV_PART_MAIN);
    lv_obj_clear_flag(hero, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * badge = Ui::CreateIconBadge(hero, LV_SYMBOL_CHARGE, lv_color_white(), Ui::Px(64, 36));
    lv_obj_set_style_bg_opa(badge, LV_OPA_30, LV_PART_MAIN);

    lv_obj_t * text = lv_obj_create(hero);
    lv_obj_remove_style_all(text);
    lv_obj_set_flex_grow(text, 1);
    lv_obj_set_height(text, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(text, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(text, Ui::Px(4, 1), LV_PART_MAIN);
    lv_obj_clear_flag(text, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(text, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t * caption = lv_label_create(text);
    lv_label_set_text_fmt(caption, "%s  •  Matter", DisplayBoard::BoardName());
    lv_obj_set_style_text_font(caption, Ui::CaptionFont(), LV_PART_MAIN);
    lv_obj_set_style_text_color(caption, lv_color_hex(0xDDE4FF), LV_PART_MAIN);

    lv_obj_t * title = lv_label_create(text);
    lv_label_set_text(title, isAllBridged ? "Bridge, all device types" : TitleCase(deviceType).c_str());
    lv_obj_set_style_text_font(title, Ui::TitleFont(), LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_white(), LV_PART_MAIN);
    lv_label_set_long_mode(title, LV_LABEL_LONG_DOT);
    lv_obj_set_width(title, LV_PCT(100));
}

} // namespace

void ShowHome(lv_obj_t * parent)
{
    Ui::LayoutColumn(parent);

    const std::string activeDev = GetActiveDeviceType();
    const bool isAllBridged     = (activeDev == "*" || activeDev == "aggregator");

    CreateHero(parent, activeDev, isAllBridged);

    Ui::CreateTile(parent, LV_SYMBOL_LIST, Ui::Accent(), "Devices", "View and control endpoints", OnDevicesClicked);

    std::string current = "Current: " + (isAllBridged ? std::string("All Bridged (*)") : activeDev);
    Ui::CreateTile(parent, LV_SYMBOL_EDIT, Ui::AccentAlt(), "Update Device Type", current.c_str(), OnSelectDeviceClicked);

    Ui::CreateTile(parent, LV_SYMBOL_SETTINGS, Ui::Warning(), "System", "QR code, status and operations", OnSystemClicked);
}
