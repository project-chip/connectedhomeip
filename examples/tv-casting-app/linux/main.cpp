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

#include <app/server/Mdns.h>
#include <app/server/Server.h>
#include <controller/CHIPCommissionableNodeController.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConfigurationManager.h>
#include <system/SystemLayer.h>
#include <transport/raw/PeerAddress.h>

using namespace chip;
using namespace chip::Controller;

constexpr int kTvDeviceType                         = 35;
constexpr uint16_t commissioningWindowTimeoutInSec  = 3 * 60;
constexpr uint32_t commissionerDiscoveryTimeoutInMs = 5 * 1000;

CommissionableNodeController commissionableNodeController;
chip::System::SocketWatchToken token;

/**
 * Enters commissioning mode, opens commissioning window, logs onboarding payload.
 * If non-null selectedCommissioner is provided, sends user directed commissioning
 * request to the selectedCommissioner and advertises self as commissionable node over DNS-SD
 */
void PrepareForCommissioning(const Mdns::DiscoveredNodeData * selectedCommissioner = nullptr)
{
    // Enter commissioning mode, open commissioning window
    InitServer();
    ReturnOnFailure(OpenBasicCommissioningWindow(ResetFabrics::kYes, commissioningWindowTimeoutInSec));

    // Display onboarding payload
    chip::DeviceLayer::ConfigurationMgr().LogDeviceConfig();

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    if (selectedCommissioner != nullptr)
    {
        // Advertise self as Commissionable Node over mDNS
        ReturnOnFailure(app::Mdns::AdvertiseCommissionableNode(app::Mdns::CommissioningMode::kEnabledBasic));

        // Send User Directed commissioning request
        ReturnOnFailure(SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress::UDP(
            selectedCommissioner->ipAddress[0], selectedCommissioner->port, selectedCommissioner->interfaceId[0])));
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
}

/**
 * Accepts user input of selected commissioner and calls PrepareForCommissioning with
 * the selected commissioner
 */
void RequestUserDirectedCommissioning(System::SocketEvents events, intptr_t data)
{
    // Accept user selection for commissioner to request commissioning from.
    // Assuming kernel has line buffering, this will unblock on '\n' character
    // on stdin i.e. when user hits 'Enter'
    int selectedCommissionerNumber = CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES;
    scanf("%d", &selectedCommissionerNumber);
    printf("%d\n", selectedCommissionerNumber);
    chip::DeviceLayer::SystemLayer.StopWatchingSocket(&token);

    const Mdns::DiscoveredNodeData * selectedCommissioner =
        commissionableNodeController.GetDiscoveredCommissioner(selectedCommissionerNumber - 1);
    VerifyOrReturn(selectedCommissioner != nullptr, ChipLogError(Zcl, "No such commissioner!"));
    PrepareForCommissioning(selectedCommissioner);
}

void InitCommissioningFlow(intptr_t commandArg)
{
    int commissionerCount = 0;

    // Display discovered commissioner TVs to ask user to select one
    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; i++)
    {
        const Mdns::DiscoveredNodeData * commissioner = commissionableNodeController.GetDiscoveredCommissioner(i);
        if (commissioner != nullptr)
        {
            ChipLogProgress(Zcl, "Discovered Commissioner #%d", ++commissionerCount);
            commissioner->LogDetail();
        }
    }

    if (commissionerCount > 0)
    {
        ChipLogProgress(
            Zcl, "%d commissioner(s) discovered. Select one (by number# above) to request commissioning from: ", commissionerCount);

        // Setup for async/non-blocking user input from stdin
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        VerifyOrReturn(fcntl(0, F_SETFL, flags | O_NONBLOCK) == 0,
                       ChipLogError(Zcl, "Could not set non-blocking mode for user input!"));
        ReturnOnFailure(chip::DeviceLayer::SystemLayer.StartWatchingSocket(STDIN_FILENO, &token));
        ReturnOnFailure(chip::DeviceLayer::SystemLayer.SetCallback(token, RequestUserDirectedCommissioning, (intptr_t) NULL));
        ReturnOnFailure(chip::DeviceLayer::SystemLayer.RequestCallbackOnPendingRead(token));
    }
    else
    {
        ChipLogError(Zcl, "No commissioner discovered, commissioning must be initiated manually!");
        PrepareForCommissioning();
    }
}

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SuccessOrExit(err = chip::Platform::MemoryInit());
    SuccessOrExit(err = chip::DeviceLayer::PlatformMgr().InitChipStack());

    // Send discover commissioners request
    SuccessOrExit(err = commissionableNodeController.DiscoverCommissioners(
                      Mdns::DiscoveryFilter(Mdns::DiscoveryFilterType::kDeviceType, kTvDeviceType)));

    // Give commissioners some time to respond and then ScheduleWork to initiate commissioning
    DeviceLayer::SystemLayer.StartTimer(
        commissionerDiscoveryTimeoutInMs,
        [](System::Layer *, void *) { chip::DeviceLayer::PlatformMgr().ScheduleWork(InitCommissioningFlow); }, nullptr);

    // TBD: Content casting commands

    DeviceLayer::PlatformMgr().RunEventLoop();
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to run TV Casting App: %s", ErrorStr(err));
        // End the program with non zero error code to indicate an error.
        return 1;
    }
    return 0;
}
