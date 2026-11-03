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
#include <app/data-model-provider/MetadataTypes.h>
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <deque>
#include <functional>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/TypeTraits.h>
#include <map>
#include <optional>
#include <protocols/interaction_model/StatusCode.h>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace chip {

// Forward declaration for friend class
class JFAManager;

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

    // Value the node is believed to hold while an update is Pending. Refresh must not re-adopt it, and
    // the delegate must replace it. Kept at the last committed value across back-to-back updates.
    std::optional<AccessControlEntryStruct> supersededValue;

    // Set while this entry is being removed from the node. Survives a failure recorded as CommitFailed,
    // which the cluster reports identically for adds and removals.
    bool pendingRemoval = false;
};

// The stored forms of the cluster's entry types. The base is what the cluster reports; the added state is
// the datastore's own and is not on the wire.
struct EndpointGroupIDEntryStruct : Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type
{
    // As ACLEntryStruct::pendingRemoval.
    bool pendingRemoval = false;

    // Changed when the entry is marked for a sync of a new value while an earlier sync may be in flight. A sync that
    // adds the entry commits it only if this is unchanged since the sync started; otherwise the node holds an older
    // value, which a later sync replaces.
    uint32_t syncRevision = 0;
};

struct EndpointBindingEntryStruct : Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type
{
    // As ACLEntryStruct::pendingRemoval.
    bool pendingRemoval = false;

    // As EndpointGroupIDEntryStruct::syncRevision.
    uint32_t syncRevision = 0;
};

struct NodeKeySetEntryStruct : Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type
{
    // As ACLEntryStruct::pendingRemoval.
    bool pendingRemoval = false;

    // As EndpointGroupIDEntryStruct::syncRevision. Changed when the key set is updated, and by CancelRemoval.
    uint32_t syncRevision = 0;
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

/**
 * The FailureCode, an IM status, recorded for a sync that failed with `err`. A transport timeout and a busy CASE session
 * carry no IM status, and are recorded as TIMEOUT and BUSY instead of FAILURE.
 */
inline uint8_t SyncFailureCode(CHIP_ERROR err)
{
    if (err == CHIP_ERROR_TIMEOUT)
    {
        return to_underlying(Protocols::InteractionModel::Status::Timeout);
    }
    if (err == CHIP_ERROR_BUSY)
    {
        return to_underlying(Protocols::InteractionModel::Status::Busy);
    }
    return to_underlying(Protocols::InteractionModel::ClusterStatusCode(err).GetStatus());
}

/**
 * Records a failed sync on `entry`, as CommitFailed with the IM status of `err` (see SyncFailureCode).
 */
template <typename T>
void MarkEntrySyncFailed(T & entry, CHIP_ERROR err)
{
    entry.statusEntry.state       = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitFailed;
    entry.statusEntry.failureCode = SyncFailureCode(err);
}

/**
 * Records a failed sync on the first entry matching `pred`, as CommitFailed with the IM status of `err`.
 * Adds and removals are recorded identically, as the specification requires; whether the entry is
 * being removed is tracked separately (JointFabricDatastore::HasRemovalIntent).
 */
template <typename T, typename Pred>
void MarkEntrySyncFailedIfFound(std::vector<T> & vec, Pred pred, CHIP_ERROR err)
{
    auto it = std::find_if(vec.begin(), vec.end(), pred);
    if (it == vec.end())
    {
        return;
    }
    MarkEntrySyncFailed(*it, err);
}

/**
 * The Acl and Binding attributes carry no per-entry identity on the wire, so entries on a node are
 * matched by value. A null list and an empty list compare equal: nodes report an empty subject or
 * target list as null, while the datastore sends an empty non-null list.
 */
bool AclTargetValueEquals(const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type & target1,
                          const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type & target2);

/**
 * Compares privilege, authMode, subjects and targets. Subjects and targets are compared as multisets.
 */
bool AclEntryValueEquals(const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::Type & a,
                         const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::Type & b);

/**
 * Returns the ACL list that results from applying `entry` to `current`, a node's fetched ACL list.
 *
 * - If `entry` is DeletePending, erases every entry equal to `entry`, or to `superseded` if given.
 * - Otherwise, erases every entry equal to `superseded` if given, then appends `entry` unless an equal
 *   entry is already present.
 *
 * The returned entries view the subject and target storage of `current` and `entry`.
 */
std::vector<Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>
ApplyAclEdit(const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type> & current,
             const Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type & entry,
             const std::optional<Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::Type> & superseded);

bool BindingTargetValueEquals(const Clusters::JointFabricDatastore::Structs::DatastoreBindingTargetStruct::Type & binding1,
                              const Clusters::JointFabricDatastore::Structs::DatastoreBindingTargetStruct::Type & binding2);

/**
 * Compares endpointID and the binding target.
 */
bool BindingEntryValueEquals(const Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type & a,
                             const Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type & b);

/**
 * Returns the binding list that results from applying `entry` to `current`, a node's fetched binding
 * list. If `entry` is DeletePending, erases every entry equal to it; otherwise appends `entry` unless
 * an equal entry is already present.
 */
std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>
ApplyBindingEdit(const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> & current,
                 const Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type & entry);

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

    // Single-entry syncs that may wait per node while another runs. Commands that would exceed it fail with BUSY.
    static constexpr size_t kMaxQueuedNodeSyncs = 64;

    /**
     * Pushes datastore changes to nodes and reads their current state.
     *
     * - If a SyncNode or Fetch* call returns CHIP_NO_ERROR, its callback is invoked exactly once, with
     *   the result of the operation. If the call returns an error, the callback is never invoked.
     * - Span-backed data in the arguments is valid only until the call returns; copy what is kept.
     * - Vectors passed to a Fetch* callback are valid only during the callback.
     * - Delegates do not modify the vectors they are given.
     */
    class Delegate
    {
    public:
        Delegate() {}
        virtual ~Delegate() {}

        virtual CHIP_ERROR
        SyncNode(NodeId nodeId,
                 const Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type & endpointGroupIDEntry,
                 std::function<void(CHIP_ERROR)> onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        virtual CHIP_ERROR
        SyncNode(NodeId nodeId,
                 const Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type & nodeKeySetEntry,
                 std::function<void(CHIP_ERROR)> onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        virtual CHIP_ERROR
        SyncNode(NodeId nodeId,
                 const Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type & bindingEntry,
                 std::function<void(CHIP_ERROR)> onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        virtual CHIP_ERROR
        SyncNode(NodeId nodeId, EndpointId endpointId,
                 std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> & bindingEntries,
                 std::function<void(CHIP_ERROR)> onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        virtual CHIP_ERROR
        SyncNode(NodeId nodeId,
                 std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> & bindingEntries,
                 std::function<void(CHIP_ERROR)> onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        /**
         * Edits one entry of the node's ACL. The node's entries carry no nodeID or listID, so the edit matches them by value
         * (see detail::ApplyAclEdit):
         *
         * - If `aclEntry` is DeletePending, every entry equal to it, or to `superseded` if set, is removed.
         * - Otherwise every entry equal to `superseded`, if set, is removed, and `aclEntry` is added unless an equal entry
         *   is present. `superseded` is the value the entry had before an update.
         *
         * Removing a value the node does not hold succeeds.
         */
        virtual CHIP_ERROR
        SyncNode(NodeId nodeId, const Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type & aclEntry,
                 const std::optional<Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::Type> & superseded,
                 std::function<void(CHIP_ERROR)> onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        virtual CHIP_ERROR
        SyncNode(NodeId nodeId,
                 const std::vector<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type> & aclEntries,
                 std::function<void(CHIP_ERROR)> onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        virtual CHIP_ERROR SyncNode(NodeId nodeId,
                                    const Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet,
                                    std::function<void(CHIP_ERROR)> onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        virtual CHIP_ERROR FetchEndpointList(
            NodeId nodeId,
            std::function<void(CHIP_ERROR,
                               const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type> &)>
                onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        virtual CHIP_ERROR FetchEndpointGroupList(
            NodeId nodeId, EndpointId endpointId,
            std::function<
                void(CHIP_ERROR,
                     const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreGroupInformationEntryStruct::Type> &)>
                onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        virtual CHIP_ERROR FetchEndpointBindingList(
            NodeId nodeId, EndpointId endpointId,
            std::function<
                void(CHIP_ERROR,
                     const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> &)>
                onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        virtual CHIP_ERROR FetchGroupKeySetList(NodeId nodeId,
                                                std::function<void(CHIP_ERROR, const std::vector<uint16_t> &)> onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        virtual CHIP_ERROR FetchGroupKeySet(
            NodeId nodeId, uint16_t groupKeySetID,
            std::function<void(CHIP_ERROR, const app::Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type &)>
                onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        virtual CHIP_ERROR FetchACLList(
            NodeId nodeId,
            std::function<void(CHIP_ERROR,
                               const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type> &)>
                onSuccess)
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }
    };

    CHIP_ERROR SetAnchorRootCA(const ByteSpan & anchorRootCA)
    {
        if (anchorRootCA.size() >= sizeof(mAnchorRootCA))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        mAnchorRootCALength = anchorRootCA.size();
        memcpy(mAnchorRootCA, anchorRootCA.data(), mAnchorRootCALength);
        return CHIP_NO_ERROR;
    }
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

    std::vector<datastore::EndpointGroupIDEntryStruct> & GetEndpointGroupIDList() { return mEndpointGroupIDEntries; }

    std::vector<datastore::EndpointBindingEntryStruct> & GetEndpointBindingList() { return mEndpointBindingEntries; }

    std::vector<datastore::NodeKeySetEntryStruct> & GetNodeKeySetList() { return mNodeKeySetEntries; }

    std::vector<datastore::ACLEntryStruct> & GetNodeACLList() { return mACLEntries; }

    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type> & GetNodeEndpointList()
    {
        return mEndpointEntries;
    }

    CHIP_ERROR AddPendingNode(NodeId nodeId, const CharSpan & friendlyName);
    CHIP_ERROR UpdateNode(NodeId nodeId, const CharSpan & friendlyName);
    CHIP_ERROR RemoveNode(NodeId nodeId);
    CHIP_ERROR RefreshNode(NodeId nodeId);
    CHIP_ERROR ContinueRefresh();

    CHIP_ERROR SetNode(NodeId nodeId, Clusters::JointFabricDatastore::DatastoreStateEnum state);

    CHIP_ERROR AddGroupKeySetEntry(const Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet);
    bool IsGroupKeySetEntryPresent(uint16_t groupKeySetId);
    CHIP_ERROR RemoveGroupKeySetEntry(uint16_t groupKeySetId);
    CHIP_ERROR UpdateGroupKeySetEntry(Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet);

    CHIP_ERROR
    AddAdmin(const Clusters::JointFabricDatastore::Structs::DatastoreAdministratorInformationEntryStruct::Type & adminId);
    bool IsAdminEntryPresent(NodeId nodeId);
    CHIP_ERROR UpdateAdmin(NodeId nodeId, Optional<CharSpan> friendlyName, Optional<ByteSpan> icac);
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

    CHIP_ERROR TestAddNodeKeySetEntry(GroupId groupId, uint16_t groupKeySetId, NodeId nodeId);
    CHIP_ERROR TestAddEndpointEntry(EndpointId endpointId, NodeId nodeId, CharSpan friendlyName);

    CHIP_ERROR ForceAddNodeKeySetEntry(uint16_t groupKeySetId, NodeId nodeId);

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

    struct GroupInformationStorage
    {
        // Friendly names are surfaced as CharSpan and may come from non-null-terminated buffers.
        // Keep raw owned bytes in vector<char> to preserve exact length semantics for Span use.
        std::vector<char> friendlyName;
    };

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
        // Operations in flight complete later; their completions find a new generation and return. Queued ones
        // never start.
        ++mSyncGeneration;
        ResetRefreshState();
        mNodeSyncQueues.clear();
        mAclTombstones.clear();
        mBindingTombstones.clear();
        mEndpointGroupTombstones.clear();
        mNodeKeySetTombstones.clear();

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
    std::vector<GenericDatastoreNodeInformationEntry> mNodeInformationEntries;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type> mGroupKeySetList;
    std::unordered_map<uint16_t, GroupKeySetStorage> mGroupKeySetStorage;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreAdministratorInformationEntryStruct::Type> mAdminEntries;
    std::unordered_map<NodeId, AdminEntryStorage> mAdminEntryStorage;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreGroupInformationEntryStruct::Type> mGroupInformationEntries;
    std::unordered_map<GroupId, GroupInformationStorage> mGroupInformationStorage;
    std::vector<datastore::EndpointGroupIDEntryStruct> mEndpointGroupIDEntries;
    std::vector<datastore::EndpointBindingEntryStruct> mEndpointBindingEntries;
    std::vector<datastore::NodeKeySetEntryStruct> mNodeKeySetEntries;
    struct NodeSyncQueue
    {
        bool inFlight = false;
        std::deque<std::function<CHIP_ERROR()>> waiting;
    };
    // Present only for nodes with an operation in flight.
    std::map<NodeId, NodeSyncQueue> mNodeSyncQueues;

    // Values whose removal failed with an unrecoverable status. RefreshNode drops such an entry, as the
    // specification requires, and would then adopt the node's copy as a new entry. A tombstoned value that
    // a later refresh finds on the node is added back as DeletePending and removed instead. A tombstone is
    // dropped when the node no longer holds its value, or when an add for the same value cancels it.
    struct AclTombstone
    {
        NodeId nodeId = kUndefinedNodeId;
        datastore::AccessControlEntryStruct value;
        std::optional<datastore::AccessControlEntryStruct> supersededValue;
    };
    std::vector<AclTombstone> mAclTombstones;
    // Generated types without span members, so they can be stored directly.
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> mBindingTombstones;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type> mEndpointGroupTombstones;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type> mNodeKeySetTombstones;

    std::vector<std::pair<NodeId, uint16_t>> mRefreshingNodeKeySetDeletions;
    size_t mRefreshingNodeKeySetDeletionIndex = 0;
    std::vector<datastore::ACLEntryStruct> mACLEntries;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type> mEndpointEntries;
    std::map<std::pair<NodeId, EndpointId>, std::vector<char>> mEndpointFriendlyNameStorage;

    Listener * mListeners = nullptr;

    friend class chip::JFAManager;

    CHIP_ERROR
    ForceAddGroup(const Clusters::JointFabricDatastore::Commands::AddGroup::DecodableType & commandData);

    CHIP_ERROR IsNodeIDInDatastore(NodeId nodeId, size_t & index);

    // Marks the node entries of `groupKeySetId` Pending and syncs the stored key set to their nodes.
    CHIP_ERROR UpdateNodeKeySetList(uint16_t groupKeySetId);
    CHIP_ERROR RemoveKeySet(uint16_t groupKeySetId);

    CHIP_ERROR IsGroupIDInDatastore(GroupId groupId, size_t & index);
    CHIP_ERROR IsNodeIdInNodeInformationEntries(NodeId nodeId, size_t & index);
    CHIP_ERROR IsNodeIdAndEndpointInEndpointInformationEntries(NodeId nodeId, EndpointId endpointId, size_t & index);

    CHIP_ERROR GenerateAndAssignAUniqueListID(uint16_t & listId);
    bool BindingMatches(const Clusters::JointFabricDatastore::Structs::DatastoreBindingTargetStruct::Type & binding1,
                        const Clusters::JointFabricDatastore::Structs::DatastoreBindingTargetStruct::Type & binding2);

    /**
     * Marks `entry` DeletePending and records that it is being removed from its node. A failed sync
     * leaves the entry CommitFailed, which does not say whether an add or a removal failed, and
     * RefreshNode re-writes recoverable CommitFailed entries. The recorded intent makes RefreshNode
     * retry the removal instead. Any add for the same entry clears it.
     */
    template <typename T>
    static void MarkRemovalRequested(T & entry)
    {
        entry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;
        entry.pendingRemoval    = true;
    }

    // Adding an entry that is being removed cancels the removal: the entry is added again.
    //
    // The sync revision changes, so that an add in flight from before the removal was requested does not commit the
    // entry. Updates made while the entry was being removed were not synced to it (UpdateKeySet and UpdateGroup skip
    // such entries), so the node may hold an older value than the entry. ACL entries have no sync revision: their syncs
    // compare the value sent with the entry's value, which UpdateGroup does not change while the entry is being removed.
    template <typename T>
    static void CancelRemoval(T & entry)
    {
        entry.pendingRemoval          = false;
        entry.statusEntry.state       = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;
        entry.statusEntry.failureCode = 0;
        if constexpr (!std::is_same_v<T, datastore::ACLEntryStruct>)
        {
            ++entry.syncRevision;
        }
    }

    // True if `entry` is DeletePending or has recorded removal intent.
    template <typename T>
    static bool HasRemovalIntent(const T & entry)
    {
        return entry.statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending ||
            entry.pendingRemoval;
    }

    // Called when the refresh triage drops an entry with an unrecoverable failure. If the entry was being
    // removed, records a tombstone for its value.
    void RecordTombstoneIfRemoving(const datastore::ACLEntryStruct & entry);
    void RecordTombstoneIfRemoving(const datastore::EndpointBindingEntryStruct & entry);
    void RecordTombstoneIfRemoving(const datastore::EndpointGroupIDEntryStruct & entry);
    void RecordTombstoneIfRemoving(const datastore::NodeKeySetEntryStruct & entry);

    // True if `tombstone` is for `value` on `nodeId`, as its value or as the value it superseded.
    static bool AclTombstoneMatches(const AclTombstone & tombstone, NodeId nodeId,
                                    const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::Type & value);

    // True if `entry` was in the active refresh's ACL or binding write.
    bool InRefreshWrite(const datastore::ACLEntryStruct & entry) const;
    bool InRefreshWrite(const datastore::EndpointBindingEntryStruct & entry) const;

    // The sync of `entry` alone, through StartAclEntrySync or StartBindingEntrySync, to start or queue with
    // RunOrQueueNodeSync. It captures only the entry's key, which it looks the entry up by when it starts.
    std::function<CHIP_ERROR()> EntrySyncStart(const datastore::ACLEntryStruct & entry);
    std::function<CHIP_ERROR()> EntrySyncStart(const datastore::EndpointBindingEntryStruct & entry);

    // Records a failed refresh write on `nodeId`'s entries that were in the write and not Committed, and
    // on its entries being removed, which keep their removal intent. An unrecoverable status is recorded only
    // on the write's single entry that was not Committed. Otherwise it is recorded as FAILURE, and each such
    // entry is synced on its own once the refresh ends.
    template <typename Entry>
    void MarkRefreshWriteFailed(std::vector<Entry> & entries, NodeId nodeId, CHIP_ERROR err);

    /**
     * The datastore runs at most one single-entry sync per node at a time. Single-entry ACL and binding syncs read the
     * node's whole list, edit it and write it back, and an add and a removal of the same group or key set must reach
     * the node in order. RefreshNode, whose own syncs do not queue, holds the node's slot until it finishes.
     *
     * Starts `start` now if `nodeId` has nothing in flight and returns its result; otherwise queues it and returns
     * CHIP_NO_ERROR, or BUSY if kMaxQueuedNodeSyncs operations already wait. `start` must end in exactly one
     * FinishNodeSync(nodeId), synchronously or from a completion.
     *
     * Commands check HasNodeSyncCapacity before changing any entry, so that a BUSY rejection leaves the datastore
     * unchanged and the commissioner's retry repeats the whole command.
     */
    CHIP_ERROR RunOrQueueNodeSync(NodeId nodeId, std::function<CHIP_ERROR()> start);
    void FinishNodeSync(NodeId nodeId);
    bool IsNodeSyncIdle(NodeId nodeId) const;
    bool HasNodeSyncCapacity(NodeId nodeId, size_t count = 1) const;
    bool HasNodeSyncCapacity(const std::map<NodeId, size_t> & countsPerNode) const;

    // Queued operations. Each looks its entry up by key when it starts, and syncs a removal if the entry is being
    // removed, or an add of its current value if it is not Committed. Each ends with FinishNodeSync(nodeId).
    CHIP_ERROR StartAclEntrySync(NodeId nodeId, uint16_t listId);
    CHIP_ERROR StartBindingEntrySync(NodeId nodeId, EndpointId endpointId, uint16_t listId);
    // An add is not synced while the entry for `requiredKeySetId` on the node is CommitFailed.
    CHIP_ERROR StartEndpointGroupEntrySync(NodeId nodeId, EndpointId endpointId, GroupId groupId,
                                           std::optional<uint16_t> requiredKeySetId);
    CHIP_ERROR StartNodeKeySetEntrySync(NodeId nodeId, uint16_t groupKeySetId);
    template <typename Wire, typename Entry, typename Match>
    CHIP_ERROR StartEntrySync(std::vector<Entry> & entries, NodeId nodeId, Match match);

    // Records that a stage of `nodeId`'s refresh failed, if that refresh is still active.
    void MarkRefreshFailed(NodeId nodeId, CHIP_ERROR err);

    // Ends the active refresh, if any, and resets all refresh state. The only place a refresh ends.
    void FinishRefresh(CHIP_ERROR err);

    // Defined here because ClearAllRecords calls it: Server.h reaches ClearAllRecords through OnFabricRemoved in builds
    // that define CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC but don't link JointFabricDatastore.cpp.
    void ResetRefreshState()
    {
        mRefreshingNodeId           = kUndefinedNodeId;
        mRefreshState               = kIdle;
        mRefreshingEndpointIndex    = 0;
        mRefreshingGroupKeySetIndex = 0;
        mRefreshHadFailure          = false;
        mRefreshingEndpointsList.clear();
        mRefreshingBindingEntries.clear();
        mRefreshingACLEntries.clear();
        mRefreshingGroupKeySetIDs.clear();
        mRefreshingNodeKeySetDeletions.clear();
        mRefreshingNodeKeySetDeletionIndex = 0;
    }

    // The result views the subject and target storage of `entry`, so it is valid only while `entry` is
    // unchanged.
    Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type
    EncodeAclEntryForSync(const datastore::ACLEntryStruct & entry) const;

    // Queues a sync of `groupKeySetId`'s entry on each of `nodeIds`, and returns the first error.
    CHIP_ERROR QueueNodeKeySetSyncs(const std::vector<NodeId> & nodeIds, uint16_t groupKeySetId);
    // Nodes with an endpoint in `groupId`.
    std::unordered_set<NodeId> NodesInGroup(GroupId groupId) const;
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

    // Changed by ClearAllRecords. Refresh and queued-sync completions capture it when their operation starts, and
    // return without touching the datastore if it has changed since.
    uint32_t mSyncGeneration = 0;

    NodeId mRefreshingNodeId           = kUndefinedNodeId;
    RefreshState mRefreshState         = kIdle;
    size_t mRefreshingEndpointIndex    = 0;
    size_t mRefreshingGroupKeySetIndex = 0;
    // Set when a stage of the active refresh fails without ending it; the node is then not marked Committed.
    bool mRefreshHadFailure = false;

    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type> mRefreshingEndpointsList;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> mRefreshingBindingEntries;
    std::vector<Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type> mRefreshingACLEntries;
    std::vector<uint16_t> mRefreshingGroupKeySetIDs;
};

} // namespace app
} // namespace chip
