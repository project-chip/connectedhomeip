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

#include "ValveConfigurationAndControlClusterWidget.h"
#include "DisplayNotificationHub.h"

#include <clusters/ValveConfigurationAndControl/Attributes.h>
#include <clusters/ValveConfigurationAndControl/Ids.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#include <algorithm>
#include <cstdio>

namespace chip::app {

namespace {

using Clusters::ValveConfigurationAndControlCluster;
using Clusters::ValveConfigurationAndControl::Feature;
using Clusters::ValveConfigurationAndControl::ValveStateEnum;

constexpr uint8_t kMinLevel = ValveConfigurationAndControlCluster::kMinLevelValuePercent;
constexpr uint8_t kMaxLevel = ValveConfigurationAndControlCluster::kMaxLevelValuePercent;

struct StateAppearance
{
    const char * text;
    lv_palette_t palette;
};

StateAppearance AppearanceFor(const DataModel::Nullable<ValveStateEnum> & state)
{
    if (state.IsNull())
    {
        return { "Unknown", LV_PALETTE_GREY };
    }

    switch (state.Value())
    {
    case ValveStateEnum::kClosed:
        return { "Closed", LV_PALETTE_GREY };
    case ValveStateEnum::kOpen:
        return { "Open", LV_PALETTE_GREEN };
    case ValveStateEnum::kTransitioning:
        return { "Transitioning", LV_PALETTE_ORANGE };
    default:
        return { "Unknown", LV_PALETTE_GREY };
    }
}

void UpdateStateLabel(lv_obj_t * stateLabel, ValveConfigurationAndControlCluster & cluster)
{
    const StateAppearance appearance = AppearanceFor(cluster.GetCurrentState());
    const auto currentLevel          = cluster.GetCurrentLevel();

    char text[48];
    if (cluster.GetFeatureMap().Has(Feature::kLevel) && !currentLevel.IsNull())
    {
        snprintf(text, sizeof(text), "Valve: %s (%u%%)", appearance.text, static_cast<unsigned int>(currentLevel.Value()));
    }
    else
    {
        snprintf(text, sizeof(text), "Valve: %s", appearance.text);
    }

    lv_label_set_text(stateLabel, text);
    lv_obj_set_style_text_color(stateLabel, lv_palette_main(appearance.palette), LV_PART_MAIN);
}

/// The remaining duration only exists while a timed Open is counting down, so the label is hidden
/// rather than showing a placeholder for the common "open until closed" case.
void UpdateRemainingLabel(lv_obj_t * remainingLabel, ValveConfigurationAndControlCluster & cluster)
{
    const auto remaining = cluster.GetRemainingDuration();
    if (remaining.IsNull())
    {
        lv_obj_add_flag(remainingLabel, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    char text[40];
    snprintf(text, sizeof(text), "Closes in %us", static_cast<unsigned int>(remaining.Value()));
    lv_label_set_text(remainingLabel, text);
    lv_obj_clear_flag(remainingLabel, LV_OBJ_FLAG_HIDDEN);
}

void UpdateTargetLevelLabel(lv_obj_t * levelLabel, uint8_t level)
{
    char text[32];
    snprintf(text, sizeof(text), "Open to: %u%%", static_cast<unsigned int>(level));
    lv_label_set_text(levelLabel, text);
}

/// LevelStep constrains which levels the cluster accepts: multiples of the step, plus 100.
uint8_t SnapToLevelStep(uint8_t level, uint8_t levelStep)
{
    VerifyOrReturnValue(levelStep > 1, level);
    VerifyOrReturnValue(level < kMaxLevel, kMaxLevel);

    const uint8_t snapped = static_cast<uint8_t>((level / levelStep) * levelStep);
    return std::max(snapped, levelStep);
}

lv_obj_t * CreateButtonRow(lv_obj_t * card)
{
    lv_obj_t * row = lv_obj_create(card);
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_height(row, 36);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(row, 6, LV_PART_MAIN);
    lv_obj_set_style_border_width(row, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    return row;
}

lv_obj_t * CreateActionButton(lv_obj_t * row, const char * text, lv_palette_t palette)
{
    lv_obj_t * button = lv_button_create(row);
    lv_obj_set_height(button, 32);
    lv_obj_set_flex_grow(button, 1);
    lv_obj_set_flex_flow(button, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(button, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(button, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, lv_palette_main(palette), LV_PART_MAIN);

    lv_obj_t * label = lv_label_create(button);
    lv_label_set_text_static(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);

    return button;
}

} // namespace

lv_obj_t * CreateValveConfigurationAndControlClusterWidget(lv_obj_t * parent, ValveConfigurationAndControlCluster & cluster)
{
    const bool hasLevel = cluster.GetFeatureMap().Has(Feature::kLevel);

    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(card, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(card, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_row(card, 6, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * stateLabel     = lv_label_create(card);
    lv_obj_t * remainingLabel = lv_label_create(card);
    lv_obj_set_style_text_color(remainingLabel, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);

    lv_obj_t * levelSlider = nullptr;
    if (hasLevel)
    {
        lv_obj_t * levelLabel = lv_label_create(card);

        levelSlider = lv_slider_create(card);
        lv_obj_set_width(levelSlider, LV_PCT(96));
        lv_slider_set_range(levelSlider, kMinLevel, kMaxLevel);
        lv_slider_set_value(levelSlider, cluster.GetDefaultOpenLevel(), LV_ANIM_OFF);
        UpdateTargetLevelLabel(levelLabel, cluster.GetDefaultOpenLevel());

        // The slider only picks the level the next Open command uses, so nothing is sent while it
        // moves; the label tracks the knob.
        lv_obj_add_event_cb(
            levelSlider,
            [](lv_event_t * event) {
                auto * slider = static_cast<lv_obj_t *>(lv_event_get_target(event));
                auto * label  = static_cast<lv_obj_t *>(lv_event_get_user_data(event));
                UpdateTargetLevelLabel(label, static_cast<uint8_t>(lv_slider_get_value(slider)));
            },
            LV_EVENT_VALUE_CHANGED, levelLabel);
    }

    lv_obj_t * buttonRow = CreateButtonRow(card);
    lv_obj_t * openBtn   = CreateActionButton(buttonRow, "Open", LV_PALETTE_GREEN);
    lv_obj_t * closeBtn  = CreateActionButton(buttonRow, "Close", LV_PALETTE_RED);

    // The Open command carries no duration: the valve stays open until it is closed from the
    // display or over the network.
    lv_obj_set_user_data(openBtn, levelSlider);
    lv_obj_add_event_cb(
        openBtn,
        [](lv_event_t * event) {
            auto * clusterPtr = static_cast<ValveConfigurationAndControlCluster *>(lv_event_get_user_data(event));
            auto * button     = static_cast<lv_obj_t *>(lv_event_get_target(event));
            auto * slider     = static_cast<lv_obj_t *>(lv_obj_get_user_data(button));

            DataModel::Nullable<Percent> targetLevel;
            if (slider != nullptr)
            {
                targetLevel.SetNonNull(
                    SnapToLevelStep(static_cast<uint8_t>(lv_slider_get_value(slider)), clusterPtr->GetLevelStep()));
            }

            DeviceLayer::SystemLayer().ScheduleLambda(
                [clusterPtr, targetLevel]() { LogErrorOnFailure(clusterPtr->OpenValve(targetLevel, DataModel::NullNullable)); });
        },
        LV_EVENT_CLICKED, &cluster);

    lv_obj_add_event_cb(
        closeBtn,
        [](lv_event_t * event) {
            auto * clusterPtr = static_cast<ValveConfigurationAndControlCluster *>(lv_event_get_user_data(event));
            DeviceLayer::SystemLayer().ScheduleLambda([clusterPtr]() { LogErrorOnFailure(clusterPtr->CloseValve()); });
        },
        LV_EVENT_CLICKED, &cluster);

    // Initial render state
    UpdateStateLabel(stateLabel, cluster);
    UpdateRemainingLabel(remainingLabel, cluster);

    // Data model notifications (from local touch or network). Subscribing with 'card' automatically
    // unregisters when 'card' is deleted.
    DisplayNotificationHub::Instance().Subscribe(
        card, cluster.GetPaths()[0].mEndpointId, Clusters::ValveConfigurationAndControl::Id,
        [stateLabel, remainingLabel, &cluster](const ConcreteAttributePath & path) {
            namespace Attributes = Clusters::ValveConfigurationAndControl::Attributes;

            if (path.mAttributeId == Attributes::CurrentState::Id || path.mAttributeId == Attributes::CurrentLevel::Id)
            {
                UpdateStateLabel(stateLabel, cluster);
            }
            else if (path.mAttributeId == Attributes::RemainingDuration::Id)
            {
                UpdateRemainingLabel(remainingLabel, cluster);
            }
        });

    return card;
}

} // namespace chip::app
