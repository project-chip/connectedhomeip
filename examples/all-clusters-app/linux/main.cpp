/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "include/tv-callbacks.h"
#include <app/CommandHandler.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <lib/support/CHIPMem.h>
#include <platform/Linux/NetworkCommissioningDriver.h>
#include <platform/PlatformManager.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SessionManager.h>
#include <transport/raw/PeerAddress.h>

#include "AppMain.h"
#include "binding-handler.h"

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;

namespace {
static LowPowerManager lowPowerManager;
} // namespace

bool emberAfBasicClusterMfgSpecificPingCallback(chip::app::CommandHandler * commandObj)
{
    emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

void OnIdentifyStart(::Identify *)
{
    ChipLogProgress(Zcl, "OnIdentifyStart");
}

void OnIdentifyStop(::Identify *)
{
    ChipLogProgress(Zcl, "OnIdentifyStop");
}

void OnTriggerEffect(::Identify * identify)
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

static Identify gIdentify0 = {
    chip::EndpointId{ 0 }, OnIdentifyStart, OnIdentifyStop, EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED, OnTriggerEffect,
};

static Identify gIdentify1 = {
    chip::EndpointId{ 1 }, OnIdentifyStart, OnIdentifyStop, EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED, OnTriggerEffect,
};

// Network commissioning
namespace {
// This file is being used by platforms other than Linux, so we need this check to disable related features since we only
// implemented them on linux.
#if CHIP_DEVICE_LAYER_TARGET_LINUX
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
NetworkCommissioning::LinuxThreadDriver sLinuxThreadDriver;
Clusters::NetworkCommissioning::Instance sThreadNetworkCommissioningInstance(0 /* Endpoint Id */, &sLinuxThreadDriver);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
NetworkCommissioning::LinuxWiFiDriver sLinuxWiFiDriver;
Clusters::NetworkCommissioning::Instance sWiFiNetworkCommissioningInstance(1 /* Endpoint Id */, &sLinuxWiFiDriver);
#endif
#endif
} // namespace

void ApplicationInit()
{
#if CHIP_DEVICE_LAYER_TARGET_LINUX && defined(ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER)
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    sThreadNetworkCommissioningInstance.Init();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    sWiFiNetworkCommissioningInstance.Init();
#endif
#endif
}

#if !LIBFUZZER_ENABLED
int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);
    VerifyOrDie(InitBindingHandlers() == CHIP_NO_ERROR);
    ChipLinuxAppMainLoop();
    return 0;
}
#else  // !LIBFUZZER_ENABLED
extern "C" int LLVMFuzzerTestOneInput(const uint8_t * aData, size_t aSize)
{
    static bool matterStackInitialized = false;
    if (!matterStackInitialized)
    {
        // Might be simpler to do ChipLinuxAppInit() with argc == 0, argv set to
        // just a fake executable name?
        VerifyOrDie(Platform::MemoryInit() == CHIP_NO_ERROR);
        VerifyOrDie(DeviceLayer::PlatformMgr().InitChipStack() == CHIP_NO_ERROR);

        // ChipLinuxAppMainLoop blocks, and we don't want that here.
        VerifyOrDie(Server::GetInstance().Init() == CHIP_NO_ERROR);

        // We don't start the event loop task, because we don't plan to deliver
        // data on a separate thread.

        matterStackInitialized = true;
    }

    // For now, just dump the data as a UDP payload into the session manager.
    // But maybe we should try to separately extract a PeerAddress and data from
    // the incoming data?
    Transport::PeerAddress peerAddr;
    System::PacketBufferHandle buf =
        System::PacketBufferHandle::NewWithData(aData, aSize, /* aAdditionalSize = */ 0, /* aReservedSize = */ 0);

    // Ignoring the return value from OnMessageReceived, because we might be
    // passing it all sorts of garbage that will cause it to fail.
    Server::GetInstance().GetSecureSessionManager().OnMessageReceived(peerAddr, std::move(buf));

    // Now process pending events until our sentinel is reached.
    DeviceLayer::PlatformMgr().ScheduleWork([](intptr_t) { DeviceLayer::PlatformMgr().StopEventLoopTask(); });
    DeviceLayer::PlatformMgr().RunEventLoop();
    return 0;
}
#endif // !LIBFUZZER_ENABLED

void emberAfLowPowerClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: LowPower::SetDefaultDelegate");
    chip::app::Clusters::LowPower::SetDefaultDelegate(endpoint, &lowPowerManager);
}
