/*
 *
 * SPDX-FileCopyrightText: 2020-2023 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file QRCodeScreen.cpp
 *
 * Screen which displays a QR code.
 *
 */

#include "QRCodeScreen.h"

#if CONFIG_HAVE_DISPLAY

// TODO organize includes below

#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "qrcodegen.h"

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

namespace {

constexpr int kVersion    = 4;
constexpr int kModuleSize = 4;
constexpr int kBorderSize = 1;

color_t qrCodeColor = TFT_LIGHTGREY;

}; // namespace

QRCodeScreen::QRCodeScreen(std::string text, std::string title) : title(title)
{
    constexpr int qrCodeSize = qrcodegen_BUFFER_LEN_FOR_VERSION(kVersion);

    // TODO check text length against max size permitted, or maybe adjust version used accordingly

    std::vector<uint8_t> temp(qrCodeSize);
    qrCode.resize(qrCodeSize);

    if (!qrcodegen_encodeText(text.c_str(), temp.data(), qrCode.data(), qrcodegen_Ecc_LOW, kVersion, kVersion, qrcodegen_Mask_AUTO,
                              true))
    {
        ESP_LOGE("QRCodeScreen", "qrcodegen_encodeText() failed");
        qrCode.clear();
    }
}

void QRCodeScreen::Display()
{
    if (qrCode.empty())
    {
        return;
    }

    const uint8_t * data  = qrCode.data();
    const int size        = qrcodegen_getSize(data);
    const int displaySize = (2 * kBorderSize + size) * kModuleSize;
    const int displayX    = (DisplayWidth - displaySize) / 2;
    const int displayY    = ScreenTitleSafeTop + ((DisplayHeight - ScreenTitleSafeTop - ScreenTitleSafeBottom) - displaySize) / 2;

    TFT_fillRect(displayX, displayY, displaySize, displaySize, qrCodeColor);

    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            if (qrcodegen_getModule(data, x, y))
            {
                TFT_fillRect(displayX + (kBorderSize + x) * kModuleSize, displayY + (kBorderSize + y) * kModuleSize, kModuleSize,
                             kModuleSize, TFT_BLACK);
            }
        }
    }
}

#endif // CONFIG_HAVE_DISPLAY
