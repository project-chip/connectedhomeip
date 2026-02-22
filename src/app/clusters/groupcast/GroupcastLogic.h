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

class GroupcastLogic : public Credentials::GroupDataProvider::GroupListener
{
public:
    /**
     * @brief Interface to listen for changes in the per-group address count.
     */
    class Listener
    {
    public:
        virtual ~Listener() = default;
        /**
         *  Callback invoked when membership changes (groups added or removed).
         */
        virtual void OnMembershipChanged() = 0;
        /**
         *  Callback invoked when used multicast addresses count changes.
         */
        virtual void OnUsedMcastAddrCountChange() = 0;
    };

public:
    static constexpr uint16_t kMaxMembershipEndpoints = 255;
    static constexpr uint16_t kMaxCommandEndpoints    = 20;

    struct EndpointList
    {
        EndpointId entries[kMaxMembershipEndpoints];
        uint16_t count = 0;
    };

    GroupcastLogic(GroupcastContext & context);
    GroupcastLogic(GroupcastContext & context, BitFlags<Groupcast::Feature> features);
    ~GroupcastLogic() override;
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

    // Listener
    void SetListener(Listener * listener) { mListener = listener; }
    void RemoveListener() { mListener = nullptr; }

private:
    Credentials::GroupDataProvider & Provider() { return mContext.groupDataProvider; }
    chip::FabricTable & Fabrics() { return mContext.fabricTable; }

    Status SetKeySet(FabricIndex fabric_index, GroupId group_id, KeysetId keyset_id, const chip::Optional<chip::ByteSpan> & key);
    Status RemoveGroup(FabricIndex fabric_index, GroupId group_id, const Groupcast::Commands::LeaveGroup::DecodableType & data,
                       EndpointList & endpoints);
    Status RemoveGroupEndpoint(FabricIndex fabric_index, GroupId group_id, EndpointId endpoint_id, EndpointList & endpoints);
    uint16_t GetUsedMcastAddrCount();
    // GroupListener implementation
    void OnGroupAdded(FabricIndex fabric_index, const Credentials::GroupDataProvider::GroupInfo & new_group) override;
    void OnGroupRemoved(FabricIndex fabric_index, const Credentials::GroupDataProvider::GroupInfo & old_group) override;
    void NotifyUsedMcastAddrCountChange();
    void NotifyMembershipChanged();

    GroupcastContext & mContext;
    const BitFlags<Groupcast::Feature> mFeatures;
    DataModel::Provider * mDataModelProvider = nullptr;
    uint16_t mUsedMcastAddrCount             = 0;
    Listener * mListener                     = nullptr;
};

} // namespace Clusters
} // namespace app
} // namespace chip
