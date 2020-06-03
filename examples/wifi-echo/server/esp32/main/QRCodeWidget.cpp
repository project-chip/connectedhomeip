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

/**
 * @file QRCodeWidget.cpp
 *
 * This file implements the QRCodeWidget that displays a QRCode centered on the screen
 * It generates a CHIP SetupPayload containing the device's Soft-AP SSID and then
 * encodes that into a QRCode.
 *
 */

#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "qrcodegen.h"

#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <support/CodeUtils.h>

#include "Display.h"
#include "QRCodeWidget.h"

#define MAX_SSID_LEN 32
// A temporary value assigned for this example's QRCode
// Spells CHIP on a dialer
#define EXAMPLE_VENDOR_ID 3447
// Used to indicate that an SSID has been added to the QRCode
#define EXAMPLE_VENDOR_TAG_SSID 1

#define EXAMPLE_VENDOR_TAG_IP 2

#if CONFIG_HAVE_DISPLAY

extern const char * TAG;

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

void GetGatewayIP(char * ip_buf, size_t ip_len)
{

    tcpip_adapter_ip_info_t ip;
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip);
    IPAddress::FromIPv4(ip.ip).ToString(ip_buf, ip_len);
    ESP_LOGE(TAG, "Got gateway ip %s", ip_buf);
}

string createSetupPayload()
{
    char ap_ssid[MAX_SSID_LEN];
    GetAPName(ap_ssid, sizeof(ap_ssid));
    SetupPayload payload;
    payload.version   = 1;
    payload.vendorID  = EXAMPLE_VENDOR_ID;
    payload.productID = 1;

    OptionalQRCodeInfo ssidInfo;
    ssidInfo.tag  = EXAMPLE_VENDOR_TAG_SSID;
    ssidInfo.type = optionalQRCodeInfoTypeString;
    ssidInfo.data = ap_ssid;
    payload.addVendorOptionalData(ssidInfo);

    char gw_ip[INET6_ADDRSTRLEN];
    GetGatewayIP(gw_ip, sizeof(gw_ip));
    OptionalQRCodeInfo ipInfo;
    ipInfo.tag  = EXAMPLE_VENDOR_TAG_IP;
    ipInfo.type = optionalQRCodeInfoTypeString;
    ipInfo.data = gw_ip;
    payload.addVendorOptionalData(ipInfo);

    QRCodeSetupPayloadGenerator generator(payload);
    string result;
    size_t tlvDataLen = sizeof(ap_ssid) + sizeof(gw_ip);
    uint8_t tlvDataStart[tlvDataLen];
    CHIP_ERROR err = generator.payloadBase41Representation(result, tlvDataStart, tlvDataLen);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Couldn't get payload string %d", generator.payloadBase41Representation(result));
    }
    return result;
};

QRCodeWidget::QRCodeWidget()
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

    string generatedQRCodeData;
    char * qrCodeStr        = NULL;
    uint8_t * qrCodeTempBuf = NULL;
    uint8_t * qrCode        = NULL;
    uint16_t qrCodeDisplaySize, qrCodeX, qrCodeY, qrCodeXOffset, qrCodeYOffset;

    generatedQRCodeData = createSetupPayload();
    qrCodeStr           = (char *) generatedQRCodeData.c_str();

    // Generate the QR code.
    uint16_t qrCodeSize = qrcodegen_BUFFER_LEN_FOR_VERSION(kQRCodeVersion);
    qrCodeTempBuf       = (uint8_t *) malloc(qrCodeSize);
    qrCode              = (uint8_t *) malloc(qrCodeSize);
    VerifyOrExit(qrCodeTempBuf != NULL, err = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(qrCode != NULL, err = CHIP_ERROR_NO_MEMORY);
    if (!qrcodegen_encodeText(qrCodeStr, qrCodeTempBuf, qrCode, qrcodegen_Ecc_LOW, kQRCodeVersion, kQRCodeVersion,
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
    if (qrCodeTempBuf != NULL)
    {
        free(qrCodeTempBuf);
    }
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "QRCodeWidget::Display() failed: %s", ErrorStr(err));
    }
}

#endif // CONFIG_HAVE_DISPLAY
