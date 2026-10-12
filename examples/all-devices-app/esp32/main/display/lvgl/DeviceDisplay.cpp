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

#include "DeviceDisplay.h"
#include "CommissioningCodesScreen.h"
#include "CommissioningMonitor.h"
#include "DisplayBoard.h"
#include "DisplayNotificationHub.h"
#include "DisplayWake.h"
#include "HomeScreen.h"
#include "NavigationStack.h"
#include "SystemMenuScreen.h"
#include "Theme.h"

#include <app/server/Server.h>
#include <cinttypes>
#include <esp_log.h>
#include <lvgl.h>

namespace {

const char TAG[] = "Display";

// 30 seconds of inactivity before blanking the LCD backlight.
constexpr uint32_t kInactivityTimeoutMs = 30000;

// Non-null once the panel is up; both entry points are no-ops before that and
// after a failed bring-up.
lv_display_t * gDisplay = nullptr;

constexpr int kBrightnessOn   = 100;
constexpr uint32_t kFadeOutMs = 700;

bool sBacklightOn       = true;
lv_obj_t * sWakeOverlay = nullptr;

void SetBrightnessStep(void *, int32_t value)
{
    DisplayBoard::SetBrightness(value);
}

void FadeOut()
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, gDisplay);
    lv_anim_set_exec_cb(&anim, SetBrightnessStep);
    lv_anim_set_values(&anim, kBrightnessOn, 0);
    lv_anim_set_duration(&anim, kFadeOutMs);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in);
    lv_anim_start(&anim);
}

void OnWakeOverlayTouch(lv_event_t * event)
{
    ESP_LOGI(TAG, "Touch detected while sleeping; waking display");
    WakeDisplay();
}

void CheckInactivityTimer(lv_timer_t * timer)
{
    if (gDisplay == nullptr)
    {
        return;
    }

    if (sBacklightOn)
    {
        uint32_t inactiveMs = lv_display_get_inactive_time(gDisplay);
        if (inactiveMs >= kInactivityTimeoutMs)
        {
            ESP_LOGI(TAG, "Display inactive for %" PRIu32 " ms; turning off backlight", inactiveMs);
            FadeOut();
            sBacklightOn = false;

            // Place an invisible overlay on the top layer to intercept the wake touch
            // so waking the device does not trigger underlying button actions.
            if (sWakeOverlay == nullptr)
            {
                sWakeOverlay = lv_obj_create(lv_layer_top());
                lv_obj_remove_style_all(sWakeOverlay);
                lv_obj_set_size(sWakeOverlay, LV_PCT(100), LV_PCT(100));
                lv_obj_clear_flag(sWakeOverlay, LV_OBJ_FLAG_SCROLLABLE);
                lv_obj_add_event_cb(sWakeOverlay, OnWakeOverlayTouch, LV_EVENT_PRESSED, nullptr);
            }
        }
    }
}

} // namespace

void WakeDisplay()
{
    if (gDisplay == nullptr)
    {
        return;
    }

    if (!sBacklightOn)
    {
        lv_anim_delete(gDisplay, SetBrightnessStep);
        DisplayBoard::SetBrightness(kBrightnessOn);
        sBacklightOn = true;
    }

    if (sWakeOverlay != nullptr)
    {
        lv_obj_delete(sWakeOverlay);
        sWakeOverlay = nullptr;
    }
    lv_display_trigger_activity(gDisplay);
}

void InitDeviceDisplay()
{
    // Powers the panel rails, starts the SPI panel and the LVGL port task.
    gDisplay = DisplayBoard::Start();
    if (gDisplay == nullptr)
    {
        ESP_LOGE(TAG, "DisplayBoard::Start() failed");
        return;
    }

    if (!DisplayBoard::Lock())
    {
        ESP_LOGE(TAG, "Could not take the LVGL lock");
        gDisplay = nullptr;
        return;
    }

    InitTheme(gDisplay);

    lv_obj_t * screen = lv_display_get_screen_active(gDisplay);

    // Initialize the hierarchical navigation shell
    NavigationStack::Init(screen);

    // Root is always Home. Whether to jump straight to the QR code is decided in
    // InitDisplayDataModelListener(): the fabric table is not populated until
    // Server::Init() runs, which happens after this function.
    NavigationStack::Push("Home", ShowHome);

    // Register inactivity timer to blank display after 30s
    lv_timer_create(CheckInactivityTimer, 1000, nullptr);

    // Only turn the backlight on once there is something to look at, to avoid
    // showing the uninitialized panel contents.
    DisplayBoard::SetBrightness(kBrightnessOn);
    sBacklightOn = true;

    DisplayBoard::Unlock();
}

void ShowRestartingMessage()
{
    if (gDisplay == nullptr)
    {
        return;
    }

    if (!DisplayBoard::Lock())
    {
        ESP_LOGE(TAG, "Could not acquire display lock for the restart message");
        return;
    }

    WakeDisplay();

    lv_obj_t * screen = lv_display_get_screen_active(gDisplay);

    // The clean below deletes the top bar and content container, so the shell has to let go of
    // them first.
    NavigationStack::Detach();
    lv_obj_clean(screen);
    lv_obj_set_layout(screen, LV_LAYOUT_NONE);
    lv_obj_add_flag(lv_layer_top(), LV_OBJ_FLAG_HIDDEN);
    lv_obj_t * label = lv_label_create(screen);
    lv_label_set_text_static(label, "Restarting...");
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_center(label);

    // UI-triggered restarts run inside an LVGL click callback and block before calling
    // esp_restart(), so flush immediately instead of waiting for the next timer pass.
    lv_display_trigger_activity(gDisplay);
    lv_refr_now(gDisplay);

    DisplayBoard::Unlock();
}

void InitDisplayDataModelListener()
{
    // Nothing below is safe without a display: the hub takes the LVGL port lock from its
    // attribute callback, and that lock asserts when the port was never initialized.
    if (gDisplay == nullptr)
    {
        return;
    }

    chip::app::DisplayNotificationHub::Instance().Init();
    CommissioningMonitor::Init();

    // Called on the CHIP thread once Server::Init() has populated the fabric table. An
    // uncommissioned device jumps straight to the onboarding payload, so the breadcrumbs
    // read: Home > System > QR Code.
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() != 0)
    {
        return;
    }

    if (!DisplayBoard::Lock())
    {
        ESP_LOGE(TAG, "Could not acquire display lock for the initial screen");
        return;
    }

    NavigationStack::ResetToRoot();
    NavigationStack::Push("System", ShowSystemMenu);
    NavigationStack::Push("QR Code", ShowCommissioningCodesScreen);

    DisplayBoard::Unlock();
}
