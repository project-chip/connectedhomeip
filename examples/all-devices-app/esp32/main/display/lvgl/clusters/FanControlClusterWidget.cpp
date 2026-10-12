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

#include "FanControlClusterWidget.h"
#include "CommandStatusLog.h"
#include "DisplayNotificationHub.h"

#include <clusters/FanControl/Attributes.h>
#include <clusters/FanControl/Enums.h>
#include <clusters/FanControl/Ids.h>
#include <cstdio>
#include <platform/CHIPDeviceLayer.h>

namespace chip::app {

namespace {

using FanModeEnum         = Clusters::FanControl::FanModeEnum;
using FanModeSequenceEnum = Clusters::FanControl::FanModeSequenceEnum;

struct ModeButtonDef
{
    FanModeEnum mode;
    const char * label;
};

constexpr ModeButtonDef kAllModeButtons[] = {
    { FanModeEnum::kOff, "Off" },   { FanModeEnum::kLow, "Low" },   { FanModeEnum::kMedium, "Med" },
    { FanModeEnum::kHigh, "High" }, { FanModeEnum::kAuto, "Auto" },
};

bool IsModeSupported(FanModeSequenceEnum seq, FanModeEnum mode)
{
    switch (mode)
    {
    case FanModeEnum::kOff:
    case FanModeEnum::kHigh:
        return true;
    case FanModeEnum::kLow:
        return seq != FanModeSequenceEnum::kOffHigh && seq != FanModeSequenceEnum::kOffHighAuto;
    case FanModeEnum::kMedium:
        return seq == FanModeSequenceEnum::kOffLowMedHigh || seq == FanModeSequenceEnum::kOffLowMedHighAuto;
    case FanModeEnum::kAuto:
        return seq == FanModeSequenceEnum::kOffLowHighAuto || seq == FanModeSequenceEnum::kOffLowMedHighAuto ||
            seq == FanModeSequenceEnum::kOffHighAuto;
    default:
        return false;
    }
}

void UpdateSpeedLabel(lv_obj_t * label, uint8_t pct, FanModeEnum mode)
{
    char buf[32];
    if (mode == FanModeEnum::kAuto)
    {
        snprintf(buf, sizeof(buf), "Speed: Auto (%u%%)", static_cast<unsigned int>(pct));
    }
    else
    {
        snprintf(buf, sizeof(buf), "Speed: %u%%", static_cast<unsigned int>(pct));
    }
    lv_label_set_text(label, buf);
}

void UpdateModeButtons(lv_obj_t * modeRow, FanModeEnum activeMode)
{
    uint32_t count = lv_obj_get_child_cnt(modeRow);
    for (uint32_t i = 0; i < count; ++i)
    {
        lv_obj_t * btn = lv_obj_get_child(modeRow, i);
        auto btnMode   = static_cast<FanModeEnum>(reinterpret_cast<uintptr_t>(lv_obj_get_user_data(btn)));
        if (btnMode == activeMode)
        {
            lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
        }
        else
        {
            lv_obj_set_style_bg_color(btn, lv_palette_darken(LV_PALETTE_GREY, 2), LV_PART_MAIN);
        }
    }
}

} // namespace

lv_obj_t * CreateFanControlClusterWidget(lv_obj_t * parent, Clusters::FanControlCluster & cluster)
{
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(card, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(card, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_row(card, 6, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    // Row 1: Mode selector buttons
    lv_obj_t * modeRow = lv_obj_create(card);
    lv_obj_set_width(modeRow, LV_PCT(100));
    lv_obj_set_height(modeRow, 32);
    lv_obj_set_flex_flow(modeRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(modeRow, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(modeRow, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(modeRow, 4, LV_PART_MAIN);
    lv_obj_set_style_border_width(modeRow, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(modeRow, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_clear_flag(modeRow, LV_OBJ_FLAG_SCROLLABLE);

    FanModeSequenceEnum seq = cluster.GetFanModeSequence();
    for (const auto & def : kAllModeButtons)
    {
        if (!IsModeSupported(seq, def.mode))
        {
            continue;
        }
        lv_obj_t * btn = lv_button_create(modeRow);
        lv_obj_set_height(btn, 28);
        lv_obj_set_flex_grow(btn, 1);
        lv_obj_set_style_pad_all(btn, 2, LV_PART_MAIN);
        lv_obj_set_user_data(btn, reinterpret_cast<void *>(static_cast<uintptr_t>(def.mode)));

        lv_obj_t * lbl = lv_label_create(btn);
        lv_label_set_text_static(lbl, def.label);
        lv_obj_set_style_text_color(lbl, lv_color_white(), LV_PART_MAIN);
        lv_obj_center(lbl);

        lv_obj_add_event_cb(
            btn,
            [](lv_event_t * event) {
                auto * clusterPtr = static_cast<Clusters::FanControlCluster *>(lv_event_get_user_data(event));
                auto * btnObj     = static_cast<lv_obj_t *>(lv_event_get_target(event));
                auto mode         = static_cast<FanModeEnum>(reinterpret_cast<uintptr_t>(lv_obj_get_user_data(btnObj)));
                DeviceLayer::SystemLayer().ScheduleLambda(
                    [clusterPtr, mode]() { LogCommandFailure("SetFanMode", clusterPtr->SetFanMode(mode)); });
            },
            LV_EVENT_CLICKED, &cluster);
    }

    // Row 2: Speed percentage readout label
    lv_obj_t * speedLabel = lv_label_create(card);
    lv_obj_set_style_text_align(speedLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    // Row 3: Speed slider (0 - 100%)
    lv_obj_t * slider = lv_slider_create(card);
    lv_obj_set_width(slider, LV_PCT(96));
    lv_slider_set_range(slider, 0, 100);

    uint8_t curPct      = cluster.GetPercentSetting().ValueOr(cluster.GetPercentCurrent());
    FanModeEnum curMode = cluster.GetFanMode();
    lv_slider_set_value(slider, curPct, LV_ANIM_OFF);
    UpdateSpeedLabel(speedLabel, curPct, curMode);
    UpdateModeButtons(modeRow, curMode);

    // The label follows the knob, while the write is sent once the finger lifts: writing on every
    // value change turns a single drag into a burst of PercentSetting writes.
    lv_obj_add_event_cb(
        slider,
        [](lv_event_t * event) {
            auto * clusterPtr = static_cast<Clusters::FanControlCluster *>(lv_event_get_user_data(event));
            auto * sliderObj  = static_cast<lv_obj_t *>(lv_event_get_target(event));
            auto * labelObj   = lv_obj_get_child(lv_obj_get_parent(sliderObj), 1);

            UpdateSpeedLabel(labelObj, static_cast<uint8_t>(lv_slider_get_value(sliderObj)), clusterPtr->GetFanMode());
        },
        LV_EVENT_VALUE_CHANGED, &cluster);

    lv_obj_add_event_cb(
        slider,
        [](lv_event_t * event) {
            auto * clusterPtr = static_cast<Clusters::FanControlCluster *>(lv_event_get_user_data(event));
            auto * sliderObj  = static_cast<lv_obj_t *>(lv_event_get_target(event));
            uint8_t pct       = static_cast<uint8_t>(lv_slider_get_value(sliderObj));

            DeviceLayer::SystemLayer().ScheduleLambda([clusterPtr, pct]() {
                LogCommandFailure("SetPercentSetting", clusterPtr->SetPercentSetting(DataModel::MakeNullable<chip::Percent>(pct)));
            });
        },
        LV_EVENT_RELEASED, &cluster);

    DisplayNotificationHub::Instance().Subscribe(
        card, cluster.GetPaths()[0].mEndpointId, Clusters::FanControl::Id,
        [modeRow, speedLabel, slider, &cluster](const ConcreteAttributePath & path) {
            if (path.mAttributeId == Clusters::FanControl::Attributes::FanMode::Id ||
                path.mAttributeId == Clusters::FanControl::Attributes::PercentSetting::Id ||
                path.mAttributeId == Clusters::FanControl::Attributes::PercentCurrent::Id)
            {
                FanModeEnum mode = cluster.GetFanMode();
                uint8_t pct      = cluster.GetPercentSetting().ValueOr(cluster.GetPercentCurrent());
                UpdateModeButtons(modeRow, mode);
                if (!lv_slider_is_dragged(slider))
                {
                    lv_slider_set_value(slider, pct, LV_ANIM_OFF);
                    UpdateSpeedLabel(speedLabel, pct, mode);
                }
            }
        });

    return card;
}

} // namespace chip::app
