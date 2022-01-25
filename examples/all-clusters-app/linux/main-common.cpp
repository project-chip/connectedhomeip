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

void emberAfLowPowerClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: LowPower::SetDefaultDelegate");
    chip::app::Clusters::LowPower::SetDefaultDelegate(endpoint, &lowPowerManager);
}
