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
#include "chip_porting.h"
#include <lwip_netconf.h>

#include <app/clusters/identify-server/identify-server.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/ErrorStr.h>
#include <platform/Ameba/AmebaConfig.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <support/CHIPMem.h>

extern "C" {
void * __dso_handle = 0;
}

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;

Identify gIdentify0 = {
    chip::EndpointId{ 0 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
};

Identify gIdentify1 = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
};

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
    ChipLogProgress(DeviceLayer, "Got gateway ip: %s\r\n", ip_buf);
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
        ChipLogError(DeviceLayer, "Couldn't get discriminator: %s\r\n", ErrorStr(err));
        return result;
    }
    ChipLogProgress(DeviceLayer, "Setup discriminator: %d (0x%x)\r\n", discriminator, discriminator);

    uint32_t setupPINCode;
    err = ConfigurationMgr().GetSetupPinCode(setupPINCode);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Couldn't get setupPINCode: %s\r\n", ErrorStr(err));
        return result;
    }
    ChipLogProgress(DeviceLayer, "Setup PIN code: %lu (0x%lx)\r\n", setupPINCode, setupPINCode);

    uint16_t vendorId;
    err = ConfigurationMgr().GetVendorId(vendorId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Couldn't get vendorId: %s\r\n", ErrorStr(err));
        return result;
    }

    uint16_t productId;
    err = ConfigurationMgr().GetProductId(productId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Couldn't get productId: %s\r\n", ErrorStr(err));
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
            ChipLogProgress(DeviceLayer, "Short Manual(decimal) setup code: %s\r\n", outCode.c_str());
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to get decimal setup code\r\n");
        }

        payload.commissioningFlow = CommissioningFlow::kCustom;
        generator                 = ManualSetupPayloadGenerator(payload);

        if (generator.payloadDecimalStringRepresentation(outCode) == CHIP_NO_ERROR)
        {
            // intentional extra space here to align the log with the short code
            ChipLogProgress(DeviceLayer, "Long Manual(decimal) setup code:  %s\r\n", outCode.c_str());
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to get decimal setup code\r\n");
        }
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Couldn't get payload string %lu\r\n" CHIP_ERROR_FORMAT, err.Format());
    }
    return result;
};

extern "C" void ChipTest(void)
{
    ChipLogProgress(DeviceLayer, "All Clusters Demo!");
    CHIP_ERROR err = CHIP_NO_ERROR;

    initPref();

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    err = deviceMgr.Init(&EchoCallbacks);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "DeviceManagerInit() - ERROR!\r\n");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "DeviceManagerInit() - OK\r\n");
    }

    chip::Server::GetInstance().Init();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    if (RTW_SUCCESS != wifi_is_connected_to_ap())
    {
        std::string qrCodeText = createSetupPayload();
        ChipLogProgress(DeviceLayer, "QR CODE Text: '%s'\r\n", qrCodeText.c_str());

        std::vector<char> qrCode(3 * qrCodeText.size() + 1);
        err = EncodeQRCodeToUrl(qrCodeText.c_str(), qrCodeText.size(), qrCode.data(), qrCode.max_size());
        if (err == CHIP_NO_ERROR)
        {
            ChipLogProgress(DeviceLayer, "Copy/paste the below URL in a browser to see the QR CODE:\n\t%s?data=%s \r\n",
                            QRCODE_BASE_URL, qrCode.data());
        }
    }

    statusLED1.Init(STATUS_LED_GPIO_NUM);

    while (true)
        vTaskDelay(pdMS_TO_TICKS(50));
}

bool lowPowerClusterSleep()
{
    return true;
}
