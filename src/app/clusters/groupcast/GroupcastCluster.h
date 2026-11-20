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
#include <access/AccessControl.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/Groupcast/AttributeIds.h>
#include <clusters/Groupcast/ClusterId.h>
#include <clusters/Groupcast/CommandIds.h>
#include <clusters/Groupcast/Commands.h>
#include <clusters/Groupcast/Enums.h>
#include <credentials/GroupDataProvider.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/TimerDelegate.h>
#include <protocols/interaction_model/StatusCode.h>
#include <transport/raw/GroupcastTesting.h>

#include <optional>

namespace chip {
namespace app {
namespace Clusters {

/**
 * @brief Provides code-driven implementation for the Groupcast cluster server.
 */
class GroupcastCluster : public DefaultServerCluster,
                         public Credentials::GroupDataProvider::GroupListener,
                         public ::chip::Groupcast::Testing::Delegate
{
public:
    static constexpr uint16_t kMaxMembershipEndpoints = 255;
    static constexpr uint16_t kMaxCommandEndpoints    = 20;

    GroupcastCluster(GroupcastContext && context);
    GroupcastCluster(GroupcastContext && context, BitFlags<Groupcast::Feature> features);
    virtual ~GroupcastCluster() override;

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType shutdownType) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & arguments, CommandHandler * handler) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    // Groupcast::Testing::Delegate implementation
    void FlushGroupcastTestingEvent() override;

private:
    struct EndpointList
    {
        EndpointId entries[kMaxMembershipEndpoints];
        uint16_t count = 0;
    };

    // What RollbackJoinGroup needs to undo a failed JoinGroup: the state of the group before the command, and which
    // changes the command made.
    struct JoinGroupRollbackState
    {
        bool is_new_group           = false;
        bool had_previous_key       = false;
        KeysetId previous_keyset_id = kInvalidKeysetId;
        bool created_keyset         = false;
        bool group_info_stored      = false;
        // Only valid when is_new_group is false.
        Credentials::GroupDataProvider::GroupInfo previous_info;
    };

    Protocols::InteractionModel::Status GroupcastTesting(FabricIndex fabricIndex,
                                                         Groupcast::Commands::GroupcastTesting::DecodableType data);

    const BitFlags<Groupcast::Feature> & Features() const { return mFeatures; }

    // Methods moved from GroupcastLogic
    CHIP_ERROR ReadMembership(const chip::Access::SubjectDescriptor & subject, EndpointId endpoint,
                              AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadMaxMembershipCount(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadMaxMcastAddrCount(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadUsedMcastAddrCount(EndpointId endpoint, AttributeValueEncoder & aEncoder);

    Protocols::InteractionModel::Status JoinGroup(const ConcreteCommandPath & path,
                                                  const Groupcast::Commands::JoinGroup::DecodableType & data,
                                                  const chip::Access::SubjectDescriptor & subjectDescriptor);
    Protocols::InteractionModel::Status LeaveGroup(const Groupcast::Commands::LeaveGroup::DecodableType & data,
                                                   EndpointList & endpoints,
                                                   const chip::Access::SubjectDescriptor & subjectDescriptor);
    Protocols::InteractionModel::Status UpdateGroupKey(const ConcreteCommandPath & path,
                                                       const Groupcast::Commands::UpdateGroupKey::DecodableType & data,
                                                       const chip::Access::SubjectDescriptor & subjectDescriptor);
    Protocols::InteractionModel::Status
    ConfigureAuxiliaryACL(const Groupcast::Commands::ConfigureAuxiliaryACL::DecodableType & data,
                          const chip::Access::SubjectDescriptor & subjectDescriptor);

    void SetDataModelProvider(DataModel::Provider & provider) { mDataModelProvider = &provider; }
    void ResetDataModelProvider() { mDataModelProvider = nullptr; }

    void SetFabricUnderTest(FabricIndex fabricUnderTest);
    static void OnGroupcastTestingDone(System::Layer * aLayer, void * appState);
    TimerDelegate & GetTimerDelegate() const { return mGroupcastContext.timerDelegate; }

    // GroupDataProvider::GroupListener implementation
    void OnGroupAdded(FabricIndex fabric_index, const Credentials::GroupDataProvider::GroupInfo & new_group) override;
    void OnGroupRemoved(FabricIndex fabric_index, const Credentials::GroupDataProvider::GroupInfo & old_group) override;
    void OnGroupModified(FabricIndex fabric_index, const GroupId & modified_group_id) override;

    Credentials::GroupDataProvider & Provider() { return mGroupcastContext.groupDataProvider; }
    chip::FabricTable & Fabrics() { return mGroupcastContext.fabricTable; }

    // Maps `group_id` to `keyset_id`, creating the keyset first when `key` has a value.
    // When Success is returned, `created_keyset` tells whether this call created the keyset.
    Protocols::InteractionModel::Status SetKeySet(const ConcreteCommandPath & path,
                                                  const chip::Access::SubjectDescriptor & subjectDescriptor, GroupId group_id,
                                                  KeysetId keyset_id, const chip::Optional<chip::ByteSpan> & key,
                                                  bool & created_keyset);
    // Stores the GroupInfo and the endpoints of a validated JoinGroup, and records in `state` what was stored.
    CHIP_ERROR StoreJoinGroupMembership(FabricIndex fabric_index, const Groupcast::Commands::JoinGroup::DecodableType & data,
                                        const Credentials::GroupDataProvider::GroupInfo & info, JoinGroupRollbackState & state);
    // Undoes the changes of a failed JoinGroup: the GroupInfo, the group-key mapping and a keyset created by the command.
    // Best effort: the command is already failing, so a step that fails is logged and the remaining steps still run.
    // Endpoint changes to a group that existed before the command are not undone.
    void RollbackJoinGroup(FabricIndex fabric_index, const Groupcast::Commands::JoinGroup::DecodableType & data,
                           const JoinGroupRollbackState & state);
    Protocols::InteractionModel::Status RemoveGroup(GroupId group_id, const Groupcast::Commands::LeaveGroup::DecodableType & data,
                                                    EndpointList * endpoints,
                                                    const chip::Access::SubjectDescriptor & subjectDescriptor);
    Protocols::InteractionModel::Status RemoveGroupEndpoint(FabricIndex fabric_index, GroupId group_id, EndpointId endpoint_id,
                                                            EndpointList * endpoints);
    void UpdateUsedMcastAddrCount();
    void NotifyUsedMcastAddrCountOnChange();
    void NotifyMembershipChanged();
    // Removes, for every fabric, the group-key mappings whose group has no GroupInfo.
    void ReconcileGroupKeys();
    // Removes group-key mappings of the fabric: those of `group_id` when it has a value, otherwise those whose group
    // has no GroupInfo. Best effort: a mapping that cannot be removed is logged.
    void RemoveGroupKeyMappings(FabricIndex fabric_index, std::optional<GroupId> group_id);

    GroupcastContext mGroupcastContext;
    const BitFlags<Groupcast::Feature> mFeatures;
    DataModel::Provider * mDataModelProvider = nullptr;
    uint16_t mUsedMcastAddrCount             = 0;
    bool mIanaAddressUsed                    = false;

    Groupcast::GroupcastTestingEnum mTestingState = Groupcast::GroupcastTestingEnum::kDisableTesting;
    class MembershipChangedTimer : public TimerContext
    {
    public:
        MembershipChangedTimer(GroupcastCluster & cluster) : mCluster(cluster) {}
        void Start();
        void Cancel();
        void TimerFired() override;

    private:
        GroupcastCluster & mCluster;
    };

    class GroupcastTestingTimer : public TimerContext
    {
    public:
        GroupcastTestingTimer(GroupcastCluster & cluster) : mCluster(cluster) {}
        void Start(uint32_t seconds);
        void Cancel();
        void TimerFired() override;

    private:
        GroupcastCluster & mCluster;
    };

    MembershipChangedTimer mMembershipChangedTimer;
    GroupcastTestingTimer mGroupcastTestingTimer;
};

} // namespace Clusters
} // namespace app
} // namespace chip
