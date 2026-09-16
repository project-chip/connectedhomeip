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

#include "CommissioningCodesScreen.h"

#include <esp_log.h>

#include <cstring>

namespace {

const char TAG[] = "Display";

// The panel is 320x240.  A 150px code leaves room for a caption above and the
// manual pairing code below, and keeps the modules large enough to scan.
constexpr int32_t kQRCodeSize = 150;

} // namespace

void ShowCommissioningCodes(lv_obj_t * parent, const char * qrCodePayload, const char * manualPairingCode)
{
    // QR codes are only reliably scanned against a light background, so the
    // whole screen is light rather than following the default theme.
    lv_obj_set_style_bg_color(parent, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_pad_all(parent, 6, LV_PART_MAIN);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * caption = lv_label_create(parent);
    lv_label_set_text_static(caption, "Scan to commission");
    lv_obj_set_style_text_color(caption, lv_color_black(), LV_PART_MAIN);

    lv_obj_t * qrCode = lv_qrcode_create(parent);
    lv_qrcode_set_size(qrCode, kQRCodeSize);
    lv_qrcode_set_dark_color(qrCode, lv_color_black());
    lv_qrcode_set_light_color(qrCode, lv_color_white());
    // Scanners need the light margin around the code even on a light screen.
    lv_qrcode_set_quiet_zone(qrCode, true);

    if (lv_qrcode_update(qrCode, qrCodePayload, strlen(qrCodePayload)) != LV_RESULT_OK)
    {
        ESP_LOGE(TAG, "Failed to encode the QR code payload");
        lv_obj_delete(qrCode);
    }

    lv_obj_t * manualCode = lv_label_create(parent);
    lv_label_set_text(manualCode, manualPairingCode);
    lv_obj_set_style_text_color(manualCode, lv_color_black(), LV_PART_MAIN);
}
