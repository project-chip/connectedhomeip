/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "CastingUtils.h"

using namespace chip;
using namespace chip::System;
using namespace chip::DeviceLayer;
using namespace chip::Dnssd;

// TODO: Accept these values over CLI
const char * kContentUrl        = "https://www.test.com/videoid";
const char * kContentDisplayStr = "Test video";

CHIP_ERROR DiscoverCommissioners()
{
    // Send discover commissioners request
    ReturnErrorOnFailure(CastingServer::GetInstance()->DiscoverCommissioners());

    // Give commissioners some time to respond and then ScheduleWork to initiate commissioning
    return DeviceLayer::SystemLayer().StartTimer(
        chip::System::Clock::Milliseconds32(kCommissionerDiscoveryTimeoutInMs),
        [](System::Layer *, void *) { chip::DeviceLayer::PlatformMgr().ScheduleWork(InitCommissioningFlow); }, nullptr);
}

CHIP_ERROR RequestCommissioning(int index)
{
    const Dnssd::DiscoveredNodeData * selectedCommissioner = CastingServer::GetInstance()->GetDiscoveredCommissioner(index);
    if (selectedCommissioner == nullptr)
    {
        ChipLogError(AppServer, "No such commissioner with index %d exists", index);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    PrepareForCommissioning(selectedCommissioner);
    return CHIP_NO_ERROR;
}

/**
 * Enters commissioning mode, opens commissioning window, logs onboarding payload.
 * If non-null selectedCommissioner is provided, sends user directed commissioning
 * request to the selectedCommissioner and advertises self as commissionable node over DNS-SD
 */
void PrepareForCommissioning(const Dnssd::DiscoveredNodeData * selectedCommissioner)
{
    CastingServer::GetInstance()->Init();

    CastingServer::GetInstance()->OpenBasicCommissioningWindow(HandleCommissioningCompleteCallback);

    // Display onboarding payload
    chip::DeviceLayer::ConfigurationMgr().LogDeviceConfig();

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    if (selectedCommissioner != nullptr)
    {
        // Send User Directed commissioning request
        // Wait 1 second to allow our commissionee DNS records to publish (needed on Mac)
        int32_t expiration = 1;
        ReturnOnFailure(DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(expiration), HandleUDCSendExpiration,
                                                              (void *) selectedCommissioner));
    }
    else
    {
        ChipLogProgress(AppServer, "To run discovery again, enter: cast discover");
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
}

void InitCommissioningFlow(intptr_t commandArg)
{
    int commissionerCount = 0;

    // Display discovered commissioner TVs to ask user to select one
    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; i++)
    {
        const Dnssd::DiscoveredNodeData * commissioner = CastingServer::GetInstance()->GetDiscoveredCommissioner(i);
        if (commissioner != nullptr)
        {
            ChipLogProgress(AppServer, "Discovered Commissioner #%d", commissionerCount++);
            commissioner->LogDetail();
        }
    }

    if (commissionerCount > 0)
    {
        ChipLogProgress(AppServer, "%d commissioner(s) discovered. Select one (by number# above) to request commissioning from: ",
                        commissionerCount);

        ChipLogProgress(AppServer, "Example: cast request 0");
    }
    else
    {
        ChipLogError(AppServer, "No commissioner discovered, commissioning must be initiated manually!");
        PrepareForCommissioning();
    }
}

void LaunchURLResponseCallback(CHIP_ERROR err)
{
    ChipLogProgress(AppServer, "LaunchURLResponseCallback called with %" CHIP_ERROR_FORMAT, err.Format());
}

void HandleCommissioningCompleteCallback(CHIP_ERROR err)
{
    ChipLogProgress(AppServer, "HandleCommissioningCompleteCallback called with %" CHIP_ERROR_FORMAT, err.Format());
    if (err == CHIP_NO_ERROR)
    {
        ReturnOnFailure(
            CastingServer::GetInstance()->ContentLauncherLaunchURL(kContentUrl, kContentDisplayStr, LaunchURLResponseCallback));
        ChipLogProgress(AppServer, "ContentLauncherLaunchURL called successfully");
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
void HandleUDCSendExpiration(System::Layer * aSystemLayer, void * context)
{
    Dnssd::DiscoveredNodeData * selectedCommissioner = (Dnssd::DiscoveredNodeData *) context;

    // Send User Directed commissioning request
    ReturnOnFailure(CastingServer::GetInstance()->SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress::UDP(
        selectedCommissioner->resolutionData.ipAddress[0], selectedCommissioner->resolutionData.port,
        selectedCommissioner->resolutionData.interfaceId)));
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

void PrintFabrics()
{
    // set fabric to be the first in the list
    for (const auto & fb : chip::Server::GetInstance().GetFabricTable())
    {
        FabricIndex fabricIndex = fb.GetFabricIndex();
        ChipLogError(AppServer, "Next Fabric index=%d", fabricIndex);
        if (!fb.IsInitialized())
        {
            ChipLogError(AppServer, " -- Not initialized");
            continue;
        }
        NodeId myNodeId = fb.GetNodeId();
        ChipLogProgress(NotSpecified,
                        "---- Current Fabric nodeId=0x" ChipLogFormatX64 " fabricId=0x" ChipLogFormatX64 " fabricIndex=%d",
                        ChipLogValueX64(myNodeId), ChipLogValueX64(fb.GetFabricId()), fabricIndex);
    }
}
