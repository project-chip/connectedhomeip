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
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <cstring>

namespace {

const char TAG[] = "Display";

// With a 32px top bar, a 130px code leaves room for a caption above and the
// manual pairing code below without vertical overflow.
constexpr int32_t kQRCodeSize = 130;

} // namespace

void ShowCommissioningCodes(lv_obj_t * parent, const char * qrCodePayload, const char * manualPairingCode)
{
    lv_obj_set_style_pad_all(parent, 4, LV_PART_MAIN);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * caption = lv_label_create(parent);
    lv_label_set_text_static(caption, "Scan to commission");
    lv_obj_set_style_text_color(caption, lv_color_white(), LV_PART_MAIN);

    // QR code card with white background for high camera contrast
    lv_obj_t * qrCard = lv_obj_create(parent);
    lv_obj_set_size(qrCard, kQRCodeSize + 8, kQRCodeSize + 8);
    lv_obj_set_style_bg_color(qrCard, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(qrCard, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(qrCard, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(qrCard, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_all(qrCard, 4, LV_PART_MAIN);
    lv_obj_clear_flag(qrCard, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * qrCode = lv_qrcode_create(qrCard);
    lv_qrcode_set_size(qrCode, kQRCodeSize);
    lv_qrcode_set_dark_color(qrCode, lv_color_black());
    lv_qrcode_set_light_color(qrCode, lv_color_white());
    lv_qrcode_set_quiet_zone(qrCode, true);
    lv_obj_center(qrCode);

    if (lv_qrcode_update(qrCode, qrCodePayload, strlen(qrCodePayload)) != LV_RESULT_OK)
    {
        ESP_LOGE(TAG, "Failed to encode the QR code payload");
        lv_obj_delete(qrCode);
    }

    lv_obj_t * manualCode = lv_label_create(parent);
    lv_label_set_text(manualCode, manualPairingCode);
    lv_obj_set_style_text_color(manualCode, lv_color_white(), LV_PART_MAIN);
}

void ShowCommissioningCodesScreen(lv_obj_t * parent)
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

    ShowCommissioningCodes(parent, qrCodeText.data(), manualCodeText.data());
}
