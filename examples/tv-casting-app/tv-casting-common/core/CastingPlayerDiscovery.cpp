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


void DeviceDiscoveryDelegateImpl::OnDiscoveredDevice(const chip::Dnssd::DiscoveredNodeData & nodeData) {

    VerifyOrReturn(mClientDelegate != nullptr, ChipLogError(NotSpecified, "CastingPlayerDeviceDiscoveryDelegate, mClientDelegate is a nullptr"));

    //convert nodeData to CastingPlayer
    Attributes attributes;
    strcpy(attributes.id, nodeData.resolutionData.hostName);

    char port[kPortMaxLength] = {};
    snprintf(port, sizeof(port), "%u", nodeData.resolutionData.port);
    strcat(attributes.id,port);

    strcpy(attributes.name, nodeData.commissionData.deviceName);
    strcpy(attributes.host_name, nodeData.resolutionData.hostName);
    attributes.num_IPs = nodeData.resolutionData.numIPs;
    for (unsigned j = 0; j < attributes.num_IPs; j++)
    {
        attributes.ip_address[j] = nodeData.resolutionData.ipAddress[j];
    }
    attributes.product_id = nodeData.commissionData.productId;
    attributes.vendor_id = nodeData.commissionData.vendorId;
    attributes.type = nodeData.commissionData.deviceType;


    Strong<CastingPlayer> player = std::make_shared<CastingPlayer>(attributes);

    std::vector<Strong<CastingPlayer>> * castingPlayers = CastingPlayerDiscovery::GetInstance()->GetCastingPlayers();

    //Add to or update castingPlayers
    if(castingPlayers->size() != 0){

        auto it = std::find_if((*castingPlayers).begin(), (*castingPlayers).end(),
            [&player](const Strong<CastingPlayer>& castingPlayer) {
                int compareResult = strcmp(castingPlayer->GetId(),player->GetId());
                return (compareResult == 0) ? 1 : 0;
            }
        );

        //ID match found in castingPlayer, perfom update
        if (it != (*castingPlayers).end()) {
            unsigned index = (unsigned int)std::distance((*castingPlayers).begin(), it);
            (*castingPlayers)[index] = *it;
            ChipLogProgress(AppServer, "Updated Casting Player");

            mClientDelegate->HandleOnUpdated(player);
            return;
        }
    }

    castingPlayers->push_back(player);
    mClientDelegate->HandleOnAdded(player);
    return;

}

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

CHIP_ERROR CastingPlayerDiscovery::StartDiscovery(uint64_t deviceTypeFilter)
{
    VerifyOrReturnError(mState == DISCOVERY_READY, CHIP_ERROR_INCORRECT_STATE);

    mCommissionableNodeController.RegisterDeviceDiscoveryDelegate(&mDelegate);

    if(deviceTypeFilter > 0){
        ReturnErrorOnFailure(mCommissionableNodeController.DiscoverCommissioners(DiscoveryFilter(DiscoveryFilterType::kDeviceType, deviceTypeFilter)));

    }
    else{
        ReturnErrorOnFailure(mCommissionableNodeController.DiscoverCommissioners());

    }

    mState = DISCOVERY_RUNNING;
    return CHIP_NO_ERROR;
}

CHIP_ERROR CastingPlayerDiscovery::StopDiscovery(){
    VerifyOrReturnError(mState == DISCOVERY_RUNNING, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mCommissionableNodeController.StopDiscovery());

    mState = DISCOVERY_READY;

    return CHIP_NO_ERROR;
}

}; // namespace core
}; // namespace casting
}; // namespace matter
