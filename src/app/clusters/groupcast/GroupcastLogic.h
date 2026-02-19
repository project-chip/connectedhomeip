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

#include "GroupcastContext.h"
#include <app/AttributeValueEncoder.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/server/Server.h>
#include <clusters/Groupcast/AttributeIds.h>
#include <clusters/Groupcast/ClusterId.h>
#include <clusters/Groupcast/CommandIds.h>
#include <clusters/Groupcast/Commands.h>
#include <clusters/Groupcast/Enums.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

using Status = chip::Protocols::InteractionModel::Status;

/**
 * @brief Implements the Matter specifications for the Groupcast cluster
 */

class GroupcastLogic
{
public:
    static constexpr uint16_t kMaxMembershipEndpoints = 255;
    static constexpr uint16_t kMaxCommandEndpoints    = 20;

    struct EndpointList
    {
        EndpointId entries[kMaxMembershipEndpoints];
        uint16_t count = 0;
    };

    GroupcastLogic(GroupcastContext & context) : mContext(context) {}
    GroupcastLogic(GroupcastContext & context, BitFlags<Groupcast::Feature> features) : mContext(context), mFeatures(features) {}
    const BitFlags<Groupcast::Feature> & Features() const { return mFeatures; }

    CHIP_ERROR ReadMembership(const chip::Access::SubjectDescriptor * subject, EndpointId endpoint,
                              AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadMaxMembershipCount(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadMaxMcastAddrCount(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadUsedMcastAddrCount(EndpointId endpoint, AttributeValueEncoder & aEncoder);

    Status JoinGroup(FabricIndex fabric_index, const Groupcast::Commands::JoinGroup::DecodableType & data);
    Status LeaveGroup(FabricIndex fabric_index, const Groupcast::Commands::LeaveGroup::DecodableType & data,
                      EndpointList & endpoints);
    Status UpdateGroupKey(FabricIndex fabric_index, const Groupcast::Commands::UpdateGroupKey::DecodableType & data);
    Status ConfigureAuxiliaryACL(FabricIndex fabric_index, const Groupcast::Commands::ConfigureAuxiliaryACL::DecodableType & data);

    void SetDataModelProvider(DataModel::Provider & provider) { mDataModelProvider = &provider; }
    void ResetDataModelProvider() { mDataModelProvider = nullptr; }

private:
    Credentials::GroupDataProvider & Provider() { return mContext.groupDataProvider; }
    chip::FabricTable & Fabrics() { return mContext.fabricTable; }

    Status SetKeySet(FabricIndex fabric_index, KeysetId keyset_id, const chip::ByteSpan & key);
    Status RemoveGroup(FabricIndex fabric_index, GroupId group_id, const Groupcast::Commands::LeaveGroup::DecodableType & data,
                       EndpointList & endpoints);
    Status RemoveGroupEndpoint(FabricIndex fabric_index, GroupId group_id, EndpointId endpoint_id, EndpointList & endpoints);

    GroupcastContext & mContext;
    const BitFlags<Groupcast::Feature> mFeatures;
    DataModel::Provider * mDataModelProvider = nullptr;
};

} // namespace Clusters
} // namespace app
} // namespace chip
