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

namespace chip {
namespace app {

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
                // TODO: Remove the binding from the actual device. If successful, then remove the entry.
                // otherwise update status to CommitFailed and return error
                it = mEndpointGroupIDEntries.erase(it);
                continue;
            }
            else if (it->statusEntry.state == Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitFailed)
            {
                CHIP_ERROR failureCode(it->statusEntry.failureCode);

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
            }
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
                    // TODO: update binding on actual device.
                    // TODO: check if change has been made, retry if not. Mark as committed upon success.
                    // TODO: retry delete, remove entry if successful.
                }
            }
        }

        ++it;
    }

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

    mGroupKeySetList.push_back(groupKeySet);

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

            ReturnErrorOnFailure(UpdateNodeKeySetList(entry));

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
            entry.friendlyName = friendlyName;
            entry.icac         = icac;
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
            // TODO: Need to update the keySetList on the actual device
            entry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
            return CHIP_NO_ERROR;
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
            // TODO: Iterate through each Endpoint Information Entry:
            // If the GroupIDList contains an entry with the given GroupID:
            // Update the GroupIDList Entry in the Datastore with the new values and Status Pending
            // Update the Groups on the given Node with the new values....
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
        // TODO: iterate through NodeACLList for entries referencing this group.
        // if found, set status to pending, update corresponding ACL on device, and then mark as committed.
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

            mNodeKeySetEntries.back().statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
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

    mEndpointGroupIDEntries.back().statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;

    return CHIP_NO_ERROR;
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

    // TODO: Update device

    mEndpointBindingEntries.back().statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;

    return CHIP_NO_ERROR;
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

            // TODO: Update device

            mEndpointBindingEntries.erase(it);

            return CHIP_NO_ERROR;
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
            // TODO: Update device
            mACLEntries.erase(it);
            return CHIP_NO_ERROR;
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

    // After adding the new entry, we can set it to committed
    mNodeKeySetEntries.back().statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;

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

    return any_node_removed ? CHIP_NO_ERROR : CHIP_ERROR_NOT_FOUND;
}

} // namespace app
} // namespace chip
