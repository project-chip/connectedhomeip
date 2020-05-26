/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "qrcodegen.h"

#include <platform/CHIPDeviceLayer.h>
#include <support/CodeUtils.h>

#include "Display.h"
#include "QRCodeWidget.h"

#define MAX_SSID_LEN 32

#if CONFIG_HAVE_DISPLAY

extern const char * TAG;
const char * const QRCODE_DATA = "https://github.com/project-chip/connectedhomeip";

using namespace ::chip;
using namespace ::chip::DeviceLayer;

// QRCode config
enum
{
    kQRCodeVersion       = 4,
    kQRCodeModuleSizePix = 4,
    kQRCodePadding       = 2
};

// TODO Pull this from the configuration manager
void GetAPName(char * ssid, size_t ssid_len)
{
    uint8_t mac[6];
    CHIP_ERROR err = esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_wifi_get_mac(ESP_IF_WIFI_STA) failed: %s", chip::ErrorStr(err));
    }
    snprintf(ssid, ssid_len, "%s%02X%02X", "CHIP_DEMO-", mac[4], mac[5]);
}

void QRCodeWidget::Init()
{
    QRCodeColor = TFT_DARKGREY;
    VMargin     = 10;
}

void QRCodeWidget::Display()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    enum
    {
        kMaxQRCodeStrLength = 120
    };
    char * qrCodeStr        = NULL;
    uint8_t * qrCodeDataBuf = NULL;
    uint8_t * qrCode        = NULL;
    uint16_t qrCodeDisplaySize, qrCodeX, qrCodeY, qrCodeXOffset, qrCodeYOffset;

    // get the ssid of the Soft-AP
    char ap_ssid[MAX_SSID_LEN];
    GetAPName(ap_ssid, sizeof(ap_ssid));
    // TODO create a CHIP qrcode here with the soft-ap in the tlv secion
    qrCodeStr = ap_ssid;

    // Generate the QR code.
    uint16_t qrCodeSize = qrcodegen_BUFFER_LEN_FOR_VERSION(kQRCodeVersion);
    qrCodeDataBuf       = (uint8_t *) malloc(qrCodeSize);
    qrCode              = (uint8_t *) malloc(qrCodeSize);
    VerifyOrExit(qrCodeDataBuf != NULL, err = CHIP_ERROR_NO_MEMORY);
    if (!qrcodegen_encodeText(qrCodeStr, qrCodeDataBuf, qrCode, qrcodegen_Ecc_LOW, kQRCodeVersion, kQRCodeVersion,
                              qrcodegen_Mask_AUTO, true))
    {
        ESP_LOGE(TAG, "qrcodegen_encodeText() failed");
        ExitNow(err = CHIP_ERROR_INCORRECT_STATE);
    }

    // Draw the QR code image on the screen.
    qrCodeDisplaySize = (qrcodegen_getSize(qrCode) + kQRCodePadding) * kQRCodeModuleSizePix;
    qrCodeX           = (DisplayWidth - qrCodeDisplaySize) / 2;
    qrCodeY           = (DisplayHeight * VMargin) / 100;
    qrCodeXOffset     = qrCodeX + kQRCodeModuleSizePix;
    qrCodeYOffset     = qrCodeY + kQRCodeModuleSizePix;
    TFT_fillRect(qrCodeX, qrCodeY, (int) qrCodeDisplaySize, (int) qrCodeDisplaySize, QRCodeColor);
    for (uint8_t y = 0; y < qrcodegen_getSize(qrCode); y++)
    {
        for (uint8_t x = 0; x < qrcodegen_getSize(qrCode); x++)
        {
            if (qrcodegen_getModule(qrCode, x, y))
            {
                TFT_fillRect(x * kQRCodeModuleSizePix + qrCodeXOffset, y * kQRCodeModuleSizePix + qrCodeYOffset,
                             (int) kQRCodeModuleSizePix, (int) kQRCodeModuleSizePix, TFT_BLACK);
            }
        }
    }

exit:
    if (qrCode != NULL)
    {
        free(qrCode);
    }
    if (qrCodeDataBuf != NULL)
    {
        free(qrCodeDataBuf);
    }
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "QRCodeWidget::Display() failed: %s", ErrorStr(err));
    }
}

#endif // CONFIG_HAVE_DISPLAY
