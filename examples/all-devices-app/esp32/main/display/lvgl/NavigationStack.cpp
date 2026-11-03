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

#include "NavigationStack.h"
#include "DisplayBoard.h"
#include "Ui.h"

#include <cstdio>
#include <vector>

namespace NavigationStack {
namespace {

struct StackEntry
{
    std::string title;
    RenderScreenFn renderFn;
};

struct Indicators
{
    bool wifiConnected  = false;
    bool bleConnected   = false;
    uint8_t fabricCount = 0;
};

std::vector<StackEntry> sStack;
Indicators sIndicators;

lv_obj_t * sTopBar           = nullptr;
lv_obj_t * sCrumbContainer   = nullptr;
lv_obj_t * sStatusContainer  = nullptr;
lv_obj_t * sContentContainer = nullptr;

void OnCrumbClicked(lv_event_t * event)
{
    auto targetLevel = static_cast<size_t>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(event)));
    PopTo(targetLevel);
}

lv_obj_t * CreateIndicator(lv_obj_t * parent, const char * text, lv_color_t color)
{
    lv_obj_t * label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, Ui::CaptionFont(), LV_PART_MAIN);
    lv_obj_set_style_text_color(label, color, LV_PART_MAIN);
    return label;
}

void RenderIndicators()
{
    if (sStatusContainer == nullptr)
    {
        return;
    }
    lv_obj_clean(sStatusContainer);

    if (sIndicators.bleConnected)
    {
        CreateIndicator(sStatusContainer, LV_SYMBOL_BLUETOOTH, Ui::Accent());
    }

    CreateIndicator(sStatusContainer, LV_SYMBOL_WIFI, sIndicators.wifiConnected ? Ui::Success() : Ui::TextMuted());

    if (sIndicators.fabricCount > 0)
    {
        char text[16];
        snprintf(text, sizeof(text), LV_SYMBOL_HOME " %u", static_cast<unsigned>(sIndicators.fabricCount));
        CreateIndicator(sStatusContainer, text, Ui::Success());
    }
}

void AddChevron(lv_obj_t * parent)
{
    lv_obj_t * chevron = lv_label_create(parent);
    lv_label_set_text_static(chevron, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_font(chevron, Ui::CaptionFont(), LV_PART_MAIN);
    lv_obj_set_style_text_color(chevron, Ui::TextMuted(), LV_PART_MAIN);
}

void AddAncestorPill(lv_obj_t * parent, const StackEntry & entry, size_t level)
{
    lv_obj_t * pill = lv_button_create(parent);
    lv_obj_set_height(pill, Ui::Px(36, 28));
    lv_obj_set_style_radius(pill, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(pill, Ui::SurfaceRaised(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(pill, Ui::Outline(), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_pad_hor(pill, Ui::Px(14, 8), LV_PART_MAIN);
    lv_obj_set_style_pad_ver(pill, 0, LV_PART_MAIN);
    lv_obj_set_ext_click_area(pill, 8);
    lv_obj_add_event_cb(pill, OnCrumbClicked, LV_EVENT_CLICKED, reinterpret_cast<void *>(static_cast<uintptr_t>(level)));

    lv_obj_t * label = lv_label_create(pill);
    if (level == 0)
    {
        lv_label_set_text_static(label, LV_SYMBOL_HOME);
    }
    else
    {
        lv_label_set_text(label, entry.title.c_str());
    }
    lv_obj_set_style_text_font(label, Ui::CaptionFont(), LV_PART_MAIN);
    lv_obj_set_style_text_color(label, Ui::Text(), LV_PART_MAIN);
    lv_obj_center(label);
}

void AddCurrentTitle(lv_obj_t * parent, const StackEntry & entry, bool isRoot)
{
    lv_obj_t * title = lv_label_create(parent);
    if (isRoot)
    {
        lv_label_set_text_fmt(title, LV_SYMBOL_HOME "  %s", entry.title.c_str());
    }
    else
    {
        lv_label_set_text(title, entry.title.c_str());
    }
    lv_obj_set_style_text_font(title, Ui::HeadingFont(), LV_PART_MAIN);
    lv_obj_set_style_text_color(title, Ui::Text(), LV_PART_MAIN);
    lv_obj_set_style_pad_left(title, Ui::Px(6, 2), LV_PART_MAIN);
    lv_obj_scroll_to_view(title, LV_ANIM_OFF);
}

void RefreshView(bool forward)
{
    if (sStack.empty() || sContentContainer == nullptr || sTopBar == nullptr)
    {
        return;
    }

    lv_obj_clean(sCrumbContainer);
    for (size_t i = 0; i + 1 < sStack.size(); ++i)
    {
        if (i > 0)
        {
            AddChevron(sCrumbContainer);
        }
        AddAncestorPill(sCrumbContainer, sStack[i], i);
    }
    if (sStack.size() > 1)
    {
        AddChevron(sCrumbContainer);
    }
    AddCurrentTitle(sCrumbContainer, sStack.back(), sStack.size() == 1);

    lv_obj_clean(sContentContainer);
    lv_obj_set_style_translate_x(sContentContainer, 0, LV_PART_MAIN);
    sStack.back().renderFn(sContentContainer);
    Ui::AnimateEntrance(sContentContainer, forward);
}

} // namespace

void Init(lv_obj_t * rootScreen)
{
    sStack.clear();

    lv_obj_clean(rootScreen);
    lv_obj_set_flex_flow(rootScreen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(rootScreen, DisplayBoard::EdgeInset(), LV_PART_MAIN);
    lv_obj_set_style_pad_row(rootScreen, Ui::Px(6, 0), LV_PART_MAIN);
    lv_obj_clear_flag(rootScreen, LV_OBJ_FLAG_SCROLLABLE);

    sTopBar = lv_obj_create(rootScreen);
    lv_obj_remove_style_all(sTopBar);
    lv_obj_set_size(sTopBar, LV_PCT(100), Ui::Px(56, 38));
    lv_obj_set_flex_flow(sTopBar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sTopBar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(sTopBar, Ui::Px(8, 6), LV_PART_MAIN);
    lv_obj_set_style_pad_column(sTopBar, Ui::Px(10, 6), LV_PART_MAIN);
    lv_obj_set_style_border_side(sTopBar, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
    lv_obj_set_style_border_width(sTopBar, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(sTopBar, Ui::Outline(), LV_PART_MAIN);
    lv_obj_set_style_border_opa(sTopBar, LV_OPA_50, LV_PART_MAIN);
    lv_obj_clear_flag(sTopBar, LV_OBJ_FLAG_SCROLLABLE);

    sCrumbContainer = lv_obj_create(sTopBar);
    lv_obj_remove_style_all(sCrumbContainer);
    lv_obj_set_height(sCrumbContainer, LV_PCT(100));
    lv_obj_set_flex_grow(sCrumbContainer, 1);
    lv_obj_set_flex_flow(sCrumbContainer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sCrumbContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(sCrumbContainer, Ui::Px(8, 4), LV_PART_MAIN);
    lv_obj_add_flag(sCrumbContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(sCrumbContainer, LV_DIR_HOR);
    lv_obj_set_scrollbar_mode(sCrumbContainer, LV_SCROLLBAR_MODE_OFF);

    sStatusContainer = lv_obj_create(sTopBar);
    lv_obj_remove_style_all(sStatusContainer);
    lv_obj_set_size(sStatusContainer, LV_SIZE_CONTENT, LV_PCT(100));
    lv_obj_set_flex_flow(sStatusContainer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sStatusContainer, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(sStatusContainer, Ui::Px(12, 6), LV_PART_MAIN);
    lv_obj_clear_flag(sStatusContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(sStatusContainer, LV_OBJ_FLAG_CLICKABLE);
    RenderIndicators();

    sContentContainer = lv_obj_create(rootScreen);
    lv_obj_remove_style_all(sContentContainer);
    lv_obj_set_width(sContentContainer, LV_PCT(100));
    lv_obj_set_flex_grow(sContentContainer, 1);
    lv_obj_add_flag(sContentContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(sContentContainer, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(sContentContainer, LV_SCROLLBAR_MODE_AUTO);
}

void Push(const std::string & title, RenderScreenFn renderFn)
{
    sStack.push_back({ title, renderFn });
    RefreshView(true);
}

void Pop()
{
    if (sStack.size() > 1)
    {
        sStack.pop_back();
        RefreshView(false);
    }
}

void PopTo(size_t level)
{
    if (level < sStack.size())
    {
        sStack.erase(sStack.begin() + level + 1, sStack.end());
        RefreshView(false);
    }
}

void ResetToRoot()
{
    if (sStack.size() > 1)
    {
        sStack.erase(sStack.begin() + 1, sStack.end());
        RefreshView(false);
    }
}

void RefreshCurrent()
{
    RefreshView(false);
}

size_t GetDepth()
{
    return sStack.size();
}

void SetIndicators(bool wifiConnected, bool bleConnected, uint8_t fabricCount)
{
    sIndicators = { wifiConnected, bleConnected, fabricCount };
    RenderIndicators();
}

void Detach()
{
    sStack.clear();
    sTopBar           = nullptr;
    sCrumbContainer   = nullptr;
    sStatusContainer  = nullptr;
    sContentContainer = nullptr;
}

} // namespace NavigationStack
