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

#include "DeviceOperationsScreen.h"
#include "DeviceDisplay.h"

#include <app/server/Server.h>
#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>

namespace {

const char TAG[] = "DeviceOps";

lv_obj_t * sActiveModal = nullptr;

void CloseModal()
{
    if (sActiveModal != nullptr)
    {
        lv_msgbox_close(sActiveModal);
        sActiveModal = nullptr;
    }
}

void OnCancelClicked(lv_event_t * event)
{
    CloseModal();
}

void OnConfirmRestartClicked(lv_event_t * event)
{
    CloseModal();
    ESP_LOGI(TAG, "Reboot requested via UI, restarting...");
    ShowRestartingMessage();
    vTaskDelay(pdMS_TO_TICKS(300));
    esp_restart();
}

void OnConfirmResetClicked(lv_event_t * event)
{
    CloseModal();
    ESP_LOGI(TAG, "Factory reset confirmed via UI, scheduling reset...");
    LogErrorOnFailure(
        chip::DeviceLayer::PlatformMgr().ScheduleWork([](intptr_t) { chip::Server::GetInstance().ScheduleFactoryReset(); }));
}

void OnRestartButtonClicked(lv_event_t * event)
{
    CloseModal();

    sActiveModal = lv_msgbox_create(nullptr);
    lv_msgbox_add_title(sActiveModal, "Restart Device");
    lv_msgbox_add_text(sActiveModal, "Restart the device now?");

    lv_obj_t * cancelBtn = lv_msgbox_add_footer_button(sActiveModal, "Cancel");
    lv_obj_add_event_cb(cancelBtn, OnCancelClicked, LV_EVENT_CLICKED, nullptr);

    lv_obj_t * restartBtn = lv_msgbox_add_footer_button(sActiveModal, "Restart");
    lv_obj_add_event_cb(restartBtn, OnConfirmRestartClicked, LV_EVENT_CLICKED, nullptr);
}

void OnResetButtonClicked(lv_event_t * event)
{
    CloseModal();

    sActiveModal = lv_msgbox_create(nullptr);
    lv_msgbox_add_title(sActiveModal, "Factory Reset?");
    lv_msgbox_add_text(sActiveModal, "Erase all fabrics, credentials, and settings?");

    lv_obj_t * cancelBtn = lv_msgbox_add_footer_button(sActiveModal, "Cancel");
    lv_obj_add_event_cb(cancelBtn, OnCancelClicked, LV_EVENT_CLICKED, nullptr);

    lv_obj_t * resetBtn = lv_msgbox_add_footer_button(sActiveModal, "Reset Now");
    lv_obj_set_style_bg_color(resetBtn, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    lv_obj_add_event_cb(resetBtn, OnConfirmResetClicked, LV_EVENT_CLICKED, nullptr);
}

} // namespace

void ShowDeviceOperations(lv_obj_t * parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_row(parent, 12, LV_PART_MAIN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * restartBtn = lv_button_create(parent);
    lv_obj_set_width(restartBtn, LV_PCT(85));
    lv_obj_set_height(restartBtn, 42);
    lv_obj_t * restartLabel = lv_label_create(restartBtn);
    lv_label_set_text_static(restartLabel, "Restart Device");
    lv_obj_center(restartLabel);
    lv_obj_add_event_cb(restartBtn, OnRestartButtonClicked, LV_EVENT_CLICKED, nullptr);

    lv_obj_t * info = lv_label_create(parent);
    lv_label_set_text_static(info, "Factory reset erases all fabrics and credentials.");
    lv_obj_set_width(info, LV_PCT(90));
    lv_obj_set_style_text_align(info, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_color(info, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);

    lv_obj_t * resetBtn = lv_button_create(parent);
    lv_obj_set_width(resetBtn, LV_PCT(85));
    lv_obj_set_height(resetBtn, 42);
    lv_obj_set_style_bg_color(resetBtn, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    lv_obj_t * resetLabel = lv_label_create(resetBtn);
    lv_label_set_text_static(resetLabel, "Factory Reset Device");
    lv_obj_center(resetLabel);
    lv_obj_add_event_cb(resetBtn, OnResetButtonClicked, LV_EVENT_CLICKED, nullptr);
}
