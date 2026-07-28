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

    return CHIP_ERROR_KEY_NOT_FOUND;
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

    return CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR JointFabricDatastore::RefreshNode(NodeId nodeId)
{
    // 1. && 2.
    ReturnErrorOnFailure(SetNode(nodeId, Clusters::JointFabricDatastore::DatastoreStateEnum::kPending));

    // 3. TODO: Read the PartsList of the Descriptor cluster from the Node.

    // 4. TODO

    // 5. TODO

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

    return CHIP_ERROR_KEY_NOT_FOUND;
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

    return CHIP_ERROR_KEY_NOT_FOUND;
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

            // TODO: RefreshNodes

            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_KEY_NOT_FOUND;
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

    return CHIP_ERROR_KEY_NOT_FOUND;
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

<<<<<<< HEAD
    return CHIP_ERROR_KEY_NOT_FOUND;
=======
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

            ReturnErrorOnFailure(mDelegate->SyncNode(nodeId, newNodeKeySet, [this, nodeId, groupKeySetID]() {
                detail::MarkEntryCommittedIfFound(mNodeKeySetEntries, [&](const auto & entry) {
                    return entry.nodeID == nodeId && entry.groupKeySetID == groupKeySetID;
                });
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

    return mDelegate->SyncNode(nodeId, newGroupEntry, [this, nodeId, endpointId, groupId]() {
        detail::MarkEntryCommittedIfFound(mEndpointGroupIDEntries, [&](const auto & entry) {
            return entry.nodeID == nodeId && entry.endpointID == endpointId && entry.groupID == groupId;
        });
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
            it->statusEntry.state       = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;
            const auto erasedNodeId     = it->nodeID;
            const auto erasedEndpointId = it->endpointID;
            const auto erasedGroupId    = it->groupID;
            ReturnErrorOnFailure(mDelegate->SyncNode(nodeId, *it, [this, erasedNodeId, erasedEndpointId, erasedGroupId]() {
                for (auto eraseIt = mEndpointGroupIDEntries.begin(); eraseIt != mEndpointGroupIDEntries.end(); ++eraseIt)
                {
                    if (eraseIt->nodeID == erasedNodeId && eraseIt->endpointID == erasedEndpointId &&
                        eraseIt->groupID == erasedGroupId)
                    {
                        mEndpointGroupIDEntries.erase(eraseIt);
                        break;
                    }
                }
            }));

            if (IsGroupIDInDatastore(groupId, index) == CHIP_NO_ERROR)
            {
                for (auto it2 = mNodeKeySetEntries.begin(); it2 != mNodeKeySetEntries.end(); ++it2)
                {
                    if (it2->nodeID == nodeId && mGroupInformationEntries[index].groupKeySetID.IsNull() == false &&
                        it2->groupKeySetID == mGroupInformationEntries[index].groupKeySetID.Value())
                    {
                        it2->statusEntry.state         = Clusters::JointFabricDatastore::DatastoreStateEnum::kDeletePending;
                        const auto erasedKeySetNodeId  = it2->nodeID;
                        const auto erasedKeySetGroupId = it2->groupKeySetID;
                        ReturnErrorOnFailure(mDelegate->SyncNode(nodeId, *it2, [this, erasedKeySetNodeId, erasedKeySetGroupId]() {
                            for (auto eraseIt = mNodeKeySetEntries.begin(); eraseIt != mNodeKeySetEntries.end(); ++eraseIt)
                            {
                                if (eraseIt->nodeID == erasedKeySetNodeId && eraseIt->groupKeySetID == erasedKeySetGroupId)
                                {
                                    mNodeKeySetEntries.erase(eraseIt);
                                    break;
                                }
                            }
                        }));

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

    const uint16_t listID = newBindingEntry.listID;
    return mDelegate->SyncNode(nodeId, newBindingEntry, [this, nodeId, endpointId, listID]() {
        detail::MarkEntryCommittedIfFound(mEndpointBindingEntries, [&](const auto & entry) {
            return entry.nodeID == nodeId && entry.endpointID == endpointId && entry.listID == listID;
        });
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

    if (acl2.subjects.IsNull())
    {
        if (!acl1.subjects.empty())
        {
            return false;
        }
    }
    else
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

        if (it1 != acl1.subjects.end() || it2.Next())
        {
            return false;
        }
    }

    if (acl2.targets.IsNull())
    {
        if (!acl1.targets.empty())
        {
            return false;
        }
    }
    else
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

        if (it1 != acl1.targets.end() || it2.Next())
        {
            return false;
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
    const auto & storedEntry = mACLEntries.back();

    Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type entryToEncode;
    entryToEncode.nodeID             = storedEntry.nodeID;
    entryToEncode.listID             = storedEntry.listID;
    entryToEncode.ACLEntry.authMode  = storedEntry.ACLEntry.authMode;
    entryToEncode.ACLEntry.privilege = storedEntry.ACLEntry.privilege;
    entryToEncode.ACLEntry.subjects =
        DataModel::List<const uint64_t>(storedEntry.ACLEntry.subjects.data(), storedEntry.ACLEntry.subjects.size());
    entryToEncode.ACLEntry.targets =
        DataModel::List<const Clusters::JointFabricDatastore::Structs::DatastoreAccessControlTargetStruct::Type>(
            storedEntry.ACLEntry.targets.data(), storedEntry.ACLEntry.targets.size());
    entryToEncode.statusEntry = storedEntry.statusEntry;

    const auto committedNodeId = storedEntry.nodeID;
    const auto committedListId = storedEntry.listID;

    return mDelegate->SyncNode(nodeId, entryToEncode, [this, committedNodeId, committedListId]() {
        for (auto & entry : mACLEntries)
        {
            if (entry.nodeID == committedNodeId && entry.listID == committedListId)
            {
                entry.statusEntry.state = Clusters::JointFabricDatastore::DatastoreStateEnum::kCommitted;
                break;
            }
        }
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

            // Sync to the node and mark committed on success. Re-resolve by stable key inside the
            // completion; capturing the index would mark the wrong/invalid slot if an interleaved
            // Invoke mutated the vector before the async completion fires.
            ReturnErrorOnFailure(mDelegate->SyncNode(nodeId, newEntry, [this, nodeId, groupKeySetId]() {
                detail::MarkEntryCommittedIfFound(
                    mNodeKeySetEntries, [&](const auto & e) { return e.nodeID == nodeId && e.groupKeySetID == groupKeySetId; });
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
>>>>>>> 90f5775d89 (Apply AI-generated patch for unchecked nullable dereferences in JFDS (#73096))
}

} // namespace app
} // namespace chip
