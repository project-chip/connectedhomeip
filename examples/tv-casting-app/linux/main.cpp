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
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <controller/CHIPCommissionableNodeController.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConfigurationManager.h>
#include <setup_payload/SetupPayload.h>
#include <system/SystemLayer.h>
#include <transport/raw/PeerAddress.h>

using namespace chip;

#define TV_DEVICE_TYPE 35

chip::Controller::CommissionableNodeController commissionableNodeController;

void RequestCommissioning(intptr_t commandArg)
{
    int commissionerCount = 0, selectedCommissionerNumber = CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES;
    chip::SetupPayload payload;
    const Mdns::DiscoveredNodeData * selectedCommissioner = nullptr;

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
    ChipLogProgress(Zcl, "%d commissioner(s) discovered", commissionerCount);

    if (commissionerCount > 0)
    {
        ChipLogProgress(Zcl, "Choose a commissioner (by number# above) to request commissioning from: ");
        scanf("%d", &selectedCommissionerNumber);
        selectedCommissioner = commissionableNodeController.GetDiscoveredCommissioner(selectedCommissionerNumber - 1);
        VerifyOrReturn(selectedCommissioner != nullptr, ChipLogError(Zcl, "No such commissioner!"));
    }
    else
    {
        ChipLogError(Zcl, "No commissioner discovered, commissioning must be initiated manually!");
    }

    // Enter commissioning mode, open commissioning window
    InitServer();
    ReturnOnFailure(OpenBasicCommissioningWindow(ResetFabrics::kYes, 3 * 60));

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

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SuccessOrExit(err = chip::Platform::MemoryInit());
    SuccessOrExit(err = chip::DeviceLayer::PlatformMgr().InitChipStack());

    // Send discover commissioner TVs request
    SuccessOrExit(err = commissionableNodeController.DiscoverCommissioners(
                      Mdns::DiscoveryFilter(Mdns::DiscoveryFilterType::kDeviceType, TV_DEVICE_TYPE)));

    // Give commissioner TVs some time to respond and then ScheduleWork to get commissioned
    DeviceLayer::SystemLayer.StartTimer(
        5 * 1000, [](System::Layer *, void *) { chip::DeviceLayer::PlatformMgr().ScheduleWork(RequestCommissioning); }, nullptr);

    DeviceLayer::PlatformMgr().RunEventLoop();
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to run TV Casting App:: %s", ErrorStr(err));
        // End the program with non zero error code to indicate an error.
        return 1;
    }
    return 0;
}
