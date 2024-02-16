/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "CastingPlayerDiscovery.h"

namespace matter {
namespace casting {
namespace core {

using namespace chip::System;
using namespace chip::Controller;
using namespace chip::Dnssd;
using namespace std;

CastingPlayerDiscovery * CastingPlayerDiscovery::_castingPlayerDiscovery = nullptr;

CastingPlayerDiscovery::CastingPlayerDiscovery() {}

CastingPlayerDiscovery * CastingPlayerDiscovery::GetInstance()
{
    if (_castingPlayerDiscovery == nullptr)
    {
        _castingPlayerDiscovery = new CastingPlayerDiscovery();
    }
    return _castingPlayerDiscovery;
}

CHIP_ERROR CastingPlayerDiscovery::StartDiscovery(uint32_t deviceTypeFilter)
{
    ChipLogProgress(Discovery, "CastingPlayerDiscovery::StartDiscovery() called");
    VerifyOrReturnError(mState == DISCOVERY_READY, CHIP_ERROR_INCORRECT_STATE);

    mCommissionableNodeController.RegisterDeviceDiscoveryDelegate(&mDelegate);

    if (deviceTypeFilter > 0)
    {
        ReturnErrorOnFailure(mCommissionableNodeController.DiscoverCommissioners(
            DiscoveryFilter(DiscoveryFilterType::kDeviceType, deviceTypeFilter)));
    }
    else
    {
        ReturnErrorOnFailure(mCommissionableNodeController.DiscoverCommissioners());
    }

    mState = DISCOVERY_RUNNING;
    return CHIP_NO_ERROR;
}

CHIP_ERROR CastingPlayerDiscovery::StopDiscovery()
{
    ChipLogProgress(Discovery, "CastingPlayerDiscovery::StopDiscovery() called");
    VerifyOrReturnError(mState == DISCOVERY_RUNNING, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mCommissionableNodeController.StopDiscovery());

    mCastingPlayers.clear();
    mState = DISCOVERY_READY;

    return CHIP_NO_ERROR;
}

void DeviceDiscoveryDelegateImpl::OnDiscoveredDevice(const chip::Dnssd::DiscoveredNodeData & nodeData)
{
    ChipLogProgress(Discovery, "DeviceDiscoveryDelegateImpl::OnDiscoveredDevice() called");
    VerifyOrReturn(mClientDelegate != nullptr,
                   ChipLogError(Discovery, "DeviceDiscoveryDelegateImpl::OnDiscoveredDevice mClientDelegate is a nullptr"));

    // convert nodeData to CastingPlayer
    CastingPlayerAttributes attributes;
    snprintf(attributes.id, kIdMaxLength + 1, "%s%u", nodeData.resolutionData.hostName, nodeData.resolutionData.port);

    chip::Platform::CopyString(attributes.deviceName, chip::Dnssd::kMaxDeviceNameLen + 1, nodeData.commissionData.deviceName);
    chip::Platform::CopyString(attributes.hostName, chip::Dnssd::kHostNameMaxLength + 1, nodeData.resolutionData.hostName);
    chip::Platform::CopyString(attributes.instanceName, chip::Dnssd::Commission::kInstanceNameMaxLength + 1,
                               nodeData.commissionData.instanceName);

    attributes.numIPs = (unsigned int) nodeData.resolutionData.numIPs;
    for (unsigned j = 0; j < attributes.numIPs; j++)
    {
        attributes.ipAddresses[j] = nodeData.resolutionData.ipAddress[j];
    }
    attributes.interfaceId = nodeData.resolutionData.interfaceId;
    attributes.port        = nodeData.resolutionData.port;
    attributes.productId   = nodeData.commissionData.productId;
    attributes.vendorId    = nodeData.commissionData.vendorId;
    attributes.deviceType  = nodeData.commissionData.deviceType;

    memory::Strong<CastingPlayer> player = std::make_shared<CastingPlayer>(attributes);

    std::vector<memory::Strong<CastingPlayer>> castingPlayers = CastingPlayerDiscovery::GetInstance()->GetCastingPlayers();

    // Add to or update castingPlayers
    if (castingPlayers.size() != 0)
    {
        auto it =
            std::find_if(castingPlayers.begin(), castingPlayers.end(),
                         [&player](const memory::Strong<CastingPlayer> & castingPlayer) { return *castingPlayer == *player; });

        // ID match found in castingPlayer, perfom update
        if (it != castingPlayers.end())
        {
            unsigned index                                         = (unsigned int) std::distance(castingPlayers.begin(), it);
            castingPlayers[index]                                  = player;
            CastingPlayerDiscovery::GetInstance()->mCastingPlayers = castingPlayers;
            ChipLogProgress(AppServer, "Updated Casting Player");

            mClientDelegate->HandleOnUpdated(player);
            return;
        }
    }

    castingPlayers.push_back(player);
    CastingPlayerDiscovery::GetInstance()->mCastingPlayers = castingPlayers;
    mClientDelegate->HandleOnAdded(player);
}

}; // namespace core
}; // namespace casting
}; // namespace matter
