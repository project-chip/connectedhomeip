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

// module header, comes first
#include <controller/CHIPCommissionableNodeController.h>

#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace Controller {

CHIP_ERROR CommissionableNodeController::DiscoverCommissioners(Mdns::DiscoveryFilter discoveryFilter)
{
    ReturnErrorOnFailure(SetUpNodeDiscovery());
    return chip::Mdns::Resolver::Instance().FindCommissioners(discoveryFilter);
}

const Mdns::DiscoveredNodeData * CommissionableNodeController::GetDiscoveredCommissioner(int idx)
{
    return GetDiscoveredNode(idx);
}

CHIP_ERROR CommissionableNodeController::RequestCommissioning(int commissionerNumber)
{
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    const Mdns::DiscoveredNodeData * commissioner;
    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; i++)
    {
        commissioner = GetDiscoveredCommissioner(i);
        if (commissioner != nullptr && --commissionerNumber == 0)
        {
            break;
        }
    }
    if (commissionerNumber != 0)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Enter commissioning mode, open commissioning window
    InitServer();

    // Send User Directed commissioning request
    chip::Transport::PeerAddress commissionerAddress =
        chip::Transport::PeerAddress::UDP(commissioner->ipAddress[0], commissioner->port);
    SendUserDirectedCommissioningRequest(commissionerAddress);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionableNodeController::GetOnboardingPayload(chip::SetupPayload & aOnboardingPayload)
{
    return GetSetupPayload(aOnboardingPayload, chip::RendezvousInformationFlag::kOnNetwork);
}
} // namespace Controller
} // namespace chip
