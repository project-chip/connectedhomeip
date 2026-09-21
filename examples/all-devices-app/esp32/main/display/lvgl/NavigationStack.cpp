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

#include <vector>

namespace NavigationStack {

namespace {

struct StackEntry
{
    std::string title;
    RenderScreenFn renderFn;
};

std::vector<StackEntry> sStack;
lv_obj_t * sTopBar           = nullptr;
lv_obj_t * sCrumbContainer   = nullptr;
lv_obj_t * sContentContainer = nullptr;

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

    // Refresh breadcrumbs. The current page is not shown: its name is already in the page
    // header and long names push the clickable ancestors off screen. At the root there is no
    // ancestor, so that single entry is shown instead.
    lv_obj_clean(sCrumbContainer);
    const size_t crumbCount = (sStack.size() == 1) ? 1 : sStack.size() - 1;
    for (size_t i = 0; i < crumbCount; ++i)
    {
        if (i > 0)
        {
            lv_obj_t * sep = lv_label_create(sCrumbContainer);
            lv_label_set_text_static(sep, ">");
            lv_obj_set_style_text_color(sep, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
        }

        if (sStack.size() == 1)
        {
            // Root: nothing to navigate back to, so a static label rather than a pill.
            lv_obj_t * rootLabel = lv_label_create(sCrumbContainer);
            lv_label_set_text(rootLabel, sStack[i].title.c_str());
            lv_obj_set_style_text_color(rootLabel, lv_color_white(), LV_PART_MAIN);
            lv_obj_set_style_pad_left(rootLabel, 4, LV_PART_MAIN);
            lv_obj_set_style_pad_right(rootLabel, 4, LV_PART_MAIN);
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

            // Keep the deepest crumb visible when the chain is wider than the bar.
            if (i == crumbCount - 1)
            {
                lv_obj_scroll_to_view(crumbBtn, LV_ANIM_OFF);
            }
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

    // Breadcrumb container (spans full top bar)
    sCrumbContainer = lv_obj_create(sTopBar);
    lv_obj_set_size(sCrumbContainer, LV_PCT(100), LV_PCT(100));
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

void Detach()
{
    sStack.clear();
    sTopBar           = nullptr;
    sCrumbContainer   = nullptr;
    sContentContainer = nullptr;
}

} // namespace NavigationStack
