/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#pragma once

#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <clusters/Groupcast/AttributeIds.h>
#include <clusters/Groupcast/ClusterId.h>
#include <clusters/Groupcast/CommandIds.h>
#include <clusters/Groupcast/Commands.h>
#include <clusters/Groupcast/Enums.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {

/**
 * @brief Implements the Matter specifications for the Groupcast cluster
 */
class GroupcastLogic
{
public:
    GroupcastLogic(BitFlags<Groupcast::Feature> features) : mFeatures(features) {}
    const BitFlags<Groupcast::Feature> & Features() const { return mFeatures; }

    CHIP_ERROR ReadMembership(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadMaxMembershipCount(EndpointId endpoint, AttributeValueEncoder & aEncoder);

    CHIP_ERROR JoinGroup(FabricIndex fabric_index, const Groupcast::Commands::JoinGroup::DecodableType & data);
    CHIP_ERROR LeaveGroup(FabricIndex fabric_index, const Groupcast::Commands::LeaveGroup::DecodableType & data,
                          Groupcast::Commands::LeaveGroupResponse::Type & response);
    CHIP_ERROR UpdateGroupKey(FabricIndex fabric_index, const Groupcast::Commands::UpdateGroupKey::DecodableType & data);
    CHIP_ERROR ExpireGracePeriod(FabricIndex fabric_index, const Groupcast::Commands::ExpireGracePeriod::DecodableType & data);
    CHIP_ERROR ConfigureAuxiliaryACL(FabricIndex fabric_index,
                                     const Groupcast::Commands::ConfigureAuxiliaryACL::DecodableType & data);

private:
    CHIP_ERROR RegisterAccessControl(FabricIndex fabric_index, GroupId group_id);

    const BitFlags<Groupcast::Feature> mFeatures;
};

} // namespace Clusters
} // namespace app
} // namespace chip
