/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <credentials/CHIPCert.h>
#include <functional>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/core/NodeId.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <vector>

namespace chip {
namespace app {

namespace datastore {

struct AccessControlEntryStruct
{
    Clusters::JointFabricDatastore::DatastoreAccessControlEntryPrivilegeEnum privilege =
        static_cast<Clusters::JointFabricDatastore::DatastoreAccessControlEntryPrivilegeEnum>(0);
    Clusters::JointFabricDatastore::DatastoreAccessControlEntryAuthModeEnum authMode =
        static_cast<Clusters::JointFabricDatastore::DatastoreAccessControlEntryAuthModeEnum>(0);
    std::vector<uint64_t> subjects;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type> targets;
};

struct ACLEntryStruct
{
    chip::NodeId nodeID = static_cast<chip::NodeId>(0);
    uint16_t listID     = static_cast<uint16_t>(0);
    AccessControlEntryStruct ACLEntry;
    Clusters::JointFabricDatastore::Structs::DatastoreStatusEntryStruct::Type statusEntry;
};

} // namespace datastore

/**
 * A struct which extends the DatastoreNodeInformationEntry type with FriendlyName buffer reservation.
 */
struct GenericDatastoreNodeInformationEntry
    : public Clusters::JointFabricDatastore::Structs::DatastoreNodeInformationEntryStruct::Type
{
    GenericDatastoreNodeInformationEntry(NodeId nodeId = 0,
                                         Clusters::JointFabricDatastore::DatastoreStateEnum state =
                                             Clusters::JointFabricDatastore::DatastoreStateEnum::kUnknownEnumValue,
                                         Optional<CharSpan> label = NullOptional)
    {
        Set(nodeId, state, label);
    }

    GenericDatastoreNodeInformationEntry(const GenericDatastoreNodeInformationEntry & op) { *this = op; }

    GenericDatastoreNodeInformationEntry & operator=(const GenericDatastoreNodeInformationEntry & op)
    {
        Set(op.nodeID, op.commissioningStatusEntry.state, MakeOptional(op.friendlyName));
        return *this;
    }

    void Set(NodeId nodeId, Clusters::JointFabricDatastore::DatastoreStateEnum state, Optional<CharSpan> label = NullOptional)
    {
        this->nodeID                         = nodeId;
        this->commissioningStatusEntry.state = state;
        Set(label);
    }

    void Set(Optional<CharSpan> label = NullOptional)
    {
        if (label.HasValue())
        {
            memset(mFriendlyNameBuffer, 0, sizeof(mFriendlyNameBuffer));
            if (label.Value().size() > sizeof(mFriendlyNameBuffer))
            {
                memcpy(mFriendlyNameBuffer, label.Value().data(), sizeof(mFriendlyNameBuffer));
                this->friendlyName = CharSpan(mFriendlyNameBuffer, sizeof(mFriendlyNameBuffer));
            }
            else
            {
                memcpy(mFriendlyNameBuffer, label.Value().data(), label.Value().size());
                this->friendlyName = CharSpan(mFriendlyNameBuffer, label.Value().size());
            }
        }
        else
        {
            this->friendlyName = CharSpan();
        }
    }

private:
    static constexpr size_t kFriendlyNameMaxSize = 32u;

    char mFriendlyNameBuffer[kFriendlyNameMaxSize];
};

class JointFabricDatastore
{
public:
    static JointFabricDatastore & GetInstance()
    {
        static JointFabricDatastore sInstance;
        return sInstance;
    }

    class Delegate
    {
    public:
        Delegate() {}
        virtual ~Delegate() {}

        virtual CHIP_ERROR
        SyncNode(NodeId nodeId,
                 const Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type & endpointGroupIDEntry,
                 std::function<void()> onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        virtual CHIP_ERROR
        SyncNode(NodeId nodeId,
                 const Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type & nodeKeySetEntry,
                 std::function<void()> onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        virtual CHIP_ERROR
        SyncNode(NodeId nodeId,
                 const Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type & bindingEntry,
                 std::function<void()> onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        virtual CHIP_ERROR SyncNode(NodeId nodeId,
                                    const Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type & aclEntry,
                                    std::function<void()> onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }
    };

    ByteSpan GetAnchorRootCA() const { return ByteSpan(mAnchorRootCA, mAnchorRootCALength); }

    CHIP_ERROR SetAnchorNodeId(NodeId anchorNodeId)
    {
        mAnchorNodeId = anchorNodeId;
        return CHIP_NO_ERROR;
    }
    NodeId GetAnchorNodeId() { return mAnchorNodeId; }

    CHIP_ERROR SetAnchorVendorId(VendorId anchorVendorId)
    {
        mAnchorVendorId = anchorVendorId;
        return CHIP_NO_ERROR;
    }
    VendorId GetAnchorVendorId() { return mAnchorVendorId; }

    CHIP_ERROR SetFriendlyName(const CharSpan & friendlyName)
    {
        if (friendlyName.size() >= sizeof(mFriendlyNameBuffer))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        mFriendlyNameBufferLength = friendlyName.size();
        memcpy(mFriendlyNameBuffer, friendlyName.data(), mFriendlyNameBufferLength);
        mFriendlyNameBuffer[mFriendlyNameBufferLength] = '\0'; // Ensure null-termination
        return CHIP_NO_ERROR;
    }
    CharSpan GetFriendlyName() const { return CharSpan(mFriendlyNameBuffer, mFriendlyNameBufferLength); }

    const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreGroupInformationEntryStruct::Type> & GetGroupEntries()
    {
        return mGroupInformationEntries;
    }

    Clusters::JointFabricDatastore::Structs::DatastoreStatusEntryStruct::Type & GetStatus() { return mDatastoreStatusEntry; }

    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type> & GetEndpointGroupIDList()
    {
        return mEndpointGroupIDEntries;
    }

    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> & GetEndpointBindingList()
    {
        return mEndpointBindingEntries;
    }

    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type> & GetNodeKeySetList()
    {
        return mNodeKeySetEntries;
    }

    std::vector<datastore::ACLEntryStruct> & GetNodeACLList() { return mACLEntries; }

    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type> & GetNodeEndpointList()
    {
        return mEndpointEntries;
    }

    CHIP_ERROR AddPendingNode(NodeId nodeId, const CharSpan & friendlyName);
    CHIP_ERROR UpdateNode(NodeId nodeId, const CharSpan & friendlyName);
    CHIP_ERROR RemoveNode(NodeId nodeId);
    CHIP_ERROR RefreshNode(NodeId nodeId, ReadOnlyBuffer<DataModel::EndpointEntry> endpointsList);

    CHIP_ERROR SetNode(NodeId nodeId, Clusters::JointFabricDatastore::DatastoreStateEnum state);

    CHIP_ERROR AddGroupKeySetEntry(Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet);
    bool IsGroupKeySetEntryPresent(uint16_t groupKeySetId);
    CHIP_ERROR RemoveGroupKeySetEntry(uint16_t groupKeySetId);
    CHIP_ERROR UpdateGroupKeySetEntry(Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet);

    CHIP_ERROR AddAdmin(Clusters::JointFabricDatastore::Structs::DatastoreAdministratorInformationEntryStruct::Type & adminId);
    bool IsAdminEntryPresent(NodeId nodeId);
    CHIP_ERROR UpdateAdmin(NodeId nodeId, CharSpan friendlyName, ByteSpan icac);
    CHIP_ERROR RemoveAdmin(NodeId nodeId);

    CHIP_ERROR AddGroup(const Clusters::JointFabricDatastore::Commands::AddGroup::DecodableType & commandData);
    CHIP_ERROR UpdateGroup(const Clusters::JointFabricDatastore::Commands::UpdateGroup::DecodableType & commandData);
    CHIP_ERROR RemoveGroup(const Clusters::JointFabricDatastore::Commands::RemoveGroup::DecodableType & commandData);

    CHIP_ERROR UpdateEndpointForNode(NodeId nodeId, EndpointId endpointId, CharSpan friendlyName);

    CHIP_ERROR AddGroupIDToEndpointForNode(NodeId nodeId, EndpointId endpointId, GroupId groupId);
    CHIP_ERROR RemoveGroupIDFromEndpointForNode(NodeId nodeId, EndpointId endpointId, GroupId groupId);

    CHIP_ERROR
    AddBindingToEndpointForNode(NodeId nodeId, EndpointId endpointId,
                                const Clusters::JointFabricDatastore::Structs::DatastoreBindingTargetStruct::Type & binding);
    CHIP_ERROR
    RemoveBindingFromEndpointForNode(uint16_t listId, NodeId nodeId, EndpointId endpointId);

    CHIP_ERROR
    AddACLToNode(NodeId nodeId,
                 const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::DecodableType & aclEntry);
    CHIP_ERROR RemoveACLFromNode(uint16_t listId, NodeId nodeId);

    const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type> & GetGroupKeySetList()
    {
        return mGroupKeySetList;
    }
    const std::vector<GenericDatastoreNodeInformationEntry> & GetNodeInformationEntries() { return mNodeInformationEntries; }
    const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreAdministratorInformationEntryStruct::Type> &
    GetAdminEntries()
    {
        return mAdminEntries;
    }

    /**
     * Used to notify of changes in the node list and more TODO.
     */
    class Listener
    {
    public:
        virtual ~Listener() = default;

        /**
         * Notifies of a change in the node list.
         */
        virtual void MarkNodeListChanged() = 0;

    private:
        Listener * mNext = nullptr;

        friend class JointFabricDatastore;
    };

    /**
     * Add a listener to be notified of changes in the Joint Fabric Datastore.
     *
     * @param [in] listener  The listener to add.
     */
    void AddListener(Listener & listener);

    /**
     * Remove a listener from being notified of changes in the Joint Fabric Datastore.
     *
     * @param [in] listener  The listener to remove.
     */
    void RemoveListener(Listener & listener);

    CHIP_ERROR SetDelegate(Delegate * delegate)
    {
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        mDelegate = delegate;

        return CHIP_NO_ERROR;
    }

private:
    static constexpr size_t kMaxNodes            = 256;
    static constexpr size_t kMaxAdminNodes       = 32;
    static constexpr size_t kMaxGroups           = kMaxNodes / 16;
    static constexpr size_t kMaxGroupKeySet      = kMaxGroups * 16;
    static constexpr size_t kMaxFriendlyNameSize = 32;
    static constexpr size_t kMaxACLs             = 64;

    uint8_t mAnchorRootCA[Credentials::kMaxDERCertLength] = { 0 };
    size_t mAnchorRootCALength                            = 0;
    char mFriendlyNameBuffer[kMaxFriendlyNameSize]        = { 0 };
    size_t mFriendlyNameBufferLength                      = 0;
    NodeId mAnchorNodeId                                  = kUndefinedNodeId;
    VendorId mAnchorVendorId                              = VendorId::NotSpecified;
    Clusters::JointFabricDatastore::Structs::DatastoreStatusEntryStruct::Type mDatastoreStatusEntry;

    // TODO: Persist these members to local storage
    std::vector<GenericDatastoreNodeInformationEntry> mNodeInformationEntries;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type> mGroupKeySetList;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreAdministratorInformationEntryStruct::Type> mAdminEntries;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreGroupInformationEntryStruct::Type> mGroupInformationEntries;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type> mEndpointGroupIDEntries;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> mEndpointBindingEntries;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type> mNodeKeySetEntries;
    std::vector<datastore::ACLEntryStruct> mACLEntries;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type> mEndpointEntries;

    Listener * mListeners = nullptr;

    CHIP_ERROR IsNodeIDInDatastore(NodeId nodeId, size_t & index);

    CHIP_ERROR UpdateNodeKeySetList(Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet);
    CHIP_ERROR RemoveKeySet(uint16_t groupKeySetId);

    CHIP_ERROR IsGroupIDInDatastore(GroupId groupId, size_t & index);
    CHIP_ERROR IsNodeIdInNodeInformationEntries(NodeId nodeId, size_t & index);
    CHIP_ERROR IsNodeIdAndEndpointInEndpointInformationEntries(NodeId nodeId, EndpointId endpointId, size_t & index);

    CHIP_ERROR GenerateAndAssignAUniqueListID(uint16_t & listId);
    bool BindingMatches(const Clusters::JointFabricDatastore::Structs::DatastoreBindingTargetStruct::Type & binding1,
                        const Clusters::JointFabricDatastore::Structs::DatastoreBindingTargetStruct::Type & binding2);
    bool ACLMatches(const datastore::AccessControlEntryStruct & acl1,
                    const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::DecodableType & acl2);
    bool ACLTargetMatches(const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type & target1,
                          const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type & target2);

    CHIP_ERROR AddNodeKeySetEntry(GroupId groupId, uint16_t groupKeySetId);
    CHIP_ERROR RemoveNodeKeySetEntry(GroupId groupId, uint16_t groupKeySetId);

    Delegate * mDelegate = nullptr;
};

} // namespace app
} // namespace chip
