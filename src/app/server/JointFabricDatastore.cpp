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

#include <app/server/JointFabricDatastore.h>

#include <lib/core/CASEAuthTag.h>
#include <protocols/interaction_model/StatusCode.h>

#include <algorithm>
#include <cstring>
#include <unordered_set>

namespace chip {
namespace app {

namespace {
/**
 * True if `key` is null or exactly as long as an epoch key. Nodes reject any other length with CONSTRAINT_ERROR, which
 * RefreshNode handles by dropping the node's entry for the key set.
 */
bool IsValidEpochKey(const DataModel::Nullable<ByteSpan> & key)
{
    using EpochKeyStorage = Crypto::SensitiveDataBuffer<Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES>;
    return key.IsNull() || key.Value().size() == EpochKeyStorage::Capacity();
}

/**
 * True for a CommitFailed entry whose failure RefreshNode handles by removing the entry: its FailureCode, an IM
 * Status Code, is CONSTRAINT_ERROR or RESOURCE_EXHAUSTED.
 */
bool IsUnrecoverableCommitFailure(const Clusters::JointFabricDatastore::Structs::DatastoreStatusEntryStruct::Type & statusEntry)
{
    if (statusEntry.state != Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitFailed)
    {
        return false;
    }
    const auto failureStatus = static_cast<Protocols::InteractionModel::Status>(statusEntry.failureCode);
    return failureStatus == Protocols::InteractionModel::Status::ConstraintError ||
        failureStatus == Protocols::InteractionModel::Status::ResourceExhausted;
}

/**
 * True if a command may give a group `groupCat`: null, or 1 to 65534 other than the Administrator and Anchor CATs, which
 * AddGroup and UpdateGroup reject with CONSTRAINT_ERROR (Matter Core R1.4.2 11.24.7.4, 11.24.7.5).
 */
bool IsAssignableGroupCat(const DataModel::Nullable<uint16_t> & groupCat)
{
    return groupCat.IsNull() ||
        (groupCat.Value() != 0 && groupCat.Value() != kAdminCATIdentifier && groupCat.Value() != kAnchorCATIdentifier);
}

template <typename T>
void PushTombstone(std::vector<T> & tombstones, T tombstone, size_t capacity)
{
    if (tombstones.size() >= capacity)
    {
        ChipLogError(AppServer, "Removal tombstone list full; discarding the oldest tombstone");
        tombstones.erase(tombstones.begin());
    }
    tombstones.push_back(std::move(tombstone));
}

/**
 * Returns a view of `value` in the cluster's type. The view is valid only while `value` is unchanged.
 */
Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::Type
EncodeAccessControlEntry(const datastore::AccessControlEntryStruct & value)
{
    Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::Type encoded;
    encoded.authMode  = value.authMode;
    encoded.privilege = value.privilege;
    encoded.subjects  = DataModel::List<const uint64_t>(value.subjects.data(), value.subjects.size());
    encoded.targets   = DataModel::List<const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type>(
        value.targets.data(), value.targets.size());
    return encoded;
}
} // namespace

CHIP_ERROR JointFabricDatastore::CopyGroupKeySetWithOwnedSpans(
    const Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & source,
    Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & destination)
{
    // Validate before mutating any storage so a non-conformant input leaves existing entries untouched.
    VerifyOrReturnError(IsValidEpochKey(source.epochKey0) && IsValidEpochKey(source.epochKey1) && IsValidEpochKey(source.epochKey2),
                        CHIP_IM_GLOBAL_STATUS(ConstraintError));

    auto & storage = mGroupKeySetStorage[source.groupKeySetID];

    destination.groupKeySetID          = source.groupKeySetID;
    destination.groupKeySecurityPolicy = source.groupKeySecurityPolicy;

    CopyByteSpanWithOwnedStorage(source.epochKey0, storage.epochKey0, destination.epochKey0);
    CopyByteSpanWithOwnedStorage(source.epochKey1, storage.epochKey1, destination.epochKey1);
    CopyByteSpanWithOwnedStorage(source.epochKey2, storage.epochKey2, destination.epochKey2);

    CopyNullableValue(source.epochStartTime0, destination.epochStartTime0);
    CopyNullableValue(source.epochStartTime1, destination.epochStartTime1);
    CopyNullableValue(source.epochStartTime2, destination.epochStartTime2);

    return CHIP_NO_ERROR;
}

void JointFabricDatastore::RemoveGroupKeySetStorage(uint16_t groupKeySetId)
{
    // The epoch-key buffers self-zeroize in their destructors when the entry is erased.
    mGroupKeySetStorage.erase(groupKeySetId);
}

void JointFabricDatastore::SetGroupInformationFriendlyNameWithOwnedStorage(
    GroupId groupId, const CharSpan & friendlyName,
    Clusters::JointFabricDatastore::Structs::DatastoreGroupInformationEntryStruct::Type & destination)
{
    auto & storage = mGroupInformationStorage[groupId];
    storage.friendlyName.assign(friendlyName.data(), friendlyName.data() + friendlyName.size());
    destination.friendlyName = CharSpan(storage.friendlyName.data(), storage.friendlyName.size());
}

void JointFabricDatastore::RemoveGroupInformationStorage(GroupId groupId)
{
    mGroupInformationStorage.erase(groupId);
}

CHIP_ERROR JointFabricDatastore::SetAdminEntryWithOwnedStorage(
    NodeId nodeId, const CharSpan & friendlyName, const ByteSpan & icac,
    Clusters::JointFabricDatastore::Structs::DatastoreAdministratorInformationEntryStruct::Type & destination)
{
    auto & storage = mAdminEntryStorage[nodeId];

    storage.friendlyName.assign(friendlyName.data(), friendlyName.data() + friendlyName.size());
    destination.friendlyName = CharSpan(storage.friendlyName.data(), storage.friendlyName.size());

    ReturnErrorOnFailure(storage.icac.SetLength(icac.size()));
    memcpy(storage.icac.Bytes(), icac.data(), icac.size());
    destination.icac = storage.icac.Span();

    return CHIP_NO_ERROR;
}

void JointFabricDatastore::RemoveAdminEntryStorage(NodeId nodeId)
{
    // The ICAC buffer self-zeroizes in its destructor when the entry is erased.
    mAdminEntryStorage.erase(nodeId);
}

void JointFabricDatastore::SetEndpointFriendlyNameWithOwnedStorage(
    NodeId nodeId, EndpointId endpointId, const CharSpan & friendlyName,
    Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type & destination)
{
    auto & storage = mEndpointFriendlyNameStorage[{ nodeId, endpointId }];
    storage.assign(friendlyName.data(), friendlyName.data() + friendlyName.size());
    destination.friendlyName = CharSpan(storage.data(), storage.size());
}

void JointFabricDatastore::RemoveEndpointFriendlyNameStorage(NodeId nodeId, EndpointId endpointId)
{
    mEndpointFriendlyNameStorage.erase({ nodeId, endpointId });
}

void JointFabricDatastore::CopyByteSpanWithOwnedStorage(const DataModel::Nullable<ByteSpan> & source, EpochKeyStorage & storage,
                                                        DataModel::Nullable<ByteSpan> & destination)
{
    // Epoch keys of the wrong length are rejected by CopyGroupKeySetWithOwnedSpans before reaching here, so the
    // SetLength below is expected to succeed; the failure branch remains as a defensive fallback only.
    if (!source.IsNull() && storage.SetLength(source.Value().size()) == CHIP_NO_ERROR)
    {
        memcpy(storage.Bytes(), source.Value().data(), source.Value().size());
        destination = storage.Span();
    }
    else
    {
        storage.Clear();
        destination.SetNull();
    }
}

void JointFabricDatastore::AddListener(Listener & listener)
{
    if (mListeners == nullptr)
    {
        mListeners     = &listener;
        listener.mNext = nullptr;
        return;
    }

    for (Listener * l = mListeners; /**/; l = l->mNext)
    {
        if (l == &listener)
        {
            return;
        }

        if (l->mNext == nullptr)
        {
            l->mNext       = &listener;
            listener.mNext = nullptr;
            return;
        }
    }
}

void JointFabricDatastore::RemoveListener(Listener & listener)
{
    if (mListeners == &listener)
    {
        mListeners     = listener.mNext;
        listener.mNext = nullptr;
        return;
    }

    for (Listener * l = mListeners; l != nullptr; l = l->mNext)
    {
        if (l->mNext == &listener)
        {
            l->mNext       = listener.mNext;
            listener.mNext = nullptr;
            return;
        }
    }
}

CHIP_ERROR JointFabricDatastore::AddPendingNode(NodeId nodeId, const CharSpan & friendlyName)
{
    VerifyOrReturnError(mNodeInformationEntries.size() < kMaxNodes, CHIP_ERROR_NO_MEMORY);
    // check that nodeId does not already exist
    VerifyOrReturnError(
        std::none_of(mNodeInformationEntries.begin(), mNodeInformationEntries.end(),
                     [nodeId](const GenericDatastoreNodeInformationEntry & entry) { return entry.nodeID == nodeId; }),
        CHIP_IM_GLOBAL_STATUS(ConstraintError));

    mNodeInformationEntries.push_back(GenericDatastoreNodeInformationEntry(
        nodeId, Clusters::JointFabricDatastore::DatastoreStateEnum::kPending, MakeOptional(friendlyName)));

    for (Listener * listener = mListeners; listener != nullptr; listener = listener->mNext)
    {
        listener->MarkNodeListChanged();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabricDatastore::UpdateNode(NodeId nodeId, const CharSpan & friendlyName)
{
    for (auto & entry : mNodeInformationEntries)
    {
        if (entry.nodeID == nodeId)
        {
            entry.Set(MakeOptional(friendlyName));

            for (Listener * listener = mListeners; listener != nullptr; listener = listener->mNext)
            {
                listener->MarkNodeListChanged();
            }

            return CHIP_NO_ERROR;
        }
    }

    return CHIP_IM_GLOBAL_STATUS(ConstraintError);
}

CHIP_ERROR JointFabricDatastore::RemoveNode(NodeId nodeId)
{
    for (auto it = mNodeInformationEntries.begin(); it != mNodeInformationEntries.end(); ++it)
    {
        if (it->nodeID == nodeId)
        {
            mNodeInformationEntries.erase(it);

            for (Listener * listener = mListeners; listener != nullptr; listener = listener->mNext)
            {
                listener->MarkNodeListChanged();
            }

            return CHIP_NO_ERROR;
        }
    }

    return CHIP_IM_GLOBAL_STATUS(ConstraintError);
}

bool JointFabricDatastore::AclTombstoneMatches(
    const AclTombstone & tombstone, NodeId nodeId,
    const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::Type & value)
{
    return tombstone.nodeId == nodeId &&
        (detail::AclEntryValueEquals(value, EncodeAccessControlEntry(tombstone.value)) ||
         (tombstone.supersededValue.has_value() &&
          detail::AclEntryValueEquals(value, EncodeAccessControlEntry(*tombstone.supersededValue))));
}

bool JointFabricDatastore::InRefreshWrite(const datastore::ACLEntryStruct & entry) const
{
    return std::any_of(mRefreshingACLEntries.begin(), mRefreshingACLEntries.end(),
                       [&entry](const auto & written) { return written.listID == entry.listID; });
}

bool JointFabricDatastore::InRefreshWrite(const datastore::EndpointBindingEntryStruct & entry) const
{
    return std::any_of(mRefreshingBindingEntries.begin(), mRefreshingBindingEntries.end(), [&entry](const auto & written) {
        return written.endpointID == entry.endpointID && written.listID == entry.listID;
    });
}

std::function<CHIP_ERROR()> JointFabricDatastore::EntrySyncStart(const datastore::ACLEntryStruct & entry)
{
    return [this, nodeId = entry.nodeID, listId = entry.listID]() { return StartAclEntrySync(nodeId, listId); };
}

std::function<CHIP_ERROR()> JointFabricDatastore::EntrySyncStart(const datastore::EndpointBindingEntryStruct & entry)
{
    return [this, nodeId = entry.nodeID, endpointId = entry.endpointID, listId = entry.listID]() {
        return StartBindingEntrySync(nodeId, endpointId, listId);
    };
}

template <typename Entry>
void JointFabricDatastore::MarkRefreshWriteFailed(std::vector<Entry> & entries, NodeId nodeId, CHIP_ERROR err)
{
    // The node reports one status for the whole list, and only an entry it did not hold yet can have caused the
    // failure. An unrecoverable status, on which RefreshNode drops the entry, is recorded only when the write held a
    // single such entry: recorded on several, it would drop all of them for the fault of one. They record FAILURE
    // instead and are synced on their own to get their own status. Entries being removed, which the write leaves
    // out, record FAILURE instead of an unrecoverable status too.
    Clusters::JointFabricDatastore::Structs::DatastoreStatusEntryStruct::Type writeStatus;
    writeStatus.state                = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitFailed;
    writeStatus.failureCode          = to_underlying(Protocols::InteractionModel::ClusterStatusCode(err).GetStatus());
    const bool unrecoverable         = IsUnrecoverableCommitFailure(writeStatus);
    const CHIP_ERROR unattributedErr = unrecoverable ? CHIP_IM_GLOBAL_STATUS(Failure) : err;

    auto uncommittedInWrite = [this, nodeId](const Entry & entry) {
        return entry.nodeID == nodeId && InRefreshWrite(entry) &&
            entry.statusEntry.state != Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
    };
    const bool attributable = std::count_if(entries.begin(), entries.end(), uncommittedInWrite) == 1;

    std::vector<std::function<CHIP_ERROR()>> entrySyncs;
    for (auto & entry : entries)
    {
        if (entry.nodeID != nodeId)
        {
            continue;
        }
        if (HasRemovalIntent(entry))
        {
            // Record the intent before the state stops saying DeletePending.
            MarkRemovalRequested(entry);
            detail::MarkEntrySyncFailed(entry, unattributedErr);
            continue;
        }
        if (!uncommittedInWrite(entry))
        {
            continue;
        }
        if (attributable || !unrecoverable)
        {
            detail::MarkEntrySyncFailed(entry, err);
            continue;
        }
        detail::MarkEntrySyncFailed(entry, unattributedErr);
        entrySyncs.push_back(EntrySyncStart(entry));
    }

    // The refresh holds the node's slot, so these syncs start once it ends.
    for (auto & start : entrySyncs)
    {
        if (!HasNodeSyncCapacity(nodeId))
        {
            ChipLogError(AppServer, "Sync queue of node 0x" ChipLogFormatX64 " full; its next refresh retries the entries",
                         ChipLogValueX64(nodeId));
            return;
        }
        LogErrorOnFailure(RunOrQueueNodeSync(nodeId, std::move(start)));
    }
}

CHIP_ERROR JointFabricDatastore::RefreshNode(NodeId nodeId)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mRefreshingNodeId == kUndefinedNodeId, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mRefreshState == kIdle, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsNodeSyncIdle(nodeId), CHIP_IM_GLOBAL_STATUS(Busy));

    mNodeSyncQueues[nodeId].inFlight = true;
    mRefreshingNodeId                = nodeId;

    CHIP_ERROR err = ContinueRefresh();
    if (err != CHIP_NO_ERROR)
    {
        FinishRefresh(err);
    }
    return err;
}

CHIP_ERROR JointFabricDatastore::ContinueRefresh()
{

    switch (mRefreshState)
    {
    case kIdle: {
        // 1. Confirm that a Node Information Entry exists for the given NodeID, and if not, return NOT_FOUND.
        // 2. Set the Node Information Entry's state to Pending.
        ReturnErrorOnFailure(SetNode(mRefreshingNodeId, Clusters::JointFabricDatastore::DatastoreStateEnum::kPending));

        // Request endpoints from the device's Descriptor cluster and transition to the
        // kRefreshingEndpoints state. The delegate call is asynchronous and will invoke
        // the provided callback when complete; the callback stores the received list
        // and calls ContinueRefresh() to advance the state machine.

        ReturnErrorOnFailure(mDelegate->FetchEndpointList(
            mRefreshingNodeId,
            [this, generation = mSyncGeneration](
                CHIP_ERROR err,
                const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type> & endpoints) {
                VerifyOrReturn(generation == mSyncGeneration);
                if (err == CHIP_NO_ERROR)
                {
                    // Store the fetched endpoints for processing in the next state.
                    mRefreshingEndpointsList = endpoints;

                    // Advance the state machine to process the endpoints.
                    mRefreshState = kRefreshingEndpoints;
                }
                else
                {
                    // Leave node as pending but tear down the refresh state.
                    FinishRefresh(err);
                    return;
                }

                // Continue the state machine (will enter kRefreshingEndpoints branch
                // when successful and process mRefreshingEndpointsList).
                CHIP_ERROR continueErr = ContinueRefresh();
                if (continueErr != CHIP_NO_ERROR)
                {
                    FinishRefresh(continueErr);
                }
            }));
    }
    break;
    case kRefreshingEndpoints: {
        // 3. cycle through mRefreshingEndpointsList and add them to the endpoint entries
        for (const auto & endpoint : mRefreshingEndpointsList)
        {
            auto it = std::find_if(
                mEndpointEntries.begin(), mEndpointEntries.end(),
                [this, &endpoint](const Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type & entry) {
                    return entry.nodeID == mRefreshingNodeId && entry.endpointID == endpoint.endpointID;
                });
            if (it == mEndpointEntries.end())
            {
                Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type newEntry;
                newEntry.endpointID = endpoint.endpointID;
                newEntry.nodeID     = mRefreshingNodeId;
                mEndpointEntries.push_back(newEntry);
            }
        }

        // TODO: sync friendly name between datastore entry and basic cluster

        // Remove EndpointEntries that are not in the mRefreshingEndpointsList
        mEndpointEntries.erase(
            std::remove_if(mEndpointEntries.begin(), mEndpointEntries.end(),
                           [&](const auto & entry) {
                               if (entry.nodeID != mRefreshingNodeId)
                               {
                                   return false;
                               }
                               const bool shouldRemove =
                                   std::none_of(mRefreshingEndpointsList.begin(), mRefreshingEndpointsList.end(),
                                                [&](const auto & endpoint) { return entry.endpointID == endpoint.endpointID; });
                               if (shouldRemove)
                               {
                                   RemoveEndpointFriendlyNameStorage(entry.nodeID, entry.endpointID);
                               }
                               return shouldRemove;
                           }),
            mEndpointEntries.end());

        if (std::none_of(mRefreshingEndpointsList.begin(), mRefreshingEndpointsList.end(),
                         [](const auto & endpoint) { return endpoint.endpointID == kRootEndpointId; }))
        {
            Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type rootEndpoint;
            rootEndpoint.nodeID     = mRefreshingNodeId;
            rootEndpoint.endpointID = kRootEndpointId;
            mRefreshingEndpointsList.push_back(rootEndpoint);
        }

        // Start fetching groups from the first endpoint
        mRefreshingEndpointIndex = 0;
        mRefreshState            = kRefreshingGroups;

        // Fall through to kRefreshingGroups to start fetching
        return ContinueRefresh();
    }
    break;

    case kRefreshingGroups: {
        // Check if we still have endpoints to process for group fetching
        if (mRefreshingEndpointIndex < mRefreshingEndpointsList.size())
        {
            // Fetch group list for the current endpoint
            EndpointId currentEndpointId = mRefreshingEndpointsList[mRefreshingEndpointIndex].endpointID;

            ReturnErrorOnFailure(mDelegate->FetchEndpointGroupList(
                mRefreshingNodeId, currentEndpointId,
                [this, currentEndpointId, generation = mSyncGeneration](
                    CHIP_ERROR err,
                    const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreGroupInformationEntryStruct::Type> &
                        endpointGroups) {
                    VerifyOrReturn(generation == mSyncGeneration);
                    if (err == CHIP_NO_ERROR)
                    {
                        // Convert endpointGroups to mEndpointGroupIDEntries for this specific endpoint
                        for (const auto & endpointGroup : endpointGroups)
                        {
                            auto it = std::find_if(
                                mEndpointGroupIDEntries.begin(), mEndpointGroupIDEntries.end(),
                                [this, currentEndpointId, &endpointGroup](
                                    const Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type &
                                        entry) {
                                    return entry.nodeID == mRefreshingNodeId && entry.endpointID == currentEndpointId &&
                                        entry.groupID == endpointGroup.groupID;
                                });

                            if (it == mEndpointGroupIDEntries.end())
                            {
                                datastore::EndpointGroupIDEntryStruct newEntry;
                                newEntry.nodeID            = mRefreshingNodeId;
                                newEntry.endpointID        = currentEndpointId;
                                newEntry.groupID           = static_cast<GroupId>(endpointGroup.groupID);
                                newEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;

                                // A value whose removal failed unrecoverably is added back to be removed.
                                auto tombstone = std::find_if(
                                    mEndpointGroupTombstones.begin(), mEndpointGroupTombstones.end(), [&newEntry](const auto & t) {
                                        return t.nodeID == newEntry.nodeID && t.endpointID == newEntry.endpointID &&
                                            t.groupID == newEntry.groupID;
                                    });
                                if (tombstone != mEndpointGroupTombstones.end())
                                {
                                    mEndpointGroupTombstones.erase(tombstone);
                                    MarkRemovalRequested(newEntry);
                                }
                                mEndpointGroupIDEntries.push_back(newEntry);
                            }
                        }

                        // Remove entries not in endpointGroups for this specific endpoint
                        mEndpointGroupIDEntries.erase(
                            std::remove_if(mEndpointGroupIDEntries.begin(), mEndpointGroupIDEntries.end(),
                                           [&, currentEndpointId](const auto & entry) {
                                               if (entry.nodeID != mRefreshingNodeId || entry.endpointID != currentEndpointId)
                                               {
                                                   return false;
                                               }
                                               if (entry.statusEntry.state !=
                                                       Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted &&
                                                   !HasRemovalIntent(entry))
                                               {
                                                   return false;
                                               }
                                               return std::none_of(endpointGroups.begin(), endpointGroups.end(),
                                                                   [&](const auto & eg) { return entry.groupID == eg.groupID; });
                                           }),
                            mEndpointGroupIDEntries.end());

                        // The node no longer holds the values of the remaining tombstones for this endpoint.
                        mEndpointGroupTombstones.erase(
                            std::remove_if(mEndpointGroupTombstones.begin(), mEndpointGroupTombstones.end(),
                                           [this, currentEndpointId](const auto & t) {
                                               return t.nodeID == mRefreshingNodeId && t.endpointID == currentEndpointId;
                                           }),
                            mEndpointGroupTombstones.end());
                    }

                    else
                    {
                        // Endpoints without the Groups cluster also fail here; the merge is skipped.
                        ChipLogProgress(AppServer, "Skipping groups of endpoint %u during refresh: %" CHIP_ERROR_FORMAT,
                                        currentEndpointId, err.Format());
                    }

                    // Move to the next endpoint
                    mRefreshingEndpointIndex++;

                    // Continue to process next endpoint or move to syncing phase
                    CHIP_ERROR continueErr = ContinueRefresh();
                    if (continueErr != CHIP_NO_ERROR)
                    {
                        FinishRefresh(continueErr);
                    }
                }));

            // Return here - the callback will call ContinueRefresh() again
            return CHIP_NO_ERROR;
        }

        // All endpoints processed. Collect this node's work before issuing any SyncNode: a completion that
        // runs synchronously can erase entries.
        std::vector<datastore::EndpointGroupIDEntryStruct> groupEntriesToSync;
        for (auto it = mEndpointGroupIDEntries.begin(); it != mEndpointGroupIDEntries.end();)
        {
            if (it->nodeID != mRefreshingNodeId)
            {
                ++it;
                continue;
            }

            if (IsUnrecoverableCommitFailure(it->statusEntry))
            {
                // remove entry from the list
                RecordTombstoneIfRemoving(*it);
                it = mEndpointGroupIDEntries.erase(it);
                continue;
            }

            if (HasRemovalIntent(*it))
            {
                // Retry the removal, including one that failed earlier.
                MarkRemovalRequested(*it);
                groupEntriesToSync.push_back(*it);
            }
            else if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kPending)
            {
                groupEntriesToSync.push_back(*it);
            }
            else if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitFailed)
            {
                // Retry the failed add.
                auto entryToSync              = *it;
                entryToSync.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;
                groupEntriesToSync.push_back(entryToSync);
            }

            ++it;
        }

        for (const auto & entryToSync : groupEntriesToSync)
        {
            const bool removal =
                entryToSync.statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;
            auto match = [nodeId = entryToSync.nodeID, endpointId = entryToSync.endpointID, groupId = entryToSync.groupID](
                             const auto & e) { return e.nodeID == nodeId && e.endpointID == endpointId && e.groupID == groupId; };
            // The refresh does not wait for its group syncs, so an entry can be marked for another sync before this one
            // completes. The result applies only if the entry is still being added or removed as when the sync started, and
            // its revision is unchanged.
            auto sameSync = [match, removal, revision = entryToSync.syncRevision](const auto & e) {
                return match(e) && HasRemovalIntent(e) == removal && e.syncRevision == revision;
            };
            CHIP_ERROR syncErr = mDelegate->SyncNode(
                mRefreshingNodeId, entryToSync,
                [this, entryToSync, removal, sameSync, generation = mSyncGeneration](CHIP_ERROR innerErr) {
                    VerifyOrReturn(generation == mSyncGeneration);
                    if (innerErr != CHIP_NO_ERROR)
                    {
                        detail::MarkEntrySyncFailedIfFound(mEndpointGroupIDEntries, sameSync, innerErr);
                        MarkRefreshFailed(entryToSync.nodeID, innerErr);
                        return;
                    }
                    if (removal)
                    {
                        auto erased = std::find_if(mEndpointGroupIDEntries.begin(), mEndpointGroupIDEntries.end(), sameSync);
                        if (erased != mEndpointGroupIDEntries.end())
                        {
                            mEndpointGroupIDEntries.erase(erased);
                        }
                        return;
                    }
                    detail::MarkEntryCommittedIfFound(mEndpointGroupIDEntries, sameSync);
                });
            if (syncErr != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer,
                             "Failed syncing group entry during refresh for node 0x" ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                             ChipLogValueX64(mRefreshingNodeId), syncErr.Format());
                detail::MarkEntrySyncFailedIfFound(mEndpointGroupIDEntries, match, syncErr);
                mRefreshHadFailure = true;
            }
        }

        // Start fetching groups from the first endpoint
        mRefreshingEndpointIndex = 0;
        mRefreshState            = kRefreshingBindings;

        // Fall through to kRefreshingGroups to start fetching
        return ContinueRefresh();
    }
    break;

    case kRefreshingBindings: {
        mRefreshingBindingEntries.clear();

        // Check if we still have endpoints to process for group fetching
        if (mRefreshingEndpointIndex < mRefreshingEndpointsList.size())
        {
            // Fetch group list for the current endpoint
            EndpointId currentEndpointId = mRefreshingEndpointsList[mRefreshingEndpointIndex].endpointID;

            ReturnErrorOnFailure(mDelegate->FetchEndpointBindingList(
                mRefreshingNodeId, currentEndpointId,
                [this, currentEndpointId, generation = mSyncGeneration](
                    CHIP_ERROR err,
                    const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> &
                        endpointBindings) {
                    VerifyOrReturn(generation == mSyncGeneration);
                    if (err == CHIP_NO_ERROR)
                    {
                        // Convert endpointBindings to mEndpointBindingEntries
                        for (const auto & endpointBinding : endpointBindings)
                        {
                            auto it = std::find_if(
                                mEndpointBindingEntries.begin(), mEndpointBindingEntries.end(),
                                [this, &endpointBinding](
                                    const Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type &
                                        entry) {
                                    return entry.nodeID == mRefreshingNodeId && entry.endpointID == endpointBinding.endpointID &&
                                        BindingMatches(entry.binding, endpointBinding.binding);
                                });

                            if (it == mEndpointBindingEntries.end())
                            {
                                datastore::EndpointBindingEntryStruct newEntry;
                                newEntry.nodeID     = mRefreshingNodeId;
                                newEntry.endpointID = endpointBinding.endpointID;
                                newEntry.binding    = endpointBinding.binding;
                                if (GenerateAndAssignAUniqueListID(newEntry.listID) != CHIP_NO_ERROR)
                                {
                                    // Unable to generate a unique List ID; skip this entry.
                                    continue;
                                }
                                newEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;

                                // A value whose removal failed unrecoverably is added back to be removed.
                                auto tombstone =
                                    std::find_if(mBindingTombstones.begin(), mBindingTombstones.end(), [&newEntry](const auto & t) {
                                        return t.nodeID == newEntry.nodeID && detail::BindingEntryValueEquals(t, newEntry);
                                    });
                                if (tombstone != mBindingTombstones.end())
                                {
                                    mBindingTombstones.erase(tombstone);
                                    MarkRemovalRequested(newEntry);
                                }
                                mEndpointBindingEntries.push_back(newEntry);
                            }
                        }

                        // Remove entries not in endpointBindings, but only if they are Committed or being removed
                        mEndpointBindingEntries.erase(
                            std::remove_if(
                                mEndpointBindingEntries.begin(), mEndpointBindingEntries.end(),
                                [&](const auto & entry) {
                                    if (entry.nodeID != mRefreshingNodeId || entry.endpointID != currentEndpointId)
                                    {
                                        return false;
                                    }
                                    if (entry.statusEntry.state != Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted &&
                                        !HasRemovalIntent(entry))
                                    {
                                        return false;
                                    }
                                    return std::none_of(endpointBindings.begin(), endpointBindings.end(), [&](const auto & eb) {
                                        return entry.endpointID == eb.endpointID && BindingMatches(entry.binding, eb.binding);
                                    });
                                }),
                            mEndpointBindingEntries.end());

                        // The node no longer holds the values of the remaining tombstones for this endpoint.
                        mBindingTombstones.erase(std::remove_if(mBindingTombstones.begin(), mBindingTombstones.end(),
                                                                [this, currentEndpointId](const auto & t) {
                                                                    return t.nodeID == mRefreshingNodeId &&
                                                                        t.endpointID == currentEndpointId;
                                                                }),
                                                 mBindingTombstones.end());
                    }

                    else
                    {
                        // Endpoints without the Binding cluster also fail here; the merge is skipped.
                        ChipLogProgress(AppServer, "Skipping bindings of endpoint %u during refresh: %" CHIP_ERROR_FORMAT,
                                        currentEndpointId, err.Format());
                    }

                    // Move to the next endpoint
                    mRefreshingEndpointIndex++;

                    // Continue the state machine to let the kRefreshingBindings branch process mEndpointBindingList.
                    CHIP_ERROR continueErr = ContinueRefresh();
                    if (continueErr != CHIP_NO_ERROR)
                    {
                        FinishRefresh(continueErr);
                    }
                }));

            // Return here - the callback will call ContinueRefresh() again
            return CHIP_NO_ERROR;
        }

        for (auto it = mEndpointBindingEntries.begin(); it != mEndpointBindingEntries.end();)
        {
            if (it->nodeID != mRefreshingNodeId)
            {
                ++it;
                continue;
            }

            if (IsUnrecoverableCommitFailure(it->statusEntry))
            {
                // remove entry from the list
                RecordTombstoneIfRemoving(*it);
                it = mEndpointBindingEntries.erase(it);
                continue;
            }

            // Entries being removed are left out of the write, and erased once it succeeds. Recoverable
            // CommitFailed entries are retried.
            if (!HasRemovalIntent(*it))
            {
                mRefreshingBindingEntries.push_back(*it);
            }

            ++it;
        }

        const NodeId refreshingNodeId = mRefreshingNodeId;
        CHIP_ERROR bindingSyncErr     = mDelegate->SyncNode(
            mRefreshingNodeId, mRefreshingBindingEntries,
            [this, refreshingNodeId, generation = mSyncGeneration](CHIP_ERROR syncErr) {
                VerifyOrReturn(generation == mSyncGeneration);
                if (syncErr != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer,
                                     "Failed syncing bindings during refresh for node 0x" ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                                     ChipLogValueX64(refreshingNodeId), syncErr.Format());
                    MarkRefreshWriteFailed(mEndpointBindingEntries, refreshingNodeId, syncErr);
                    MarkRefreshFailed(refreshingNodeId, syncErr);
                }
                else
                {
                    for (auto & entry : mEndpointBindingEntries)
                    {
                        if (entry.nodeID != refreshingNodeId ||
                            (entry.statusEntry.state != Clusters::JointFabricDatastore::DatastoreStateEnum::kPending &&
                             entry.statusEntry.state != Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitFailed))
                        {
                            continue;
                        }
                        if (InRefreshWrite(entry))
                        {
                            entry.statusEntry.state       = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                            entry.statusEntry.failureCode = 0;
                        }
                    }

                    // The node no longer holds the entries being removed that the write left out. A removal
                    // requested while the write was in flight is still queued and runs after the refresh.
                    mEndpointBindingEntries.erase(std::remove_if(mEndpointBindingEntries.begin(), mEndpointBindingEntries.end(),
                                                                     [this, refreshingNodeId](const auto & entry) {
                                                                     return entry.nodeID == refreshingNodeId &&
                                                                         HasRemovalIntent(entry) && !InRefreshWrite(entry);
                                                                 }),
                                                      mEndpointBindingEntries.end());
                }

                if (mRefreshingNodeId != refreshingNodeId)
                {
                    return;
                }

                // A failed binding write does not end the refresh: the remaining stages still run.
                mRefreshState          = kFetchingGroupKeySetList;
                CHIP_ERROR continueErr = ContinueRefresh();
                if (continueErr != CHIP_NO_ERROR)
                {
                    FinishRefresh(continueErr);
                }
            });
        if (bindingSyncErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed syncing bindings during refresh for node 0x" ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                         ChipLogValueX64(refreshingNodeId), bindingSyncErr.Format());
            MarkRefreshWriteFailed(mEndpointBindingEntries, refreshingNodeId, bindingSyncErr);
            mRefreshHadFailure = true;
            mRefreshState      = kFetchingGroupKeySetList;
            return ContinueRefresh();
        }
    }
    break;
    case kFetchingGroupKeySetList: {
        ReturnErrorOnFailure(mDelegate->FetchGroupKeySetList(
            mRefreshingNodeId, [this, generation = mSyncGeneration](CHIP_ERROR err, const std::vector<uint16_t> & groupKeySets) {
                VerifyOrReturn(generation == mSyncGeneration);
                if (err == CHIP_NO_ERROR)
                {
                    // Store the fetched group key sets for processing in the next state.
                    mRefreshingGroupKeySetIDs = groupKeySets;

                    // Key sets whose removal failed unrecoverably and that the node still holds are added back to
                    // be removed. The node no longer holds the others, so their tombstones are dropped.
                    for (auto tombstone = mNodeKeySetTombstones.begin(); tombstone != mNodeKeySetTombstones.end();)
                    {
                        if (tombstone->nodeID != mRefreshingNodeId)
                        {
                            ++tombstone;
                            continue;
                        }
                        const bool nodeHoldsKeySet =
                            std::find(groupKeySets.begin(), groupKeySets.end(), tombstone->groupKeySetID) != groupKeySets.end();
                        const bool tracked =
                            std::any_of(mNodeKeySetEntries.begin(), mNodeKeySetEntries.end(), [&tombstone](const auto & entry) {
                                return entry.nodeID == tombstone->nodeID && entry.groupKeySetID == tombstone->groupKeySetID;
                            });
                        if (nodeHoldsKeySet && !tracked)
                        {
                            datastore::NodeKeySetEntryStruct entry{ *tombstone };
                            entry.statusEntry.failureCode = 0;
                            MarkRemovalRequested(entry);
                            mNodeKeySetEntries.push_back(entry);
                        }
                        tombstone = mNodeKeySetTombstones.erase(tombstone);
                    }

                    // Advance the state machine to process the group key sets.
                    mRefreshState               = kFetchingGroupKeySets;
                    mRefreshingGroupKeySetIndex = 0;
                }
                else
                {
                    // Leave node as pending but tear down the refresh state.
                    FinishRefresh(err);
                    return;
                }

                // Continue the state machine to let the kFetchingGroupKeySets branch process mRefreshingGroupKeySetIDs.
                CHIP_ERROR continueErr = ContinueRefresh();
                if (continueErr != CHIP_NO_ERROR)
                {
                    FinishRefresh(continueErr);
                }
            }));
    }
    break;
    case kFetchingGroupKeySets: {
        // Request each Group Key Set from the device and transition to kRefreshingGroupKeySets once all indices are read.
        if (mRefreshingGroupKeySetIndex < mRefreshingGroupKeySetIDs.size())
        {
            const uint16_t groupKeySetID = mRefreshingGroupKeySetIDs[mRefreshingGroupKeySetIndex];

            return mDelegate->FetchGroupKeySet(
                mRefreshingNodeId, groupKeySetID,
                [this, generation = mSyncGeneration](
                    CHIP_ERROR err, const Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet) {
                    VerifyOrReturn(generation == mSyncGeneration);
                    if (err == CHIP_NO_ERROR)
                    {
                        auto it = std::find_if(
                            mGroupKeySetList.begin(), mGroupKeySetList.end(),
                            [&groupKeySet](
                                const Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & entry) {
                                return entry.groupKeySetID == groupKeySet.groupKeySetID;
                            });

                        if (it == mGroupKeySetList.end())
                        {
                            Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type copiedKeySet;
                            LogErrorOnFailure(CopyGroupKeySetWithOwnedSpans(groupKeySet, copiedKeySet));
                            mGroupKeySetList.push_back(copiedKeySet);
                        }
                        else
                        {
                            // Update existing entry
                            LogErrorOnFailure(CopyGroupKeySetWithOwnedSpans(groupKeySet, *it));
                        }

                        ++mRefreshingGroupKeySetIndex;
                    }
                    else
                    {
                        // Leave node as pending but tear down the refresh state.
                        FinishRefresh(err);
                        return;
                    }

                    // Continue fetching key sets until complete, then process mGroupKeySetList in kRefreshingGroupKeySets.
                    CHIP_ERROR continueErr = ContinueRefresh();
                    if (continueErr != CHIP_NO_ERROR)
                    {
                        FinishRefresh(continueErr);
                    }
                });
        }

        // No group key sets to fetch; advance the state machine to process group key sets (which will be empty) and sync to
        // nodes.
        mRefreshState = kRefreshingGroupKeySets;
        return ContinueRefresh();
    }
    break;
    case kRefreshingGroupKeySets: {
        // 4. Ensure the node's key-set entries are synced to it. Other nodes' entries are synced by their own refreshes
        // and through their sync queues, which keep a node's key-set writes and removals in order.
        if (mRefreshingNodeKeySetDeletions.empty())
        {
            for (const auto & groupKeySet : mGroupKeySetList)
            {
                for (auto nkIt = mNodeKeySetEntries.begin(); nkIt != mNodeKeySetEntries.end();)
                {
                    if (nkIt->nodeID != mRefreshingNodeId || nkIt->groupKeySetID != groupKeySet.groupKeySetID ||
                        !HasRemovalIntent(*nkIt))
                    {
                        ++nkIt;
                        continue;
                    }

                    if (IsUnrecoverableCommitFailure(nkIt->statusEntry))
                    {
                        // remove entry from the list
                        RecordTombstoneIfRemoving(*nkIt);
                        nkIt = mNodeKeySetEntries.erase(nkIt);
                        continue;
                    }

                    // Retry the removal, including one that failed earlier.
                    MarkRemovalRequested(*nkIt);
                    mRefreshingNodeKeySetDeletions.emplace_back(nkIt->nodeID, nkIt->groupKeySetID);
                    ++nkIt;
                }
            }
        }

        if (mRefreshingNodeKeySetDeletionIndex < mRefreshingNodeKeySetDeletions.size())
        {
            // Structured bindings cannot be captured by lambdas prior to C++20.
            const auto & nodeKeySetDeletion = mRefreshingNodeKeySetDeletions[mRefreshingNodeKeySetDeletionIndex];
            NodeId nodeIdToErase            = nodeKeySetDeletion.first;
            uint16_t groupKeySetIdToErase   = nodeKeySetDeletion.second;

            auto stillRemoving = [nodeIdToErase, groupKeySetIdToErase](const auto & entry) {
                return entry.nodeID == nodeIdToErase && entry.groupKeySetID == groupKeySetIdToErase && HasRemovalIntent(entry);
            };
            // An add can cancel a collected removal while an earlier one is in flight. The add is queued behind the
            // refresh, and sending the removal would undo it on the node.
            if (std::none_of(mNodeKeySetEntries.begin(), mNodeKeySetEntries.end(), stillRemoving))
            {
                ++mRefreshingNodeKeySetDeletionIndex;
                return ContinueRefresh();
            }

            Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type entryToRemove;
            entryToRemove.nodeID            = nodeIdToErase;
            entryToRemove.groupKeySetID     = groupKeySetIdToErase;
            entryToRemove.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;
            CHIP_ERROR syncErr =
                mDelegate->SyncNode(nodeIdToErase, entryToRemove,
                                    [this, nodeIdToErase, stillRemoving, generation = mSyncGeneration](CHIP_ERROR innerErr) {
                                        VerifyOrReturn(generation == mSyncGeneration);
                                        // An add that cancelled the removal while it was in flight owns the entry now.
                                        if (innerErr == CHIP_NO_ERROR)
                                        {
                                            auto erased =
                                                std::find_if(mNodeKeySetEntries.begin(), mNodeKeySetEntries.end(), stillRemoving);
                                            if (erased != mNodeKeySetEntries.end())
                                            {
                                                mNodeKeySetEntries.erase(erased);
                                            }
                                        }
                                        else
                                        {
                                            detail::MarkEntrySyncFailedIfFound(mNodeKeySetEntries, stillRemoving, innerErr);
                                            MarkRefreshFailed(nodeIdToErase, innerErr);
                                        }

                                        ++mRefreshingNodeKeySetDeletionIndex;
                                        CHIP_ERROR continueErr = ContinueRefresh();
                                        if (continueErr != CHIP_NO_ERROR)
                                        {
                                            FinishRefresh(continueErr);
                                        }
                                    });
            if (syncErr != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer,
                             "Failed deleting group key set during refresh for node 0x" ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                             ChipLogValueX64(mRefreshingNodeId), syncErr.Format());
                detail::MarkEntrySyncFailedIfFound(
                    mNodeKeySetEntries,
                    [&](const auto & e) { return e.nodeID == nodeIdToErase && e.groupKeySetID == groupKeySetIdToErase; }, syncErr);
                mRefreshHadFailure = true;
                ++mRefreshingNodeKeySetDeletionIndex;
                return ContinueRefresh();
            }

            return CHIP_NO_ERROR;
        }

        // The refresh does not wait for its key set writes, so an entry can be marked for another sync before its write
        // completes. The write's result applies only if the entry is still being added with the value written.
        auto keySetWriteCompletion = [this](NodeId entryNodeId, uint16_t groupKeySetId, uint32_t revision) {
            return [this, entryNodeId, groupKeySetId, revision, generation = mSyncGeneration](CHIP_ERROR innerErr) {
                VerifyOrReturn(generation == mSyncGeneration);
                auto sameWrite = [&](const auto & e) {
                    return e.nodeID == entryNodeId && e.groupKeySetID == groupKeySetId && !HasRemovalIntent(e) &&
                        e.syncRevision == revision;
                };
                if (innerErr != CHIP_NO_ERROR)
                {
                    detail::MarkEntrySyncFailedIfFound(mNodeKeySetEntries, sameWrite, innerErr);
                    MarkRefreshFailed(entryNodeId, innerErr);
                    return;
                }
                detail::MarkEntryCommittedIfFound(mNodeKeySetEntries, sameWrite);
            };
        };

        for (auto gksIt = mGroupKeySetList.begin(); gksIt != mGroupKeySetList.end(); ++gksIt)
        {
            const uint16_t groupKeySetId = gksIt->groupKeySetID;

            for (auto nkIt = mNodeKeySetEntries.begin(); nkIt != mNodeKeySetEntries.end();)
            {
                if (nkIt->nodeID != mRefreshingNodeId || nkIt->groupKeySetID != groupKeySetId)
                {
                    ++nkIt;
                    continue;
                }

                // nkIt references the current groupKeySetId
                if (nkIt->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kPending)
                {
                    // Make a copy of the group key set to send to the node.
                    auto groupKeySet   = *gksIt;
                    CHIP_ERROR syncErr = mDelegate->SyncNode(
                        nkIt->nodeID, groupKeySet, keySetWriteCompletion(nkIt->nodeID, groupKeySetId, nkIt->syncRevision));
                    if (syncErr != CHIP_NO_ERROR)
                    {
                        detail::MarkEntrySyncFailed(*nkIt, syncErr);
                        ChipLogError(AppServer,
                                     "Failed syncing group key set during refresh for node 0x" ChipLogFormatX64
                                     ": %" CHIP_ERROR_FORMAT,
                                     ChipLogValueX64(mRefreshingNodeId), syncErr.Format());
                        mRefreshHadFailure = true;
                    }
                    ++nkIt;
                }
                else if (nkIt->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitFailed)
                {
                    if (IsUnrecoverableCommitFailure(nkIt->statusEntry))
                    {
                        // remove entry from the list
                        RecordTombstoneIfRemoving(*nkIt);
                        nkIt = mNodeKeySetEntries.erase(nkIt);
                    }
                    else if (HasRemovalIntent(*nkIt))
                    {
                        // A failed removal is not re-written to the node.
                        ++nkIt;
                    }
                    else
                    {
                        // Retry the failed commit by attempting to SyncNode again.
                        auto groupKeySet   = *gksIt;
                        CHIP_ERROR syncErr = mDelegate->SyncNode(
                            nkIt->nodeID, groupKeySet, keySetWriteCompletion(nkIt->nodeID, groupKeySetId, nkIt->syncRevision));
                        if (syncErr != CHIP_NO_ERROR)
                        {
                            detail::MarkEntrySyncFailed(*nkIt, syncErr);
                            ChipLogError(AppServer,
                                         "Failed retrying group key set during refresh for node 0x" ChipLogFormatX64
                                         ": %" CHIP_ERROR_FORMAT,
                                         ChipLogValueX64(mRefreshingNodeId), syncErr.Format());
                            mRefreshHadFailure = true;
                        }
                        ++nkIt;
                    }
                }
                else
                {
                    ++nkIt;
                }
            }
        }

        mRefreshingNodeKeySetDeletions.clear();
        mRefreshingNodeKeySetDeletionIndex = 0;

        // Request ACL List from the device and transition to kRefreshingACLs.
        ReturnErrorOnFailure(mDelegate->FetchACLList(
            mRefreshingNodeId,
            [this, generation = mSyncGeneration](
                CHIP_ERROR err, const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type> & acls) {
                VerifyOrReturn(generation == mSyncGeneration);
                if (err == CHIP_NO_ERROR)
                {
                    // The kRefreshingACLs triage drops the node's entries whose sync failed for good, other than updates
                    // whose old value the node still holds, which the merge below restores. The merge counts their room
                    // as free: otherwise a full ACL list would end every refresh of the node before that triage.
                    size_t droppedEntries = 0;
                    for (const auto & entry : mACLEntries)
                    {
                        if (entry.nodeID != mRefreshingNodeId || !IsUnrecoverableCommitFailure(entry.statusEntry))
                        {
                            continue;
                        }
                        const bool restored = !HasRemovalIntent(entry) && entry.supersededValue.has_value() &&
                            std::any_of(acls.begin(), acls.end(), [&entry](const auto & acl) {
                                return detail::AclEntryValueEquals(acl.ACLEntry, EncodeAccessControlEntry(*entry.supersededValue));
                            });
                        if (!restored)
                        {
                            ++droppedEntries;
                        }
                    }
                    auto hasRoom = [this, droppedEntries]() { return mACLEntries.size() - droppedEntries < kMaxACLs; };

                    // A node's ACL entries carry no listID, so fetched entries are matched to datastore
                    // entries by value.
                    std::vector<uint16_t> seenListIds;
                    size_t notAdoptedCount = 0;
                    // Fetched values with a tombstone that are not added back for lack of room.
                    std::vector<const Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type *>
                        tombstonedValuesLeftOut;
                    for (const auto & acl : acls)
                    {
                        bool matched = false;
                        for (const auto & entry : mACLEntries)
                        {
                            if (entry.nodeID == mRefreshingNodeId &&
                                detail::AclEntryValueEquals(acl.ACLEntry, EncodeAclEntryForSync(entry).ACLEntry))
                            {
                                seenListIds.push_back(entry.listID);
                                matched = true;
                            }
                        }
                        if (matched)
                        {
                            continue;
                        }

                        // A value whose removal failed unrecoverably: added back to be removed.
                        auto tombstone = std::find_if(mAclTombstones.begin(), mAclTombstones.end(), [this, &acl](const auto & t) {
                            return AclTombstoneMatches(t, mRefreshingNodeId, acl.ACLEntry);
                        });
                        if (tombstone != mAclTombstones.end())
                        {
                            if (!hasRoom())
                            {
                                // The ACL write replaces the node's whole list, so leaving the value out of it removes
                                // the value too. The tombstone is kept until the node no longer holds the value.
                                tombstonedValuesLeftOut.push_back(&acl);
                                continue;
                            }
                            datastore::ACLEntryStruct readded;
                            if (GenerateAndAssignAUniqueListID(readded.listID) != CHIP_NO_ERROR)
                            {
                                continue;
                            }
                            readded.nodeID             = mRefreshingNodeId;
                            readded.ACLEntry.authMode  = acl.ACLEntry.authMode;
                            readded.ACLEntry.privilege = acl.ACLEntry.privilege;
                            if (!acl.ACLEntry.subjects.IsNull())
                            {
                                readded.ACLEntry.subjects.assign(acl.ACLEntry.subjects.Value().begin(),
                                                                 acl.ACLEntry.subjects.Value().end());
                            }
                            if (!acl.ACLEntry.targets.IsNull())
                            {
                                readded.ACLEntry.targets.assign(acl.ACLEntry.targets.Value().begin(),
                                                                acl.ACLEntry.targets.Value().end());
                            }
                            MarkRemovalRequested(readded);
                            mAclTombstones.erase(tombstone);
                            seenListIds.push_back(readded.listID);
                            mACLEntries.push_back(std::move(readded));
                            continue;
                        }

                        // The value an entry had before its update: the node has not applied the update. Not
                        // adopted, so the write replaces it. The entry is seen: if it is being removed, the
                        // removal has not taken effect.
                        bool superseded = false;
                        for (auto & entry : mACLEntries)
                        {
                            if (entry.nodeID != mRefreshingNodeId || !entry.supersededValue.has_value() ||
                                !detail::AclEntryValueEquals(acl.ACLEntry, EncodeAccessControlEntry(*entry.supersededValue)))
                            {
                                continue;
                            }
                            superseded = true;
                            seenListIds.push_back(entry.listID);
                            if (IsUnrecoverableCommitFailure(entry.statusEntry) && !HasRemovalIntent(entry))
                            {
                                // The node rejected the update for good. Dropping the entry would make the write
                                // delete the old value too, so the entry goes back to it.
                                entry.ACLEntry = std::move(*entry.supersededValue);
                                entry.supersededValue.reset();
                                entry.statusEntry.state       = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                                entry.statusEntry.failureCode = 0;
                            }
                        }
                        if (superseded)
                        {
                            continue;
                        }

                        // Added to the node outside the datastore: adopted as Committed, as RefreshNode specifies.
                        if (!hasRoom())
                        {
                            ++notAdoptedCount;
                            continue;
                        }

                        datastore::ACLEntryStruct newEntry;
                        if (GenerateAndAssignAUniqueListID(newEntry.listID) != CHIP_NO_ERROR)
                        {
                            continue;
                        }
                        newEntry.nodeID             = mRefreshingNodeId;
                        newEntry.ACLEntry.authMode  = acl.ACLEntry.authMode;
                        newEntry.ACLEntry.privilege = acl.ACLEntry.privilege;
                        if (!acl.ACLEntry.subjects.IsNull())
                        {
                            newEntry.ACLEntry.subjects.assign(acl.ACLEntry.subjects.Value().begin(),
                                                              acl.ACLEntry.subjects.Value().end());
                        }
                        if (!acl.ACLEntry.targets.IsNull())
                        {
                            newEntry.ACLEntry.targets.assign(acl.ACLEntry.targets.Value().begin(),
                                                             acl.ACLEntry.targets.Value().end());
                        }
                        newEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                        seenListIds.push_back(newEntry.listID);
                        mACLEntries.push_back(std::move(newEntry));
                    }

                    // Entries the node does not hold: Committed ones were removed outside the datastore, and
                    // removals have taken effect.
                    mACLEntries.erase(std::remove_if(mACLEntries.begin(), mACLEntries.end(),
                                                     [&](const auto & entry) {
                                                         if (entry.nodeID != mRefreshingNodeId ||
                                                             std::find(seenListIds.begin(), seenListIds.end(), entry.listID) !=
                                                                 seenListIds.end())
                                                         {
                                                             return false;
                                                         }
                                                         return entry.statusEntry.state ==
                                                             Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted ||
                                                             HasRemovalIntent(entry);
                                                     }),
                                      mACLEntries.end());

                    if (notAdoptedCount > 0)
                    {
                        // The ACL write replaces the node's whole list, so it would delete the entries that were not
                        // adopted.
                        ChipLogError(AppServer, "ACL list full; %u ACL entries of node 0x" ChipLogFormatX64 " not adopted",
                                     static_cast<unsigned>(notAdoptedCount), ChipLogValueX64(mRefreshingNodeId));
                        FinishRefresh(CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
                        return;
                    }

                    // The node no longer holds the values of the remaining tombstones, other than those left out of the
                    // write for lack of room.
                    mAclTombstones.erase(
                        std::remove_if(mAclTombstones.begin(), mAclTombstones.end(),
                                       [this, &tombstonedValuesLeftOut](const auto & t) {
                                           return t.nodeId == mRefreshingNodeId &&
                                               std::none_of(tombstonedValuesLeftOut.begin(), tombstonedValuesLeftOut.end(),
                                                            [this, &t](const auto * acl) {
                                                                return AclTombstoneMatches(t, mRefreshingNodeId, acl->ACLEntry);
                                                            });
                                       }),
                        mAclTombstones.end());

                    // Advance the state machine to process the ACLs.
                    mRefreshState = kRefreshingACLs;
                }
                else
                {
                    // Leave node as pending but tear down the refresh state.
                    FinishRefresh(err);
                    return;
                }

                // Continue the state machine to let the kRefreshingACLs branch process mACLList.
                CHIP_ERROR continueErr = ContinueRefresh();
                if (continueErr != CHIP_NO_ERROR)
                {
                    FinishRefresh(continueErr);
                }
            }));
    }
    break;
    case kRefreshingACLs: {
        mRefreshingACLEntries.clear();

        // 5.
        // Owned copies of the written values: an entry can be updated again while the write is in flight.
        std::vector<std::pair<uint16_t, datastore::AccessControlEntryStruct>> writtenValues;
        for (auto it = mACLEntries.begin(); it != mACLEntries.end();)
        {
            if (it->nodeID != mRefreshingNodeId)
            {
                ++it;
                continue;
            }

            if (IsUnrecoverableCommitFailure(it->statusEntry))
            {
                // remove entry from the list
                RecordTombstoneIfRemoving(*it);
                it = mACLEntries.erase(it);
                continue;
            }

            // Entries being removed are left out of the write, and erased once it succeeds. Recoverable
            // CommitFailed entries are retried.
            if (!HasRemovalIntent(*it))
            {
                mRefreshingACLEntries.push_back(EncodeAclEntryForSync(*it));
                writtenValues.emplace_back(it->listID, it->ACLEntry);
            }

            ++it;
        }

        const NodeId refreshingNodeId = mRefreshingNodeId;
        CHIP_ERROR syncErr            = mDelegate->SyncNode(
            mRefreshingNodeId, mRefreshingACLEntries,
            [this, refreshingNodeId, writtenValues = std::move(writtenValues), generation = mSyncGeneration](CHIP_ERROR innerErr) {
                VerifyOrReturn(generation == mSyncGeneration);
                if (innerErr != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer,
                                            "Failed syncing ACLs during refresh for node 0x" ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                                            ChipLogValueX64(refreshingNodeId), innerErr.Format());

                    // Keep entries for retry. The node stays Pending.
                    MarkRefreshWriteFailed(mACLEntries, refreshingNodeId, innerErr);
                    FinishRefresh(innerErr);
                    return;
                }

                // The node now holds the written values, also those of entries updated or marked for removal while the
                // write was in flight. The queued sync of such an entry must replace or remove the written value, not
                // the value the write replaced, so the written value becomes its superseded value.
                for (auto & entry : mACLEntries)
                {
                    if (entry.nodeID != refreshingNodeId)
                    {
                        continue;
                    }
                    auto written = std::find_if(writtenValues.begin(), writtenValues.end(),
                                                           [&entry](const auto & value) { return value.first == entry.listID; });
                    if (written == writtenValues.end())
                    {
                        continue;
                    }
                    if (detail::AclEntryValueEquals(EncodeAccessControlEntry(entry.ACLEntry),
                                                               EncodeAccessControlEntry(written->second)))
                    {
                        entry.supersededValue.reset();
                        if (!HasRemovalIntent(entry))
                        {
                            entry.statusEntry.state       = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                            entry.statusEntry.failureCode = 0;
                        }
                    }
                    else
                    {
                        entry.supersededValue = written->second;
                    }
                }

                // The node no longer holds the entries being removed that the write left out. A removal requested
                // while the write was in flight is still queued and runs after the refresh.
                mACLEntries.erase(std::remove_if(mACLEntries.begin(), mACLEntries.end(),
                                                            [this, refreshingNodeId](const auto & entry) {
                                                     return entry.nodeID == refreshingNodeId && HasRemovalIntent(entry) &&
                                                         !InRefreshWrite(entry);
                                                 }),
                                             mACLEntries.end());

                if (mRefreshingNodeId != refreshingNodeId)
                {
                    return;
                }

                if (mRefreshHadFailure)
                {
                    ChipLogError(AppServer,
                                            "Finished refreshing node (ID: 0x" ChipLogFormatX64 ") with failures. Node is left Pending.",
                                            ChipLogValueX64(refreshingNodeId));
                    FinishRefresh(CHIP_NO_ERROR);
                    return;
                }

                // 6.
                CHIP_ERROR setErr = SetNode(refreshingNodeId, Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted);
                if (setErr != CHIP_NO_ERROR)
                {
                    FinishRefresh(setErr);
                    return;
                }

                ChipLogDetail(AppServer, "Finished refreshing node (ID: 0x" ChipLogFormatX64 "). Node is now marked as Committed.",
                                         ChipLogValueX64(refreshingNodeId));

                for (Listener * listener = mListeners; listener != nullptr; listener = listener->mNext)
                {
                    listener->MarkNodeListChanged();
                }

                FinishRefresh(CHIP_NO_ERROR);
            });
        if (syncErr != CHIP_NO_ERROR)
        {
            MarkRefreshWriteFailed(mACLEntries, refreshingNodeId, syncErr);
            return syncErr;
        }
    }
    break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabricDatastore::SetNode(NodeId nodeId, Clusters::JointFabricDatastore::DatastoreStateEnum state)
{
    size_t index = 0;
    ReturnErrorOnFailure(IsNodeIDInDatastore(nodeId, index));
    mNodeInformationEntries[index].commissioningStatusEntry.state = state;
    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabricDatastore::IsNodeIDInDatastore(NodeId nodeId, size_t & index)
{
    for (auto & entry : mNodeInformationEntries)
    {
        if (entry.nodeID == nodeId)
        {
            index = static_cast<size_t>(&entry - &mNodeInformationEntries[0]);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR
JointFabricDatastore::AddGroupKeySetEntry(
    const Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet)
{
    VerifyOrReturnError(IsGroupKeySetEntryPresent(groupKeySet.groupKeySetID) == false, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(groupKeySet.groupKeySecurityPolicy <
                            Clusters::JointFabricDatastore::DatastoreGroupKeySecurityPolicyEnum::kUnknownEnumValue,
                        CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(mGroupKeySetList.size() < kMaxGroupKeySet, CHIP_ERROR_NO_MEMORY);

    Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type copiedKeySet;
    ReturnErrorOnFailure(CopyGroupKeySetWithOwnedSpans(groupKeySet, copiedKeySet));

    mGroupKeySetList.push_back(copiedKeySet);

    return CHIP_NO_ERROR;
}

bool JointFabricDatastore::IsGroupKeySetEntryPresent(uint16_t groupKeySetId)
{
    for (auto & entry : mGroupKeySetList)
    {
        if (entry.groupKeySetID == groupKeySetId)
        {
            return true;
        }
    }

    return false;
}

CHIP_ERROR JointFabricDatastore::RemoveGroupKeySetEntry(uint16_t groupKeySetId)
{
    VerifyOrReturnValue(groupKeySetId != 0, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    for (auto it = mGroupKeySetList.begin(); it != mGroupKeySetList.end(); ++it)
    {
        if (it->groupKeySetID == groupKeySetId)
        {
            RemoveGroupKeySetStorage(groupKeySetId);
            mGroupKeySetList.erase(it);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_IM_GLOBAL_STATUS(NotFound);
}

CHIP_ERROR
JointFabricDatastore::UpdateGroupKeySetEntry(
    Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet)
{
    for (auto & entry : mGroupKeySetList)
    {
        if (entry.groupKeySetID == groupKeySet.groupKeySetID)
        {
            VerifyOrReturnValue(groupKeySet.groupKeySecurityPolicy <
                                    Clusters::JointFabricDatastore::DatastoreGroupKeySecurityPolicyEnum::kUnknownEnumValue,
                                CHIP_IM_GLOBAL_STATUS(ConstraintError));

            // Checked before any change, so that a BUSY rejection leaves the datastore as it was.
            std::map<NodeId, size_t> syncsPerNode;
            for (const auto & nodeKeySet : mNodeKeySetEntries)
            {
                if (nodeKeySet.groupKeySetID == groupKeySet.groupKeySetID && !HasRemovalIntent(nodeKeySet))
                {
                    ++syncsPerNode[nodeKeySet.nodeID];
                }
            }
            VerifyOrReturnError(HasNodeSyncCapacity(syncsPerNode), CHIP_IM_GLOBAL_STATUS(Busy));

            ReturnErrorOnFailure(CopyGroupKeySetWithOwnedSpans(groupKeySet, entry));

            // The nodes are sent the stored key set, so it is updated first.
            LogErrorOnFailure(UpdateNodeKeySetList(groupKeySet.groupKeySetID));

            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR
JointFabricDatastore::AddAdmin(
    const Clusters::JointFabricDatastore::Structs::DatastoreAdministratorInformationEntryStruct::Type & adminId)
{
    VerifyOrReturnError(IsAdminEntryPresent(adminId.nodeID) == false, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(mAdminEntries.size() < kMaxAdminNodes, CHIP_ERROR_NO_MEMORY);

    Clusters::JointFabricDatastore::Structs::DatastoreAdministratorInformationEntryStruct::Type entryToStore;
    entryToStore.nodeID   = adminId.nodeID;
    entryToStore.vendorID = adminId.vendorID;

    ReturnErrorOnFailure(SetAdminEntryWithOwnedStorage(adminId.nodeID, adminId.friendlyName, adminId.icac, entryToStore));

    mAdminEntries.push_back(entryToStore);

    return CHIP_NO_ERROR;
}

bool JointFabricDatastore::IsAdminEntryPresent(NodeId nodeId)
{
    for (auto & entry : mAdminEntries)
    {
        if (entry.nodeID == nodeId)
        {
            return true;
        }
    }

    return false;
}

CHIP_ERROR JointFabricDatastore::UpdateAdmin(NodeId nodeId, Optional<CharSpan> friendlyName, Optional<ByteSpan> icac)
{
    for (auto & entry : mAdminEntries)
    {
        if (entry.nodeID == nodeId)
        {
            auto & storage = mAdminEntryStorage[nodeId];
            if (friendlyName.HasValue())
            {
                const auto & name = friendlyName.Value();
                storage.friendlyName.assign(name.data(), name.data() + name.size());
                entry.friendlyName = CharSpan(storage.friendlyName.data(), storage.friendlyName.size());
            }
            if (icac.HasValue())
            {
                const auto & icacVal = icac.Value();
                ReturnErrorOnFailure(storage.icac.SetLength(icacVal.size()));
                memcpy(storage.icac.Bytes(), icacVal.data(), icacVal.size());
                entry.icac = storage.icac.Span();
            }
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR JointFabricDatastore::RemoveAdmin(NodeId nodeId)
{
    for (auto it = mAdminEntries.begin(); it != mAdminEntries.end(); ++it)
    {
        if (it->nodeID == nodeId)
        {
            mAdminEntries.erase(it);
            RemoveAdminEntryStorage(nodeId);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR JointFabricDatastore::UpdateNodeKeySetList(uint16_t groupKeySetId)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    bool entryFound = false;
    std::vector<NodeId> nodesToSync;
    for (auto & entry : mNodeKeySetEntries)
    {
        if (entry.groupKeySetID != groupKeySetId)
        {
            continue;
        }
        entryFound = true;
        // A key set being removed from the node is not written back to it.
        if (HasRemovalIntent(entry))
        {
            continue;
        }
        entry.statusEntry.state       = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;
        entry.statusEntry.failureCode = 0;
        ++entry.syncRevision;
        nodesToSync.push_back(entry.nodeID);
    }

    return entryFound ? QueueNodeKeySetSyncs(nodesToSync, groupKeySetId) : CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR JointFabricDatastore::RemoveKeySet(uint16_t groupKeySetId)
{
    for (auto it = mNodeKeySetEntries.begin(); it != mNodeKeySetEntries.end(); ++it)
    {
        if (it->groupKeySetID == groupKeySetId)
        {
            if (it->statusEntry.state != Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending)
            {
                return CHIP_IM_GLOBAL_STATUS(ConstraintError); // Cannot remove a key set that is not pending
            }

            ReturnErrorOnFailure(RemoveGroupKeySetEntry(groupKeySetId));

            return CHIP_NO_ERROR;
        }
    }

    return CHIP_IM_GLOBAL_STATUS(NotFound);
}

CHIP_ERROR JointFabricDatastore::AddGroup(const Clusters::JointFabricDatastore::Commands::AddGroup::DecodableType & commandData)
{
    size_t index = 0;
    // Check if the group ID already exists in the datastore
    VerifyOrReturnError(IsGroupIDInDatastore(commandData.groupID, index) == CHIP_ERROR_NOT_FOUND,
                        CHIP_IM_GLOBAL_STATUS(ConstraintError));

    VerifyOrReturnError(IsAssignableGroupCat(commandData.groupCAT), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    // A CAT version of 0 is not a valid CASE Authenticated Tag, and AddGroup takes versions up to 65534.
    VerifyOrReturnError(commandData.groupCATVersion.IsNull() ||
                            (commandData.groupCATVersion.Value() >= 1 && commandData.groupCATVersion.Value() <= 65534),
                        CHIP_IM_GLOBAL_STATUS(ConstraintError));

    Clusters::JointFabricDatastore::Structs::DatastoreGroupInformationEntryStruct::Type groupEntry;
    groupEntry.groupID         = commandData.groupID;
    groupEntry.groupKeySetID   = commandData.groupKeySetID;
    groupEntry.groupCAT        = commandData.groupCAT;
    groupEntry.groupCATVersion = commandData.groupCATVersion;
    groupEntry.groupPermission = commandData.groupPermission;
    SetGroupInformationFriendlyNameWithOwnedStorage(commandData.groupID, commandData.friendlyName, groupEntry);

    // Add the group entry to the datastore
    mGroupInformationEntries.push_back(groupEntry);

    return CHIP_NO_ERROR;
}

CHIP_ERROR
JointFabricDatastore::ForceAddGroup(const Clusters::JointFabricDatastore::Commands::AddGroup::DecodableType & commandData)
{
    size_t index = 0;
    // Check if the group ID already exists in the datastore
    VerifyOrReturnError(IsGroupIDInDatastore(commandData.groupID, index) == CHIP_ERROR_NOT_FOUND,
                        CHIP_IM_GLOBAL_STATUS(ConstraintError));

    Clusters::JointFabricDatastore::Structs::DatastoreGroupInformationEntryStruct::Type groupEntry;
    groupEntry.groupID         = commandData.groupID;
    groupEntry.groupKeySetID   = commandData.groupKeySetID;
    groupEntry.groupCAT        = commandData.groupCAT;
    groupEntry.groupCATVersion = commandData.groupCATVersion;
    groupEntry.groupPermission = commandData.groupPermission;
    SetGroupInformationFriendlyNameWithOwnedStorage(commandData.groupID, commandData.friendlyName, groupEntry);

    // Add the group entry to the datastore
    mGroupInformationEntries.push_back(groupEntry);

    return CHIP_NO_ERROR;
}

CHIP_ERROR
JointFabricDatastore::UpdateGroup(const Clusters::JointFabricDatastore::Commands::UpdateGroup::DecodableType & commandData)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    size_t index = 0;
    // Check if the group ID exists in the datastore
    VerifyOrReturnError(IsGroupIDInDatastore(commandData.groupID, index) == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    auto & group = mGroupInformationEntries[index];
    if (group.groupCAT.ValueOr(0) == kAdminCATIdentifier || group.groupCAT.ValueOr(0) == kAnchorCATIdentifier)
    {
        // If the group is an AdminCAT or AnchorCAT, we cannot update it
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }
    VerifyOrReturnError(IsAssignableGroupCat(commandData.groupCAT), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // A CAT version of 0 is not a valid CASE Authenticated Tag.
    VerifyOrReturnError(commandData.groupCATVersion.IsNull() || commandData.groupCATVersion.Value() != 0,
                        CHIP_IM_GLOBAL_STATUS(ConstraintError));

    const GroupId updatedGroupId = commandData.groupID;
    const bool friendlyNameChanged =
        !commandData.friendlyName.IsNull() && !group.friendlyName.data_equal(commandData.friendlyName.Value());
    const bool keySetChanged = !commandData.groupKeySetID.IsNull() &&
        (group.groupKeySetID.IsNull() || group.groupKeySetID.Value() != commandData.groupKeySetID.Value());
    const bool permissionSet = !commandData.groupPermission.IsNull() &&
        commandData.groupPermission.Value() !=
            Clusters::JointFabricDatastore::DatastoreAccessControlEntryPrivilegeEnum::kUnknownEnumValue;
    const bool permissionChanged = permissionSet && commandData.groupPermission.Value() != group.groupPermission;
    const bool anyGroupCATFieldUpdated =
        (!commandData.groupCAT.IsNull() && (group.groupCAT.IsNull() || group.groupCAT.Value() != commandData.groupCAT.Value())) ||
        (!commandData.groupCATVersion.IsNull() &&
         (group.groupCATVersion.IsNull() || group.groupCATVersion.Value() != commandData.groupCATVersion.Value())) ||
        permissionChanged;

    // The group's CAT and version before and after the update, and its new permission.
    const auto previousCat     = group.groupCAT;
    const auto previousVersion = group.groupCATVersion;
    const auto newCat          = commandData.groupCAT.IsNull() ? previousCat : commandData.groupCAT;
    const auto newVersion      = commandData.groupCATVersion.IsNull() ? previousVersion : commandData.groupCATVersion;
    const bool catChanged =
        !newCat.IsNull() && !newVersion.IsNull() && (!(previousCat == newCat) || !(previousVersion == newVersion));
    // A command without a version keeps the stored one, which can be 0: ForceAddGroup does not check it.
    VerifyOrReturnError(!catChanged || newVersion.Value() != 0, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    const NodeId newCatSubject = catChanged
        ? NodeIdFromCASEAuthTag((static_cast<CASEAuthTag>(newCat.Value()) << kTagIdentifierShift) | newVersion.Value())
        : kUndefinedNodeId;
    const auto newPermission   = permissionSet ? commandData.groupPermission.Value() : group.groupPermission;

    // Indices of the subjects in `acl` that name this group: its GroupID in a Group-auth entry, or its CAT, at any
    // version, in a CASE entry.
    auto groupSubjectIndices = [&](const datastore::ACLEntryStruct & acl) {
        std::vector<size_t> indices;
        for (size_t i = 0; i < acl.ACLEntry.subjects.size(); ++i)
        {
            const NodeId subject = acl.ACLEntry.subjects[i];
            if (acl.ACLEntry.authMode == Clusters::JointFabricDatastore::DatastoreAccessControlEntryAuthModeEnum::kGroup &&
                subject == static_cast<NodeId>(updatedGroupId))
            {
                indices.push_back(i);
            }
            else if (acl.ACLEntry.authMode == Clusters::JointFabricDatastore::DatastoreAccessControlEntryAuthModeEnum::kCase &&
                     !previousCat.IsNull() && IsCASEAuthTag(subject) &&
                     GetCASEAuthTagIdentifier(CASEAuthTagFromNodeId(subject)) == previousCat.Value())
            {
                indices.push_back(i);
            }
        }
        return indices;
    };

    // What updating `acl` for this command involves.
    struct AclChange
    {
        std::vector<size_t> groupSubjects;
        bool rewrite         = false; // CAT subjects are rewritten to the new CAT and version
        bool changePrivilege = false; // the group's subjects get the new permission
        bool Needed() const { return rewrite || changePrivilege; }
        // Other subjects keep their privilege, so the group's subjects move to a new entry.
        bool Splits(const datastore::ACLEntryStruct & acl) const
        {
            return changePrivilege && groupSubjects.size() < acl.ACLEntry.subjects.size();
        }
    };
    auto aclChangeFor = [&](const datastore::ACLEntryStruct & acl) {
        AclChange change;
        // An entry being removed keeps the value its removal is sent with. Splitting it would add the group's subjects
        // back to the node in a new entry.
        if (HasRemovalIntent(acl))
        {
            return change;
        }
        change.groupSubjects = groupSubjectIndices(acl);
        if (!change.groupSubjects.empty())
        {
            change.rewrite = catChanged &&
                acl.ACLEntry.authMode == Clusters::JointFabricDatastore::DatastoreAccessControlEntryAuthModeEnum::kCase;
            change.changePrivilege = permissionChanged && acl.ACLEntry.privilege != newPermission;
        }
        return change;
    };

    // Checked before any change. A split adds an ACL entry, so the list must have room for every split.
    size_t splits = 0;
    for (const auto & acl : mACLEntries)
    {
        if (aclChangeFor(acl).Splits(acl))
        {
            ++splits;
        }
    }
    VerifyOrReturnError(splits == 0 || mACLEntries.size() + splits <= kMaxACLs, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));

    // Checked before any change, so that a BUSY rejection leaves the datastore as it was. Key set changes are counted
    // as an add and a removal on every node in the group, which is at least what they queue. A split entry is synced
    // twice: the original, then the new entry.
    std::map<NodeId, size_t> syncsPerNode;
    for (const auto & epGroupEntry : mEndpointGroupIDEntries)
    {
        if (friendlyNameChanged && epGroupEntry.groupID == updatedGroupId)
        {
            ++syncsPerNode[epGroupEntry.nodeID];
        }
    }
    if (keySetChanged)
    {
        for (const NodeId nodeId : NodesInGroup(updatedGroupId))
        {
            syncsPerNode[nodeId] += 2;
        }
    }
    for (const auto & acl : mACLEntries)
    {
        const auto change = aclChangeFor(acl);
        if (anyGroupCATFieldUpdated && change.Needed())
        {
            syncsPerNode[acl.nodeID] += change.Splits(acl) ? 2 : 1;
        }
    }
    VerifyOrReturnError(HasNodeSyncCapacity(syncsPerNode), CHIP_IM_GLOBAL_STATUS(Busy));

    CHIP_ERROR firstErr = CHIP_NO_ERROR;
    auto recordFirst    = [&firstErr](CHIP_ERROR err) {
        if (firstErr == CHIP_NO_ERROR)
        {
            firstErr = err;
        }
    };

    if (friendlyNameChanged)
    {
        // Friendly name changed. For every endpoint that references this group, mark the endpoint's
        // GroupIDList entry as pending and push the change to the node. If the push fails, the entry
        // records CommitFailed so a subsequent Refresh can apply it.
        std::vector<std::pair<NodeId, EndpointId>> updatedEndpoints;
        for (auto & epGroupEntry : mEndpointGroupIDEntries)
        {
            if (epGroupEntry.groupID == updatedGroupId)
            {
                epGroupEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;
                ++epGroupEntry.syncRevision;
                updatedEndpoints.emplace_back(epGroupEntry.nodeID, epGroupEntry.endpointID);
            }
        }

        // Update the friendly name in the datastore
        SetGroupInformationFriendlyNameWithOwnedStorage(static_cast<GroupId>(group.groupID), commandData.friendlyName.Value(),
                                                        group);

        for (const auto & [epNodeId, epEndpointId] : updatedEndpoints)
        {
            recordFirst(RunOrQueueNodeSync(epNodeId, [this, nodeId = epNodeId, endpointId = epEndpointId, updatedGroupId]() {
                return StartEndpointGroupEntrySync(nodeId, endpointId, updatedGroupId, std::nullopt);
            }));
        }
    }

    if (!commandData.groupKeySetID.IsNull())
    {
        if (keySetChanged)
        {
            // The new key set is added to the group's nodes before the old one is removed.
            recordFirst(AddNodeKeySetEntry(updatedGroupId, commandData.groupKeySetID.Value()));
            if (!group.groupKeySetID.IsNull())
            {
                LogErrorOnFailure(RemoveNodeKeySetEntry(updatedGroupId, group.groupKeySetID.Value()));
            }
        }
        group.groupKeySetID = commandData.groupKeySetID;
    }

    if (!commandData.groupCAT.IsNull())
    {
        group.groupCAT = commandData.groupCAT;
    }
    if (!commandData.groupCATVersion.IsNull())
    {
        group.groupCATVersion = commandData.groupCATVersion;
    }
    if (permissionSet)
    {
        group.groupPermission = commandData.groupPermission.Value();
    }

    if (anyGroupCATFieldUpdated)
    {
        // Removes repeated subjects, keeping the first of each. Rewriting several versions of one CAT yields repeats.
        auto removeRepeatedSubjects = [](std::vector<uint64_t> & subjects) {
            std::vector<uint64_t> unique;
            for (const auto subject : subjects)
            {
                if (std::find(unique.begin(), unique.end(), subject) == unique.end())
                {
                    unique.push_back(subject);
                }
            }
            subjects = std::move(unique);
        };

        std::vector<std::pair<NodeId, uint16_t>> updatedAcls;

        // Iterate by index over the entries present before the loop: a split appends to mACLEntries. Subjects are
        // selected by the CAT the group had before this update.
        const size_t aclCount = mACLEntries.size();
        for (size_t i = 0; i < aclCount; ++i)
        {
            const auto change = aclChangeFor(mACLEntries[i]);
            if (!change.Needed())
            {
                continue;
            }

            auto & acl = mACLEntries[i];
            // The node holds the last committed value until the update reaches it; keep that value so the update
            // replaces it.
            if (!acl.supersededValue.has_value())
            {
                acl.supersededValue = acl.ACLEntry;
            }

            if (change.rewrite)
            {
                for (const auto subjectIndex : change.groupSubjects)
                {
                    acl.ACLEntry.subjects[subjectIndex] = newCatSubject;
                }
            }

            std::optional<datastore::ACLEntryStruct> groupEntry;
            if (change.Splits(acl))
            {
                // The entry also names other subjects, which keep their privilege. The group's subjects move to a new
                // entry with the new privilege.
                groupEntry.emplace();
                groupEntry->nodeID             = acl.nodeID;
                groupEntry->ACLEntry.authMode  = acl.ACLEntry.authMode;
                groupEntry->ACLEntry.privilege = newPermission;
                groupEntry->ACLEntry.targets   = acl.ACLEntry.targets;
                groupEntry->statusEntry.state  = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

                std::vector<uint64_t> otherSubjects;
                for (size_t subjectIndex = 0; subjectIndex < acl.ACLEntry.subjects.size(); ++subjectIndex)
                {
                    const auto & groupSubjects = change.groupSubjects;
                    if (std::find(groupSubjects.begin(), groupSubjects.end(), subjectIndex) != groupSubjects.end())
                    {
                        groupEntry->ACLEntry.subjects.push_back(acl.ACLEntry.subjects[subjectIndex]);
                    }
                    else
                    {
                        otherSubjects.push_back(acl.ACLEntry.subjects[subjectIndex]);
                    }
                }
                acl.ACLEntry.subjects = std::move(otherSubjects);
                removeRepeatedSubjects(groupEntry->ACLEntry.subjects);
            }
            else if (change.changePrivilege)
            {
                acl.ACLEntry.privilege = newPermission;
            }
            removeRepeatedSubjects(acl.ACLEntry.subjects);
            acl.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

            // The original entry is replaced before the new one is added, so the node never grants more than before.
            updatedAcls.emplace_back(acl.nodeID, acl.listID);
            if (groupEntry.has_value())
            {
                ReturnErrorOnFailure(GenerateAndAssignAUniqueListID(groupEntry->listID));
                updatedAcls.emplace_back(groupEntry->nodeID, groupEntry->listID);
                mACLEntries.push_back(std::move(*groupEntry));
            }
        }

        // Sync after the loop: a sync that completes synchronously can erase entries.
        for (const auto & [aclNodeId, aclListId] : updatedAcls)
        {
            recordFirst(RunOrQueueNodeSync(
                aclNodeId, [this, nodeId = aclNodeId, listId = aclListId]() { return StartAclEntrySync(nodeId, listId); }));
        }
    }

    return firstErr;
}

CHIP_ERROR
JointFabricDatastore::RemoveGroup(const Clusters::JointFabricDatastore::Commands::RemoveGroup::DecodableType & commandData)
{
    size_t index = 0;
    // Check if the group ID exists in the datastore
    VerifyOrReturnError(IsGroupIDInDatastore(commandData.groupID, index) == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Remove the group entry from the datastore
    auto it = mGroupInformationEntries.begin();
    std::advance(it, index);

    if (it->groupCAT.ValueOr(0) == kAdminCATIdentifier || it->groupCAT.ValueOr(0) == kAnchorCATIdentifier)
    {
        // If the group is an AdminCAT or AnchorCAT, we cannot remove it
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    const GroupId removedGroupId = static_cast<GroupId>(it->groupID);
    mGroupInformationEntries.erase(it);
    RemoveGroupInformationStorage(removedGroupId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabricDatastore::IsGroupIDInDatastore(chip::GroupId groupId, size_t & index)
{
    for (auto & entry : mGroupInformationEntries)
    {
        if (entry.groupID == groupId)
        {
            index = static_cast<size_t>(&entry - &mGroupInformationEntries[0]);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR JointFabricDatastore::IsNodeIdInNodeInformationEntries(NodeId nodeId, size_t & index)
{
    for (auto & entry : mNodeInformationEntries)
    {
        if (entry.nodeID == nodeId)
        {
            index = static_cast<size_t>(&entry - &mNodeInformationEntries[0]);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_IM_GLOBAL_STATUS(ConstraintError);
}

CHIP_ERROR JointFabricDatastore::UpdateEndpointForNode(NodeId nodeId, chip::EndpointId endpointId, CharSpan friendlyName)
{
    for (auto & entry : mEndpointEntries)
    {
        if (entry.nodeID == nodeId && entry.endpointID == endpointId)
        {
            SetEndpointFriendlyNameWithOwnedStorage(nodeId, endpointId, friendlyName, entry);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_IM_GLOBAL_STATUS(ConstraintError);
}

CHIP_ERROR JointFabricDatastore::IsNodeIdAndEndpointInEndpointInformationEntries(NodeId nodeId, EndpointId endpointId,
                                                                                 size_t & index)
{
    for (auto & entry : mEndpointEntries)
    {
        if (entry.nodeID == nodeId && entry.endpointID == endpointId)
        {
            index = static_cast<size_t>(&entry - &mEndpointEntries[0]);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_IM_GLOBAL_STATUS(ConstraintError);
}

CHIP_ERROR JointFabricDatastore::AddGroupIDToEndpointForNode(NodeId nodeId, chip::EndpointId endpointId, chip::GroupId groupId)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    size_t index = 0;
    ReturnErrorOnFailure(IsNodeIdAndEndpointInEndpointInformationEntries(nodeId, endpointId, index));

    VerifyOrReturnError(IsGroupIDInDatastore(groupId, index) == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    auto groupMatch = [nodeId, endpointId, groupId](const auto & entry) {
        return entry.nodeID == nodeId && entry.endpointID == endpointId && entry.groupID == groupId;
    };
    auto existing = std::find_if(mEndpointGroupIDEntries.begin(), mEndpointGroupIDEntries.end(), groupMatch);
    if (existing != mEndpointGroupIDEntries.end() && !HasRemovalIntent(*existing))
    {
        return CHIP_NO_ERROR;
    }
    VerifyOrReturnError(existing != mEndpointGroupIDEntries.end() || mEndpointGroupIDEntries.size() < kMaxGroups,
                        CHIP_ERROR_NO_MEMORY);

    // The node needs the group's key set before the group can be added.
    std::optional<uint16_t> groupKeySetId;
    if (!mGroupInformationEntries[index].groupKeySetID.IsNull())
    {
        groupKeySetId = mGroupInformationEntries[index].groupKeySetID.Value();
    }
    auto keySetMatch = [nodeId, groupKeySetId](const auto & entry) {
        return entry.nodeID == nodeId && entry.groupKeySetID == groupKeySetId.value_or(0);
    };
    auto existingKeySet = std::find_if(mNodeKeySetEntries.begin(), mNodeKeySetEntries.end(), keySetMatch);
    const bool keySetNeeded =
        groupKeySetId.has_value() && (existingKeySet == mNodeKeySetEntries.end() || HasRemovalIntent(*existingKeySet));

    // Checked before any change, so that a BUSY rejection leaves the datastore as it was.
    VerifyOrReturnError(HasNodeSyncCapacity(nodeId, keySetNeeded ? 2 : 1), CHIP_IM_GLOBAL_STATUS(Busy));

    if (existing != mEndpointGroupIDEntries.end())
    {
        // Adding an entry that is being removed cancels the removal.
        CancelRemoval(*existing);
    }
    else
    {
        datastore::EndpointGroupIDEntryStruct newGroupEntry;
        newGroupEntry.nodeID            = nodeId;
        newGroupEntry.endpointID        = endpointId;
        newGroupEntry.groupID           = groupId;
        newGroupEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;
        mEndpointGroupIDEntries.push_back(newGroupEntry);
    }
    mEndpointGroupTombstones.erase(std::remove_if(mEndpointGroupTombstones.begin(), mEndpointGroupTombstones.end(), groupMatch),
                                   mEndpointGroupTombstones.end());

    CHIP_ERROR firstErr = CHIP_NO_ERROR;
    if (keySetNeeded)
    {
        if (existingKeySet != mNodeKeySetEntries.end())
        {
            // Adding an entry that is being removed cancels the removal.
            CancelRemoval(*existingKeySet);
        }
        else
        {
            datastore::NodeKeySetEntryStruct newNodeKeySet;
            newNodeKeySet.nodeID            = nodeId;
            newNodeKeySet.groupKeySetID     = *groupKeySetId;
            newNodeKeySet.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;
            mNodeKeySetEntries.push_back(newNodeKeySet);
        }
        mNodeKeySetTombstones.erase(std::remove_if(mNodeKeySetTombstones.begin(), mNodeKeySetTombstones.end(), keySetMatch),
                                    mNodeKeySetTombstones.end());

        firstErr = RunOrQueueNodeSync(
            nodeId, [this, nodeId, keySetId = *groupKeySetId]() { return StartNodeKeySetEntrySync(nodeId, keySetId); });
    }

    // Queued after the key set, so that it starts once the key set sync has finished.
    const std::optional<uint16_t> requiredKeySetId = keySetNeeded ? groupKeySetId : std::nullopt;
    CHIP_ERROR groupErr = RunOrQueueNodeSync(nodeId, [this, nodeId, endpointId, groupId, requiredKeySetId]() {
        return StartEndpointGroupEntrySync(nodeId, endpointId, groupId, requiredKeySetId);
    });
    return firstErr != CHIP_NO_ERROR ? firstErr : groupErr;
}

CHIP_ERROR JointFabricDatastore::RemoveGroupIDFromEndpointForNode(NodeId nodeId, chip::EndpointId endpointId, chip::GroupId groupId)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    size_t index = 0;
    ReturnErrorOnFailure(IsNodeIdAndEndpointInEndpointInformationEntries(nodeId, endpointId, index));

    auto group = std::find_if(mEndpointGroupIDEntries.begin(), mEndpointGroupIDEntries.end(), [&](const auto & entry) {
        return entry.nodeID == nodeId && entry.endpointID == endpointId && entry.groupID == groupId;
    });
    VerifyOrReturnError(group != mEndpointGroupIDEntries.end(), CHIP_IM_GLOBAL_STATUS(NotFound));

    auto keySet = mNodeKeySetEntries.end();
    if (IsGroupIDInDatastore(groupId, index) == CHIP_NO_ERROR && !mGroupInformationEntries[index].groupKeySetID.IsNull())
    {
        const uint16_t groupKeySetId = mGroupInformationEntries[index].groupKeySetID.Value();
        keySet                       = std::find_if(mNodeKeySetEntries.begin(), mNodeKeySetEntries.end(),
                                                    [&](const auto & entry) { return entry.nodeID == nodeId && entry.groupKeySetID == groupKeySetId; });
    }

    // Checked before any change, so that a BUSY rejection leaves the datastore as it was.
    VerifyOrReturnError(HasNodeSyncCapacity(nodeId, keySet != mNodeKeySetEntries.end() ? 2 : 1), CHIP_IM_GLOBAL_STATUS(Busy));

    MarkRemovalRequested(*group);
    std::optional<uint16_t> keySetIdToRemove;
    if (keySet != mNodeKeySetEntries.end())
    {
        MarkRemovalRequested(*keySet);
        keySetIdToRemove = keySet->groupKeySetID;
    }

    // Syncs start only after every change: one that completes synchronously can erase entries. The group is removed
    // before its key set.
    CHIP_ERROR firstErr = RunOrQueueNodeSync(nodeId, [this, nodeId, endpointId, groupId]() {
        return StartEndpointGroupEntrySync(nodeId, endpointId, groupId, std::nullopt);
    });
    if (keySetIdToRemove.has_value())
    {
        CHIP_ERROR keySetErr = RunOrQueueNodeSync(
            nodeId, [this, nodeId, keySetId = *keySetIdToRemove]() { return StartNodeKeySetEntrySync(nodeId, keySetId); });
        if (firstErr == CHIP_NO_ERROR)
        {
            firstErr = keySetErr;
        }
    }

    return firstErr;
}

// look-up the highest listId used so far, from Endpoint Binding Entries and ACL Entries
CHIP_ERROR JointFabricDatastore::GenerateAndAssignAUniqueListID(uint16_t & listId)
{
    uint16_t highestListID = 0;
    for (auto & entry : mEndpointBindingEntries)
    {
        if (entry.listID >= highestListID)
        {
            highestListID = entry.listID + 1;
        }
    }
    for (auto & entry : mACLEntries)
    {
        if (entry.listID >= highestListID)
        {
            highestListID = entry.listID + 1;
        }
    }

    listId = highestListID;

    return CHIP_NO_ERROR;
}

namespace detail {

bool BindingTargetValueEquals(const Clusters::JointFabricDatastore::Structs::DatastoreBindingTargetStruct::Type & binding1,
                              const Clusters::JointFabricDatastore::Structs::DatastoreBindingTargetStruct::Type & binding2)
{
    if (binding1.node.HasValue() && binding2.node.HasValue())
    {
        if (binding1.node.Value() != binding2.node.Value())
        {
            return false;
        }
    }
    else if (binding1.node.HasValue() || binding2.node.HasValue())
    {
        return false;
    }

    if (binding1.group.HasValue() && binding2.group.HasValue())
    {
        if (binding1.group.Value() != binding2.group.Value())
        {
            return false;
        }
    }
    else if (binding1.group.HasValue() || binding2.group.HasValue())
    {
        return false;
    }

    if (binding1.endpoint.HasValue() && binding2.endpoint.HasValue())
    {
        if (binding1.endpoint.Value() != binding2.endpoint.Value())
        {
            return false;
        }
    }
    else if (binding1.endpoint.HasValue() || binding2.endpoint.HasValue())
    {
        return false;
    }

    if (binding1.cluster.HasValue() && binding2.cluster.HasValue())
    {
        if (binding1.cluster.Value() != binding2.cluster.Value())
        {
            return false;
        }
    }
    else if (binding1.cluster.HasValue() || binding2.cluster.HasValue())
    {
        return false;
    }

    return true;
}

bool BindingEntryValueEquals(const Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type & a,
                             const Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type & b)
{
    return a.endpointID == b.endpointID && BindingTargetValueEquals(a.binding, b.binding);
}

std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>
ApplyBindingEdit(const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> & current,
                 const Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type & entry)
{
    auto result        = current;
    const auto matches = [&entry](const auto & candidate) { return BindingEntryValueEquals(candidate, entry); };

    if (entry.statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending)
    {
        result.erase(std::remove_if(result.begin(), result.end(), matches), result.end());
        return result;
    }

    if (std::none_of(result.begin(), result.end(), matches))
    {
        result.push_back(entry);
    }
    return result;
}

} // namespace detail

bool JointFabricDatastore::BindingMatches(
    const Clusters::JointFabricDatastore::Structs::DatastoreBindingTargetStruct::Type & binding1,
    const Clusters::JointFabricDatastore::Structs::DatastoreBindingTargetStruct::Type & binding2)
{
    return detail::BindingTargetValueEquals(binding1, binding2);
}

CHIP_ERROR
JointFabricDatastore::AddBindingToEndpointForNode(
    NodeId nodeId, chip::EndpointId endpointId,
    const Clusters::JointFabricDatastore::Structs::DatastoreBindingTargetStruct::Type & binding)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    size_t index = 0;
    ReturnErrorOnFailure(IsNodeIdAndEndpointInEndpointInformationEntries(nodeId, endpointId, index));

    datastore::EndpointBindingEntryStruct newBindingEntry;
    auto existing = std::find_if(mEndpointBindingEntries.begin(), mEndpointBindingEntries.end(), [&](const auto & entry) {
        return entry.nodeID == nodeId && entry.endpointID == endpointId && BindingMatches(entry.binding, binding);
    });
    if (existing != mEndpointBindingEntries.end() && !HasRemovalIntent(*existing))
    {
        return CHIP_NO_ERROR;
    }

    // Checked before any change, so that a BUSY rejection leaves the datastore as it was.
    VerifyOrReturnError(HasNodeSyncCapacity(nodeId), CHIP_IM_GLOBAL_STATUS(Busy));

    if (existing != mEndpointBindingEntries.end())
    {
        // Adding an entry that is being removed cancels the removal.
        CancelRemoval(*existing);
        newBindingEntry = *existing;
    }
    else
    {
        VerifyOrReturnError(mEndpointBindingEntries.size() < kMaxGroups, CHIP_ERROR_NO_MEMORY);

        // Create a new binding entry
        newBindingEntry.nodeID     = nodeId;
        newBindingEntry.endpointID = endpointId;
        newBindingEntry.binding    = binding;
        ReturnErrorOnFailure(GenerateAndAssignAUniqueListID(newBindingEntry.listID));
        newBindingEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

        // Add the new binding entry to the datastore
        mEndpointBindingEntries.push_back(newBindingEntry);
    }
    mBindingTombstones.erase(std::remove_if(mBindingTombstones.begin(), mBindingTombstones.end(),
                                            [&newBindingEntry](const auto & t) {
                                                return t.nodeID == newBindingEntry.nodeID &&
                                                    detail::BindingEntryValueEquals(t, newBindingEntry);
                                            }),
                             mBindingTombstones.end());

    const uint16_t listId = newBindingEntry.listID;
    return RunOrQueueNodeSync(nodeId,
                              [this, nodeId, endpointId, listId]() { return StartBindingEntrySync(nodeId, endpointId, listId); });
}

CHIP_ERROR
JointFabricDatastore::RemoveBindingFromEndpointForNode(uint16_t listId, NodeId nodeId, chip::EndpointId endpointId)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    size_t index = 0;
    ReturnErrorOnFailure(IsNodeIdAndEndpointInEndpointInformationEntries(nodeId, endpointId, index));

    for (auto it = mEndpointBindingEntries.begin(); it != mEndpointBindingEntries.end(); ++it)
    {
        if (it->nodeID == nodeId && it->listID == listId && it->endpointID == endpointId)
        {
            VerifyOrReturnError(HasNodeSyncCapacity(nodeId), CHIP_IM_GLOBAL_STATUS(Busy));
            MarkRemovalRequested(*it);
            return RunOrQueueNodeSync(
                nodeId, [this, nodeId, endpointId, listId]() { return StartBindingEntrySync(nodeId, endpointId, listId); });
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

namespace detail {

bool AclTargetValueEquals(const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type & target1,
                          const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type & target2)
{
    if (!target1.cluster.IsNull() && !target2.cluster.IsNull())
    {
        if (target1.cluster.Value() != target2.cluster.Value())
        {
            return false;
        }
    }
    else if (!target1.cluster.IsNull() || !target2.cluster.IsNull())
    {
        return false;
    }

    if (!target1.endpoint.IsNull() && !target2.endpoint.IsNull())
    {
        if (target1.endpoint.Value() != target2.endpoint.Value())
        {
            return false;
        }
    }
    else if (!target1.endpoint.IsNull() || !target2.endpoint.IsNull())
    {
        return false;
    }

    if (!target1.deviceType.IsNull() && !target2.deviceType.IsNull())
    {
        if (target1.deviceType.Value() != target2.deviceType.Value())
        {
            return false;
        }
    }
    else if (!target1.deviceType.IsNull() || !target2.deviceType.IsNull())
    {
        return false;
    }

    return true;
}

namespace {

template <typename T>
Span<const T> ListOrEmpty(const DataModel::Nullable<DataModel::List<const T>> & list)
{
    return list.IsNull() ? Span<const T>() : Span<const T>(list.Value().data(), list.Value().size());
}

} // namespace

bool AclEntryValueEquals(const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::Type & a,
                         const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::Type & b)
{
    if (a.privilege != b.privilege || a.authMode != b.authMode)
    {
        return false;
    }

    const auto subjectsA = ListOrEmpty(a.subjects);
    const auto subjectsB = ListOrEmpty(b.subjects);
    if (!std::is_permutation(subjectsA.begin(), subjectsA.end(), subjectsB.begin(), subjectsB.end()))
    {
        return false;
    }

    const auto targetsA = ListOrEmpty(a.targets);
    const auto targetsB = ListOrEmpty(b.targets);
    return std::is_permutation(targetsA.begin(), targetsA.end(), targetsB.begin(), targetsB.end(), AclTargetValueEquals);
}

std::vector<Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type>
ApplyAclEdit(const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type> & current,
             const Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type & entry,
             const std::optional<Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::Type> & superseded)
{
    using AccessControlEntryType = Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::Type;

    auto result = current;

    const auto eraseEqual = [&result](const AccessControlEntryType & value) {
        result.erase(std::remove_if(result.begin(), result.end(),
                                    [&value](const auto & candidate) { return AclEntryValueEquals(candidate.ACLEntry, value); }),
                     result.end());
    };

    if (superseded.has_value())
    {
        eraseEqual(superseded.value());
    }

    if (entry.statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending)
    {
        eraseEqual(entry.ACLEntry);
        return result;
    }

    if (std::none_of(result.begin(), result.end(),
                     [&entry](const auto & candidate) { return AclEntryValueEquals(candidate.ACLEntry, entry.ACLEntry); }))
    {
        result.push_back(entry);
    }
    return result;
}

} // namespace detail

Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type
JointFabricDatastore::EncodeAclEntryForSync(const datastore::ACLEntryStruct & entry) const
{
    Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type encoded;
    encoded.nodeID      = entry.nodeID;
    encoded.listID      = entry.listID;
    encoded.ACLEntry    = EncodeAccessControlEntry(entry.ACLEntry);
    encoded.statusEntry = entry.statusEntry;
    return encoded;
}

void JointFabricDatastore::RecordTombstoneIfRemoving(const datastore::ACLEntryStruct & entry)
{
    if (HasRemovalIntent(entry))
    {
        PushTombstone(mAclTombstones, AclTombstone{ entry.nodeID, entry.ACLEntry, entry.supersededValue }, kMaxACLs);
    }
}

void JointFabricDatastore::RecordTombstoneIfRemoving(const datastore::EndpointBindingEntryStruct & entry)
{
    if (HasRemovalIntent(entry))
    {
        PushTombstone(
            mBindingTombstones,
            static_cast<const Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type &>(entry),
            kMaxGroups);
    }
}

void JointFabricDatastore::RecordTombstoneIfRemoving(const datastore::EndpointGroupIDEntryStruct & entry)
{
    if (HasRemovalIntent(entry))
    {
        PushTombstone(
            mEndpointGroupTombstones,
            static_cast<const Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type &>(entry),
            kMaxGroups);
    }
}

void JointFabricDatastore::RecordTombstoneIfRemoving(const datastore::NodeKeySetEntryStruct & entry)
{
    if (HasRemovalIntent(entry))
    {
        PushTombstone(mNodeKeySetTombstones,
                      static_cast<const Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type &>(entry),
                      kMaxGroupKeySet);
    }
}

CHIP_ERROR JointFabricDatastore::RunOrQueueNodeSync(NodeId nodeId, std::function<CHIP_ERROR()> start)
{
    auto & queue = mNodeSyncQueues[nodeId];
    if (queue.inFlight)
    {
        VerifyOrReturnError(queue.waiting.size() < kMaxQueuedNodeSyncs, CHIP_IM_GLOBAL_STATUS(Busy));
        queue.waiting.push_back(std::move(start));
        return CHIP_NO_ERROR;
    }
    queue.inFlight = true;
    return start();
}

void JointFabricDatastore::FinishNodeSync(NodeId nodeId)
{
    auto it = mNodeSyncQueues.find(nodeId);
    if (it == mNodeSyncQueues.end())
    {
        return;
    }
    if (it->second.waiting.empty())
    {
        mNodeSyncQueues.erase(it);
        return;
    }
    auto next = std::move(it->second.waiting.front());
    it->second.waiting.pop_front();
    // A failure to start is recorded on the entry by the operation itself.
    LogErrorOnFailure(next());
}

bool JointFabricDatastore::IsNodeSyncIdle(NodeId nodeId) const
{
    return mNodeSyncQueues.find(nodeId) == mNodeSyncQueues.end();
}

bool JointFabricDatastore::HasNodeSyncCapacity(NodeId nodeId, size_t count) const
{
    auto it = mNodeSyncQueues.find(nodeId);
    if (it == mNodeSyncQueues.end())
    {
        // The first operation starts at once.
        return count <= kMaxQueuedNodeSyncs + 1;
    }
    return it->second.waiting.size() + count <= kMaxQueuedNodeSyncs;
}

bool JointFabricDatastore::HasNodeSyncCapacity(const std::map<NodeId, size_t> & countsPerNode) const
{
    return std::all_of(countsPerNode.begin(), countsPerNode.end(),
                       [this](const auto & nodeCount) { return HasNodeSyncCapacity(nodeCount.first, nodeCount.second); });
}

CHIP_ERROR JointFabricDatastore::StartAclEntrySync(NodeId nodeId, uint16_t listId)
{
    auto match = [nodeId, listId](const auto & entry) { return entry.nodeID == nodeId && entry.listID == listId; };
    auto it    = std::find_if(mACLEntries.begin(), mACLEntries.end(), match);
    if (it == mACLEntries.end() ||
        (!HasRemovalIntent(*it) && it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted))
    {
        FinishNodeSync(nodeId);
        return CHIP_NO_ERROR;
    }

    const bool removal        = HasRemovalIntent(*it);
    auto payload              = EncodeAclEntryForSync(*it);
    payload.statusEntry.state = removal ? Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending
                                        : Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;
    std::optional<Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::Type> superseded;
    if (it->supersededValue.has_value())
    {
        superseded = EncodeAccessControlEntry(*it->supersededValue);
    }

    // The result applies only if the entry is still being added or removed as when the sync started.
    auto sameOperation  = [match, removal](const auto & entry) { return match(entry) && HasRemovalIntent(entry) == removal; };
    CHIP_ERROR startErr = mDelegate->SyncNode(
        nodeId, payload, superseded,
        [this, nodeId, match, sameOperation, removal, sentValue = it->ACLEntry, generation = mSyncGeneration](CHIP_ERROR syncErr) {
            VerifyOrReturn(generation == mSyncGeneration);
            if (syncErr != CHIP_NO_ERROR)
            {
                detail::MarkEntrySyncFailedIfFound(mACLEntries, sameOperation, syncErr);
            }
            else if (removal)
            {
                mACLEntries.erase(std::remove_if(mACLEntries.begin(), mACLEntries.end(), sameOperation), mACLEntries.end());
            }
            else
            {
                // The node now holds the value sent here, also if the entry was marked for removal meanwhile: its
                // removal must remove that value, not the one this sync replaced.
                auto entry = std::find_if(mACLEntries.begin(), mACLEntries.end(), match);
                if (entry != mACLEntries.end())
                {
                    if (detail::AclEntryValueEquals(EncodeAccessControlEntry(entry->ACLEntry), EncodeAccessControlEntry(sentValue)))
                    {
                        entry->supersededValue.reset();
                        if (!HasRemovalIntent(*entry))
                        {
                            entry->statusEntry.state       = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                            entry->statusEntry.failureCode = 0;
                        }
                    }
                    else
                    {
                        // Changed while this sync was in flight: the queued sync replaces or removes the value sent here.
                        entry->supersededValue = sentValue;
                    }
                }
            }
            FinishNodeSync(nodeId);
        });
    if (startErr != CHIP_NO_ERROR)
    {
        detail::MarkEntrySyncFailedIfFound(mACLEntries, sameOperation, startErr);
        FinishNodeSync(nodeId);
    }
    return startErr;
}

template <typename Wire, typename Entry, typename Match>
CHIP_ERROR JointFabricDatastore::StartEntrySync(std::vector<Entry> & entries, NodeId nodeId, Match match)
{
    auto it = std::find_if(entries.begin(), entries.end(), match);
    if (it == entries.end() ||
        (!HasRemovalIntent(*it) && it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted))
    {
        FinishNodeSync(nodeId);
        return CHIP_NO_ERROR;
    }

    const bool removal        = HasRemovalIntent(*it);
    const uint32_t revision   = it->syncRevision;
    Wire payload              = *it;
    payload.statusEntry.state = removal ? Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending
                                        : Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

    // The result applies only if the entry is still being added or removed as when the sync started.
    auto sameOperation  = [match, removal](const auto & entry) { return match(entry) && HasRemovalIntent(entry) == removal; };
    CHIP_ERROR startErr = mDelegate->SyncNode(
        nodeId, payload,
        [this, &entries, nodeId, sameOperation, removal, revision, generation = mSyncGeneration](CHIP_ERROR syncErr) {
            VerifyOrReturn(generation == mSyncGeneration);
            if (syncErr != CHIP_NO_ERROR)
            {
                // Recorded even if the entry has been marked for another sync since, which still runs: the node holds
                // neither value, and StartEndpointGroupEntrySync holds back a group add while its key set has failed.
                detail::MarkEntrySyncFailedIfFound(entries, sameOperation, syncErr);
            }
            else if (removal)
            {
                auto entry = std::find_if(entries.begin(), entries.end(), sameOperation);
                if (entry != entries.end())
                {
                    entries.erase(entry);
                }
            }
            else
            {
                // An entry marked for another sync since has a newer value than the one the node now holds, which the
                // queued sync sends.
                detail::MarkEntryCommittedIfFound(entries, [&sameOperation, revision](const auto & entry) {
                    return sameOperation(entry) && entry.syncRevision == revision;
                });
            }
            FinishNodeSync(nodeId);
        });
    if (startErr != CHIP_NO_ERROR)
    {
        detail::MarkEntrySyncFailedIfFound(entries, sameOperation, startErr);
        FinishNodeSync(nodeId);
    }
    return startErr;
}

CHIP_ERROR JointFabricDatastore::StartBindingEntrySync(NodeId nodeId, EndpointId endpointId, uint16_t listId)
{
    return StartEntrySync<Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type>(
        mEndpointBindingEntries, nodeId, [nodeId, endpointId, listId](const auto & entry) {
            return entry.nodeID == nodeId && entry.endpointID == endpointId && entry.listID == listId;
        });
}

CHIP_ERROR JointFabricDatastore::StartEndpointGroupEntrySync(NodeId nodeId, EndpointId endpointId, GroupId groupId,
                                                             std::optional<uint16_t> requiredKeySetId)
{
    auto match = [nodeId, endpointId, groupId](const auto & entry) {
        return entry.nodeID == nodeId && entry.endpointID == endpointId && entry.groupID == groupId;
    };

    if (requiredKeySetId.has_value())
    {
        // Adding the group maps it to its key set on the node, which fails with an unrecoverable CONSTRAINT_ERROR
        // while the node lacks the key set. RefreshNode would then drop the group entry, so it is left Pending for
        // RefreshNode to sync once the key set is in place.
        auto group  = std::find_if(mEndpointGroupIDEntries.begin(), mEndpointGroupIDEntries.end(), match);
        auto keySet = std::find_if(mNodeKeySetEntries.begin(), mNodeKeySetEntries.end(), [&](const auto & entry) {
            return entry.nodeID == nodeId && entry.groupKeySetID == *requiredKeySetId;
        });
        if (group != mEndpointGroupIDEntries.end() && !HasRemovalIntent(*group) && keySet != mNodeKeySetEntries.end() &&
            keySet->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitFailed)
        {
            ChipLogError(AppServer, "Not adding group 0x%04x to node 0x" ChipLogFormatX64 ": its key set %u failed to sync",
                         groupId, ChipLogValueX64(nodeId), *requiredKeySetId);
            FinishNodeSync(nodeId);
            return CHIP_NO_ERROR;
        }
    }

    return StartEntrySync<Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type>(
        mEndpointGroupIDEntries, nodeId, match);
}

CHIP_ERROR JointFabricDatastore::StartNodeKeySetEntrySync(NodeId nodeId, uint16_t groupKeySetId)
{
    return StartEntrySync<Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type>(
        mNodeKeySetEntries, nodeId,
        [nodeId, groupKeySetId](const auto & entry) { return entry.nodeID == nodeId && entry.groupKeySetID == groupKeySetId; });
}

void JointFabricDatastore::MarkRefreshFailed(NodeId nodeId, CHIP_ERROR err)
{
    if (mRefreshingNodeId == nodeId)
    {
        ChipLogError(AppServer, "Sync during refresh of node 0x" ChipLogFormatX64 " failed: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(nodeId), err.Format());
        mRefreshHadFailure = true;
    }
}

void JointFabricDatastore::FinishRefresh(CHIP_ERROR err)
{
    if (mRefreshingNodeId == kUndefinedNodeId)
    {
        return;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Refresh of node 0x" ChipLogFormatX64 " ended: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(mRefreshingNodeId), err.Format());
    }

    const NodeId finishedNodeId = mRefreshingNodeId;
    ResetRefreshState();

    // Operations queued behind the refresh start now.
    FinishNodeSync(finishedNodeId);
}

void JointFabricDatastore::ResetRefreshState()
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

CHIP_ERROR
JointFabricDatastore::AddACLToNode(
    NodeId nodeId, const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::DecodableType & aclEntry)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    size_t index = 0;
    ReturnErrorOnFailure(IsNodeIdInNodeInformationEntries(nodeId, index));

    datastore::AccessControlEntryStruct value;
    value.privilege = aclEntry.privilege;
    value.authMode  = aclEntry.authMode;

    if (!aclEntry.subjects.IsNull())
    {
        auto iter = aclEntry.subjects.Value().begin();
        while (iter.Next())
        {
            value.subjects.push_back(iter.GetValue());
        }
        ReturnErrorOnFailure(iter.GetStatus());
    }

    if (!aclEntry.targets.IsNull())
    {
        auto iter = aclEntry.targets.Value().begin();
        while (iter.Next())
        {
            value.targets.push_back(iter.GetValue());
        }
        ReturnErrorOnFailure(iter.GetStatus());
    }

    // Matched as the node matches it (see detail::ApplyAclEdit): two entries that differ only in subject or target
    // order are one entry on the node, and removing either would remove both.
    const auto encodedValue                 = EncodeAccessControlEntry(value);
    datastore::ACLEntryStruct * storedEntry = nullptr;
    for (auto & entry : mACLEntries)
    {
        if (entry.nodeID == nodeId && detail::AclEntryValueEquals(EncodeAccessControlEntry(entry.ACLEntry), encodedValue))
        {
            if (!HasRemovalIntent(entry))
            {
                return CHIP_NO_ERROR;
            }
            storedEntry = &entry;
            break;
        }
    }

    // Checked before any change, so that a BUSY rejection leaves the datastore as it was.
    VerifyOrReturnError(HasNodeSyncCapacity(nodeId), CHIP_IM_GLOBAL_STATUS(Busy));

    if (storedEntry != nullptr)
    {
        // Adding an entry that is being removed cancels the removal.
        CancelRemoval(*storedEntry);
    }
    else
    {
        VerifyOrReturnError(mACLEntries.size() < kMaxACLs, CHIP_ERROR_NO_MEMORY);
        // Create a new ACL entry
        datastore::ACLEntryStruct newACLEntry;
        newACLEntry.nodeID            = nodeId;
        newACLEntry.ACLEntry          = std::move(value);
        newACLEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

        ReturnErrorOnFailure(GenerateAndAssignAUniqueListID(newACLEntry.listID));

        // Add the new ACL entry to the datastore
        mACLEntries.push_back(newACLEntry);
        storedEntry = &mACLEntries.back();
    }

    const auto addedValue = EncodeAccessControlEntry(storedEntry->ACLEntry);
    mAclTombstones.erase(
        std::remove_if(mAclTombstones.begin(), mAclTombstones.end(),
                       [nodeId, &addedValue](const auto & t) { return AclTombstoneMatches(t, nodeId, addedValue); }),
        mAclTombstones.end());

    const uint16_t listId = storedEntry->listID;
    return RunOrQueueNodeSync(nodeId, [this, nodeId, listId]() { return StartAclEntrySync(nodeId, listId); });
}

CHIP_ERROR JointFabricDatastore::RemoveACLFromNode(uint16_t listId, NodeId nodeId)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    size_t index = 0;
    ReturnErrorOnFailure(IsNodeIdInNodeInformationEntries(nodeId, index));

    for (auto it = mACLEntries.begin(); it != mACLEntries.end(); ++it)
    {
        if (it->nodeID == nodeId && it->listID == listId)
        {
            VerifyOrReturnError(HasNodeSyncCapacity(nodeId), CHIP_IM_GLOBAL_STATUS(Busy));
            MarkRemovalRequested(*it);
            return RunOrQueueNodeSync(nodeId, [this, nodeId, listId]() { return StartAclEntrySync(nodeId, listId); });
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR JointFabricDatastore::QueueNodeKeySetSyncs(const std::vector<NodeId> & nodeIds, uint16_t groupKeySetId)
{
    CHIP_ERROR firstErr = CHIP_NO_ERROR;
    for (const NodeId nodeId : nodeIds)
    {
        CHIP_ERROR err =
            RunOrQueueNodeSync(nodeId, [this, nodeId, groupKeySetId]() { return StartNodeKeySetEntrySync(nodeId, groupKeySetId); });
        if (firstErr == CHIP_NO_ERROR)
        {
            firstErr = err;
        }
    }
    return firstErr;
}

std::unordered_set<NodeId> JointFabricDatastore::NodesInGroup(GroupId groupId) const
{
    std::unordered_set<NodeId> nodes;
    for (const auto & entry : mEndpointGroupIDEntries)
    {
        if (entry.groupID == groupId)
        {
            nodes.insert(entry.nodeID);
        }
    }
    return nodes;
}

CHIP_ERROR JointFabricDatastore::AddNodeKeySetEntry(GroupId groupId, uint16_t groupKeySetId)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    std::vector<NodeId> nodesToSync;
    for (const auto nodeId : NodesInGroup(groupId))
    {
        auto match = [nodeId, groupKeySetId](const auto & e) { return e.nodeID == nodeId && e.groupKeySetID == groupKeySetId; };

        // Skip if a matching NodeKeySet entry already exists for this node, unless it is being removed:
        // adding an entry that is being removed cancels the removal.
        auto existing = std::find_if(mNodeKeySetEntries.begin(), mNodeKeySetEntries.end(), match);
        if (existing != mNodeKeySetEntries.end())
        {
            if (!HasRemovalIntent(*existing))
            {
                continue;
            }
            CancelRemoval(*existing);
        }
        else
        {
            datastore::NodeKeySetEntryStruct newEntry;
            newEntry.nodeID            = nodeId;
            newEntry.groupKeySetID     = groupKeySetId;
            newEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;
            mNodeKeySetEntries.push_back(newEntry);
        }
        mNodeKeySetTombstones.erase(std::remove_if(mNodeKeySetTombstones.begin(), mNodeKeySetTombstones.end(), match),
                                    mNodeKeySetTombstones.end());
        nodesToSync.push_back(nodeId);
    }

    return QueueNodeKeySetSyncs(nodesToSync, groupKeySetId);
}

CHIP_ERROR JointFabricDatastore::RemoveNodeKeySetEntry(GroupId groupId, uint16_t groupKeySetId)
{
    // NOTE: this method assumes its ok to remove the keyset from each node (its not in use by any group)
    const auto nodesInGroup = NodesInGroup(groupId);

    std::vector<NodeId> nodesToSync;
    for (auto & entry : mNodeKeySetEntries)
    {
        if (entry.groupKeySetID == groupKeySetId && nodesInGroup.count(entry.nodeID) != 0)
        {
            MarkRemovalRequested(entry);
            nodesToSync.push_back(entry.nodeID);
        }
    }
    VerifyOrReturnError(!nodesToSync.empty(), CHIP_ERROR_NOT_FOUND);

    return QueueNodeKeySetSyncs(nodesToSync, groupKeySetId);
}

CHIP_ERROR JointFabricDatastore::TestAddNodeKeySetEntry(GroupId groupId, uint16_t groupKeySetId, NodeId nodeId)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    datastore::NodeKeySetEntryStruct newEntry;
    newEntry.nodeID            = nodeId;
    newEntry.groupKeySetID     = groupKeySetId;
    newEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;
    mNodeKeySetEntries.push_back(newEntry);

    auto match = [nodeId, groupKeySetId](const auto & e) { return e.nodeID == nodeId && e.groupKeySetID == groupKeySetId; };
    mNodeKeySetTombstones.erase(std::remove_if(mNodeKeySetTombstones.begin(), mNodeKeySetTombstones.end(), match),
                                mNodeKeySetTombstones.end());
    return RunOrQueueNodeSync(nodeId, [this, nodeId, groupKeySetId]() { return StartNodeKeySetEntrySync(nodeId, groupKeySetId); });
}

CHIP_ERROR JointFabricDatastore::TestAddEndpointEntry(EndpointId endpointId, NodeId nodeId, CharSpan friendlyName)
{
    Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type newEntry;
    newEntry.nodeID     = nodeId;
    newEntry.endpointID = endpointId;
    SetEndpointFriendlyNameWithOwnedStorage(nodeId, endpointId, friendlyName, newEntry);

    mEndpointEntries.push_back(newEntry);

    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabricDatastore::ForceAddNodeKeySetEntry(uint16_t groupKeySetId, NodeId nodeId)
{
    datastore::NodeKeySetEntryStruct newEntry;
    newEntry.nodeID            = nodeId;
    newEntry.groupKeySetID     = groupKeySetId;
    newEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;

    mNodeKeySetEntries.push_back(newEntry);
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
