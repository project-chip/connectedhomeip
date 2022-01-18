/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <platform_stdlib.h>

#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include "Globals.h"
#include "LEDWidget.h"
#include "Server.h"

#include "chip_porting.h"
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPMem.h>

#include <app/clusters/identify-server/identify-server.h>
#include <app/server/OnboardingCodesUtil.h>
#include <lib/support/ErrorStr.h>
#include <platform/Ameba/AmebaConfig.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

#include <lwip_netconf.h>

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;

#define QRCODE_BASE_URL "https://dhrishi.github.io/connectedhomeip/qrcode.html"
#define EXAMPLE_VENDOR_TAG_IP 1

#ifdef CONFIG_PLATFORM_8721D
#define STATUS_LED_GPIO_NUM PB_5
#elif defined(CONFIG_PLATFORM_8710C)
#define STATUS_LED_GPIO_NUM PA_20
#else
#define STATUS_LED_GPIO_NUM NC
#endif

static DeviceCallbacks EchoCallbacks;

void GetGatewayIP(char * ip_buf, size_t ip_len)
{
    uint8_t * gateway = LwIP_GetGW(&xnetif[0]);
    sprintf(ip_buf, "%d.%d.%d.%d", gateway[0], gateway[1], gateway[2], gateway[3]);
    printf("Got gateway ip: %s\r\n", ip_buf);
}

// need to check CONFIG_RENDEZVOUS_MODE
bool isRendezvousBLE()
{
    RendezvousInformationFlags flags = RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE);
    return flags.Has(RendezvousInformationFlag::kBLE);
}

std::string createSetupPayload()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    std::string result;

    uint16_t discriminator;
    err = ConfigurationMgr().GetSetupDiscriminator(discriminator);
    if (err != CHIP_NO_ERROR)
    {
        printf("Couldn't get discriminator: %s\r\n", ErrorStr(err));
        return result;
    }
    printf("Setup discriminator: %u (0x%x)\r\n", discriminator, discriminator);

    uint32_t setupPINCode;
    err = ConfigurationMgr().GetSetupPinCode(setupPINCode);
    if (err != CHIP_NO_ERROR)
    {
        printf("Couldn't get setupPINCode: %s\r\n", ErrorStr(err));
        return result;
    }
    printf("Setup PIN code: %u (0x%x)\r\n", setupPINCode, setupPINCode);

    uint16_t vendorId;
    err = ConfigurationMgr().GetVendorId(vendorId);
    if (err != CHIP_NO_ERROR)
    {
        printf("Couldn't get vendorId: %s\r\n", ErrorStr(err));
        return result;
    }

    uint16_t productId;
    err = ConfigurationMgr().GetProductId(productId);
    if (err != CHIP_NO_ERROR)
    {
        printf("Couldn't get productId: %s\r\n", ErrorStr(err));
        return result;
    }

    SetupPayload payload;
    payload.version               = 0;
    payload.discriminator         = discriminator;
    payload.setUpPINCode          = setupPINCode;
    payload.rendezvousInformation = RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE);
    payload.vendorID              = vendorId;
    payload.productID             = productId;

    if (!isRendezvousBLE())
    {
        char gw_ip[INET6_ADDRSTRLEN];
        GetGatewayIP(gw_ip, sizeof(gw_ip));
        payload.addOptionalVendorData(EXAMPLE_VENDOR_TAG_IP, gw_ip);

        QRCodeSetupPayloadGenerator generator(payload);

        size_t tlvDataLen = sizeof(gw_ip);
        uint8_t tlvDataStart[tlvDataLen];
        err = generator.payloadBase38Representation(result, tlvDataStart, tlvDataLen);
    }
    else
    {
        QRCodeSetupPayloadGenerator generator(payload);
        err = generator.payloadBase38Representation(result);
    }

    {
        ManualSetupPayloadGenerator generator(payload);
        std::string outCode;

        if (generator.payloadDecimalStringRepresentation(outCode) == CHIP_NO_ERROR)
        {
            printf("Short Manual(decimal) setup code: %s\r\n", outCode.c_str());
        }
        else
        {
            printf("Failed to get decimal setup code\r\n");
        }

        payload.commissioningFlow = CommissioningFlow::kCustom;
        generator                 = ManualSetupPayloadGenerator(payload);

        if (generator.payloadDecimalStringRepresentation(outCode) == CHIP_NO_ERROR)
        {
            // intentional extra space here to align the log with the short code
            printf("Long Manual(decimal) setup code:  %s\r\n", outCode.c_str());
        }
        else
        {
            printf("Failed to get decimal setup code\r\n");
        }
    }

    if (err != CHIP_NO_ERROR)
    {
        printf("Couldn't get payload string %\r\n" CHIP_ERROR_FORMAT, err.Format());
    }
    return result;
};

void OnIdentifyStart(Identify *)
{
    ChipLogProgress(Zcl, "OnIdentifyStart");
}

void OnIdentifyStop(Identify *)
{
    ChipLogProgress(Zcl, "OnIdentifyStop");
}

void OnTriggerEffect(Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE");
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        return;
    }
}

static Identify gIdentify1 = {
    chip::EndpointId{ 1 }, OnIdentifyStart, OnIdentifyStop, EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED, OnTriggerEffect,
};

extern "C" void ChipTest(void)
{
    printf("In ChipTest()\r\n");
    CHIP_ERROR err = CHIP_NO_ERROR;

    printf("initPrefr\n");
    initPref();

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();
    err                           = deviceMgr.Init(&EchoCallbacks);

    if (err != CHIP_NO_ERROR)
    {
        printf("DeviceManagerInit() - ERROR!\r\n");
    }
    else
    {
        printf("DeviceManagerInit() - OK\r\n");
    }

    chip::Server::GetInstance().Init();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    std::string qrCodeText = createSetupPayload();

    printf("QR CODE Text: '%s'\r\n", qrCodeText.c_str());

    {
        std::vector<char> qrCode(3 * qrCodeText.size() + 1);
        err = EncodeQRCodeToUrl(qrCodeText.c_str(), qrCodeText.size(), qrCode.data(), qrCode.max_size());
        if (err == CHIP_NO_ERROR)
        {
            printf("Copy/paste the below URL in a browser to see the QR CODE:\n\t%s?data=%s", QRCODE_BASE_URL, qrCode.data());
        }
    }
    printf("\n\n");

    statusLED1.Init(STATUS_LED_GPIO_NUM);

    while (true)
        vTaskDelay(pdMS_TO_TICKS(50));
}

bool lowPowerClusterSleep()
{
    return true;
}
