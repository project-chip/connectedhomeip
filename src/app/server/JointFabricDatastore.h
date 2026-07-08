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

#include <algorithm>
#include <app-common/zap-generated/cluster-objects.h>
<<<<<<< HEAD
=======
#include <app/data-model-provider/MetadataTypes.h>
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <functional>
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>
<<<<<<< HEAD
=======
#include <lib/support/ReadOnlyBuffer.h>
#include <map>
#include <type_traits>
#include <unordered_map>
#include <utility>
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
#include <vector>

namespace chip {
namespace app {

<<<<<<< HEAD
=======
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

namespace detail {

/**
 * Searches `vec` for the first entry that returns true when passed to `pred`, then marks that entry as committed.
 *
 * @param [in,out] vec   The vector to search; the matching entry is mutated in place to mark it committed.
 * @param [in] pred      Predicate invoked as `bool(const T &)`; the first entry for which it returns true is marked.
 */
template <typename T, typename Pred>
void MarkEntryCommittedIfFound(std::vector<T> & vec, Pred pred)
{
    static_assert(std::is_invocable_r_v<bool, Pred, const T &>,
                  "MarkEntryCommittedIfFound predicate must accept a const T & and return bool");
    auto it = std::find_if(vec.begin(), vec.end(), pred);
    if (it != vec.end())
    {
        it->statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
    }
}

} // namespace detail

enum RefreshState
{
    kIdle,
    kRefreshingEndpoints,
    kRefreshingGroups,
    kRefreshingBindings,
    kFetchingGroupKeySetList,
    kFetchingGroupKeySets,
    kRefreshingGroupKeySets,
    kRefreshingACLs,
};

>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
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

<<<<<<< HEAD
=======
    /**
     * We track the FabricIndex of the Joint Fabric so that the datastore can be wiped if the Joint Fabric is removed.
     */
    void SetAnchorFabricIndex(FabricIndex anchorFabricIndex) { mAnchorFabricIndex = anchorFabricIndex; }
    FabricIndex GetAnchorFabricIndex() const { return mAnchorFabricIndex; }

    /**
     * Runs when a fabric is removed from the node's FabricTable. If the removed fabric is the anchor fabric, the entire datastore
     * should be cleared. A no-op for any other fabric.
     */
    void OnFabricRemoved(FabricIndex fabricIndex)
    {
        // The datastore is the anchor's registry for a single joint fabric. Only act when that fabric is
        // removed; records for any other fabric are not stored here.
        if (mAnchorFabricIndex == kUndefinedFabricIndex || fabricIndex != mAnchorFabricIndex)
        {
            return;
        }

        ClearAllRecords();

        for (Listener * listener = mListeners; listener != nullptr; listener = listener->mNext)
        {
            listener->MarkNodeListChanged();
        }
    }

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

    void SetStatus(Clusters::JointFabricDatastore::DatastoreStateEnum state, uint32_t updateTimestamp, uint8_t failureCode)
    {
        mDatastoreStatusEntry.state           = state;
        mDatastoreStatusEntry.updateTimestamp = updateTimestamp;
        mDatastoreStatusEntry.failureCode     = failureCode;
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

>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
    CHIP_ERROR AddPendingNode(NodeId nodeId, const CharSpan & friendlyName);
    CHIP_ERROR UpdateNode(NodeId nodeId, const CharSpan & friendlyName);
    CHIP_ERROR RemoveNode(NodeId nodeId);
    CHIP_ERROR RefreshNode(NodeId nodeId);

    CHIP_ERROR SetNode(NodeId nodeId, Clusters::JointFabricDatastore::DatastoreStateEnum state);

    CHIP_ERROR AddGroupKeySetEntry(Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet);
    bool IsGroupKeySetEntryPresent(uint16_t groupKeySetId);
    CHIP_ERROR RemoveGroupKeySetEntry(uint16_t groupKeySetId);
    CHIP_ERROR UpdateGroupKeySetEntry(Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet);

    CHIP_ERROR AddAdmin(Clusters::JointFabricDatastore::Structs::DatastoreAdministratorInformationEntryStruct::Type & adminId);
    bool IsAdminEntryPresent(NodeId nodeId);
    CHIP_ERROR UpdateAdmin(NodeId nodeId, CharSpan friendlyName, ByteSpan icac);
    CHIP_ERROR RemoveAdmin(NodeId nodeId);

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

private:
<<<<<<< HEAD
    static constexpr size_t kMaxNodes       = 256;
    static constexpr size_t kMaxAdminNodes  = 32;
    static constexpr size_t kMaxGroups      = kMaxNodes / 16;
    static constexpr size_t kMaxGroupKeySet = kMaxGroups * 16;
=======
    // Epoch keys are raw group-key secret material. Hold them in a self-zeroizing buffer so they are
    // wiped on every deallocation path (per-record erase, overwrite, and whole-map/object destruction),
    // not just the explicit removal helpers.
    using EpochKeyStorage = Crypto::SensitiveDataBuffer<Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES>;

    struct GroupKeySetStorage
    {
        EpochKeyStorage epochKey0;
        EpochKeyStorage epochKey1;
        EpochKeyStorage epochKey2;
    };
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))

    NodeId mAnchorNodeId     = kUndefinedNodeId;
    VendorId mAnchorVendorId = VendorId::NotSpecified;

<<<<<<< HEAD
=======
    struct AdminEntryStorage
    {
        // Friendly names are stored as owned raw bytes for CharSpan (not C-string) semantics.
        std::vector<char> friendlyName;
        // ICAC DER is sensitive credential material; hold it in a self-zeroizing buffer (see EpochKeyStorage).
        Crypto::SensitiveDataBuffer<Credentials::kMaxDERCertLength> icac;
    };

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
    FabricIndex mAnchorFabricIndex                        = kUndefinedFabricIndex;
    Clusters::JointFabricDatastore::Structs::DatastoreStatusEntryStruct::Type mDatastoreStatusEntry;

    /**
     * Clears all stored records and resets anchor identity. Used by OnFabricRemoved() when the joint fabric is removed.
     * Self-zeroizing buffers wipe their secrets as the containers are cleared.
     */
    void ClearAllRecords()
    {
        // Erasing the secret-bearing maps destroys the self-zeroizing EpochKeyStorage / ICAC buffers,
        // wiping their contents.
        mGroupKeySetStorage.clear();
        mAdminEntryStorage.clear();
        mGroupInformationStorage.clear();
        mEndpointFriendlyNameStorage.clear();

        mNodeInformationEntries.clear();
        mGroupKeySetList.clear();
        mAdminEntries.clear();
        mGroupInformationEntries.clear();
        mEndpointGroupIDEntries.clear();
        mEndpointBindingEntries.clear();
        mNodeKeySetEntries.clear();
        mACLEntries.clear();
        mEndpointEntries.clear();

        // Reset anchor identity: with the joint fabric gone, the datastore no longer describes a fabric.
        memset(mAnchorRootCA, 0, sizeof(mAnchorRootCA));
        mAnchorRootCALength       = 0;
        mFriendlyNameBuffer[0]    = '\0';
        mFriendlyNameBufferLength = 0;
        mAnchorNodeId             = kUndefinedNodeId;
        mAnchorVendorId           = VendorId::NotSpecified;
        mAnchorFabricIndex        = kUndefinedFabricIndex;
        mDatastoreStatusEntry     = Clusters::JointFabricDatastore::Structs::DatastoreStatusEntryStruct::Type{};
    }

    // TODO: Persist these members to local storage
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
    std::vector<GenericDatastoreNodeInformationEntry> mNodeInformationEntries;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type> mGroupKeySetList;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreAdministratorInformationEntryStruct::Type> mAdminEntries;

    Listener * mListeners = nullptr;

    CHIP_ERROR IsNodeIDInDatastore(NodeId nodeId, size_t & index);
<<<<<<< HEAD
=======

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

    CHIP_ERROR
    CopyGroupKeySetWithOwnedSpans(const Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & source,
                                  Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & destination);
    void RemoveGroupKeySetStorage(uint16_t groupKeySetId);

    void SetGroupInformationFriendlyNameWithOwnedStorage(
        GroupId groupId, const CharSpan & friendlyName,
        Clusters::JointFabricDatastore::Structs::DatastoreGroupInformationEntryStruct::Type & destination);
    void RemoveGroupInformationStorage(GroupId groupId);

    CHIP_ERROR SetAdminEntryWithOwnedStorage(
        NodeId nodeId, const CharSpan & friendlyName, const ByteSpan & icac,
        Clusters::JointFabricDatastore::Structs::DatastoreAdministratorInformationEntryStruct::Type & destination);
    void RemoveAdminEntryStorage(NodeId nodeId);

    void SetEndpointFriendlyNameWithOwnedStorage(
        NodeId nodeId, EndpointId endpointId, const CharSpan & friendlyName,
        Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type & destination);
    void RemoveEndpointFriendlyNameStorage(NodeId nodeId, EndpointId endpointId);

    // Helper methods for copying optional ByteSpan and simple nullable values
    void CopyByteSpanWithOwnedStorage(const DataModel::Nullable<ByteSpan> & source, EpochKeyStorage & storage,
                                      DataModel::Nullable<ByteSpan> & destination);

    template <typename T>
    void CopyNullableValue(const DataModel::Nullable<T> & source, DataModel::Nullable<T> & destination)
    {
        // Only update destination if source has a value; leave destination unchanged if source is null.
        if (!source.IsNull())
        {
            static_cast<void>(destination.Update(source));
        }
    }

    Delegate * mDelegate = nullptr;

    NodeId mRefreshingNodeId           = kUndefinedNodeId;
    RefreshState mRefreshState         = kIdle;
    size_t mRefreshingEndpointIndex    = 0;
    size_t mRefreshingGroupKeySetIndex = 0;

    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type> mRefreshingEndpointsList;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> mRefreshingBindingEntries;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type> mRefreshingACLEntries;
    std::vector<uint16_t> mRefreshingGroupKeySetIDs;
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
};

} // namespace app
} // namespace chip
