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

#include <bsp/esp-bsp.h>
#include <esp_log.h>
#include <lvgl.h>

#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

namespace {

const char TAG[] = "Display";

// bsp_display_lock() treats 0 as "wait indefinitely".
constexpr uint32_t kWaitForever = 0;

// Non-null once the panel is up; both entry points are no-ops before that and
// after a failed bring-up.
lv_display_t * gDisplay = nullptr;

} // namespace

void InitDeviceDisplay()
{
    char qrCodeBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    chip::MutableCharSpan qrCodeText(qrCodeBuffer);
    CHIP_ERROR err = GetQRCode(qrCodeText, chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "GetQRCode() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    char manualCodeBuffer[chip::kManualSetupLongCodeCharLength + 1];
    chip::MutableCharSpan manualCodeText(manualCodeBuffer);
    err = GetManualPairingCode(manualCodeText, chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "GetManualPairingCode() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    // Powers the panel rails, starts the SPI panel and the LVGL port task.
    gDisplay = bsp_display_start();
    if (gDisplay == nullptr)
    {
        ESP_LOGE(TAG, "bsp_display_start() failed");
        return;
    }

    if (!bsp_display_lock(kWaitForever))
    {
        ESP_LOGE(TAG, "Could not take the LVGL lock");
        return;
    }
    ShowCommissioningCodes(lv_display_get_screen_active(gDisplay), qrCodeText.data(), manualCodeText.data());
    bsp_display_unlock();

    // Only turn the backlight on once there is something to look at, to avoid
    // showing the uninitialized panel contents.
    ESP_ERROR_CHECK_WITHOUT_ABORT(bsp_display_backlight_on());
}

void ShowRestartingMessage()
{
    if (gDisplay == nullptr)
    {
        return;
    }

    if (!bsp_display_lock(kWaitForever))
    {
        return;
    }

    lv_obj_t * screen = lv_display_get_screen_active(gDisplay);
    lv_obj_clean(screen);
    lv_obj_t * label = lv_label_create(screen);
    lv_label_set_text_static(label, "Restarting...");
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_center(label);

    bsp_display_unlock();
}
