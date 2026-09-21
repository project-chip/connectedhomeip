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

#include "ChimeClusterWidget.h"
#include "CommandStatusLog.h"
#include "DisplayNotificationHub.h"

#include <clusters/Chime/Attributes.h>
#include <clusters/Chime/Ids.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip::app {

namespace {

void UpdateEnabledDisplay(lv_obj_t * stateLabel, lv_obj_t * toggleBtn, lv_obj_t * btnLabel, lv_obj_t * playBtn, bool isEnabled)
{
    if (isEnabled)
    {
        lv_label_set_text_static(stateLabel, "Chime: ENABLED");
        lv_obj_set_style_text_color(stateLabel, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
        lv_label_set_text_static(btnLabel, "Mute");
        lv_obj_set_style_bg_color(toggleBtn, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
        lv_obj_clear_state(playBtn, LV_STATE_DISABLED);
        lv_obj_set_style_bg_color(playBtn, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_MAIN);
    }
    else
    {
        lv_label_set_text_static(stateLabel, "Chime: MUTED");
        lv_obj_set_style_text_color(stateLabel, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
        lv_label_set_text_static(btnLabel, "Enable");
        lv_obj_set_style_bg_color(toggleBtn, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
        lv_obj_add_state(playBtn, LV_STATE_DISABLED);
        lv_obj_set_style_bg_color(playBtn, lv_palette_darken(LV_PALETTE_GREY, 2), LV_PART_MAIN);
    }
}

void UpdateSoundButtons(lv_obj_t * soundRow, uint8_t selectedId)
{
    uint32_t count = lv_obj_get_child_cnt(soundRow);
    for (uint32_t i = 0; i < count; ++i)
    {
        lv_obj_t * btn = lv_obj_get_child(soundRow, i);
        auto btnId     = static_cast<uint8_t>(reinterpret_cast<uintptr_t>(lv_obj_get_user_data(btn)));
        if (btnId == selectedId)
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

lv_obj_t * CreateChimeClusterWidget(lv_obj_t * parent, Chime & device)
{
    auto & cluster = device.ChimeCluster();

    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(card, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(card, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_row(card, 6, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    // Row 1: Enable / Mute toggle
    lv_obj_t * enableRow = lv_obj_create(card);
    lv_obj_set_width(enableRow, LV_PCT(100));
    lv_obj_set_height(enableRow, 36);
    lv_obj_set_flex_flow(enableRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(enableRow, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(enableRow, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(enableRow, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(enableRow, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_clear_flag(enableRow, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * stateLabel = lv_label_create(enableRow);

    lv_obj_t * toggleBtn = lv_button_create(enableRow);
    lv_obj_set_width(toggleBtn, 90);
    lv_obj_set_height(toggleBtn, 32);
    lv_obj_set_flex_flow(toggleBtn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(toggleBtn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(toggleBtn, 2, LV_PART_MAIN);

    lv_obj_t * toggleBtnLabel = lv_label_create(toggleBtn);
    lv_obj_set_style_text_color(toggleBtnLabel, lv_color_white(), LV_PART_MAIN);

    lv_obj_add_event_cb(
        toggleBtn,
        [](lv_event_t * event) {
            auto * devPtr = static_cast<Chime *>(lv_event_get_user_data(event));
            DeviceLayer::SystemLayer().ScheduleLambda([devPtr]() {
                LogCommandFailure("SetEnabled", devPtr->ChimeCluster().SetEnabled(!devPtr->ChimeCluster().GetEnabled()));
            });
        },
        LV_EVENT_CLICKED, &device);

    // Row 2: Installed sounds selector buttons
    lv_obj_t * soundRow = lv_obj_create(card);
    lv_obj_set_width(soundRow, LV_PCT(100));
    lv_obj_set_height(soundRow, 32);
    lv_obj_set_flex_flow(soundRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(soundRow, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(soundRow, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(soundRow, 4, LV_PART_MAIN);
    lv_obj_set_style_border_width(soundRow, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(soundRow, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_clear_flag(soundRow, LV_OBJ_FLAG_SCROLLABLE);

    for (uint8_t idx = 0;; ++idx)
    {
        uint8_t soundId                                                  = 0;
        char nameBuf[Clusters::ChimeCluster::kMaxChimeSoundNameSize + 1] = {};
        MutableCharSpan nameSpan(nameBuf, Clusters::ChimeCluster::kMaxChimeSoundNameSize);
        if (device.GetChimeSoundByIndex(idx, soundId, nameSpan) != CHIP_NO_ERROR)
        {
            break;
        }
        nameBuf[nameSpan.size()] = '\0';

        lv_obj_t * btn = lv_button_create(soundRow);
        lv_obj_set_height(btn, 30);
        lv_obj_set_flex_grow(btn, 1);
        lv_obj_set_style_pad_all(btn, 2, LV_PART_MAIN);
        lv_obj_set_user_data(btn, reinterpret_cast<void *>(static_cast<uintptr_t>(soundId)));

        lv_obj_t * lbl = lv_label_create(btn);
        lv_label_set_text(lbl, nameBuf);
        lv_obj_set_style_text_color(lbl, lv_color_white(), LV_PART_MAIN);
        lv_obj_center(lbl);

        lv_obj_add_event_cb(
            btn,
            [](lv_event_t * event) {
                auto * devPtr = static_cast<Chime *>(lv_event_get_user_data(event));
                auto * btnObj = static_cast<lv_obj_t *>(lv_event_get_target(event));
                auto id       = static_cast<uint8_t>(reinterpret_cast<uintptr_t>(lv_obj_get_user_data(btnObj)));
                DeviceLayer::SystemLayer().ScheduleLambda(
                    [devPtr, id]() { LogCommandFailure("SetSelectedChime", devPtr->ChimeCluster().SetSelectedChime(id)); });
            },
            LV_EVENT_CLICKED, &device);
    }

    // Row 3: Play Chime button
    lv_obj_t * playBtn = lv_button_create(card);
    lv_obj_set_width(playBtn, LV_PCT(100));
    lv_obj_set_height(playBtn, 36);
    lv_obj_set_flex_flow(playBtn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(playBtn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * playLabel = lv_label_create(playBtn);
    lv_label_set_text_static(playLabel, "Play Selected Chime");
    lv_obj_set_style_text_color(playLabel, lv_color_white(), LV_PART_MAIN);

    lv_obj_add_event_cb(
        playBtn,
        [](lv_event_t * event) {
            auto * devPtr = static_cast<Chime *>(lv_event_get_user_data(event));
            DeviceLayer::SystemLayer().ScheduleLambda([devPtr]() {
                if (devPtr->ChimeCluster().GetEnabled())
                {
                    LogCommandFailure("PlayChimeSound", devPtr->PlayChimeSound(devPtr->ChimeCluster().GetSelectedChime()));
                }
            });
        },
        LV_EVENT_CLICKED, &device);

    // Initial render state
    UpdateEnabledDisplay(stateLabel, toggleBtn, toggleBtnLabel, playBtn, cluster.GetEnabled());
    UpdateSoundButtons(soundRow, cluster.GetSelectedChime());

    // Subscribe to reactive updates
    DisplayNotificationHub::Instance().Subscribe(
        card, device.GetEndpointId(), Clusters::Chime::Id,
        [stateLabel, toggleBtn, toggleBtnLabel, playBtn, soundRow, &cluster](const ConcreteAttributePath & path) {
            if (path.mAttributeId == Clusters::Chime::Attributes::Enabled::Id ||
                path.mAttributeId == Clusters::Chime::Attributes::SelectedChime::Id)
            {
                UpdateEnabledDisplay(stateLabel, toggleBtn, toggleBtnLabel, playBtn, cluster.GetEnabled());
                UpdateSoundButtons(soundRow, cluster.GetSelectedChime());
            }
        });

    return card;
}

} // namespace chip::app
