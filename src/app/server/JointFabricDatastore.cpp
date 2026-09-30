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

#include <protocols/interaction_model/StatusCode.h>

#include <algorithm>
#include <cstring>
#include <unordered_set>

namespace chip {
namespace app {

namespace {
/**
 * Validate that an input is the correct length to be an Epoch Key. Null keys are considered valid.
 */
bool EpochKeyFitsStorage(const DataModel::Nullable<ByteSpan> & key)
{
    using EpochKeyStorage = Crypto::SensitiveDataBuffer<Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES>;
    return key.IsNull() || key.Value().size() <= EpochKeyStorage::Capacity();
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
    VerifyOrReturnError(EpochKeyFitsStorage(source.epochKey0) && EpochKeyFitsStorage(source.epochKey1) &&
                            EpochKeyFitsStorage(source.epochKey2),
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
    // Over-length epoch keys are rejected by CopyGroupKeySetWithOwnedSpans before reaching here, so the
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
            [this](CHIP_ERROR err,
                   const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type> & endpoints) {
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
                [this, currentEndpointId](
                    CHIP_ERROR err,
                    const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreGroupInformationEntryStruct::Type> &
                        endpointGroups) {
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
                                Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type newEntry;
                                newEntry.nodeID            = mRefreshingNodeId;
                                newEntry.endpointID        = currentEndpointId;
                                newEntry.groupID           = static_cast<GroupId>(endpointGroup.groupID);
                                newEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                                ClearRemovalIntent(newEntry);

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
                                               if (std::any_of(endpointGroups.begin(), endpointGroups.end(),
                                                               [&](const auto & eg) { return entry.groupID == eg.groupID; }))
                                               {
                                                   return false;
                                               }
                                               ClearRemovalIntent(entry);
                                               return true;
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
        std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type> groupEntriesToSync;
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
                ClearRemovalIntent(*it);
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
            // The result applies only if the entry is still being added or removed as when the sync started.
            auto sameOperation = [this, match, removal](const auto & e) { return match(e) && HasRemovalIntent(e) == removal; };
            CHIP_ERROR syncErr = mDelegate->SyncNode(
                mRefreshingNodeId, entryToSync, [this, entryToSync, removal, sameOperation](CHIP_ERROR innerErr) {
                    if (innerErr != CHIP_NO_ERROR)
                    {
                        detail::MarkEntrySyncFailedIfFound(mEndpointGroupIDEntries, sameOperation, innerErr);
                        MarkRefreshFailed(entryToSync.nodeID, innerErr);
                        return;
                    }
                    if (removal)
                    {
                        auto erased = std::find_if(mEndpointGroupIDEntries.begin(), mEndpointGroupIDEntries.end(), sameOperation);
                        if (erased != mEndpointGroupIDEntries.end())
                        {
                            ClearRemovalIntent(*erased);
                            mEndpointGroupIDEntries.erase(erased);
                        }
                        return;
                    }
                    detail::MarkEntryCommittedIfFound(mEndpointGroupIDEntries, sameOperation);
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
                [this, currentEndpointId](
                    CHIP_ERROR err,
                    const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> &
                        endpointBindings) {
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
                                Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type newEntry;
                                newEntry.nodeID     = mRefreshingNodeId;
                                newEntry.endpointID = endpointBinding.endpointID;
                                newEntry.binding    = endpointBinding.binding;
                                if (GenerateAndAssignAUniqueListID(newEntry.listID) != CHIP_NO_ERROR)
                                {
                                    // Unable to generate a unique List ID; skip this entry.
                                    continue;
                                }
                                newEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                                ClearRemovalIntent(newEntry);

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
                                    if (std::any_of(endpointBindings.begin(), endpointBindings.end(), [&](const auto & eb) {
                                            return entry.endpointID == eb.endpointID && BindingMatches(entry.binding, eb.binding);
                                        }))
                                    {
                                        return false;
                                    }
                                    ClearRemovalIntent(entry);
                                    return true;
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
                ClearRemovalIntent(*it);
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
        CHIP_ERROR bindingSyncErr =
            mDelegate->SyncNode(mRefreshingNodeId, mRefreshingBindingEntries, [this, refreshingNodeId](CHIP_ERROR syncErr) {
                if (syncErr != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer,
                                 "Failed syncing bindings during refresh for node 0x" ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                                 ChipLogValueX64(refreshingNodeId), syncErr.Format());
                    MarkRefreshBindingsSyncFailed(refreshingNodeId, syncErr);
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
                        if (std::any_of(mRefreshingBindingEntries.begin(), mRefreshingBindingEntries.end(),
                                        [&entry](const auto & written) {
                                            return written.endpointID == entry.endpointID && written.listID == entry.listID;
                                        }))
                        {
                            entry.statusEntry.state       = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                            entry.statusEntry.failureCode = 0;
                        }
                    }

                    // The node no longer holds the entries being removed that the write left out. A removal
                    // requested while the write was in flight is still queued and runs after the refresh.
                    mEndpointBindingEntries.erase(
                        std::remove_if(mEndpointBindingEntries.begin(), mEndpointBindingEntries.end(),
                                       [this, refreshingNodeId](const auto & entry) {
                                           if (entry.nodeID != refreshingNodeId || !HasRemovalIntent(entry) ||
                                               std::any_of(mRefreshingBindingEntries.begin(), mRefreshingBindingEntries.end(),
                                                           [&entry](const auto & written) {
                                                               return written.endpointID == entry.endpointID &&
                                                                   written.listID == entry.listID;
                                                           }))
                                           {
                                               return false;
                                           }
                                           ClearRemovalIntent(entry);
                                           return true;
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
            MarkRefreshBindingsSyncFailed(refreshingNodeId, bindingSyncErr);
            mRefreshHadFailure = true;
            mRefreshState      = kFetchingGroupKeySetList;
            return ContinueRefresh();
        }
    }
    break;
    case kFetchingGroupKeySetList: {
        ReturnErrorOnFailure(
            mDelegate->FetchGroupKeySetList(mRefreshingNodeId, [this](CHIP_ERROR err, const std::vector<uint16_t> & groupKeySets) {
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
                            auto entry                    = *tombstone;
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
                [this](CHIP_ERROR err,
                       const Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet) {
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
        // 4. Ensure per-node key-set entries for each GroupKeySet are synced to devices.
        if (mRefreshingNodeKeySetDeletions.empty())
        {
            for (const auto & groupKeySet : mGroupKeySetList)
            {
                for (auto nkIt = mNodeKeySetEntries.begin(); nkIt != mNodeKeySetEntries.end();)
                {
                    if (nkIt->groupKeySetID != groupKeySet.groupKeySetID || !HasRemovalIntent(*nkIt))
                    {
                        ++nkIt;
                        continue;
                    }

                    if (IsUnrecoverableCommitFailure(nkIt->statusEntry))
                    {
                        // remove entry from the list
                        RecordTombstoneIfRemoving(*nkIt);
                        ClearRemovalIntent(*nkIt);
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

            Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type entryToRemove;
            entryToRemove.nodeID            = nodeIdToErase;
            entryToRemove.groupKeySetID     = groupKeySetIdToErase;
            entryToRemove.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;
            CHIP_ERROR syncErr =
                mDelegate->SyncNode(nodeIdToErase, entryToRemove, [this, nodeIdToErase, groupKeySetIdToErase](CHIP_ERROR innerErr) {
                    // An add that cancelled the removal while it was in flight owns the entry now.
                    auto stillRemoving = [this, nodeIdToErase, groupKeySetIdToErase](const auto & entry) {
                        return entry.nodeID == nodeIdToErase && entry.groupKeySetID == groupKeySetIdToErase &&
                            HasRemovalIntent(entry);
                    };
                    if (innerErr == CHIP_NO_ERROR)
                    {
                        auto erased = std::find_if(mNodeKeySetEntries.begin(), mNodeKeySetEntries.end(), stillRemoving);
                        if (erased != mNodeKeySetEntries.end())
                        {
                            ClearRemovalIntent(*erased);
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

        for (auto gksIt = mGroupKeySetList.begin(); gksIt != mGroupKeySetList.end(); ++gksIt)
        {
            const uint16_t groupKeySetId = gksIt->groupKeySetID;

            for (auto nkIt = mNodeKeySetEntries.begin(); nkIt != mNodeKeySetEntries.end();)
            {
                if (nkIt->groupKeySetID != groupKeySetId)
                {
                    ++nkIt;
                    continue;
                }

                // nkIt references the current groupKeySetId
                if (nkIt->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kPending)
                {
                    // Make a copy of the group key set to send to the node.
                    const NodeId entryNodeId = nkIt->nodeID;
                    auto groupKeySet         = *gksIt;
                    CHIP_ERROR syncErr =
                        mDelegate->SyncNode(nkIt->nodeID, groupKeySet, [this, entryNodeId, groupKeySetId](CHIP_ERROR innerErr) {
                            auto match = [&](const auto & e) {
                                return e.nodeID == entryNodeId && e.groupKeySetID == groupKeySetId;
                            };
                            if (innerErr != CHIP_NO_ERROR)
                            {
                                detail::MarkEntrySyncFailedIfFound(mNodeKeySetEntries, match, innerErr);
                                MarkRefreshFailed(entryNodeId, innerErr);
                                return;
                            }
                            detail::MarkEntryCommittedIfFound(mNodeKeySetEntries, match);
                        });
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
                        ClearRemovalIntent(*nkIt);
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
                        const NodeId entryNodeId = nkIt->nodeID;
                        auto groupKeySet         = *gksIt;
                        CHIP_ERROR syncErr =
                            mDelegate->SyncNode(nkIt->nodeID, groupKeySet, [this, entryNodeId, groupKeySetId](CHIP_ERROR innerErr) {
                                auto match = [&](const auto & e) {
                                    return e.nodeID == entryNodeId && e.groupKeySetID == groupKeySetId;
                                };
                                if (innerErr != CHIP_NO_ERROR)
                                {
                                    detail::MarkEntrySyncFailedIfFound(mNodeKeySetEntries, match, innerErr);
                                    MarkRefreshFailed(entryNodeId, innerErr);
                                    return;
                                }
                                detail::MarkEntryCommittedIfFound(mNodeKeySetEntries, match);
                            });
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
            [this](CHIP_ERROR err,
                   const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type> & acls) {
                if (err == CHIP_NO_ERROR)
                {
                    // A node's ACL entries carry no listID, so fetched entries are matched to datastore
                    // entries by value.
                    std::vector<uint16_t> seenListIds;
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
                            return t.nodeId == mRefreshingNodeId &&
                                (detail::AclEntryValueEquals(acl.ACLEntry, EncodeAccessControlEntry(t.value)) ||
                                 (t.supersededValue.has_value() &&
                                  detail::AclEntryValueEquals(acl.ACLEntry, EncodeAccessControlEntry(*t.supersededValue))));
                        });
                        if (tombstone != mAclTombstones.end())
                        {
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

                        // The value an entry had before its Pending update: the node has not applied the update
                        // yet. Not adopted, so the write replaces it.
                        if (std::any_of(mACLEntries.begin(), mACLEntries.end(), [this, &acl](const auto & entry) {
                                return entry.nodeID == mRefreshingNodeId && entry.supersededValue.has_value() &&
                                    detail::AclEntryValueEquals(acl.ACLEntry, EncodeAccessControlEntry(*entry.supersededValue));
                            }))
                        {
                            continue;
                        }

                        // Added to the node outside the datastore: adopted as Committed, as RefreshNode specifies.
                        if (mACLEntries.size() >= kMaxACLs)
                        {
                            ChipLogError(AppServer, "ACL list full; not adopting an ACL entry from node 0x" ChipLogFormatX64,
                                         ChipLogValueX64(mRefreshingNodeId));
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

                    // The node no longer holds the values of the remaining tombstones.
                    mAclTombstones.erase(std::remove_if(mAclTombstones.begin(), mAclTombstones.end(),
                                                        [this](const auto & t) { return t.nodeId == mRefreshingNodeId; }),
                                         mAclTombstones.end());

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
            [this, refreshingNodeId, writtenValues = std::move(writtenValues)](CHIP_ERROR innerErr) {
                if (innerErr != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer,
                                 "Failed syncing ACLs during refresh for node 0x" ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                                 ChipLogValueX64(refreshingNodeId), innerErr.Format());

                    // Keep entries for retry. The node stays Pending.
                    MarkRefreshAclsSyncFailed(refreshingNodeId, innerErr);
                    FinishRefresh(innerErr);
                    return;
                }

                for (auto & entry : mACLEntries)
                {
                    if (entry.nodeID != refreshingNodeId ||
                        (entry.statusEntry.state != Clusters::JointFabricDatastore::DatastoreStateEnum::kPending &&
                         entry.statusEntry.state != Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitFailed))
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
                        entry.statusEntry.state       = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                        entry.statusEntry.failureCode = 0;
                        entry.supersededValue.reset();
                    }
                    else
                    {
                        // Updated while the write was in flight: the node now holds the written value, which the
                        // queued sync replaces.
                        entry.supersededValue = written->second;
                    }
                }

                // The node no longer holds the entries being removed that the write left out. A removal requested
                // while the write was in flight is still queued and runs after the refresh.
                mACLEntries.erase(
                    std::remove_if(mACLEntries.begin(), mACLEntries.end(),
                                   [this, refreshingNodeId](const auto & entry) {
                                       return entry.nodeID == refreshingNodeId && HasRemovalIntent(entry) &&
                                           std::none_of(mRefreshingACLEntries.begin(), mRefreshingACLEntries.end(),
                                                        [&entry](const auto & written) { return written.listID == entry.listID; });
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
            MarkRefreshAclsSyncFailed(refreshingNodeId, syncErr);
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
            LogErrorOnFailure(UpdateNodeKeySetList(groupKeySet));

            VerifyOrReturnValue(groupKeySet.groupKeySecurityPolicy <
                                    Clusters::JointFabricDatastore::DatastoreGroupKeySecurityPolicyEnum::kUnknownEnumValue,
                                CHIP_IM_GLOBAL_STATUS(ConstraintError));

            ReturnErrorOnFailure(CopyGroupKeySetWithOwnedSpans(groupKeySet, entry));

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

CHIP_ERROR
JointFabricDatastore::UpdateNodeKeySetList(Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    bool entryUpdated = false;

    for (size_t i = 0; i < mNodeKeySetEntries.size(); ++i)
    {
        auto & entry = mNodeKeySetEntries[i];
        if (entry.groupKeySetID == groupKeySet.groupKeySetID)
        {
            if (groupKeySet.groupKeySecurityPolicy <
                Clusters::JointFabricDatastore::DatastoreGroupKeySecurityPolicyEnum::kUnknownEnumValue)
            {

                const NodeId entryNodeId          = entry.nodeID;
                const uint16_t entryGroupKeySetID = groupKeySet.groupKeySetID;
                auto match                        = [entryNodeId, entryGroupKeySetID](const auto & e) {
                    return e.nodeID == entryNodeId && e.groupKeySetID == entryGroupKeySetID;
                };
                CHIP_ERROR startErr = mDelegate->SyncNode(entry.nodeID, groupKeySet, [this, match](CHIP_ERROR syncErr) {
                    if (syncErr != CHIP_NO_ERROR)
                    {
                        detail::MarkEntrySyncFailedIfFound(mNodeKeySetEntries, match, syncErr);
                        return;
                    }
                    detail::MarkEntryCommittedIfFound(mNodeKeySetEntries, match);
                });
                if (startErr != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer, "Failed to sync group key set to node: %" CHIP_ERROR_FORMAT, startErr.Format());
                    detail::MarkEntrySyncFailedIfFound(mNodeKeySetEntries, match, startErr);
                }

                if (entryUpdated == false)
                {
                    entryUpdated = true;
                }
            }
            else
            {
                entry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitFailed;
                return CHIP_IM_GLOBAL_STATUS(ConstraintError);
            }
        }
    }

    return entryUpdated ? CHIP_NO_ERROR : CHIP_ERROR_NOT_FOUND;
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

    if (commandData.groupCAT.ValueOr(0) == kAdminCATIdentifier || commandData.groupCAT.ValueOr(0) == kAnchorCATIdentifier)
    {
        // If the group is an AdminCAT or AnchorCAT, we cannot add it
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

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

    if (mGroupInformationEntries[index].groupCAT.ValueOr(0) == kAdminCATIdentifier ||
        mGroupInformationEntries[index].groupCAT.ValueOr(0) == kAnchorCATIdentifier)
    {
        // If the group is an AdminCAT or AnchorCAT, we cannot update it
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    // Update the group entry with the new data
    if (commandData.friendlyName.IsNull() == false)
    {
        if (mGroupInformationEntries[index].friendlyName.data_equal(commandData.friendlyName.Value()) == false)
        {
            // Friendly name changed. For every endpoint that references this group, mark the endpoint's
            // GroupIDList entry as pending and attempt to push the change to the node. If the push
            // fails, the entry records CommitFailed so a subsequent Refresh can apply it.
            const GroupId updatedGroupId = commandData.groupID;
            for (size_t i = 0; i < mEndpointGroupIDEntries.size(); ++i)
            {
                auto & epGroupEntry = mEndpointGroupIDEntries[i];
                if (epGroupEntry.groupID == updatedGroupId)
                {
                    epGroupEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

                    // Make a copy to send to the node. Do not fail the entire UpdateGroup if SyncNode
                    // returns an error; the entry records CommitFailed for a later refresh per spec.
                    auto entryToSync = epGroupEntry;

                    const NodeId entryNodeId         = epGroupEntry.nodeID;
                    const EndpointId entryEndpointId = epGroupEntry.endpointID;
                    auto match                       = [entryNodeId, entryEndpointId, updatedGroupId](const auto & e) {
                        return e.nodeID == entryNodeId && e.endpointID == entryEndpointId && e.groupID == updatedGroupId;
                    };
                    CHIP_ERROR syncErr =
                        mDelegate->SyncNode(epGroupEntry.nodeID, entryToSync, [this, match](CHIP_ERROR callbackErr) {
                            if (callbackErr != CHIP_NO_ERROR)
                            {
                                detail::MarkEntrySyncFailedIfFound(mEndpointGroupIDEntries, match, callbackErr);
                                return;
                            }
                            detail::MarkEntryCommittedIfFound(mEndpointGroupIDEntries, match);
                        });

                    if (syncErr != CHIP_NO_ERROR)
                    {
                        ChipLogError(DataManagement, "Failed to sync node for group friendly name update: %" CHIP_ERROR_FORMAT,
                                     syncErr.Format());
                        detail::MarkEntrySyncFailedIfFound(mEndpointGroupIDEntries, match, syncErr);
                    }
                }
            }

            // Update the friendly name in the datastore
            SetGroupInformationFriendlyNameWithOwnedStorage(static_cast<GroupId>(mGroupInformationEntries[index].groupID),
                                                            commandData.friendlyName.Value(), mGroupInformationEntries[index]);
        }
    }
    if (commandData.groupKeySetID.IsNull() == false)
    {
        if (mGroupInformationEntries[index].groupKeySetID.IsNull() ||
            mGroupInformationEntries[index].groupKeySetID.Value() != commandData.groupKeySetID.Value())
        {
            // If the groupKeySetID is being updated, we need to ensure that the new key set exists
            ReturnErrorOnFailure(AddNodeKeySetEntry(commandData.groupID, commandData.groupKeySetID.Value()));
            if (!mGroupInformationEntries[index].groupKeySetID.IsNull())
            {
                LogErrorOnFailure(RemoveNodeKeySetEntry(
                    commandData.groupID, mGroupInformationEntries[index].groupKeySetID.Value())); // Remove the old key set
            }
        }
        mGroupInformationEntries[index].groupKeySetID = commandData.groupKeySetID;
    }

    bool anyGroupCATFieldUpdated = false;

    if (commandData.groupCAT.IsNull() == false)
    {
        if (mGroupInformationEntries[index].groupCAT.IsNull() ||
            mGroupInformationEntries[index].groupCAT.Value() != commandData.groupCAT.Value())
        {
            anyGroupCATFieldUpdated = true;
        }
        // Update the groupCAT
        mGroupInformationEntries[index].groupCAT = commandData.groupCAT;
    }
    if (commandData.groupCATVersion.IsNull() == false)
    {
        if (mGroupInformationEntries[index].groupCATVersion.IsNull() ||
            mGroupInformationEntries[index].groupCATVersion.Value() != commandData.groupCATVersion.Value())
        {
            anyGroupCATFieldUpdated = true;
        }
        mGroupInformationEntries[index].groupCATVersion = commandData.groupCATVersion;
    }
    if (commandData.groupPermission.IsNull() == false &&
        commandData.groupPermission.Value() !=
            Clusters::JointFabricDatastore::DatastoreAccessControlEntryPrivilegeEnum::kUnknownEnumValue)
    {
        if (mGroupInformationEntries[index].groupPermission != commandData.groupPermission.Value())
        {
            anyGroupCATFieldUpdated = true;
        }
        // If the groupPermission is not set to kUnknownEnumValue, update it
        mGroupInformationEntries[index].groupPermission = commandData.groupPermission.Value();
    }

    if (anyGroupCATFieldUpdated)
    {
        const GroupId updatedGroupId = commandData.groupID;
        std::vector<std::pair<NodeId, uint16_t>> updatedAcls;

        for (size_t i = 0; i < mACLEntries.size(); ++i)
        {
            auto & acl = mACLEntries[i];

            // Determine if this ACL entry references the updated group
            bool referencesGroup = false;
            for (const auto & subject : acl.ACLEntry.subjects)
            {
                // If the target has a group field and it matches the updated group, mark for update.
                // Use IsNull() to match other usages in this file.
                if (subject == static_cast<uint64_t>(updatedGroupId))
                {
                    referencesGroup = true;
                    break;
                }
            }

            if (!referencesGroup)
            {
                continue;
            }

            // Update the ACL entry in the datastore to reflect the new group permission and mark Pending. The node
            // holds the last committed value until the update reaches it; keep that value so the update replaces it.
            if (!acl.supersededValue.has_value())
            {
                acl.supersededValue = acl.ACLEntry;
            }
            acl.ACLEntry.privilege = mGroupInformationEntries[index].groupPermission;
            acl.statusEntry.state  = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;
            updatedAcls.emplace_back(acl.nodeID, acl.listID);
        }

        // Sync after the loop: a sync that completes synchronously can erase entries.
        CHIP_ERROR firstErr = CHIP_NO_ERROR;
        for (const auto & [aclNodeId, aclListId] : updatedAcls)
        {
            CHIP_ERROR err = RunOrQueueNodeSync(
                aclNodeId, [this, nodeId = aclNodeId, listId = aclListId]() { return StartAclEntrySync(nodeId, listId); });
            if (firstErr == CHIP_NO_ERROR)
            {
                firstErr = err;
            }
        }
        ReturnErrorOnFailure(firstErr);
    }

    return CHIP_NO_ERROR;
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

    Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type newGroupEntry;
    auto existing = std::find_if(mEndpointGroupIDEntries.begin(), mEndpointGroupIDEntries.end(), groupMatch);
    if (existing != mEndpointGroupIDEntries.end())
    {
        if (!HasRemovalIntent(*existing))
        {
            return CHIP_NO_ERROR;
        }

        // Adding an entry that is being removed cancels the removal.
        ClearRemovalIntent(*existing);
        existing->statusEntry.state       = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;
        existing->statusEntry.failureCode = 0;
        newGroupEntry                     = *existing;
    }
    else
    {
        VerifyOrReturnError(mEndpointGroupIDEntries.size() < kMaxGroups, CHIP_ERROR_NO_MEMORY);

        // Create a new endpoint group ID entry
        newGroupEntry.nodeID            = nodeId;
        newGroupEntry.endpointID        = endpointId;
        newGroupEntry.groupID           = groupId;
        newGroupEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

        // Add the new endpoint-group entry to the datastore.
        ClearRemovalIntent(newGroupEntry);
        mEndpointGroupIDEntries.push_back(newGroupEntry);
    }
    mEndpointGroupTombstones.erase(std::remove_if(mEndpointGroupTombstones.begin(), mEndpointGroupTombstones.end(), groupMatch),
                                   mEndpointGroupTombstones.end());

    // Ensure the node has the required keyset before issuing AddGroup on the target endpoint.
    if (mGroupInformationEntries[index].groupKeySetID.IsNull() == false)
    {
        uint16_t groupKeySetID = mGroupInformationEntries[index].groupKeySetID.Value();

        auto keySetMatch = [nodeId, groupKeySetID](const auto & entry) {
            return entry.nodeID == nodeId && entry.groupKeySetID == groupKeySetID;
        };

        bool nodeKeySetNeeded = true;
        Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type newNodeKeySet;
        auto existingKeySet = std::find_if(mNodeKeySetEntries.begin(), mNodeKeySetEntries.end(), keySetMatch);
        if (existingKeySet != mNodeKeySetEntries.end())
        {
            if (HasRemovalIntent(*existingKeySet))
            {
                // Adding an entry that is being removed cancels the removal.
                ClearRemovalIntent(*existingKeySet);
                existingKeySet->statusEntry.state       = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;
                existingKeySet->statusEntry.failureCode = 0;
                newNodeKeySet                           = *existingKeySet;
            }
            else
            {
                nodeKeySetNeeded = false;
            }
        }
        else
        {
            newNodeKeySet.nodeID            = nodeId;
            newNodeKeySet.groupKeySetID     = groupKeySetID;
            newNodeKeySet.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

            ClearRemovalIntent(newNodeKeySet);
            mNodeKeySetEntries.push_back(newNodeKeySet);
        }

        if (nodeKeySetNeeded)
        {
            mNodeKeySetTombstones.erase(std::remove_if(mNodeKeySetTombstones.begin(), mNodeKeySetTombstones.end(), keySetMatch),
                                        mNodeKeySetTombstones.end());

            CHIP_ERROR keySetStartErr = mDelegate->SyncNode(
                nodeId, newNodeKeySet, [this, nodeId, newGroupEntry, groupMatch, keySetMatch](CHIP_ERROR syncErr) {
                    if (syncErr != CHIP_NO_ERROR)
                    {
                        detail::MarkEntrySyncFailedIfFound(mNodeKeySetEntries, keySetMatch, syncErr);
                        return;
                    }
                    // Keep the NodeKeySet entry pending here. It is intended to be finalized by the
                    // refresh flow after mirrored state is observed, not immediately on this write.

                    CHIP_ERROR err = mDelegate->SyncNode(nodeId, newGroupEntry, [this, groupMatch](CHIP_ERROR groupSyncErr) {
                        if (groupSyncErr != CHIP_NO_ERROR)
                        {
                            detail::MarkEntrySyncFailedIfFound(mEndpointGroupIDEntries, groupMatch, groupSyncErr);
                            return;
                        }
                        detail::MarkEntryCommittedIfFound(mEndpointGroupIDEntries, groupMatch);
                    });
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(DataManagement, "Failed to sync endpoint group after keyset sync: %" CHIP_ERROR_FORMAT,
                                     err.Format());
                        detail::MarkEntrySyncFailedIfFound(mEndpointGroupIDEntries, groupMatch, err);
                    }
                });
            if (keySetStartErr != CHIP_NO_ERROR)
            {
                detail::MarkEntrySyncFailedIfFound(mNodeKeySetEntries, keySetMatch, keySetStartErr);
            }
            return keySetStartErr;
        }
    }

    CHIP_ERROR startErr = mDelegate->SyncNode(nodeId, newGroupEntry, [this, groupMatch](CHIP_ERROR syncErr) {
        if (syncErr != CHIP_NO_ERROR)
        {
            detail::MarkEntrySyncFailedIfFound(mEndpointGroupIDEntries, groupMatch, syncErr);
            return;
        }
        detail::MarkEntryCommittedIfFound(mEndpointGroupIDEntries, groupMatch);
    });
    if (startErr != CHIP_NO_ERROR)
    {
        detail::MarkEntrySyncFailedIfFound(mEndpointGroupIDEntries, groupMatch, startErr);
    }
    return startErr;
}

CHIP_ERROR JointFabricDatastore::RemoveGroupIDFromEndpointForNode(NodeId nodeId, chip::EndpointId endpointId, chip::GroupId groupId)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    size_t index = 0;
    ReturnErrorOnFailure(IsNodeIdAndEndpointInEndpointInformationEntries(nodeId, endpointId, index));

    for (auto it = mEndpointGroupIDEntries.begin(); it != mEndpointGroupIDEntries.end(); ++it)
    {
        if (it->nodeID == nodeId && it->endpointID == endpointId && it->groupID == groupId)
        {
            MarkRemovalRequested(*it);
            const auto erasedNodeId     = it->nodeID;
            const auto erasedEndpointId = it->endpointID;
            const auto erasedGroupId    = it->groupID;
            auto groupMatch             = [erasedNodeId, erasedEndpointId, erasedGroupId](const auto & entry) {
                return entry.nodeID == erasedNodeId && entry.endpointID == erasedEndpointId && entry.groupID == erasedGroupId;
            };
            // An add that cancelled the removal while it was in flight owns the entry now.
            auto stillRemoving = [this, groupMatch](const auto & entry) { return groupMatch(entry) && HasRemovalIntent(entry); };
            CHIP_ERROR groupStartErr = mDelegate->SyncNode(nodeId, *it, [this, stillRemoving](CHIP_ERROR syncErr) {
                if (syncErr != CHIP_NO_ERROR)
                {
                    detail::MarkEntrySyncFailedIfFound(mEndpointGroupIDEntries, stillRemoving, syncErr);
                    return;
                }
                auto eraseIt = std::find_if(mEndpointGroupIDEntries.begin(), mEndpointGroupIDEntries.end(), stillRemoving);
                if (eraseIt != mEndpointGroupIDEntries.end())
                {
                    ClearRemovalIntent(*eraseIt);
                    mEndpointGroupIDEntries.erase(eraseIt);
                }
            });
            if (groupStartErr != CHIP_NO_ERROR)
            {
                detail::MarkEntrySyncFailedIfFound(mEndpointGroupIDEntries, groupMatch, groupStartErr);
                return groupStartErr;
            }

            if (IsGroupIDInDatastore(groupId, index) == CHIP_NO_ERROR)
            {
                for (auto it2 = mNodeKeySetEntries.begin(); it2 != mNodeKeySetEntries.end(); ++it2)
                {
                    if (it2->nodeID == nodeId && mGroupInformationEntries[index].groupKeySetID.IsNull() == false &&
                        it2->groupKeySetID == mGroupInformationEntries[index].groupKeySetID.Value())
                    {
                        MarkRemovalRequested(*it2);
                        const auto erasedKeySetNodeId  = it2->nodeID;
                        const auto erasedKeySetGroupId = it2->groupKeySetID;
                        auto keySetMatch               = [erasedKeySetNodeId, erasedKeySetGroupId](const auto & entry) {
                            return entry.nodeID == erasedKeySetNodeId && entry.groupKeySetID == erasedKeySetGroupId;
                        };
                        auto keySetStillRemoving = [this, keySetMatch](const auto & entry) {
                            return keySetMatch(entry) && HasRemovalIntent(entry);
                        };
                        CHIP_ERROR keySetStartErr =
                            mDelegate->SyncNode(nodeId, *it2, [this, keySetStillRemoving](CHIP_ERROR syncErr) {
                                if (syncErr != CHIP_NO_ERROR)
                                {
                                    detail::MarkEntrySyncFailedIfFound(mNodeKeySetEntries, keySetStillRemoving, syncErr);
                                    return;
                                }
                                auto eraseIt =
                                    std::find_if(mNodeKeySetEntries.begin(), mNodeKeySetEntries.end(), keySetStillRemoving);
                                if (eraseIt != mNodeKeySetEntries.end())
                                {
                                    ClearRemovalIntent(*eraseIt);
                                    mNodeKeySetEntries.erase(eraseIt);
                                }
                            });
                        if (keySetStartErr != CHIP_NO_ERROR)
                        {
                            detail::MarkEntrySyncFailedIfFound(mNodeKeySetEntries, keySetMatch, keySetStartErr);
                            return keySetStartErr;
                        }

                        break;
                    }
                }
            }

            return CHIP_NO_ERROR;
        }
    }

    return CHIP_IM_GLOBAL_STATUS(NotFound);
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

    Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type newBindingEntry;
    auto existing = std::find_if(mEndpointBindingEntries.begin(), mEndpointBindingEntries.end(), [&](const auto & entry) {
        return entry.nodeID == nodeId && entry.endpointID == endpointId && BindingMatches(entry.binding, binding);
    });
    if (existing != mEndpointBindingEntries.end())
    {
        if (!HasRemovalIntent(*existing))
        {
            return CHIP_NO_ERROR;
        }

        // Adding an entry that is being removed cancels the removal.
        ClearRemovalIntent(*existing);
        existing->statusEntry.state       = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;
        existing->statusEntry.failureCode = 0;
        newBindingEntry                   = *existing;
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
        ClearRemovalIntent(newBindingEntry);
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

void JointFabricDatastore::MarkRemovalRequested(datastore::ACLEntryStruct & entry)
{
    entry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;
    entry.pendingRemoval    = true;
}

void JointFabricDatastore::MarkRemovalRequested(
    Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type & entry)
{
    entry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;
    mBindingRemovalIntents.emplace(entry.nodeID, entry.endpointID, entry.listID);
}

void JointFabricDatastore::MarkRemovalRequested(
    Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type & entry)
{
    entry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;
    mEndpointGroupRemovalIntents.emplace(entry.nodeID, entry.endpointID, entry.groupID);
}

void JointFabricDatastore::MarkRemovalRequested(
    Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type & entry)
{
    entry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;
    mNodeKeySetRemovalIntents.emplace(entry.nodeID, entry.groupKeySetID);
}

bool JointFabricDatastore::HasRemovalIntent(const datastore::ACLEntryStruct & entry) const
{
    return entry.statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending || entry.pendingRemoval;
}

bool JointFabricDatastore::HasRemovalIntent(
    const Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type & entry) const
{
    return entry.statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending ||
        mBindingRemovalIntents.count(std::make_tuple(entry.nodeID, entry.endpointID, entry.listID)) != 0;
}

bool JointFabricDatastore::HasRemovalIntent(
    const Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type & entry) const
{
    return entry.statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending ||
        mEndpointGroupRemovalIntents.count(std::make_tuple(entry.nodeID, entry.endpointID, entry.groupID)) != 0;
}

bool JointFabricDatastore::HasRemovalIntent(
    const Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type & entry) const
{
    return entry.statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending ||
        mNodeKeySetRemovalIntents.count(std::make_pair(entry.nodeID, entry.groupKeySetID)) != 0;
}

void JointFabricDatastore::ClearRemovalIntent(
    const Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type & entry)
{
    mBindingRemovalIntents.erase(std::make_tuple(entry.nodeID, entry.endpointID, entry.listID));
}

void JointFabricDatastore::ClearRemovalIntent(
    const Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type & entry)
{
    mEndpointGroupRemovalIntents.erase(std::make_tuple(entry.nodeID, entry.endpointID, entry.groupID));
}

void JointFabricDatastore::ClearRemovalIntent(
    const Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type & entry)
{
    mNodeKeySetRemovalIntents.erase(std::make_pair(entry.nodeID, entry.groupKeySetID));
}

void JointFabricDatastore::RecordTombstoneIfRemoving(const datastore::ACLEntryStruct & entry)
{
    if (HasRemovalIntent(entry))
    {
        PushTombstone(mAclTombstones, AclTombstone{ entry.nodeID, entry.ACLEntry, entry.supersededValue }, kMaxACLs);
    }
}

void JointFabricDatastore::RecordTombstoneIfRemoving(
    const Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type & entry)
{
    if (HasRemovalIntent(entry))
    {
        PushTombstone(mBindingTombstones, entry, kMaxGroups);
    }
}

void JointFabricDatastore::RecordTombstoneIfRemoving(
    const Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type & entry)
{
    if (HasRemovalIntent(entry))
    {
        PushTombstone(mEndpointGroupTombstones, entry, kMaxGroups);
    }
}

void JointFabricDatastore::RecordTombstoneIfRemoving(
    const Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type & entry)
{
    if (HasRemovalIntent(entry))
    {
        PushTombstone(mNodeKeySetTombstones, entry, kMaxGroupKeySet);
    }
}

void JointFabricDatastore::MarkRefreshBindingsSyncFailed(NodeId nodeId, CHIP_ERROR err)
{
    for (auto & entry : mEndpointBindingEntries)
    {
        if (entry.nodeID != nodeId)
        {
            continue;
        }
        if (HasRemovalIntent(entry))
        {
            // Record the intent before the state stops saying DeletePending.
            MarkRemovalRequested(entry);
            detail::MarkEntrySyncFailed(entry, err);
            continue;
        }
        const bool inWrite =
            std::any_of(mRefreshingBindingEntries.begin(), mRefreshingBindingEntries.end(), [&entry](const auto & written) {
                return written.endpointID == entry.endpointID && written.listID == entry.listID;
            });
        if (inWrite && entry.statusEntry.state != Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted)
        {
            detail::MarkEntrySyncFailed(entry, err);
        }
    }
}

void JointFabricDatastore::MarkRefreshAclsSyncFailed(NodeId nodeId, CHIP_ERROR err)
{
    for (auto & entry : mACLEntries)
    {
        if (entry.nodeID != nodeId)
        {
            continue;
        }
        if (HasRemovalIntent(entry))
        {
            // Record the intent before the state stops saying DeletePending.
            MarkRemovalRequested(entry);
            detail::MarkEntrySyncFailed(entry, err);
            continue;
        }
        const bool inWrite = std::any_of(mRefreshingACLEntries.begin(), mRefreshingACLEntries.end(),
                                         [&entry](const auto & written) { return written.listID == entry.listID; });
        if (inWrite && entry.statusEntry.state != Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted)
        {
            detail::MarkEntrySyncFailed(entry, err);
        }
    }
}

CHIP_ERROR JointFabricDatastore::RunOrQueueNodeSync(NodeId nodeId, std::function<CHIP_ERROR()> start)
{
    auto & queue = mNodeSyncQueues[nodeId];
    if (queue.inFlight)
    {
        VerifyOrReturnError(queue.waiting.size() < kMaxACLs, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
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
    auto sameOperation  = [this, match, removal](const auto & entry) { return match(entry) && HasRemovalIntent(entry) == removal; };
    CHIP_ERROR startErr = mDelegate->SyncNode(
        nodeId, payload, superseded, [this, nodeId, sameOperation, removal, sentValue = it->ACLEntry](CHIP_ERROR syncErr) {
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
                auto entry = std::find_if(mACLEntries.begin(), mACLEntries.end(), sameOperation);
                if (entry != mACLEntries.end())
                {
                    if (detail::AclEntryValueEquals(EncodeAccessControlEntry(entry->ACLEntry), EncodeAccessControlEntry(sentValue)))
                    {
                        entry->statusEntry.state       = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                        entry->statusEntry.failureCode = 0;
                        entry->supersededValue.reset();
                    }
                    else
                    {
                        // Updated again while this sync was in flight: the node now holds the value sent
                        // here, which the queued sync replaces.
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

CHIP_ERROR JointFabricDatastore::StartBindingEntrySync(NodeId nodeId, EndpointId endpointId, uint16_t listId)
{
    auto match = [nodeId, endpointId, listId](const auto & entry) {
        return entry.nodeID == nodeId && entry.endpointID == endpointId && entry.listID == listId;
    };
    auto it = std::find_if(mEndpointBindingEntries.begin(), mEndpointBindingEntries.end(), match);
    if (it == mEndpointBindingEntries.end() ||
        (!HasRemovalIntent(*it) && it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted))
    {
        FinishNodeSync(nodeId);
        return CHIP_NO_ERROR;
    }

    const bool removal        = HasRemovalIntent(*it);
    auto payload              = *it;
    payload.statusEntry.state = removal ? Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending
                                        : Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

    // The result applies only if the entry is still being added or removed as when the sync started.
    auto sameOperation  = [this, match, removal](const auto & entry) { return match(entry) && HasRemovalIntent(entry) == removal; };
    CHIP_ERROR startErr = mDelegate->SyncNode(nodeId, payload, [this, nodeId, sameOperation, removal](CHIP_ERROR syncErr) {
        if (syncErr != CHIP_NO_ERROR)
        {
            detail::MarkEntrySyncFailedIfFound(mEndpointBindingEntries, sameOperation, syncErr);
        }
        else if (removal)
        {
            auto entry = std::find_if(mEndpointBindingEntries.begin(), mEndpointBindingEntries.end(), sameOperation);
            if (entry != mEndpointBindingEntries.end())
            {
                ClearRemovalIntent(*entry);
                mEndpointBindingEntries.erase(entry);
            }
        }
        else
        {
            detail::MarkEntryCommittedIfFound(mEndpointBindingEntries, sameOperation);
        }
        FinishNodeSync(nodeId);
    });
    if (startErr != CHIP_NO_ERROR)
    {
        detail::MarkEntrySyncFailedIfFound(mEndpointBindingEntries, sameOperation, startErr);
        FinishNodeSync(nodeId);
    }
    return startErr;
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

    // Operations queued behind the refresh start now.
    FinishNodeSync(finishedNodeId);
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

            // Adding an entry that is being removed cancels the removal.
            entry.pendingRemoval          = false;
            entry.statusEntry.state       = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;
            entry.statusEntry.failureCode = 0;
            storedEntry                   = &entry;
            break;
        }
    }

    if (storedEntry == nullptr)
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
                       [nodeId, &addedValue](const auto & t) {
                           return t.nodeId == nodeId &&
                               (detail::AclEntryValueEquals(EncodeAccessControlEntry(t.value), addedValue) ||
                                (t.supersededValue.has_value() &&
                                 detail::AclEntryValueEquals(EncodeAccessControlEntry(*t.supersededValue), addedValue)));
                       }),
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
            MarkRemovalRequested(*it);
            return RunOrQueueNodeSync(nodeId, [this, nodeId, listId]() { return StartAclEntrySync(nodeId, listId); });
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR JointFabricDatastore::AddNodeKeySetEntry(GroupId groupId, uint16_t groupKeySetId)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Find all nodes that are members of this group
    std::unordered_set<NodeId> nodesInGroup;
    for (const auto & entry : mEndpointGroupIDEntries)
    {
        if (entry.groupID == groupId)
        {
            nodesInGroup.insert(entry.nodeID);
        }
    }

    if (!nodesInGroup.empty())
    {
        for (const auto nodeId : nodesInGroup)
        {
            auto match = [nodeId, groupKeySetId](const auto & e) { return e.nodeID == nodeId && e.groupKeySetID == groupKeySetId; };

            // Skip if a matching NodeKeySet entry already exists for this node, unless it is being removed:
            // adding an entry that is being removed cancels the removal.
            Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type newEntry;
            auto existing = std::find_if(mNodeKeySetEntries.begin(), mNodeKeySetEntries.end(), match);
            if (existing != mNodeKeySetEntries.end())
            {
                if (!HasRemovalIntent(*existing))
                {
                    continue;
                }
                ClearRemovalIntent(*existing);
                existing->statusEntry.state       = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;
                existing->statusEntry.failureCode = 0;
                newEntry                          = *existing;
            }
            else
            {
                newEntry.nodeID            = nodeId;
                newEntry.groupKeySetID     = groupKeySetId;
                newEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

                ClearRemovalIntent(newEntry);
                mNodeKeySetEntries.push_back(newEntry);
            }
            mNodeKeySetTombstones.erase(std::remove_if(mNodeKeySetTombstones.begin(), mNodeKeySetTombstones.end(), match),
                                        mNodeKeySetTombstones.end());

            // Sync to the node and mark committed on success. Re-resolve by stable key inside the
            // completion; capturing the index would mark the wrong/invalid slot if an interleaved
            // Invoke mutated the vector before the async completion fires.
            CHIP_ERROR startErr = mDelegate->SyncNode(nodeId, newEntry, [this, match](CHIP_ERROR syncErr) {
                if (syncErr != CHIP_NO_ERROR)
                {
                    detail::MarkEntrySyncFailedIfFound(mNodeKeySetEntries, match, syncErr);
                    return;
                }
                detail::MarkEntryCommittedIfFound(mNodeKeySetEntries, match);
            });
            if (startErr != CHIP_NO_ERROR)
            {
                detail::MarkEntrySyncFailedIfFound(mNodeKeySetEntries, match, startErr);
                return startErr;
            }
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabricDatastore::RemoveNodeKeySetEntry(GroupId groupId, uint16_t groupKeySetId)
{
    // NOTE: this method assumes its ok to remove the keyset from each node (its not in use by any group)

    // Find all nodes that are members of this group
    std::unordered_set<NodeId> nodesInGroup;
    for (const auto & entry : mEndpointGroupIDEntries)
    {
        if (entry.groupID == groupId)
        {
            nodesInGroup.insert(entry.nodeID);
        }
    }

    for (auto it = mNodeKeySetEntries.begin(); it != mNodeKeySetEntries.end(); ++it)
    {
        for (const auto & nodeId : nodesInGroup)
        {
            if (it->nodeID == nodeId && it->groupKeySetID == groupKeySetId)
            {
                MarkRemovalRequested(*it);
                const auto entryToRemove = *it;

                auto nodeIdToErase        = it->nodeID;
                auto groupKeySetIdToErase = it->groupKeySetID;
                auto match                = [nodeIdToErase, groupKeySetIdToErase](const auto & entry) {
                    return entry.nodeID == nodeIdToErase && entry.groupKeySetID == groupKeySetIdToErase;
                };
                // An add that cancelled the removal while it was in flight owns the entry now.
                auto stillRemoving  = [this, match](const auto & entry) { return match(entry) && HasRemovalIntent(entry); };
                CHIP_ERROR startErr = mDelegate->SyncNode(nodeId, entryToRemove, [this, stillRemoving](CHIP_ERROR syncErr) {
                    if (syncErr != CHIP_NO_ERROR)
                    {
                        detail::MarkEntrySyncFailedIfFound(mNodeKeySetEntries, stillRemoving, syncErr);
                        return;
                    }
                    auto eraseIt = std::find_if(mNodeKeySetEntries.begin(), mNodeKeySetEntries.end(), stillRemoving);
                    if (eraseIt != mNodeKeySetEntries.end())
                    {
                        ClearRemovalIntent(*eraseIt);
                        mNodeKeySetEntries.erase(eraseIt);
                    }
                });
                if (startErr != CHIP_NO_ERROR)
                {
                    detail::MarkEntrySyncFailedIfFound(mNodeKeySetEntries, match, startErr);
                    return startErr;
                }

                return CHIP_NO_ERROR;
            }
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR JointFabricDatastore::TestAddNodeKeySetEntry(GroupId groupId, uint16_t groupKeySetId, NodeId nodeId)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type newEntry;
    newEntry.nodeID            = nodeId;
    newEntry.groupKeySetID     = groupKeySetId;
    newEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

    ClearRemovalIntent(newEntry);
    mNodeKeySetEntries.push_back(newEntry);

    // Sync to the node and mark committed on success. Re-resolve by stable key inside the completion
    // rather than capturing the index, which an interleaved Invoke could invalidate.
    auto match = [nodeId, groupKeySetId](const auto & e) { return e.nodeID == nodeId && e.groupKeySetID == groupKeySetId; };
    mNodeKeySetTombstones.erase(std::remove_if(mNodeKeySetTombstones.begin(), mNodeKeySetTombstones.end(), match),
                                mNodeKeySetTombstones.end());
    CHIP_ERROR startErr = mDelegate->SyncNode(nodeId, newEntry, [this, match](CHIP_ERROR syncErr) {
        if (syncErr != CHIP_NO_ERROR)
        {
            detail::MarkEntrySyncFailedIfFound(mNodeKeySetEntries, match, syncErr);
            return;
        }
        detail::MarkEntryCommittedIfFound(mNodeKeySetEntries, match);
    });
    if (startErr != CHIP_NO_ERROR)
    {
        detail::MarkEntrySyncFailedIfFound(mNodeKeySetEntries, match, startErr);
    }
    return startErr;
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
    Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type newEntry;
    newEntry.nodeID            = nodeId;
    newEntry.groupKeySetID     = groupKeySetId;
    newEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;

    ClearRemovalIntent(newEntry);
    mNodeKeySetEntries.push_back(newEntry);
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
