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
#include "CommissioningMonitor.h"
#include "Ui.h"

#include <esp_log.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <cstdio>
#include <cstring>

namespace {

const char TAG[] = "Display";

void LayoutColumn(lv_obj_t * parent)
{
    lv_obj_set_style_pad_all(parent, Ui::Px(8, 4), LV_PART_MAIN);
    lv_obj_set_style_pad_row(parent, Ui::Px(12, 4), LV_PART_MAIN);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
}

lv_obj_t * AddCenteredText(lv_obj_t * parent, const char * text, const lv_font_t * font, lv_color_t color)
{
    lv_obj_t * label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, LV_PCT(100));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, color, LV_PART_MAIN);
    return label;
}

} // namespace

void ShowCommissioningCodes(lv_obj_t * parent, const char * qrCodePayload, const char * manualPairingCode)
{
    LayoutColumn(parent);

    AddCenteredText(parent, "Scan to commission", Ui::HeadingFont(), Ui::Text());

    const int32_t qrSize = Ui::Px(236, 130);
    lv_obj_t * qrCard    = lv_obj_create(parent);
    lv_obj_set_size(qrCard, qrSize + Ui::Px(28, 8), qrSize + Ui::Px(28, 8));
    lv_obj_set_style_bg_color(qrCard, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(qrCard, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(qrCard, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(qrCard, Ui::Px(24, 6), LV_PART_MAIN);
    lv_obj_set_style_pad_all(qrCard, Ui::Px(14, 4), LV_PART_MAIN);
    lv_obj_set_style_shadow_color(qrCard, Ui::Accent(), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(qrCard, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(qrCard, Ui::Px(36, 0), LV_PART_MAIN);
    lv_obj_set_style_shadow_offset_y(qrCard, 0, LV_PART_MAIN);
    lv_obj_clear_flag(qrCard, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * qrCode = lv_qrcode_create(qrCard);
    lv_qrcode_set_size(qrCode, qrSize);
    lv_qrcode_set_dark_color(qrCode, Ui::Background());
    lv_qrcode_set_light_color(qrCode, lv_color_white());
    lv_qrcode_set_quiet_zone(qrCode, true);
    lv_obj_center(qrCode);

    if (lv_qrcode_update(qrCode, qrCodePayload, strlen(qrCodePayload)) != LV_RESULT_OK)
    {
        ESP_LOGE(TAG, "Failed to encode the QR code payload");
        lv_obj_delete(qrCode);
    }
    Ui::PopIn(qrCard);

    lv_obj_t * manualCode = Ui::CreatePill(parent, manualPairingCode, Ui::Accent());
    lv_obj_set_style_text_font(manualCode, Ui::BodyFont(), LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(manualCode, 2, LV_PART_MAIN);

    CommissioningMonitor::CreateStatusView(parent);
}

void ShowCommissionedNotice(lv_obj_t * parent, uint8_t fabricCount)
{
    LayoutColumn(parent);

    Ui::PopIn(Ui::CreateIconBadge(parent, LV_SYMBOL_OK, Ui::Success(), Ui::Px(120, 56)));

    char heading[32];
    snprintf(heading, sizeof(heading), "Commissioned, %u fabric%s", fabricCount, fabricCount == 1 ? "" : "s");
    AddCenteredText(parent, heading, Ui::HeadingFont(), Ui::Success());

    AddCenteredText(parent,
                    "To add another controller, open a commissioning window from one that is "
                    "already paired and use the code it gives you.",
                    Ui::BodyFont(), Ui::TextMuted());

    CommissioningMonitor::CreateStatusView(parent);
}

void ShowCommissioningCodesScreen(lv_obj_t * parent)
{
    const uint8_t fabricCount = CommissioningMonitor::FabricCount();
    if (fabricCount != 0)
    {
        ShowCommissionedNotice(parent, fabricCount);
        return;
    }

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
