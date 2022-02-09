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
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/OnboardingCodesUtil.h>
#include <lib/support/ErrorStr.h>
#include <platform/Ameba/AmebaConfig.h>
#include <platform/Ameba/NetworkCommissioningDriver.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

#include <lwip_netconf.h>

#if CONFIG_ENABLE_OTA_REQUESTOR
#include "app/clusters/ota-requestor/BDXDownloader.h"
#include "app/clusters/ota-requestor/OTARequestor.h"
#include "platform/Ameba/AmebaOTAImageProcessor.h"
#include "platform/GenericOTARequestorDriver.h"
#endif

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;
using namespace ::chip::System;

namespace {
app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::AmebaWiFiDriver::GetInstance()));
} // namespace

void NetWorkCommissioningInstInit()
{
    sWiFiNetworkCommissioningInstance.Init();
}

#ifdef CONFIG_PLATFORM_8721D
#define STATUS_LED_GPIO_NUM PB_5
#elif defined(CONFIG_PLATFORM_8710C)
#define STATUS_LED_GPIO_NUM PA_20
#else
#define STATUS_LED_GPIO_NUM NC
#endif

static DeviceCallbacks EchoCallbacks;

#if CONFIG_ENABLE_OTA_REQUESTOR
OTARequestor gRequestorCore;
GenericOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
AmebaOTAImageProcessor gImageProcessor;
#endif

#if CONFIG_ENABLE_OTA_REQUESTOR
extern "C" void amebaQueryImageCmdHandler(uint32_t nodeId, uint32_t fabricId)
{
    ChipLogProgress(DeviceLayer, "Calling amebaQueryImageCmdHandler");
    // In this mode Provider node ID and fabric idx must be supplied explicitly from ATS$ cmd
    gRequestorCore.TestModeSetProviderParameters(nodeId, fabricId, chip::kRootEndpointId);

    static_cast<OTARequestor *>(GetRequestorInstance())->TriggerImmediateQuery();
}

extern "C" void amebaApplyUpdateCmdHandler()
{
    ChipLogProgress(DeviceLayer, "Calling amebaApplyUpdateCmdHandler");

    static_cast<OTARequestor *>(GetRequestorInstance())->ApplyUpdate();
}

static void InitOTARequestor(void)
{
    // Initialize and interconnect the Requestor and Image Processor objects -- START
    SetRequestorInstance(&gRequestorCore);

    // Set server instance used for session establishment
    gRequestorCore.Init(&(chip::Server::GetInstance()), &gRequestorUser, &gDownloader);

    // WARNING: this is probably not realistic to know such details of the image or to even have an OTADownloader instantiated at
    // the beginning of program execution. We're using hardcoded values here for now since this is a reference application.
    // TODO: instatiate and initialize these values when QueryImageResponse tells us an image is available
    // TODO: add API for OTARequestor to pass QueryImageResponse info to the application to use for OTADownloader init
    OTAImageProcessorParams ipParams;
    gImageProcessor.SetOTAImageProcessorParams(ipParams);
    gImageProcessor.SetOTADownloader(&gDownloader);

    // Connect the Downloader and Image Processor objects
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

    // Initialize and interconnect the Requestor and Image Processor objects -- END
}
#endif // CONFIG_ENABLE_OTA_REQUESTOR

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
    ChipLogProgress(DeviceLayer, "Lighting App Demo!");
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
    NetWorkCommissioningInstInit();

    if (RTW_SUCCESS != wifi_is_connected_to_ap())
    {
        // QR code will be used with CHIP Tool
        PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
    }

    statusLED1.Init(STATUS_LED_GPIO_NUM);

#if CONFIG_ENABLE_OTA_REQUESTOR
    InitOTARequestor();
#endif
}

bool lowPowerClusterSleep()
{
    return true;
}
