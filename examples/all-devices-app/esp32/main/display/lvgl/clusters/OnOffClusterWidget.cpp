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

#include "OnOffClusterWidget.h"

#include <platform/PlatformManager.h>

namespace chip::app {

namespace {

struct OnOffWidgetContext
{
    Clusters::OnOffCluster & cluster;
    lv_obj_t * stateLabel;
    lv_obj_t * toggleBtn;
    lv_obj_t * btnLabel;
};

void UpdateOnOffDisplay(OnOffWidgetContext * ctx)
{
    bool isOn = false;
    {
        chip::DeviceLayer::StackLock lock;
        isOn = ctx->cluster.GetOnOff();
    }
    if (isOn)
    {
        lv_label_set_text_static(ctx->stateLabel, "Power: ON");
        lv_obj_set_style_text_color(ctx->stateLabel, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
        lv_label_set_text_static(ctx->btnLabel, "Turn OFF");
        lv_obj_set_style_bg_color(ctx->toggleBtn, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    }
    else
    {
        lv_label_set_text_static(ctx->stateLabel, "Power: OFF");
        lv_obj_set_style_text_color(ctx->stateLabel, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
        lv_label_set_text_static(ctx->btnLabel, "Turn ON");
        lv_obj_set_style_bg_color(ctx->toggleBtn, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
    }
}

void OnToggleClicked(lv_event_t * event)
{
    auto * ctx = static_cast<OnOffWidgetContext *>(lv_event_get_user_data(event));
    if (ctx == nullptr)
    {
        return;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;
    {
        chip::DeviceLayer::StackLock lock;
        bool newState = !ctx->cluster.GetOnOff();
        err           = ctx->cluster.SetOnOff(newState);
    }
    if (err == CHIP_NO_ERROR)
    {
        UpdateOnOffDisplay(ctx);
    }
}

void OnWidgetDeleted(lv_event_t * event)
{
    auto * ctx = static_cast<OnOffWidgetContext *>(lv_event_get_user_data(event));
    delete ctx;
}

} // namespace

lv_obj_t * CreateOnOffClusterWidget(lv_obj_t * parent, Clusters::OnOffCluster & cluster)
{
    auto * ctx = new OnOffWidgetContext{ cluster, nullptr, nullptr, nullptr };

    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(card, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_row(card, 10, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    ctx->stateLabel = lv_label_create(card);
    lv_obj_set_style_text_align(ctx->stateLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    ctx->toggleBtn = lv_button_create(card);
    lv_obj_set_width(ctx->toggleBtn, 160);
    lv_obj_set_height(ctx->toggleBtn, 48);
    lv_obj_set_flex_flow(ctx->toggleBtn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ctx->toggleBtn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    ctx->btnLabel = lv_label_create(ctx->toggleBtn);
    lv_obj_set_style_text_color(ctx->btnLabel, lv_color_white(), LV_PART_MAIN);

    lv_obj_add_event_cb(ctx->toggleBtn, OnToggleClicked, LV_EVENT_CLICKED, ctx);
    lv_obj_add_event_cb(card, OnWidgetDeleted, LV_EVENT_DELETE, ctx);

    UpdateOnOffDisplay(ctx);

    return card;
}

} // namespace chip::app
