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

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <clusters/Groupcast/ClusterId.h>
#include <clusters/Groupcast/AttributeIds.h>
#include <clusters/Groupcast/CommandIds.h>
#include <clusters/Groupcast/Commands.h>
#include <app/data-model-provider/ActionReturnStatus.h>

namespace chip {
namespace app {
namespace Clusters {

using DataModel::ActionReturnStatus;

/**
 * @brief Implements the Matter specifications for the Groupcast cluster
 */
class GroupcastLogic
{
public:
    DataModel::ActionReturnStatus ReadMembership(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    DataModel::ActionReturnStatus ReadMaxMembershipCount(EndpointId endpoint, AttributeValueEncoder & aEncoder);

    
    DataModel::ActionReturnStatus JoinGroup(FabricIndex fabric_index, const Groupcast::Commands::JoinGroup::DecodableType & data);
    DataModel::ActionReturnStatus LeaveGroup(FabricIndex fabric_index,
                          const Groupcast::Commands::LeaveGroup::DecodableType & data,
                          Groupcast::Commands::LeaveGroupResponse::Type &response);
    DataModel::ActionReturnStatus UpdateGroupKey(FabricIndex fabric_index, const Groupcast::Commands::UpdateGroupKey::DecodableType & data);
    DataModel::ActionReturnStatus ExpireGracePeriod(FabricIndex fabric_index, const Groupcast::Commands::ExpireGracePeriod::DecodableType & data);
    DataModel::ActionReturnStatus ConfigureAuxiliaryACL(FabricIndex fabric_index, const Groupcast::Commands::ConfigureAuxiliaryACL::DecodableType & data);

private:
    CHIP_ERROR RegisterAccessControl(FabricIndex fabric_index, GroupId group_id);

};

} // namespace Clusters
} // namespace app
} // namespace chip