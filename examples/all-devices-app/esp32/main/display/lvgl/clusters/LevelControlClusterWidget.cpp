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

#include "LevelControlClusterWidget.h"

#include <cstdio>
#include <platform/PlatformManager.h>

namespace chip::app {

namespace {

struct LevelControlWidgetContext
{
    Clusters::LevelControlCluster & cluster;
    lv_obj_t * valueLabel;
    lv_obj_t * slider;
    uint8_t minLevel;
    uint8_t maxLevel;
};

void UpdateLevelLabel(LevelControlWidgetContext * ctx, uint8_t level)
{
    unsigned int pct = 0;
    if (ctx->maxLevel > ctx->minLevel)
    {
        pct = static_cast<unsigned int>((static_cast<uint32_t>(level - ctx->minLevel) * 100) / (ctx->maxLevel - ctx->minLevel));
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "Brightness: %u%% (%u)", pct, static_cast<unsigned int>(level));
    lv_label_set_text(ctx->valueLabel, buf);
}

void OnSliderValueChanged(lv_event_t * event)
{
    auto * ctx = static_cast<LevelControlWidgetContext *>(lv_event_get_user_data(event));
    if (ctx == nullptr)
    {
        return;
    }

    int32_t val   = lv_slider_get_value(ctx->slider);
    uint8_t level = static_cast<uint8_t>(val);

    {
        chip::DeviceLayer::StackLock lock;
        ctx->cluster.MoveToLevelWithOnOff(level, DataModel::Nullable<uint16_t>(), BitMask<Clusters::LevelControl::OptionsBitmap>(),
                                          BitMask<Clusters::LevelControl::OptionsBitmap>());
    }

    UpdateLevelLabel(ctx, level);
}

void OnWidgetDeleted(lv_event_t * event)
{
    auto * ctx = static_cast<LevelControlWidgetContext *>(lv_event_get_user_data(event));
    delete ctx;
}

} // namespace

lv_obj_t * CreateLevelControlClusterWidget(lv_obj_t * parent, Clusters::LevelControlCluster & cluster)
{
    uint8_t minLevel = 1;
    uint8_t maxLevel = 254;
    uint8_t curLevel = 254;

    {
        chip::DeviceLayer::StackLock lock;
        minLevel = cluster.GetMinLevel();
        maxLevel = cluster.GetMaxLevel();
        curLevel = cluster.GetCurrentLevel().ValueOr(maxLevel);
    }

    auto * ctx = new LevelControlWidgetContext{ cluster, nullptr, nullptr, minLevel, maxLevel };

    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(card, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_row(card, 10, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    ctx->valueLabel = lv_label_create(card);
    lv_obj_set_style_text_align(ctx->valueLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    ctx->slider = lv_slider_create(card);
    lv_obj_set_width(ctx->slider, LV_PCT(90));
    lv_slider_set_range(ctx->slider, minLevel, maxLevel);
    lv_slider_set_value(ctx->slider, curLevel, LV_ANIM_OFF);

    lv_obj_add_event_cb(ctx->slider, OnSliderValueChanged, LV_EVENT_VALUE_CHANGED, ctx);
    lv_obj_add_event_cb(card, OnWidgetDeleted, LV_EVENT_DELETE, ctx);

    UpdateLevelLabel(ctx, curLevel);

    return card;
}

} // namespace chip::app
