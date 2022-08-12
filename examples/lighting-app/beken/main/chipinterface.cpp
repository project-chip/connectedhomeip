/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include "Server.h"

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPMem.h>

#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/OnboardingCodesUtil.h>
#include <lib/support/ErrorStr.h>
#include <platform/Beken/BekenConfig.h>
#include <platform/Beken/NetworkCommissioningDriver.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

//#if CONFIG_ENABLE_OTA_REQUESTOR
#include "app/clusters/ota-requestor/DefaultOTARequestorStorage.h"
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorUserConsent.h>
#include <app/clusters/ota-requestor/ExtendedOTARequestorDriver.h>
#include <app/clusters/ota-requestor/OTADownloader.h>
#include <lib/core/OTAImageHeader.h>
#include <platform/Beken/OTAImageProcessorImpl.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>
//#endif

// using chip::OTAImageProcessorImpl;
using chip::BDXDownloader;
using chip::ByteSpan;
using chip::DefaultOTARequestor;
using chip::EndpointId;
using chip::FabricIndex;
using chip::GetRequestorInstance;
using chip::NodeId;
using chip::OnDeviceConnected;
using chip::OnDeviceConnectionFailure;
using chip::OTADownloader;
using chip::PeerId;
using chip::Server;
using chip::VendorId;
using chip::Callback::Callback;
using chip::System::Layer;
using chip::Transport::PeerAddress;
using namespace chip::Messaging;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider::Commands;

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;

namespace {
constexpr EndpointId kNetworkCommissioningEndpointMain      = 0;
constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(kNetworkCommissioningEndpointMain /* Endpoint Id */,
                                      &(NetworkCommissioning::BekenWiFiDriver::GetInstance()));
} // namespace

static DeviceCallbacks EchoCallbacks;
DefaultOTARequestor gRequestorCore;
DefaultOTARequestorStorage gRequestorStorage;
ExtendedOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
OTAImageProcessorImpl gImageProcessor;
chip::ota::DefaultOTARequestorUserConsent gUserConsentProvider;
static chip::ota::UserConsentState gUserConsentState = chip::ota::UserConsentState::kGranted;

// need to check CONFIG_RENDEZVOUS_MODE
bool isRendezvousBLE()
{
    RendezvousInformationFlags flags = RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE);
    return flags.Has(RendezvousInformationFlag::kBLE);
}

//#if CONFIG_ENABLE_OTA_REQUESTOR
extern "C" void QueryImageCmdHandler()
{
    ChipLogProgress(DeviceLayer, "Calling QueryImageCmdHandler");
    PlatformMgr().ScheduleWork([](intptr_t) { GetRequestorInstance()->TriggerImmediateQuery(); });
}

extern "C" void ApplyUpdateCmdHandler()
{
    ChipLogProgress(DeviceLayer, "Calling ApplyUpdateCmdHandler");
    PlatformMgr().ScheduleWork([](intptr_t) { GetRequestorInstance()->ApplyUpdate(); });
}

extern "C" void NotifyUpdateAppliedHandler(uint32_t version)
{
    ChipLogProgress(DeviceLayer, "NotifyUpdateApplied");
    PlatformMgr().ScheduleWork([](intptr_t) { GetRequestorInstance()->NotifyUpdateApplied(); });
}

extern "C" void BkQueryImageCmdHandler(char * pcWriteBuffer, int xWriteBufferLen, int argc, char ** argv)
{
    uint32_t dwLoop   = 0;
    uint32_t nodeId   = 0;
    uint32_t fabricId = 0;

    char cmd0 = 0;
    char cmd1 = 0;

    for (dwLoop = 0; dwLoop < argc; dwLoop++)
    {
        ChipLogProgress(DeviceLayer, "QueryImageArgument %d = %s\r\n", dwLoop + 1, argv[dwLoop]);
    }

    if (argc == 3)
    {
        cmd0   = argv[1][0] - 0x30;
        cmd1   = argv[1][1] - 0x30;
        nodeId = (uint32_t)(cmd0 * 10 + cmd1);

        cmd0     = argv[2][0] - 0x30;
        cmd1     = argv[2][1] - 0x30;
        fabricId = (uint32_t)(cmd0 * 10 + cmd1);
        ChipLogProgress(DeviceLayer, "nodeId %lu,fabricId %lu\r\n", nodeId, fabricId);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "cmd param error ");
        return;
    }

    QueryImageCmdHandler();
    ChipLogProgress(DeviceLayer, "QueryImageCmdHandler begin");

    return;
}

extern "C" void BkApplyUpdateCmdHandler(char * pcWriteBuffer, int xWriteBufferLen, int argc, char ** argv)
{
    // ApplyUpdateCmdHandler();
    ChipLogProgress(DeviceLayer, "ApplyUpdateCmdHandler send request");

    return;
}

extern "C" void BkNotifyUpdateApplied(char * pcWriteBuffer, int xWriteBufferLen, int argc, char ** argv)
{
    uint32_t dwLoop  = 0;
    uint32_t version = 0;

    char cmd0 = 0;
    char cmd1 = 0;

    for (dwLoop = 0; dwLoop < argc; dwLoop++)
    {
        ChipLogProgress(DeviceLayer, "NotifyUpdateApplied %d = %s\r\n", dwLoop + 1, argv[dwLoop]);
    }

    if (argc == 2)
    {
        cmd0    = argv[1][0] - 0x30;
        cmd1    = argv[1][1] - 0x30;
        version = (uint32_t)(cmd0 * 10 + cmd1);

        ChipLogProgress(DeviceLayer, "version %lu \r\n", version);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "cmd param error ");
        return;
    }

    NotifyUpdateAppliedHandler(version);
    ChipLogProgress(DeviceLayer, "NotifyUpdateApplied send request");

    return;
}

static void InitOTARequestor(void)
{
    // Initialize and interconnect the Requestor and Image Processor objects -- START
    SetRequestorInstance(&gRequestorCore);

    gRequestorStorage.Init(Server::GetInstance().GetPersistentStorage());

    // Set server instance used for session establishment
    gRequestorCore.Init(Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader);

    gImageProcessor.SetOTADownloader(&gDownloader);

    // Connect the Downloader and Image Processor objects
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

    if (gUserConsentState != chip::ota::UserConsentState::kUnknown)
    {
        gUserConsentProvider.SetUserConsentState(gUserConsentState);
        gRequestorUser.SetUserConsentDelegate(&gUserConsentProvider);
    }

    // Initialize and interconnect the Requestor and Image Processor objects -- END
}
//#endif // CONFIG_ENABLE_OTA_REQUESTOR

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

// Warkaround for ld error:undefined reference to '__sync_synchronize'
// refer to https://stackoverflow.com/questions/64658430/gnu-arm-embedded-toolchain-undefined-reference-to-sync-synchronize
extern "C" void __sync_synchronize() {}

extern "C" unsigned int __atomic_fetch_add_4(volatile void * ptr, unsigned int val, int memorder)
{
    return (*(unsigned int *) ptr + val);
}

extern "C" bool __atomic_compare_exchange_4(volatile void * pulDestination, void * ulComparand, unsigned int desired, bool weak,
                                            int success_memorder, int failure_memorder)
{
    bool ulReturnValue;
    if (*(unsigned int *) pulDestination == *(unsigned int *) ulComparand)
    {
        *(unsigned int *) pulDestination = desired;
        ulReturnValue                    = true;
    }
    else
    {
        *(unsigned int *) ulComparand = *(unsigned int *) pulDestination;
        ulReturnValue                 = false;
    }
    return ulReturnValue;
}

extern "C" unsigned int __atomic_fetch_sub_4(volatile void * ptr, unsigned int val, int memorder)
{
    return (*(unsigned int *) ptr + val);
}
extern "C" bool __atomic_compare_exchange_1(volatile void * pulDestination, void * ulComparand, unsigned char desired, bool weak,
                                            int success_memorder, int failure_memorder)
{
    bool ulReturnValue;
    if (*(unsigned char *) pulDestination == *(unsigned char *) ulComparand)
    {
        *(unsigned char *) pulDestination = desired;
        ulReturnValue                     = true;
    }
    else
    {
        *(unsigned char *) ulComparand = *(unsigned char *) pulDestination;
        ulReturnValue                  = false;
    }
    return ulReturnValue;
}

extern "C" unsigned int __atomic_fetch_and_4(volatile void * pulDestination, unsigned int ulValue, int memorder)
{
    unsigned int ulCurrent;

    ulCurrent = *(unsigned int *) pulDestination;
    *(unsigned int *) pulDestination &= ulValue;
    return ulCurrent;
}

extern "C" bool __sync_bool_compare_and_swap_4(volatile void * ptr, unsigned int oldval, unsigned int newval)
{
    if (*(unsigned int *) ptr == oldval)
    {
        *(unsigned int *) ptr = newval;
        return true;
    }
    else
    {
        return false;
    }
}

extern "C" bool __sync_bool_compare_and_swap_1(volatile void * ptr, unsigned char oldval, unsigned char newval)
{
    if (*(unsigned char *) ptr == oldval)
    {
        *(unsigned char *) ptr = newval;
        return true;
    }
    else
    {
        return false;
    }
}

/* stub for __libc_init_array */
extern "C" void _fini(void) {}
extern "C" void _init(void)
{
    ;
}

static void InitServer(intptr_t context)
{
    // Init ZCL Data Model and CHIP App Server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    chip::Server::GetInstance().Init(initParams);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
    sWiFiNetworkCommissioningInstance.Init();
    InitOTARequestor();
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
}

extern "C" void ChipTest(void)
{
    ChipLogProgress(DeviceLayer, "Lighting App Demo!");
    CHIP_ERROR err = CHIP_NO_ERROR;

    // initPref();

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();
    err                           = deviceMgr.Init(&EchoCallbacks); // start the CHIP task
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "DeviceManagerInit() - ERROR!\r\n");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "DeviceManagerInit() - OK\r\n");
    }
    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, 0);

    while (true)
        vTaskDelay(pdMS_TO_TICKS(50)); // Just server the application event handler
    ChipLogProgress(SoftwareUpdate, "Exited");
    return;
}

bool lowPowerClusterSleep()
{
    return true;
}
