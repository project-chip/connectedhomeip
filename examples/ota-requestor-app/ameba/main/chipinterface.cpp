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
#include "chip_porting.h"

#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <lib/support/ErrorStr.h>
#include <platform/Ameba/AmebaConfig.h>
#include <platform/Ameba/NetworkCommissioningDriver.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPMem.h>

#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorUserConsent.h>
#include <app/clusters/ota-requestor/ExtendedOTARequestorDriver.h>
#include <platform/Ameba/AmebaOTAImageProcessor.h>

void * __dso_handle = 0;

using chip::AmebaOTAImageProcessor;
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

namespace { // Network Commissioning
constexpr EndpointId kNetworkCommissioningEndpointMain      = 0;
constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(kNetworkCommissioningEndpointMain /* Endpoint Id */,
                                      &(NetworkCommissioning::AmebaWiFiDriver::GetInstance()));
} // namespace

void NetWorkCommissioningInstInit()
{
    sWiFiNetworkCommissioningInstance.Init();

    // We only have network commissioning on endpoint 0.
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);
}

static DeviceCallbacks EchoCallbacks;

DefaultOTARequestor gRequestorCore;
DefaultOTARequestorStorage gRequestorStorage;
ExtendedOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
AmebaOTAImageProcessor gImageProcessor;
chip::ota::DefaultOTARequestorUserConsent gUserConsentProvider;
static chip::ota::UserConsentState gUserConsentState = chip::ota::UserConsentState::kGranted;

extern "C" void amebaQueryImageCmdHandler()
{
    ChipLogProgress(DeviceLayer, "Calling amebaQueryImageCmdHandler");
    PlatformMgr().ScheduleWork([](intptr_t) { GetRequestorInstance()->TriggerImmediateQuery(); });
}

extern "C" void amebaApplyUpdateCmdHandler()
{
    ChipLogProgress(DeviceLayer, "Calling amebaApplyUpdateCmdHandler");
    PlatformMgr().ScheduleWork([](intptr_t) { GetRequestorInstance()->ApplyUpdate(); });
}

static void InitOTARequestor(void)
{
    // Initialize and interconnect the Requestor and Image Processor objects -- START
    SetRequestorInstance(&gRequestorCore);

    gRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());

    // Set server instance used for session establishment
    gRequestorCore.Init(chip::Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader);

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

static void InitServer(intptr_t context)
{
    // Init ZCL Data Model and CHIP App Server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    chip::Server::GetInstance().Init(initParams);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
    NetWorkCommissioningInstInit();

    InitOTARequestor();
}

extern "C" void ChipTest(void)
{
    ChipLogProgress(SoftwareUpdate, "ota-requestor!");
    CHIP_ERROR err = CHIP_NO_ERROR;

    initPref();

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();
    err                           = deviceMgr.Init(&EchoCallbacks);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "DeviceManagerInit() - ERROR!\r\n");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "DeviceManagerInit() - OK\r\n");
    }

    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, 0);
}
