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
<<<<<<< HEAD
=======
#include <cstring>
#include <unordered_set>
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))

namespace chip {
namespace app {

<<<<<<< HEAD
=======
namespace {
/**
 * Validate that an input is the correct length to be an Epoch Key. Null keys are considered valid.
 */
bool EpochKeyFitsStorage(const DataModel::Nullable<ByteSpan> & key)
{
    using EpochKeyStorage = Crypto::SensitiveDataBuffer<Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES>;
    return key.IsNull() || key.Value().size() <= EpochKeyStorage::Capacity();
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

>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
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

CHIP_ERROR JointFabricDatastore::RefreshNode(NodeId nodeId, ReadOnlyBuffer<DataModel::EndpointEntry> endpointsList)
{
    // 1. && 2.
    ReturnErrorOnFailure(SetNode(nodeId, Clusters::JointFabricDatastore::DatastoreStateEnum::kPending));

    // 3.

    // cycle through endpointsList and add them to the endpoint entries
    for (const auto & endpoint : endpointsList)
    {
        auto it = std::find_if(
            mEndpointEntries.begin(), mEndpointEntries.end(),
            [&endpoint, &nodeId](const Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type & entry) {
                return entry.nodeID == nodeId && entry.endpointID == endpoint.id;
            });
        if (it == mEndpointEntries.end())
        {
            Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type newEntry;
            newEntry.endpointID = endpoint.id;
            newEntry.nodeID     = nodeId;
            mEndpointEntries.push_back(newEntry);
        }
    }

    // Remove EndpointEntries that are not in the endpointsList
    mEndpointEntries.erase(std::remove_if(mEndpointEntries.begin(), mEndpointEntries.end(),
                                          [&](const auto & entry) {
                                              if (entry.nodeID != nodeId)
                                              {
                                                  return false; // Not for this node, don't remove.
                                              }
                                              // Remove if not found in endpointsList.
                                              return !std::any_of(
                                                  endpointsList.begin(), endpointsList.end(),
                                                  [&](const auto & endpoint) { return entry.endpointID == endpoint.id; });
                                          }),
                           mEndpointEntries.end());

    // TODO: read the Endpoint Group ID List from the actual device

    for (auto it = mEndpointGroupIDEntries.begin(); it != mEndpointGroupIDEntries.end();)
    {
        if (it->nodeID == nodeId)
        {
            if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kPending)
            {
                // TODO: add this entry to the actual device. Only mark as committed if successful.
                // it->statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
            }
            else if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending)
            {
<<<<<<< HEAD
                // TODO: Remove the binding from the actual device. If successful, then remove the entry.
                // otherwise update status to CommitFailed and return error
                it = mEndpointGroupIDEntries.erase(it);
                continue;
            }
            else if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitFailed)
            {
                CHIP_ERROR failureCode(it->statusEntry.failureCode);
=======
                if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kPending)
                {
                    auto entryToSync      = *it;
                    const NodeId nodeId   = entryToSync.nodeID;
                    const EndpointId epId = entryToSync.endpointID;
                    const GroupId groupId = entryToSync.groupID;
                    ReturnErrorOnFailure(mDelegate->SyncNode(mRefreshingNodeId, entryToSync, [this, nodeId, epId, groupId]() {
                        detail::MarkEntryCommittedIfFound(mEndpointGroupIDEntries, [&](const auto & e) {
                            return e.nodeID == nodeId && e.endpointID == epId && e.groupID == groupId;
                        });
                    }));
                }
                else if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending)
                {
                    Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type endpointGroupIdNullEntry{
                        0
                    };
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))

                if (failureCode == CHIP_IM_GLOBAL_STATUS(ConstraintError) ||
                    failureCode == CHIP_IM_GLOBAL_STATUS(ResourceExhausted))
                {
                    // remove entry from the list
                    it = mEndpointGroupIDEntries.erase(it);
                    continue;
                }
                else
                {
                    // TODO: update binding on actual device.
                    // TODO: check if change has been made, retry if not. Mark as committed upon success.
                    // TODO: retry delete, remove entry if successful.
                }
<<<<<<< HEAD
            }
=======

                // Continue the state machine to let the kFetchingGroupKeySets branch process mRefreshingGroupKeySetIDs.
                if (ContinueRefresh() != CHIP_NO_ERROR)
                {
                    // Ignore errors in continuation from within the callback.
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
                        mRefreshingNodeId = kUndefinedNodeId;
                        mRefreshState     = kIdle;
                        return;
                    }

                    // Continue fetching key sets until complete, then process mGroupKeySetList in kRefreshingGroupKeySets.
                    if (ContinueRefresh() != CHIP_NO_ERROR)
                    {
                        // Ignore errors in continuation from within the callback.
                    }
                });
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
        }

        ++it;
    }

    // TODO: read the Endpoint Binding List from the actual device

    for (auto it = mEndpointBindingEntries.begin(); it != mEndpointBindingEntries.end();)
    {
        if (it->nodeID == nodeId)
        {
            if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kPending)
            {
                // TODO: add this entry to the actual device. Only mark as committed if successful.
                // it->statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
            }
            else if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending)
            {
                // TODO: Remove the binding from the actual device. If successful, then remove the entry.
                // otherwise update status to CommitFailed and return error
                it = mEndpointBindingEntries.erase(it);
                continue;
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
                else
                {
<<<<<<< HEAD
                    // TODO: update binding on actual device.
                    // TODO: check if change has been made, retry if not. Mark as committed upon success.
                    // TODO: retry delete, remove entry if successful.
=======
                    // Make a copy of the group key set to send to the node.
                    const NodeId entryNodeId = nkIt->nodeID;
                    auto groupKeySet         = *gksIt;
                    ReturnErrorOnFailure(mDelegate->SyncNode(nkIt->nodeID, groupKeySet, [this, entryNodeId, groupKeySetId]() {
                        detail::MarkEntryCommittedIfFound(mNodeKeySetEntries, [&](const auto & e) {
                            return e.nodeID == entryNodeId && e.groupKeySetID == groupKeySetId;
                        });
                    }));
                    ++nkIt;
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
                }
            }
        }

        ++it;
    }

<<<<<<< HEAD
    // TODO: read the Group Key Set List from the actual device

    // 4.
    for (auto it = mGroupKeySetList.begin(); it != mGroupKeySetList.end();)
    {
        // TODO: Apply any pending updates from datastore, upon success mark as committed
        // Retry any CommitFailure entries from datastore, upon success can be removed.
        // Ensure list from device matches list in datastore
        ++it;
    }

    // TODO: read the ACL List from the actual device

    // 5.
    for (auto it = mACLEntries.begin(); it != mACLEntries.end();)
    {
        if (it->nodeID == nodeId)
        {
            if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kPending)
            {
                // TODO: add this entry to the actual device. Only mark as committed if successful.
                // it->statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
            }
            else if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending)
            {
                // TODO: Remove the binding from the actual device. If successful, then remove the entry.
                // otherwise update status to CommitFailed and return error
                it = mACLEntries.erase(it);
                continue;
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
=======
                    if (failureCode == CHIP_IM_GLOBAL_STATUS(ConstraintError) ||
                        failureCode == CHIP_IM_GLOBAL_STATUS(ResourceExhausted))
                    {
                        // remove entry from the list
                        nkIt = mNodeKeySetEntries.erase(nkIt);
                    }
                    else
                    {
                        // Retry the failed commit by attempting to SyncNode again.
                        const NodeId entryNodeId = nkIt->nodeID;
                        auto groupKeySet         = *gksIt;
                        ReturnErrorOnFailure(mDelegate->SyncNode(nkIt->nodeID, groupKeySet, [this, entryNodeId, groupKeySetId]() {
                            detail::MarkEntryCommittedIfFound(mNodeKeySetEntries, [&](const auto & e) {
                                return e.nodeID == entryNodeId && e.groupKeySetID == groupKeySetId;
                            });
                        }));
                        ++nkIt;
                    }
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
                }
                else
                {
                    // TODO: update binding on actual device.
                    // TODO: check if change has been made, retry if not. Mark as committed upon success.
                    // TODO: retry delete, remove entry if successful.
                }
            }
        }

        ++it;
    }

    // 6.
    ReturnErrorOnFailure(SetNode(nodeId, Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted));

    for (Listener * listener = mListeners; listener != nullptr; listener = listener->mNext)
    {
        listener->MarkNodeListChanged();
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
JointFabricDatastore::AddGroupKeySetEntry(Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet)
{
    VerifyOrReturnError(IsGroupKeySetEntryPresent(groupKeySet.groupKeySetID) == false, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mGroupKeySetList.size() < kMaxGroupKeySet, CHIP_ERROR_NO_MEMORY);

<<<<<<< HEAD
    mGroupKeySetList.push_back(groupKeySet);
=======
    Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type copiedKeySet;
    ReturnErrorOnFailure(CopyGroupKeySetWithOwnedSpans(groupKeySet, copiedKeySet));

    mGroupKeySetList.push_back(copiedKeySet);
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))

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
    for (auto it = mGroupKeySetList.begin(); it != mGroupKeySetList.end(); ++it)
    {
        if (it->groupKeySetID == groupKeySetId)
        {
            mGroupKeySetList.erase(it);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR
JointFabricDatastore::UpdateGroupKeySetEntry(
    Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet)
{
    for (auto & entry : mGroupKeySetList)
    {
        if (entry.groupKeySetID == groupKeySet.groupKeySetID)
        {
            entry = groupKeySet;

<<<<<<< HEAD
            ReturnErrorOnFailure(UpdateNodeKeySetList(entry));
=======
            VerifyOrReturnValue(groupKeySet.groupKeySecurityPolicy <
                                    Clusters::JointFabricDatastore::DatastoreGroupKeySecurityPolicyEnum::kUnknownEnumValue,
                                CHIP_IM_GLOBAL_STATUS(ConstraintError));

            ReturnErrorOnFailure(CopyGroupKeySetWithOwnedSpans(groupKeySet, entry));
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))

            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR
JointFabricDatastore::AddAdmin(
    Clusters::JointFabricDatastore::Structs::DatastoreAdministratorInformationEntryStruct::Type & adminId)
{
    VerifyOrReturnError(IsAdminEntryPresent(adminId.nodeID) == false, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mAdminEntries.size() < kMaxAdminNodes, CHIP_ERROR_NO_MEMORY);

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
<<<<<<< HEAD
            entry.friendlyName = friendlyName;
            entry.icac         = icac;
=======
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
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
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
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR
JointFabricDatastore::UpdateNodeKeySetList(Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet)
{
    for (auto & entry : mNodeKeySetEntries)
    {
        if (entry.groupKeySetID == groupKeySet.groupKeySetID)
        {
<<<<<<< HEAD
            // TODO: Need to update the keySetList on the actual device
            entry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
            return CHIP_NO_ERROR;
=======
            if (groupKeySet.groupKeySecurityPolicy <
                Clusters::JointFabricDatastore::DatastoreGroupKeySecurityPolicyEnum::kUnknownEnumValue)
            {

                const NodeId entryNodeId          = entry.nodeID;
                const uint16_t entryGroupKeySetID = groupKeySet.groupKeySetID;
                LogErrorOnFailure(mDelegate->SyncNode(entry.nodeID, groupKeySet, [this, entryNodeId, entryGroupKeySetID]() {
                    detail::MarkEntryCommittedIfFound(mNodeKeySetEntries, [&](const auto & e) {
                        return e.nodeID == entryNodeId && e.groupKeySetID == entryGroupKeySetID;
                    });
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
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
        }
    }

    return CHIP_ERROR_NOT_FOUND;
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

    return CHIP_ERROR_NOT_FOUND;
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
    groupEntry.friendlyName    = commandData.friendlyName;
    groupEntry.groupKeySetID   = commandData.groupKeySetID;
    groupEntry.groupCAT        = commandData.groupCAT;
    groupEntry.groupCATVersion = commandData.groupCATVersion;
    groupEntry.groupPermission = commandData.groupPermission;

    // Add the group entry to the datastore
    mGroupInformationEntries.push_back(groupEntry);

    return CHIP_NO_ERROR;
}

CHIP_ERROR
JointFabricDatastore::UpdateGroup(const Clusters::JointFabricDatastore::Commands::UpdateGroup::DecodableType & commandData)
{
    size_t index = 0;
    // Check if the group ID exists in the datastore
    VerifyOrReturnError(IsGroupIDInDatastore(commandData.groupID, index) == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    if (commandData.groupCAT.ValueOr(0) == kAdminCATIdentifier || commandData.groupCAT.ValueOr(0) == kAnchorCATIdentifier)
    {
        // If the group is an AdminCAT or AnchorCAT, we cannot update it
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    // Update the group entry with the new data
    if (commandData.friendlyName.IsNull() == false)
    {
        if (mGroupInformationEntries[index].friendlyName.data_equal(commandData.friendlyName.Value()) == false)
        {
<<<<<<< HEAD
            // TODO: Iterate through each Endpoint Information Entry:
            // If the GroupIDList contains an entry with the given GroupID:
            // Update the GroupIDList Entry in the Datastore with the new values and Status Pending
            // Update the Groups on the given Node with the new values....
=======
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

                    const NodeId entryNodeId         = epGroupEntry.nodeID;
                    const EndpointId entryEndpointId = epGroupEntry.endpointID;
                    CHIP_ERROR syncErr               = mDelegate->SyncNode(
                        epGroupEntry.nodeID, entryToSync, [this, entryNodeId, entryEndpointId, updatedGroupId]() {
                            detail::MarkEntryCommittedIfFound(mEndpointGroupIDEntries, [&](const auto & e) {
                                return e.nodeID == entryNodeId && e.endpointID == entryEndpointId && e.groupID == updatedGroupId;
                            });
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
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
        }
        // Update the friendly name
        mGroupInformationEntries[index].friendlyName = commandData.friendlyName.Value();
    }
    if (commandData.groupKeySetID.IsNull() == false)
    {
        if (mGroupInformationEntries[index].groupKeySetID != commandData.groupKeySetID.Value())
        {
            // If the groupKeySetID is being updated, we need to ensure that the new key set exists
            ReturnErrorOnFailure(AddNodeKeySetEntry(commandData.groupID, commandData.groupKeySetID.Value()));
            ReturnErrorOnFailure(RemoveNodeKeySetEntry(
                commandData.groupID, mGroupInformationEntries[index].groupKeySetID.Value())); // Remove the old key set
        }
        mGroupInformationEntries[index].groupKeySetID = commandData.groupKeySetID.Value();
    }

    bool anyGroupCATFieldUpdated = false;

    if (commandData.groupCAT.IsNull() == false)
    {
        if (mGroupInformationEntries[index].groupCAT != commandData.groupCAT.Value())
        {
            anyGroupCATFieldUpdated = true;
        }
        // Update the groupCAT
        mGroupInformationEntries[index].groupCAT = commandData.groupCAT.Value();
    }
    if (commandData.groupCATVersion.IsNull() == false)
    {
        if (mGroupInformationEntries[index].groupCATVersion != commandData.groupCATVersion.Value())
        {
            anyGroupCATFieldUpdated = true;
        }
        mGroupInformationEntries[index].groupCATVersion = commandData.groupCATVersion.Value();
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
<<<<<<< HEAD
        // TODO: iterate through NodeACLList for entries referencing this group.
        // if found, set status to pending, update corresponding ACL on device, and then mark as committed.
=======
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
            // Re-resolve by stable key (nodeID + listID) inside the completion; capturing the loop
            // index would mark the wrong/invalid slot if an interleaved Invoke mutated the vector.
            const NodeId entryNodeId   = acl.nodeID;
            const uint16_t entryListId = acl.listID;
            ReturnErrorOnFailure(mDelegate->SyncNode(acl.nodeID, entryToEncode, [this, entryNodeId, entryListId]() {
                detail::MarkEntryCommittedIfFound(
                    mACLEntries, [&](const auto & e) { return e.nodeID == entryNodeId && e.listID == entryListId; });
            }));
        }
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
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

    if (it->groupCAT == kAdminCATIdentifier || it->groupCAT == kAnchorCATIdentifier)
    {
        // If the group is an AdminCAT or AnchorCAT, we cannot remove it
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mGroupInformationEntries.erase(it);

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
    size_t index = 0;
    ReturnErrorOnFailure(IsNodeIdAndEndpointInEndpointInformationEntries(nodeId, endpointId, index));

    VerifyOrReturnError(IsGroupIDInDatastore(groupId, index) == CHIP_ERROR_NOT_FOUND, CHIP_IM_GLOBAL_STATUS(ConstraintError));

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

            // TODO: Update device

            mNodeKeySetEntries.push_back(newNodeKeySet);

<<<<<<< HEAD
            mNodeKeySetEntries.back().statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
=======
            ReturnErrorOnFailure(mDelegate->SyncNode(nodeId, newNodeKeySet, [this, nodeId, groupKeySetID]() {
                detail::MarkEntryCommittedIfFound(mNodeKeySetEntries, [&](const auto & entry) {
                    return entry.nodeID == nodeId && entry.groupKeySetID == groupKeySetID;
                });
            }));
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
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

    // TODO: Update device

    // Add the new ACL entry to the datastore
    mEndpointGroupIDEntries.push_back(newGroupEntry);

<<<<<<< HEAD
    mEndpointGroupIDEntries.back().statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;

    return CHIP_NO_ERROR;
=======
    return mDelegate->SyncNode(nodeId, newGroupEntry, [this, nodeId, endpointId, groupId]() {
        detail::MarkEntryCommittedIfFound(mEndpointGroupIDEntries, [&](const auto & entry) {
            return entry.nodeID == nodeId && entry.endpointID == endpointId && entry.groupID == groupId;
        });
    });
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
}

CHIP_ERROR JointFabricDatastore::RemoveGroupIDFromEndpointForNode(NodeId nodeId, chip::EndpointId endpointId, chip::GroupId groupId)
{
    size_t index = 0;
    ReturnErrorOnFailure(IsNodeIdAndEndpointInEndpointInformationEntries(nodeId, endpointId, index));

    for (auto it = mEndpointGroupIDEntries.begin(); it != mEndpointGroupIDEntries.end(); ++it)
    {
        if (it->nodeID == nodeId && it->endpointID == endpointId && it->groupID == groupId)
        {
            it->statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;

            // TODO: Update device

            mEndpointGroupIDEntries.erase(it);

            if (IsGroupIDInDatastore(groupId, index) == CHIP_NO_ERROR)
            {
                for (auto it2 = mNodeKeySetEntries.begin(); it2 != mNodeKeySetEntries.end();)
                {
                    bool incrementIndex = true;

                    if (it2->nodeID == nodeId && mGroupInformationEntries[index].groupKeySetID.IsNull() == false &&
                        it2->groupKeySetID == mGroupInformationEntries[index].groupKeySetID.Value())
                    {
                        it2->statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;
                        // TODO: Update device
                        it2 = mNodeKeySetEntries.erase(it2);

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

<<<<<<< HEAD
    // TODO: Update device

    mEndpointBindingEntries.back().statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;

    return CHIP_NO_ERROR;
=======
    const uint16_t listID = newBindingEntry.listID;
    return mDelegate->SyncNode(nodeId, newBindingEntry, [this, nodeId, endpointId, listID]() {
        detail::MarkEntryCommittedIfFound(mEndpointBindingEntries, [&](const auto & entry) {
            return entry.nodeID == nodeId && entry.endpointID == endpointId && entry.listID == listID;
        });
    });
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
}

CHIP_ERROR
JointFabricDatastore::RemoveBindingFromEndpointForNode(uint16_t listId, NodeId nodeId, chip::EndpointId endpointId)
{
    size_t index = 0;
    ReturnErrorOnFailure(IsNodeIdAndEndpointInEndpointInformationEntries(nodeId, endpointId, index));

    for (auto it = mEndpointBindingEntries.begin(); it != mEndpointBindingEntries.end(); ++it)
    {
        if (it->nodeID == nodeId && it->listID == listId && it->endpointID == endpointId)
        {
            it->statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;
<<<<<<< HEAD

            // TODO: Update device

            mEndpointBindingEntries.erase(it);

            return CHIP_NO_ERROR;
=======
            // Re-resolve by stable key inside the async completion instead of capturing the raw
            // iterator (which dangles if an interleaved Add*/Remove* reallocates the vector).
            return mDelegate->SyncNode(nodeId, *it, [this, listId, nodeId, endpointId]() {
                mEndpointBindingEntries.erase(std::remove_if(mEndpointBindingEntries.begin(), mEndpointBindingEntries.end(),
                                                             [&](const auto & entry) {
                                                                 return entry.nodeID == nodeId && entry.listID == listId &&
                                                                     entry.endpointID == endpointId;
                                                             }),
                                              mEndpointBindingEntries.end());
            });
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
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

    {
        auto iter = aclEntry.subjects.Value().begin();
        while (iter.Next())
        {
            newACLEntry.ACLEntry.subjects.push_back(iter.GetValue());
        }
        ReturnErrorOnFailure(iter.GetStatus());
    }

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

    // TODO: Update device

    mACLEntries.back().statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;

    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabricDatastore::RemoveACLFromNode(uint16_t listId, NodeId nodeId)
{
    size_t index = 0;
    ReturnErrorOnFailure(IsNodeIdInNodeInformationEntries(nodeId, index));

    for (auto it = mACLEntries.begin(); it != mACLEntries.end(); ++it)
    {
        if (it->nodeID == nodeId && it->listID == listId)
        {
            it->statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;
<<<<<<< HEAD
            // TODO: Update device
            mACLEntries.erase(it);
            return CHIP_NO_ERROR;
=======

            // initialize struct to indicate nodeid/listid and status set to DeletePending for the SyncNode call to delete the ACL
            // entry on the node
            Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type entryToDelete{ 0 };
            entryToDelete.nodeID            = it->nodeID;
            entryToDelete.listID            = it->listID;
            entryToDelete.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;
            // Re-resolve by stable key inside the async completion instead of capturing the raw
            // iterator (which dangles if an interleaved Add*/Remove* reallocates the vector).
            return mDelegate->SyncNode(nodeId, entryToDelete, [this, listId, nodeId]() {
                mACLEntries.erase(
                    std::remove_if(mACLEntries.begin(), mACLEntries.end(),
                                   [&](const auto & entry) { return entry.nodeID == nodeId && entry.listID == listId; }),
                    mACLEntries.end());
            });
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR JointFabricDatastore::AddNodeKeySetEntry(GroupId groupId, uint16_t groupKeySetId)
{
    // TODO: Iterate through all nodes that are part of this group and add an entry for each node

    // If the key set does not exist, create a new entry
    Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type newEntry;
    newEntry.nodeID            = groupId; // Using groupId as nodeId temporarily, will be updated when iterating through nodes
    newEntry.groupKeySetID     = groupKeySetId;
    newEntry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kPending;

    mNodeKeySetEntries.push_back(newEntry);

    // TODO: (1) Add keyset to device, (2) Update group entry on device to point to this keyset

<<<<<<< HEAD
    // After adding the new entry, we can set it to committed
    mNodeKeySetEntries.back().statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
=======
            mNodeKeySetEntries.push_back(newEntry);

            // Sync to the node and mark committed on success. Re-resolve by stable key inside the
            // completion; capturing the index would mark the wrong/invalid slot if an interleaved
            // Invoke mutated the vector before the async completion fires.
            ReturnErrorOnFailure(mDelegate->SyncNode(nodeId, newEntry, [this, nodeId, groupKeySetId]() {
                detail::MarkEntryCommittedIfFound(
                    mNodeKeySetEntries, [&](const auto & e) { return e.nodeID == nodeId && e.groupKeySetID == groupKeySetId; });
            }));
        }
    }
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))

    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabricDatastore::RemoveNodeKeySetEntry(GroupId groupId, uint16_t groupKeySetId)
{
    // NOTE: this method assumes its ok to remove the keyset from each node (its not in use by any group)

    // TODO: Iterate through all nodes that are part of this group and add an entry for each node

    bool any_node_removed = false;

    for (auto it = mNodeKeySetEntries.begin(); it != mNodeKeySetEntries.end();)
    {
        // TODO: Iterate trough nodes in the group
        if (it->groupKeySetID == groupKeySetId)
        {
            any_node_removed = true;

            it->statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;

            // TODO: Update device: send command to it->nodeID to remove this keyset

            it = mNodeKeySetEntries.erase(it);
        }
        else
        {
            ++it;
        }
    }

<<<<<<< HEAD
    return any_node_removed ? CHIP_NO_ERROR : CHIP_ERROR_NOT_FOUND;
=======
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

    // Sync to the node and mark committed on success. Re-resolve by stable key inside the completion
    // rather than capturing the index, which an interleaved Invoke could invalidate.
    return mDelegate->SyncNode(nodeId, newEntry, [this, nodeId, groupKeySetId]() {
        detail::MarkEntryCommittedIfFound(mNodeKeySetEntries,
                                          [&](const auto & e) { return e.nodeID == nodeId && e.groupKeySetID == groupKeySetId; });
    });
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

    mNodeKeySetEntries.push_back(newEntry);
    return CHIP_NO_ERROR;
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
}

} // namespace app
} // namespace chip
