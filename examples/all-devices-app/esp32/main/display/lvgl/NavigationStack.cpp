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

#include <esp_log.h>
#include <vector>

namespace NavigationStack {

namespace {

const char TAG[] = "NavStack";

struct StackEntry
{
    std::string title;
    RenderScreenFn renderFn;
};

std::vector<StackEntry> sStack;
lv_obj_t * sTopBar           = nullptr;
lv_obj_t * sBackBtn          = nullptr;
lv_obj_t * sCrumbContainer   = nullptr;
lv_obj_t * sContentContainer = nullptr;

void OnBackClicked(lv_event_t * event)
{
    Pop();
}

void OnCrumbClicked(lv_event_t * event)
{
    auto targetLevel = static_cast<size_t>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(event)));
    PopTo(targetLevel);
}

void RefreshView()
{
    if (sStack.empty() || sContentContainer == nullptr || sTopBar == nullptr)
    {
        return;
    }

    // Update Back button visibility
    if (sStack.size() <= 1)
    {
        lv_obj_add_flag(sBackBtn, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_clear_flag(sBackBtn, LV_OBJ_FLAG_HIDDEN);
    }

    // Refresh breadcrumbs
    lv_obj_clean(sCrumbContainer);
    for (size_t i = 0; i < sStack.size(); ++i)
    {
        if (i > 0)
        {
            lv_obj_t * sep = lv_label_create(sCrumbContainer);
            lv_label_set_text_static(sep, ">");
            lv_obj_set_style_text_color(sep, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
        }

        if (i == sStack.size() - 1)
        {
            // Current active leaf: styled static label
            lv_obj_t * leafLabel = lv_label_create(sCrumbContainer);
            lv_label_set_text(leafLabel, sStack[i].title.c_str());
            lv_obj_set_style_text_color(leafLabel, lv_color_white(), LV_PART_MAIN);
            lv_obj_set_style_pad_left(leafLabel, 4, LV_PART_MAIN);
            lv_obj_set_style_pad_right(leafLabel, 4, LV_PART_MAIN);
            lv_obj_scroll_to_view(leafLabel, LV_ANIM_OFF);
        }
        else
        {
            // Ancestor link: clickable pill button
            lv_obj_t * crumbBtn = lv_button_create(sCrumbContainer);
            lv_obj_set_height(crumbBtn, 30);
            lv_obj_set_style_pad_top(crumbBtn, 2, LV_PART_MAIN);
            lv_obj_set_style_pad_bottom(crumbBtn, 2, LV_PART_MAIN);
            lv_obj_set_style_pad_left(crumbBtn, 8, LV_PART_MAIN);
            lv_obj_set_style_pad_right(crumbBtn, 8, LV_PART_MAIN);
            lv_obj_set_style_radius(crumbBtn, 4, LV_PART_MAIN);
            lv_obj_set_style_bg_color(crumbBtn, lv_palette_darken(LV_PALETTE_GREY, 3), LV_PART_MAIN);
            lv_obj_set_ext_click_area(crumbBtn, 6);

            lv_obj_t * crumbLabel = lv_label_create(crumbBtn);
            lv_label_set_text(crumbLabel, sStack[i].title.c_str());
            lv_obj_set_style_text_color(crumbLabel, lv_palette_lighten(LV_PALETTE_BLUE, 2), LV_PART_MAIN);
            lv_obj_center(crumbLabel);

            lv_obj_add_event_cb(crumbBtn, OnCrumbClicked, LV_EVENT_CLICKED, reinterpret_cast<void *>(static_cast<uintptr_t>(i)));
        }
    }

    // Render content
    lv_obj_clean(sContentContainer);
    sStack.back().renderFn(sContentContainer);
}

} // namespace

void Init(lv_obj_t * rootScreen)
{
    sStack.clear();

    lv_obj_clean(rootScreen);
    lv_obj_set_flex_flow(rootScreen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(rootScreen, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(rootScreen, 0, LV_PART_MAIN);
    lv_obj_clear_flag(rootScreen, LV_OBJ_FLAG_SCROLLABLE);

    // Top Bar (38px high)
    sTopBar = lv_obj_create(rootScreen);
    lv_obj_set_size(sTopBar, LV_PCT(100), 38);
    lv_obj_set_flex_flow(sTopBar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sTopBar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(sTopBar, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(sTopBar, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_left(sTopBar, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_right(sTopBar, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_column(sTopBar, 6, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sTopBar, lv_palette_darken(LV_PALETTE_GREY, 4), LV_PART_MAIN);
    lv_obj_set_style_border_width(sTopBar, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(sTopBar, 0, LV_PART_MAIN);
    lv_obj_clear_flag(sTopBar, LV_OBJ_FLAG_SCROLLABLE);

    // Back button (36x30px with 8px extended click area)
    sBackBtn = lv_button_create(sTopBar);
    lv_obj_set_size(sBackBtn, 36, 30);
    lv_obj_set_style_pad_all(sBackBtn, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(sBackBtn, 4, LV_PART_MAIN);
    lv_obj_set_ext_click_area(sBackBtn, 8);
    lv_obj_t * backLabel = lv_label_create(sBackBtn);
    lv_label_set_text_static(backLabel, "<");
    lv_obj_center(backLabel);
    lv_obj_add_event_cb(sBackBtn, OnBackClicked, LV_EVENT_CLICKED, nullptr);

    // Breadcrumb container
    sCrumbContainer = lv_obj_create(sTopBar);
    lv_obj_set_size(sCrumbContainer, LV_SIZE_CONTENT, LV_PCT(100));
    lv_obj_set_flex_grow(sCrumbContainer, 1);
    lv_obj_set_flex_flow(sCrumbContainer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sCrumbContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(sCrumbContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(sCrumbContainer, 4, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sCrumbContainer, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(sCrumbContainer, 0, LV_PART_MAIN);
    lv_obj_add_flag(sCrumbContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(sCrumbContainer, LV_DIR_HOR);
    lv_obj_set_scrollbar_mode(sCrumbContainer, LV_SCROLLBAR_MODE_OFF);

    // Content container (remaining height, ~202px)
    sContentContainer = lv_obj_create(rootScreen);
    lv_obj_set_size(sContentContainer, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(sContentContainer, 1);
    lv_obj_set_style_pad_all(sContentContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(sContentContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(sContentContainer, 0, LV_PART_MAIN);
}

void Push(const std::string & title, RenderScreenFn renderFn)
{
    sStack.push_back({ title, renderFn });
    RefreshView();
}

void Pop()
{
    if (sStack.size() > 1)
    {
        sStack.pop_back();
        RefreshView();
    }
}

void PopTo(size_t level)
{
    if (level < sStack.size())
    {
        sStack.erase(sStack.begin() + level + 1, sStack.end());
        RefreshView();
    }
}

void ResetToRoot()
{
    if (sStack.size() > 1)
    {
        sStack.erase(sStack.begin() + 1, sStack.end());
        RefreshView();
    }
}

size_t GetDepth()
{
    return sStack.size();
}

} // namespace NavigationStack
