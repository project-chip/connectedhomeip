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

#include <algorithm>
#include <unordered_set>

namespace chip {
namespace app {

void JointFabricDatastore::CopyGroupKeySetWithOwnedSpans(
    const Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & source,
    Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & destination)
{
    auto & storage = mGroupKeySetStorage[source.groupKeySetID];

    destination.groupKeySetID           = source.groupKeySetID;
    destination.groupKeySecurityPolicy  = source.groupKeySecurityPolicy;
    destination.groupKeyMulticastPolicy = source.groupKeyMulticastPolicy;

    CopyByteSpanWithOwnedStorage(source.epochKey0, storage.epochKey0, destination.epochKey0);
    CopyByteSpanWithOwnedStorage(source.epochKey1, storage.epochKey1, destination.epochKey1);
    CopyByteSpanWithOwnedStorage(source.epochKey2, storage.epochKey2, destination.epochKey2);

    CopyNullableValue(source.epochStartTime0, destination.epochStartTime0);
    CopyNullableValue(source.epochStartTime1, destination.epochStartTime1);
    CopyNullableValue(source.epochStartTime2, destination.epochStartTime2);
}

void JointFabricDatastore::RemoveGroupKeySetStorage(uint16_t groupKeySetId)
{
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

    storage.icac.assign(icac.data(), icac.data() + icac.size());
    destination.icac = ByteSpan(storage.icac.data(), storage.icac.size());

    return CHIP_NO_ERROR;
}

void JointFabricDatastore::RemoveAdminEntryStorage(NodeId nodeId)
{
    mAdminEntryStorage.erase(nodeId);
}

void JointFabricDatastore::CopyByteSpanWithOwnedStorage(const DataModel::Nullable<ByteSpan> & source,
                                                        std::vector<uint8_t> & storage, DataModel::Nullable<ByteSpan> & destination)
{
    if (!source.IsNull())
    {
        storage.assign(source.Value().data(), source.Value().data() + source.Value().size());
        destination = ByteSpan(storage.data(), storage.size());
    }
    else
    {
        storage.clear();
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

    return CHIP_ERROR_NOT_FOUND;
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

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR JointFabricDatastore::RefreshNode(NodeId nodeId)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mRefreshingNodeId == kUndefinedNodeId, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mRefreshState == kIdle, CHIP_ERROR_INCORRECT_STATE);

    mRefreshingNodeId = nodeId;

    ReturnErrorOnFailure(ContinueRefresh());

    return CHIP_NO_ERROR;
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
                    mRefreshingNodeId = kUndefinedNodeId;
                    mRefreshState     = kIdle;
                    return;
                }

                // Continue the state machine (will enter kRefreshingEndpoints branch
                // when successful and process mRefreshingEndpointsList).
                if (ContinueRefresh() != CHIP_NO_ERROR)
                {
                    // Ignore errors in continuation from within the callback.
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
                               return !std::any_of(mRefreshingEndpointsList.begin(), mRefreshingEndpointsList.end(),
                                                   [&](const auto & endpoint) { return entry.endpointID == endpoint.endpointID; });
                           }),
            mEndpointEntries.end());

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
                                                   entry.statusEntry.state !=
                                                       Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending)
                                               {
                                                   return false;
                                               }
                                               return !std::any_of(endpointGroups.begin(), endpointGroups.end(),
                                                                   [&](const auto & eg) { return entry.groupID == eg.groupID; });
                                           }),
                            mEndpointGroupIDEntries.end());

                        // Move to the next endpoint
                        mRefreshingEndpointIndex++;
                    }
                    else
                    {
                        // Leave node as pending but tear down the refresh state.
                        mRefreshingNodeId = kUndefinedNodeId;
                        mRefreshState     = kIdle;
                        return;
                    }

                    // Continue to process next endpoint or move to syncing phase
                    if (ContinueRefresh() != CHIP_NO_ERROR)
                    {
                        // Ignore errors in continuation from within the callback.
                    }
                }));

            // Return here - the callback will call ContinueRefresh() again
            return CHIP_NO_ERROR;
        }

        // All endpoints processed; now sync any pending/delete-pending entries
        for (auto it = mEndpointGroupIDEntries.begin(); it != mEndpointGroupIDEntries.end();)
        {
            if (it->nodeID == mRefreshingNodeId)
            {
                if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kPending)
                {
                    size_t idx       = static_cast<size_t>(std::distance(mEndpointGroupIDEntries.begin(), it));
                    auto entryToSync = *it;
                    ReturnErrorOnFailure(mDelegate->SyncNode(mRefreshingNodeId, entryToSync, [this, idx]() {
                        if (idx < mEndpointGroupIDEntries.size())
                        {
                            mEndpointGroupIDEntries[idx].statusEntry.state =
                                Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                        }
                    }));
                }
                else if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending)
                {
                    Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type endpointGroupIdNullEntry{
                        0
                    };

                    auto entryToErase = *it;
                    ReturnErrorOnFailure(mDelegate->SyncNode(mRefreshingNodeId, endpointGroupIdNullEntry, [this, entryToErase]() {
                        mEndpointGroupIDEntries.erase(std::remove_if(mEndpointGroupIDEntries.begin(), mEndpointGroupIDEntries.end(),
                                                                     [&](const auto & entry) {
                                                                         return entry.nodeID == entryToErase.nodeID &&
                                                                             entry.endpointID == entryToErase.endpointID &&
                                                                             entry.groupID == entryToErase.groupID;
                                                                     }),
                                                      mEndpointGroupIDEntries.end());
                    }));
                }
                else if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitFailed)
                {
                    CHIP_ERROR failureCode(it->statusEntry.failureCode);

                    if (failureCode == CHIP_IM_GLOBAL_STATUS(ConstraintError) ||
                        failureCode == CHIP_IM_GLOBAL_STATUS(ResourceExhausted))
                    {
                        ++it;
                        continue;
                    }

                    // Retry or handle failure - for now skip
                    ++it;
                    continue;
                }
            }

            ++it;
        }

        // Start fetching groups from the first endpoint
        mRefreshingEndpointIndex = 0;
        mRefreshState            = kRefreshingBindings;

        // Fall through to kRefreshingGroups to start fetching
        return ContinueRefresh();
    }
    break;

    case kRefreshingBindings: {
        // Check if we still have endpoints to process for group fetching
        if (mRefreshingEndpointIndex < mRefreshingEndpointsList.size())
        {
            // Fetch group list for the current endpoint
            EndpointId currentEndpointId = mRefreshingEndpointsList[mRefreshingEndpointIndex].endpointID;

            ReturnErrorOnFailure(mDelegate->FetchEndpointBindingList(
                mRefreshingNodeId, currentEndpointId,
                [this](CHIP_ERROR err,
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
                                mEndpointBindingEntries.push_back(newEntry);
                            }
                        }

                        // Remove entries not in endpointBindings, but only if they are Committed or DeletePending
                        mEndpointBindingEntries.erase(
                            std::remove_if(
                                mEndpointBindingEntries.begin(), mEndpointBindingEntries.end(),
                                [&](const auto & entry) {
                                    if (entry.nodeID != mRefreshingNodeId)
                                    {
                                        return false;
                                    }
                                    if (entry.statusEntry.state != Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted &&
                                        entry.statusEntry.state !=
                                            Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending)
                                    {
                                        return false;
                                    }
                                    return !std::any_of(endpointBindings.begin(), endpointBindings.end(), [&](const auto & eb) {
                                        return entry.endpointID == eb.endpointID && BindingMatches(entry.binding, eb.binding);
                                    });
                                }),
                            mEndpointBindingEntries.end());

                        // Move to the next endpoint
                        mRefreshingEndpointIndex++;
                    }
                    else
                    {
                        // Leave node as pending but tear down the refresh state.
                        mRefreshingNodeId = kUndefinedNodeId;
                        mRefreshState     = kIdle;
                        return;
                    }

                    // Continue the state machine to let the kRefreshingBindings branch process mEndpointBindingList.
                    if (ContinueRefresh() != CHIP_NO_ERROR)
                    {
                        // Ignore errors in continuation from within the callback.
                    }
                }));

            // Return here - the callback will call ContinueRefresh() again
            return CHIP_NO_ERROR;
        }

        for (auto it = mEndpointBindingEntries.begin(); it != mEndpointBindingEntries.end();)
        {
            if (it->nodeID == mRefreshingNodeId)
            {
                if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kPending ||
                    it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted)
                {
                    mRefreshingBindingEntries.push_back(*it);
                }
                else if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitFailed)
                {
                    CHIP_ERROR failureCode(it->statusEntry.failureCode);

                    if (failureCode == CHIP_IM_GLOBAL_STATUS(ConstraintError) ||
                        failureCode == CHIP_IM_GLOBAL_STATUS(ResourceExhausted))
                    {
                        // remove entry from the list
                        it = mEndpointBindingEntries.erase(it);
                        continue;
                    }

                    mRefreshingBindingEntries.push_back(*it);
                }
            }

            ++it;
        }

        ReturnErrorOnFailure(mDelegate->SyncNode(mRefreshingNodeId, mRefreshingBindingEntries, [this]() {
            for (auto & entry : mEndpointBindingEntries)
            {
                if (entry.nodeID == mRefreshingNodeId &&
                    (entry.statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kPending ||
                     entry.statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitFailed))
                {
                    for (const auto & bindingEntry : mRefreshingBindingEntries)
                    {
                        if (entry.endpointID == bindingEntry.endpointID && BindingMatches(entry.binding, bindingEntry.binding))
                        {
                            entry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                            break;
                        }
                    }
                }
            }

            // Remove all DeletePending entries for mRefreshingNodeId
            mEndpointBindingEntries.erase(std::remove_if(mEndpointBindingEntries.begin(), mEndpointBindingEntries.end(),
                                                         [this](const auto & entry) {
                                                             return entry.nodeID == mRefreshingNodeId &&
                                                                 entry.statusEntry.state ==
                                                                 Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;
                                                         }),
                                          mEndpointBindingEntries.end());

            // After syncing bindings, move to fetching group key sets
            mRefreshState = kFetchingGroupKeySets;
            if (ContinueRefresh() != CHIP_NO_ERROR)
            {
                // Ignore errors in continuation from within the callback.
            }
        }));
    }
    break;

    case kFetchingGroupKeySets: {
        // Request Group Key Set List from the device and transition to kRefreshingGroupKeySets.
        ReturnErrorOnFailure(mDelegate->FetchGroupKeySetList(
            mRefreshingNodeId,
            [this](CHIP_ERROR err,
                   const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type> & groupKeySets) {
                if (err == CHIP_NO_ERROR)
                {
                    // Convert groupKeySets to mGroupKeySetList entries
                    for (const auto & groupKeySet : groupKeySets)
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
                            CopyGroupKeySetWithOwnedSpans(groupKeySet, copiedKeySet);
                            mGroupKeySetList.push_back(copiedKeySet);
                        }
                        else
                        {
                            // Update existing entry
                            CopyGroupKeySetWithOwnedSpans(groupKeySet, *it);
                        }
                    }

                    // Remove entries not in groupKeySets
                    for (auto it = mGroupKeySetList.begin(); it != mGroupKeySetList.end();)
                    {
                        const bool existsOnNode = std::any_of(groupKeySets.begin(), groupKeySets.end(), [&](const auto & gks) {
                            return it->groupKeySetID == gks.groupKeySetID;
                        });
                        if (!existsOnNode)
                        {
                            RemoveGroupKeySetStorage(it->groupKeySetID);
                            it = mGroupKeySetList.erase(it);
                        }
                        else
                        {
                            ++it;
                        }
                    }

                    // Advance the state machine to process the group key sets.
                    mRefreshState = kRefreshingGroupKeySets;
                }
                else
                {
                    // Leave node as pending but tear down the refresh state.
                    mRefreshingNodeId = kUndefinedNodeId;
                    mRefreshState     = kIdle;
                    return;
                }

                // Continue the state machine to let the kRefreshingGroupKeySets branch process mGroupKeySetList.
                if (ContinueRefresh() != CHIP_NO_ERROR)
                {
                    // Ignore errors in continuation from within the callback.
                }
            }));
    }
    break;
    case kRefreshingGroupKeySets: {
        // 4. Ensure per-node key-set entries for each GroupKeySet are synced to devices.
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
                    size_t idx       = static_cast<size_t>(std::distance(mNodeKeySetEntries.begin(), nkIt));
                    auto groupKeySet = *gksIt;
                    ReturnErrorOnFailure(mDelegate->SyncNode(nkIt->nodeID, groupKeySet, [this, idx]() {
                        if (idx < mNodeKeySetEntries.size())
                        {
                            mNodeKeySetEntries[idx].statusEntry.state =
                                Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                        }
                    }));
                    ++nkIt;
                }
                else if (nkIt->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending)
                {
                    // zero-initialized struct to indicate deletion for the SyncNode call
                    Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type nullEntry{ 0 };

                    auto nodeIdToErase        = nkIt->nodeID;
                    auto groupKeySetIdToErase = nkIt->groupKeySetID;
                    ReturnErrorOnFailure(
                        mDelegate->SyncNode(nkIt->nodeID, nullEntry, [this, nodeIdToErase, groupKeySetIdToErase]() {
                            mNodeKeySetEntries.erase(std::remove_if(mNodeKeySetEntries.begin(), mNodeKeySetEntries.end(),
                                                                    [&](const auto & entry) {
                                                                        return entry.nodeID == nodeIdToErase &&
                                                                            entry.groupKeySetID == groupKeySetIdToErase;
                                                                    }),
                                                     mNodeKeySetEntries.end());
                        }));
                }
                else if (nkIt->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitFailed)
                {
                    CHIP_ERROR failureCode(nkIt->statusEntry.failureCode);

                    if (failureCode == CHIP_IM_GLOBAL_STATUS(ConstraintError) ||
                        failureCode == CHIP_IM_GLOBAL_STATUS(ResourceExhausted))
                    {
                        // remove entry from the list
                        nkIt = mNodeKeySetEntries.erase(nkIt);
                    }
                    else
                    {
                        // Retry the failed commit by attempting to SyncNode again.
                        size_t idx       = static_cast<size_t>(std::distance(mNodeKeySetEntries.begin(), nkIt));
                        auto groupKeySet = *gksIt;
                        ReturnErrorOnFailure(mDelegate->SyncNode(nkIt->nodeID, groupKeySet, [this, idx]() {
                            if (idx < mNodeKeySetEntries.size())
                            {
                                mNodeKeySetEntries[idx].statusEntry.state =
                                    Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                            }
                        }));
                        ++nkIt;
                    }
                }
                else
                {
                    ++nkIt;
                }
            }
        }

        // Request ACL List from the device and transition to kRefreshingACLs.
        ReturnErrorOnFailure(mDelegate->FetchACLList(
            mRefreshingNodeId,
            [this](CHIP_ERROR err,
                   const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type> & acls) {
                if (err == CHIP_NO_ERROR)
                {
                    // Convert acls to mACLEntries
                    for (const auto & acl : acls)
                    {
                        auto it = std::find_if(mACLEntries.begin(), mACLEntries.end(),
                                               [this, &acl](const datastore::ACLEntryStruct & entry) {
                                                   return entry.nodeID == mRefreshingNodeId && entry.listID == acl.listID;
                                               });

                        if (it == mACLEntries.end())
                        {
                            datastore::ACLEntryStruct newEntry;
                            newEntry.nodeID             = mRefreshingNodeId;
                            newEntry.listID             = acl.listID;
                            newEntry.ACLEntry.authMode  = acl.ACLEntry.authMode;
                            newEntry.ACLEntry.privilege = acl.ACLEntry.privilege;

                            for (size_t subjectsIndex = 0; subjectsIndex < acl.ACLEntry.subjects.Value().size(); ++subjectsIndex)
                            {
                                newEntry.ACLEntry.subjects.push_back(acl.ACLEntry.subjects.Value()[subjectsIndex]);
                            }

                            for (size_t targetsIndex = 0; targetsIndex < acl.ACLEntry.targets.Value().size(); ++targetsIndex)
                            {
                                newEntry.ACLEntry.targets.push_back(acl.ACLEntry.targets.Value()[targetsIndex]);
                            }

                            newEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                            mACLEntries.push_back(newEntry);
                        }
                    }

                    // Remove entries not in acls, but only if they are Committed or DeletePending
                    mACLEntries.erase(std::remove_if(mACLEntries.begin(), mACLEntries.end(),
                                                     [&](const auto & entry) {
                                                         if (entry.nodeID != mRefreshingNodeId)
                                                         {
                                                             return false;
                                                         }
                                                         if (entry.statusEntry.state !=
                                                                 Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted &&
                                                             entry.statusEntry.state !=
                                                                 Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending)
                                                         {
                                                             return false;
                                                         }
                                                         return !std::any_of(acls.begin(), acls.end(), [&](const auto & acl) {
                                                             return entry.listID == acl.listID;
                                                         });
                                                     }),
                                      mACLEntries.end());

                    // Advance the state machine to process the ACLs.
                    mRefreshState = kRefreshingACLs;
                }
                else
                {
                    // Leave node as pending but tear down the refresh state.
                    mRefreshingNodeId = kUndefinedNodeId;
                    mRefreshState     = kIdle;
                    return;
                }

                // Continue the state machine to let the kRefreshingACLs branch process mACLList.
                if (ContinueRefresh() != CHIP_NO_ERROR)
                {
                    // Ignore errors in continuation from within the callback.
                }
            }));
    }
    break;
    case kRefreshingACLs: {
        // 5.
        for (auto it = mACLEntries.begin(); it != mACLEntries.end();)
        {
            if (it->nodeID == mRefreshingNodeId)
            {
                if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kPending ||
                    it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted)
                {
                    {
                        // Prepare an encoded ACL entry to send to the node.
                        Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type entryToSync;
                        entryToSync.nodeID             = it->nodeID;
                        entryToSync.listID             = it->listID;
                        entryToSync.ACLEntry.authMode  = it->ACLEntry.authMode;
                        entryToSync.ACLEntry.privilege = it->ACLEntry.privilege;
                        entryToSync.ACLEntry.subjects =
                            DataModel::List<const uint64_t>(it->ACLEntry.subjects.data(), it->ACLEntry.subjects.size());
                        entryToSync.ACLEntry.targets = DataModel::List<
                            const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type>(
                            it->ACLEntry.targets.data(), it->ACLEntry.targets.size());
                        entryToSync.statusEntry = it->statusEntry;

                        mRefreshingACLEntries.push_back(entryToSync);
                    }
                }
                else if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitFailed)
                {
                    CHIP_ERROR failureCode(it->statusEntry.failureCode);

                    if (failureCode == CHIP_IM_GLOBAL_STATUS(ConstraintError) ||
                        failureCode == CHIP_IM_GLOBAL_STATUS(ResourceExhausted))
                    {
                        // remove entry from the list
                        it = mACLEntries.erase(it);
                        continue;
                    }

                    // Prepare an encoded ACL entry to retry the failed commit.
                    Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type entryToSync;
                    entryToSync.nodeID             = it->nodeID;
                    entryToSync.listID             = it->listID;
                    entryToSync.ACLEntry.authMode  = it->ACLEntry.authMode;
                    entryToSync.ACLEntry.privilege = it->ACLEntry.privilege;
                    entryToSync.ACLEntry.subjects =
                        DataModel::List<const uint64_t>(it->ACLEntry.subjects.data(), it->ACLEntry.subjects.size());
                    entryToSync.ACLEntry.targets =
                        DataModel::List<const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type>(
                            it->ACLEntry.targets.data(), it->ACLEntry.targets.size());
                    entryToSync.statusEntry = it->statusEntry;

                    mRefreshingACLEntries.push_back(entryToSync);
                }
            }

            ++it;
        }

        ReturnErrorOnFailure(mDelegate->SyncNode(mRefreshingNodeId, mRefreshingACLEntries, [this]() {
            for (auto & entry : mACLEntries)
            {
                if (entry.nodeID == mRefreshingNodeId &&
                    (entry.statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kPending ||
                     entry.statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitFailed))
                {
                    for (const auto & aclEntry : mRefreshingACLEntries)
                    {
                        if (entry.listID == aclEntry.listID)
                        {
                            entry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                            break;
                        }
                    }
                }
            }

            // Remove all DeletePending entries for mRefreshingNodeId
            mACLEntries.erase(std::remove_if(mACLEntries.begin(), mACLEntries.end(),
                                             [this](const auto & entry) {
                                                 return entry.nodeID == mRefreshingNodeId &&
                                                     entry.statusEntry.state ==
                                                     Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;
                                             }),
                              mACLEntries.end());
        }));

        // 6.
        ReturnErrorOnFailure(SetNode(mRefreshingNodeId, Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted));

        for (Listener * listener = mListeners; listener != nullptr; listener = listener->mNext)
        {
            listener->MarkNodeListChanged();
        }

        mRefreshingNodeId = kUndefinedNodeId;
        mRefreshState     = kIdle;
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
    CopyGroupKeySetWithOwnedSpans(groupKeySet, copiedKeySet);

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
                                        Clusters::JointFabricDatastore::DatastoreGroupKeySecurityPolicyEnum::kUnknownEnumValue &&
                                    groupKeySet.groupKeyMulticastPolicy <
                                        Clusters::JointFabricDatastore::DatastoreGroupKeyMulticastPolicyEnum::kUnknownEnumValue,
                                CHIP_IM_GLOBAL_STATUS(ConstraintError));

            CopyGroupKeySetWithOwnedSpans(groupKeySet, entry);

            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR
JointFabricDatastore::AddAdmin(
    Clusters::JointFabricDatastore::Structs::DatastoreAdministratorInformationEntryStruct::Type & adminId)
{
    VerifyOrReturnError(IsAdminEntryPresent(adminId.nodeID) == false, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(mAdminEntries.size() < kMaxAdminNodes, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(SetAdminEntryWithOwnedStorage(adminId.nodeID, adminId.friendlyName, adminId.icac, adminId));

    mAdminEntries.push_back(adminId);

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

CHIP_ERROR JointFabricDatastore::UpdateAdmin(NodeId nodeId, CharSpan friendlyName, ByteSpan icac)
{
    for (auto & entry : mAdminEntries)
    {
        if (entry.nodeID == nodeId)
        {
            ReturnErrorOnFailure(SetAdminEntryWithOwnedStorage(nodeId, friendlyName, icac, entry));
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
    bool entryUpdated = false;

    for (size_t i = 0; i < mNodeKeySetEntries.size(); ++i)
    {
        auto & entry = mNodeKeySetEntries[i];
        if (entry.groupKeySetID == groupKeySet.groupKeySetID)
        {
            if (groupKeySet.groupKeySecurityPolicy <
                    Clusters::JointFabricDatastore::DatastoreGroupKeySecurityPolicyEnum::kUnknownEnumValue &&
                groupKeySet.groupKeyMulticastPolicy <
                    Clusters::JointFabricDatastore::DatastoreGroupKeyMulticastPolicyEnum::kUnknownEnumValue)
            {

                size_t index = i;
                LogErrorOnFailure(mDelegate->SyncNode(entry.nodeID, groupKeySet, [this, index]() {
                    mNodeKeySetEntries[index].statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                }));

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
            // fails, leave the entry as pending so a subsequent Refresh can apply it.
            const GroupId updatedGroupId = commandData.groupID;
            for (size_t i = 0; i < mEndpointGroupIDEntries.size(); ++i)
            {
                auto & epGroupEntry = mEndpointGroupIDEntries[i];
                if (epGroupEntry.groupID == updatedGroupId)
                {
                    epGroupEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

                    // Make a copy to send to the node. Do not fail the entire UpdateGroup if SyncNode
                    // returns an error; leave the entry pending for a later refresh per spec.
                    auto entryToSync = epGroupEntry;

                    CHIP_ERROR syncErr = mDelegate->SyncNode(epGroupEntry.nodeID, entryToSync, [this, i]() {
                        mEndpointGroupIDEntries[i].statusEntry.state =
                            Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                    });

                    if (syncErr != CHIP_NO_ERROR)
                    {
                        ChipLogError(DataManagement,
                                     "Failed to sync node for group friendly name update, leaving as pending: %" CHIP_ERROR_FORMAT,
                                     syncErr.Format());
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
        if (mGroupInformationEntries[index].groupKeySetID.Value() != commandData.groupKeySetID.Value())
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
        if (mGroupInformationEntries[index].groupCAT.Value() != commandData.groupCAT.Value())
        {
            anyGroupCATFieldUpdated = true;
        }
        // Update the groupCAT
        mGroupInformationEntries[index].groupCAT = commandData.groupCAT;
    }
    if (commandData.groupCATVersion.IsNull() == false)
    {
        if (mGroupInformationEntries[index].groupCATVersion.Value() != commandData.groupCATVersion.Value())
        {
            anyGroupCATFieldUpdated = true;
        }
        mGroupInformationEntries[index].groupCATVersion = commandData.groupCATVersion;
    }
    if (commandData.groupPermission != Clusters::JointFabricDatastore::DatastoreAccessControlEntryPrivilegeEnum::kUnknownEnumValue)
    {
        if (mGroupInformationEntries[index].groupPermission != commandData.groupPermission)
        {
            anyGroupCATFieldUpdated = true;
        }
        // If the groupPermission is not set to kUnknownEnumValue, update it
        mGroupInformationEntries[index].groupPermission = commandData.groupPermission;
    }

    if (anyGroupCATFieldUpdated)
    {
        const GroupId updatedGroupId = commandData.groupID;

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

            // Update the ACL entry in the datastore to reflect the new group permission and mark Pending.
            acl.ACLEntry.privilege = mGroupInformationEntries[index].groupPermission;
            acl.statusEntry.state  = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

            // Prepare an encoded entry to send to the node.
            Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type entryToEncode;
            entryToEncode.nodeID             = acl.nodeID;
            entryToEncode.listID             = acl.listID;
            entryToEncode.ACLEntry.authMode  = acl.ACLEntry.authMode;
            entryToEncode.ACLEntry.privilege = acl.ACLEntry.privilege;
            entryToEncode.ACLEntry.subjects =
                DataModel::List<const uint64_t>(acl.ACLEntry.subjects.data(), acl.ACLEntry.subjects.size());
            entryToEncode.ACLEntry.targets =
                DataModel::List<const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type>(
                    acl.ACLEntry.targets.data(), acl.ACLEntry.targets.size());
            entryToEncode.statusEntry = acl.statusEntry;

            // Attempt to update the ACL on the node. On success, mark the ACL entry as Committed.
            // Capture index 'i' to safely identify the entry inside the callback.
            ReturnErrorOnFailure(mDelegate->SyncNode(acl.nodeID, entryToEncode, [this, i]() {
                if (i < mACLEntries.size())
                {
                    mACLEntries[i].statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                }
            }));
        }
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

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR JointFabricDatastore::UpdateEndpointForNode(NodeId nodeId, chip::EndpointId endpointId, CharSpan friendlyName)
{
    for (auto & entry : mEndpointEntries)
    {
        if (entry.nodeID == nodeId && entry.endpointID == endpointId)
        {
            entry.friendlyName = friendlyName;
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NOT_FOUND;
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

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR JointFabricDatastore::AddGroupIDToEndpointForNode(NodeId nodeId, chip::EndpointId endpointId, chip::GroupId groupId)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    size_t index = 0;
    ReturnErrorOnFailure(IsNodeIdAndEndpointInEndpointInformationEntries(nodeId, endpointId, index));

    VerifyOrReturnError(IsGroupIDInDatastore(groupId, index) == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    if (mGroupInformationEntries[index].groupKeySetID.IsNull() == false)
    {
        uint16_t groupKeySetID = mGroupInformationEntries[index].groupKeySetID.Value();

        // make sure mNodeKeySetEntries contains an entry for this keyset and node, else add one and update device
        bool nodeKeySetExists = false;
        for (auto & entry : mNodeKeySetEntries)
        {
            if (entry.nodeID == nodeId && entry.groupKeySetID == groupKeySetID)
            {
                nodeKeySetExists = true;
                break; // Found the group key set, no need to add it again
            }
        }

        if (!nodeKeySetExists)
        {
            // Create a new group key set entry if it doesn't exist
            Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type newNodeKeySet;
            newNodeKeySet.nodeID            = nodeId;
            newNodeKeySet.groupKeySetID     = groupKeySetID;
            newNodeKeySet.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

            mNodeKeySetEntries.push_back(newNodeKeySet);

            ReturnErrorOnFailure(mDelegate->SyncNode(nodeId, newNodeKeySet, [this]() {
                mNodeKeySetEntries.back().statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
            }));
        }
    }

    // Check if the group ID already exists for the endpoint
    for (auto & entry : mEndpointGroupIDEntries)
    {
        if (entry.nodeID == nodeId && entry.endpointID == endpointId && entry.groupID == groupId)
        {
            return CHIP_NO_ERROR;
        }
    }

    VerifyOrReturnError(mEndpointGroupIDEntries.size() < kMaxGroups, CHIP_ERROR_NO_MEMORY);

    // Create a new endpoint group ID entry
    Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type newGroupEntry;
    newGroupEntry.nodeID            = nodeId;
    newGroupEntry.endpointID        = endpointId;
    newGroupEntry.groupID           = groupId;
    newGroupEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

    // Add the new ACL entry to the datastore
    mEndpointGroupIDEntries.push_back(newGroupEntry);

    return mDelegate->SyncNode(nodeId, newGroupEntry, [this]() {
        mEndpointGroupIDEntries.back().statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
    });
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
            it->statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;

            // zero-initialized struct to indicate deletion for the SyncNode call
            Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type endpointGroupIdNullEntry{ 0 };

            ReturnErrorOnFailure(
                mDelegate->SyncNode(nodeId, endpointGroupIdNullEntry, [this, it]() { mEndpointGroupIDEntries.erase(it); }));

            if (IsGroupIDInDatastore(groupId, index) == CHIP_NO_ERROR)
            {
                for (auto it2 = mNodeKeySetEntries.begin(); it2 != mNodeKeySetEntries.end();)
                {
                    bool incrementIndex = true;

                    if (it2->nodeID == nodeId && mGroupInformationEntries[index].groupKeySetID.IsNull() == false &&
                        it2->groupKeySetID == mGroupInformationEntries[index].groupKeySetID.Value())
                    {
                        it2->statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;

                        // zero-initialized struct to indicate deletion for the SyncNode call
                        Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type nodeKeySetNullEntry{ 0 };
                        ReturnErrorOnFailure(
                            mDelegate->SyncNode(nodeId, nodeKeySetNullEntry, [this, it2]() { mNodeKeySetEntries.erase(it2); }));

                        incrementIndex = false;
                    }

                    if (incrementIndex)
                    {
                        ++it2;
                    }
                    else
                    {
                        incrementIndex = true;
                    }
                }
            }

            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NOT_FOUND;
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

bool JointFabricDatastore::BindingMatches(
    const Clusters::JointFabricDatastore::Structs::DatastoreBindingTargetStruct::Type & binding1,
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

CHIP_ERROR
JointFabricDatastore::AddBindingToEndpointForNode(
    NodeId nodeId, chip::EndpointId endpointId,
    const Clusters::JointFabricDatastore::Structs::DatastoreBindingTargetStruct::Type & binding)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    size_t index = 0;
    ReturnErrorOnFailure(IsNodeIdAndEndpointInEndpointInformationEntries(nodeId, endpointId, index));

    // Check if the group ID already exists for the endpoint
    for (auto & entry : mEndpointBindingEntries)
    {
        if (entry.nodeID == nodeId && entry.endpointID == endpointId)
        {
            if (BindingMatches(entry.binding, binding))
            {
                return CHIP_NO_ERROR;
            }
        }
    }

    VerifyOrReturnError(mEndpointBindingEntries.size() < kMaxGroups, CHIP_ERROR_NO_MEMORY);

    // Create a new binding entry
    Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type newBindingEntry;
    newBindingEntry.nodeID     = nodeId;
    newBindingEntry.endpointID = endpointId;
    newBindingEntry.binding    = binding;
    ReturnErrorOnFailure(GenerateAndAssignAUniqueListID(newBindingEntry.listID));
    newBindingEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

    // Add the new binding entry to the datastore
    mEndpointBindingEntries.push_back(newBindingEntry);

    return mDelegate->SyncNode(nodeId, newBindingEntry, [this]() {
        mEndpointBindingEntries.back().statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
    });
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
            it->statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;

            // zero-initialized struct to indicate deletion for the SyncNode call
            Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type nullEntry{ 0 };
            return mDelegate->SyncNode(nodeId, nullEntry, [this, it]() { mEndpointBindingEntries.erase(it); });
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

bool JointFabricDatastore::ACLTargetMatches(
    const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type & target1,
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

bool JointFabricDatastore::ACLMatches(
    const datastore::AccessControlEntryStruct & acl1,
    const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::DecodableType & acl2)
{
    if (acl1.privilege != acl2.privilege)
    {
        return false;
    }

    if (acl1.authMode != acl2.authMode)
    {
        return false;
    }

    {
        auto it1 = acl1.subjects.begin();
        auto it2 = acl2.subjects.Value().begin();

        while (it1 != acl1.subjects.end() && it2.Next())
        {
            if (*it1 != it2.GetValue())
            {
                return false;
            }
            ++it1;
        }

        if (it2.Next())
        {
            return false; // acl2 has more subjects
        }
    }

    {
        auto it1 = acl1.targets.begin();
        auto it2 = acl2.targets.Value().begin();

        while (it1 != acl1.targets.end() && it2.Next())
        {
            if (ACLTargetMatches(*it1, it2.GetValue()) == false)
            {
                return false;
            }
            ++it1;
        }

        if (it2.Next())
        {
            return false; // acl2 has more targets
        }
    }

    return true;
}

CHIP_ERROR
JointFabricDatastore::AddACLToNode(
    NodeId nodeId, const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::DecodableType & aclEntry)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    size_t index = 0;
    ReturnErrorOnFailure(IsNodeIdInNodeInformationEntries(nodeId, index));

    // Check if the ACL entry already exists for the node
    for (auto & entry : mACLEntries)
    {
        if (entry.nodeID == nodeId)
        {
            if (ACLMatches(entry.ACLEntry, aclEntry))
            {
                return CHIP_NO_ERROR;
            }
        }
    }
    VerifyOrReturnError(mACLEntries.size() < kMaxACLs, CHIP_ERROR_NO_MEMORY);
    // Create a new ACL entry
    datastore::ACLEntryStruct newACLEntry;
    newACLEntry.nodeID             = nodeId;
    newACLEntry.ACLEntry.privilege = aclEntry.privilege;
    newACLEntry.ACLEntry.authMode  = aclEntry.authMode;

    newACLEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

    if (!aclEntry.subjects.IsNull())
    {
        auto iter = aclEntry.subjects.Value().begin();
        while (iter.Next())
        {
            newACLEntry.ACLEntry.subjects.push_back(iter.GetValue());
        }
        ReturnErrorOnFailure(iter.GetStatus());
    }

    if (!aclEntry.targets.IsNull())
    {
        auto iter = aclEntry.targets.Value().begin();
        while (iter.Next())
        {
            newACLEntry.ACLEntry.targets.push_back(iter.GetValue());
        }
        ReturnErrorOnFailure(iter.GetStatus());
    }

    ReturnErrorOnFailure(GenerateAndAssignAUniqueListID(newACLEntry.listID));

    // Add the new ACL entry to the datastore
    mACLEntries.push_back(newACLEntry);

    Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type entryToEncode;
    entryToEncode.nodeID             = newACLEntry.nodeID;
    entryToEncode.listID             = newACLEntry.listID;
    entryToEncode.ACLEntry.authMode  = newACLEntry.ACLEntry.authMode;
    entryToEncode.ACLEntry.privilege = newACLEntry.ACLEntry.privilege;
    entryToEncode.ACLEntry.subjects =
        DataModel::List<const uint64_t>(newACLEntry.ACLEntry.subjects.data(), newACLEntry.ACLEntry.subjects.size());
    entryToEncode.ACLEntry.targets =
        DataModel::List<const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type>(
            newACLEntry.ACLEntry.targets.data(), newACLEntry.ACLEntry.targets.size());
    entryToEncode.statusEntry = newACLEntry.statusEntry;

    return mDelegate->SyncNode(nodeId, entryToEncode, [this]() {
        mACLEntries.back().statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
    });
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
            it->statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;

            // zero-initialized struct to indicate deletion for the SyncNode call
            Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type nullEntry{ 0 };
            return mDelegate->SyncNode(nodeId, nullEntry, [this, it]() { mACLEntries.erase(it); });
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
            // Skip if a matching NodeKeySet entry already exists for this node
            bool exists = false;
            for (const auto & nkse : mNodeKeySetEntries)
            {
                if (nkse.nodeID == nodeId && nkse.groupKeySetID == groupKeySetId)
                {
                    exists = true;
                    break;
                }
            }
            if (exists)
            {
                continue;
            }

            Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type newEntry;
            newEntry.nodeID            = nodeId;
            newEntry.groupKeySetID     = groupKeySetId;
            newEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

            mNodeKeySetEntries.push_back(newEntry);

            size_t index = mNodeKeySetEntries.size() - 1;
            // Sync to the node and mark committed on success
            ReturnErrorOnFailure(mDelegate->SyncNode(nodeId, newEntry, [this, index]() {
                if (index < mNodeKeySetEntries.size())
                {
                    mNodeKeySetEntries[index].statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                }
            }));
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
                // zero-initialized struct to indicate deletion for the SyncNode call
                Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type nullEntry{ 0 };

                auto nodeIdToErase        = it->nodeID;
                auto groupKeySetIdToErase = it->groupKeySetID;
                ReturnErrorOnFailure(mDelegate->SyncNode(nodeId, nullEntry, [this, nodeIdToErase, groupKeySetIdToErase]() {
                    mNodeKeySetEntries.erase(std::remove_if(mNodeKeySetEntries.begin(), mNodeKeySetEntries.end(),
                                                            [&](const auto & entry) {
                                                                return entry.nodeID == nodeIdToErase &&
                                                                    entry.groupKeySetID == groupKeySetIdToErase;
                                                            }),
                                             mNodeKeySetEntries.end());
                }));

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

    mNodeKeySetEntries.push_back(newEntry);

    size_t index = mNodeKeySetEntries.size() - 1;
    // Sync to the node and mark committed on success
    return mDelegate->SyncNode(nodeId, newEntry, [this, index]() {
        if (index < mNodeKeySetEntries.size())
        {
            mNodeKeySetEntries[index].statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
        }
    });
}

CHIP_ERROR JointFabricDatastore::TestAddEndpointEntry(EndpointId endpointId, NodeId nodeId, CharSpan friendlyName)
{
    Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type newEntry;
    newEntry.nodeID       = nodeId;
    newEntry.endpointID   = endpointId;
    newEntry.friendlyName = friendlyName;

    mEndpointEntries.push_back(newEntry);

    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabricDatastore::ForceAddNodeKeySetEntry(uint16_t groupKeySetId, NodeId nodeId)
{
    Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type newEntry;
    newEntry.nodeID            = nodeId;
    newEntry.groupKeySetID     = groupKeySetId;
    newEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;

    mNodeKeySetEntries.push_back(newEntry);
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
