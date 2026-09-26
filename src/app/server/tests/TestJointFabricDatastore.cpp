#include "app/server/JointFabricDatastore.h"

#include <lib/core/CASEAuthTag.h>
#include <protocols/interaction_model/StatusCode.h>
#include <pw_unit_test/framework.h>

#include <map>
#include <optional>

using namespace chip;
using namespace chip::app;

namespace {

namespace JointFabricCluster   = chip::app::Clusters::JointFabricDatastore;
using GroupKeySetType          = JointFabricCluster::Structs::DatastoreGroupKeySetStruct::Type;
using AdminEntryType           = JointFabricCluster::Structs::DatastoreAdministratorInformationEntryStruct::Type;
using EndpointEntryType        = JointFabricCluster::Structs::DatastoreEndpointEntryStruct::Type;
using EndpointGroupIdEntryType = JointFabricCluster::Structs::DatastoreEndpointGroupIDEntryStruct::Type;
using NodeKeySetEntryType      = JointFabricCluster::Structs::DatastoreNodeKeySetEntryStruct::Type;
using GroupInfoEntryType       = JointFabricCluster::Structs::DatastoreGroupInformationEntryStruct::Type;
using BindingEntryType         = JointFabricCluster::Structs::DatastoreEndpointBindingEntryStruct::Type;
using ACLEntryType             = JointFabricCluster::Structs::DatastoreACLEntryStruct::Type;
using AclType                  = JointFabricCluster::Structs::DatastoreAccessControlEntryStruct::Type;
using TargetType               = JointFabricCluster::Structs::DatastoreAccessControlTargetStruct::Type;
using Privilege                = JointFabricCluster::DatastoreAccessControlEntryPrivilegeEnum;
using AuthMode                 = JointFabricCluster::DatastoreAccessControlEntryAuthModeEnum;
using State                    = JointFabricCluster::DatastoreStateEnum;

void SeedAcl(JointFabricDatastore & store, NodeId nodeId, uint16_t listId, Privilege privilege, AuthMode authMode,
             std::vector<uint64_t> subjects, State state)
{
    datastore::ACLEntryStruct entry;
    entry.nodeID             = nodeId;
    entry.listID             = listId;
    entry.ACLEntry.privilege = privilege;
    entry.ACLEntry.authMode  = authMode;
    entry.ACLEntry.subjects  = std::move(subjects);
    entry.statusEntry.state  = state;
    store.GetNodeACLList().push_back(std::move(entry));
}

const datastore::ACLEntryStruct * FindAcl(JointFabricDatastore & store, NodeId nodeId, uint16_t listId)
{
    for (const auto & entry : store.GetNodeACLList())
    {
        if (entry.nodeID == nodeId && entry.listID == listId)
        {
            return &entry;
        }
    }
    return nullptr;
}

// Deep copy, so a recorded payload stays valid after the datastore mutates.
datastore::AccessControlEntryStruct ToOwned(const JointFabricCluster::Structs::DatastoreAccessControlEntryStruct::Type & source)
{
    datastore::AccessControlEntryStruct owned;
    owned.privilege = source.privilege;
    owned.authMode  = source.authMode;
    if (!source.subjects.IsNull())
    {
        owned.subjects.assign(source.subjects.Value().begin(), source.subjects.Value().end());
    }
    if (!source.targets.IsNull())
    {
        owned.targets.assign(source.targets.Value().begin(), source.targets.Value().end());
    }
    return owned;
}

void ExpectCharSpanEquals(const CharSpan & actual, const char * expected)
{
    EXPECT_TRUE(actual.data_equal(CharSpan::fromCharString(expected)));
}

void ExpectByteSpanEquals(const ByteSpan & actual, const ByteSpan & expected)
{
    EXPECT_TRUE(actual.data_equal(expected));
}

void ExpectNullableByteSpanEquals(const DataModel::Nullable<ByteSpan> & actual, const ByteSpan & expected)
{
    ASSERT_FALSE(actual.IsNull());
    ExpectByteSpanEquals(actual.Value(), expected);
}

class DummyListener : public JointFabricDatastore::Listener
{
public:
    void MarkNodeListChanged() override { mNotified = true; }
    void Reset() { mNotified = false; }

    bool mNotified = false;
};

enum class SyncKind : uint8_t
{
    kEndpointGroup,
    kNodeKeySet,
    kBinding,
    kBindingList,
    kAcl,
    kAclList,
    kGroupKeySet,
};

class TrackingDelegate : public JointFabricDatastore::Delegate
{
public:
    CHIP_ERROR SyncNode(NodeId nodeId, const EndpointGroupIdEntryType & endpointGroupIDEntry,
                        std::function<void(CHIP_ERROR)> onSuccess) override
    {
        lastEndpointGroupSync    = endpointGroupIDEntry;
        hasLastEndpointGroupSync = true;
        return Dispatch(nodeId, SyncKind::kEndpointGroup, std::move(onSuccess));
    }

    CHIP_ERROR SyncNode(NodeId nodeId, const NodeKeySetEntryType & nodeKeySetEntry,
                        std::function<void(CHIP_ERROR)> onSuccess) override
    {
        lastNodeKeySetSync    = nodeKeySetEntry;
        hasLastNodeKeySetSync = true;
        return Dispatch(nodeId, SyncKind::kNodeKeySet, std::move(onSuccess));
    }

    CHIP_ERROR SyncNode(NodeId nodeId, const BindingEntryType & bindingEntry, std::function<void(CHIP_ERROR)> onSuccess) override
    {
        lastBindingSync    = bindingEntry;
        hasLastBindingSync = true;
        return Dispatch(nodeId, SyncKind::kBinding, std::move(onSuccess));
    }

    CHIP_ERROR SyncNode(NodeId nodeId, EndpointId endpointId, std::vector<BindingEntryType> & bindingEntries,
                        std::function<void(CHIP_ERROR)> onSuccess) override
    {
        bindingListSyncs.emplace_back(nodeId, bindingEntries);
        return Dispatch(nodeId, SyncKind::kBindingList, std::move(onSuccess));
    }

    CHIP_ERROR SyncNode(NodeId nodeId, std::vector<BindingEntryType> & bindingEntries,
                        std::function<void(CHIP_ERROR)> onSuccess) override
    {
        bindingListSyncs.emplace_back(nodeId, bindingEntries);
        return Dispatch(nodeId, SyncKind::kBindingList, std::move(onSuccess));
    }

    CHIP_ERROR SyncNode(NodeId nodeId, const ACLEntryType & aclEntry, const std::optional<AclType> & superseded,
                        std::function<void(CHIP_ERROR)> onSuccess) override
    {
        lastAclSync      = aclEntry;
        hasLastAclSync   = true;
        lastAclSyncOwned = ToOwned(aclEntry.ACLEntry);
        lastAclSyncState = aclEntry.statusEntry.state;
        lastAclSuperseded.reset();
        if (superseded.has_value())
        {
            lastAclSuperseded = ToOwned(*superseded);
        }
        return Dispatch(nodeId, SyncKind::kAcl, std::move(onSuccess));
    }

    CHIP_ERROR SyncNode(NodeId nodeId, const std::vector<ACLEntryType> & aclEntries,
                        std::function<void(CHIP_ERROR)> onSuccess) override
    {
        auto & written = aclListSyncs.emplace_back(nodeId, std::vector<datastore::AccessControlEntryStruct>()).second;
        for (const auto & aclEntry : aclEntries)
        {
            written.push_back(ToOwned(aclEntry.ACLEntry));
        }
        return Dispatch(nodeId, SyncKind::kAclList, std::move(onSuccess));
    }

    CHIP_ERROR SyncNode(NodeId nodeId, const GroupKeySetType & groupKeySet, std::function<void(CHIP_ERROR)> onSuccess) override
    {
        return Dispatch(nodeId, SyncKind::kGroupKeySet, std::move(onSuccess));
    }

    // Completes synchronously unless the kind is deferred or set up to fail to start.
    CHIP_ERROR Dispatch(NodeId nodeId, SyncKind kind, std::function<void(CHIP_ERROR)> onCompletion)
    {
        syncCalls.emplace_back(nodeId, kind);
        if (auto it = failStartWith.find(kind); it != failStartWith.end())
        {
            CHIP_ERROR err = it->second;
            failStartWith.erase(it);
            return err;
        }
        if (deferKind == kind)
        {
            deferred.push_back(std::move(onCompletion));
            return CHIP_NO_ERROR;
        }
        CHIP_ERROR result = CHIP_NO_ERROR;
        if (auto it = completeWith.find(kind); it != completeWith.end())
        {
            result = it->second;
            completeWith.erase(it);
        }
        if (onCompletion)
        {
            onCompletion(result);
        }
        return CHIP_NO_ERROR;
    }

    void RunDeferred(size_t index = 0, CHIP_ERROR result = CHIP_NO_ERROR)
    {
        auto callback = std::move(deferred.at(index));
        deferred.erase(deferred.begin() + static_cast<std::ptrdiff_t>(index));
        callback(result);
    }

    CHIP_ERROR FetchEndpointList(NodeId nodeId,
                                 std::function<void(CHIP_ERROR, const std::vector<EndpointEntryType> &)> onSuccess) override
    {
        ++fetchEndpointListCalls;
        onSuccess(CHIP_NO_ERROR, endpointsToFetch);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FetchEndpointGroupList(NodeId nodeId, EndpointId endpointId,
                                      std::function<void(CHIP_ERROR, const std::vector<GroupInfoEntryType> &)> onSuccess) override
    {
        std::vector<GroupInfoEntryType> groups;
        if (auto it = groupsToFetch.find(endpointId); it != groupsToFetch.end())
        {
            for (const auto groupId : it->second)
            {
                GroupInfoEntryType group;
                group.groupID = groupId;
                groups.push_back(group);
            }
        }
        onSuccess(fetchGroupListResult, fetchGroupListResult == CHIP_NO_ERROR ? groups : std::vector<GroupInfoEntryType>());
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FetchEndpointBindingList(NodeId nodeId, EndpointId endpointId,
                                        std::function<void(CHIP_ERROR, const std::vector<BindingEntryType> &)> onSuccess) override
    {
        std::vector<BindingEntryType> bindings;
        for (const auto & binding : bindingsToFetch)
        {
            if (binding.endpointID == endpointId)
            {
                bindings.push_back(binding);
            }
        }
        onSuccess(CHIP_NO_ERROR, bindings);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FetchGroupKeySetList(NodeId nodeId,
                                    std::function<void(CHIP_ERROR, const std::vector<uint16_t> &)> onSuccess) override
    {
        ++fetchGroupKeySetListCalls;
        onSuccess(CHIP_NO_ERROR, fetchedGroupKeySetIDs);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FetchGroupKeySet(NodeId nodeId, uint16_t groupKeySetID,
                                std::function<void(CHIP_ERROR, const GroupKeySetType &)> onSuccess) override
    {
        ++fetchGroupKeySetCalls;

        fetchedGroupKeySet.groupKeySetID = groupKeySetID;
        fetchedGroupKeySet.epochKey0.SetNonNull(ByteSpan(epochKey0));
        fetchedGroupKeySet.epochKey1.SetNonNull(ByteSpan(epochKey1));
        fetchedGroupKeySet.epochKey2.SetNull();
        onSuccess(CHIP_NO_ERROR, fetchedGroupKeySet);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FetchACLList(NodeId nodeId, std::function<void(CHIP_ERROR, const std::vector<ACLEntryType> &)> onSuccess) override
    {
        ++fetchAclListCalls;
        // Views over aclListToFetch. nodeID and listID stay 0: a node's ACL entries carry neither.
        std::vector<ACLEntryType> acls;
        for (const auto & held : aclListToFetch)
        {
            ACLEntryType acl;
            acl.ACLEntry.privilege = held.ACLEntry.privilege;
            acl.ACLEntry.authMode  = held.ACLEntry.authMode;
            acl.ACLEntry.subjects.SetNonNull(held.ACLEntry.subjects.data(), held.ACLEntry.subjects.size());
            if (held.ACLEntry.targets.empty())
            {
                acl.ACLEntry.targets.SetNull();
            }
            else
            {
                acl.ACLEntry.targets.SetNonNull(held.ACLEntry.targets.data(), held.ACLEntry.targets.size());
            }
            acls.push_back(acl);
        }
        onSuccess(fetchAclListResult, fetchAclListResult == CHIP_NO_ERROR ? acls : std::vector<ACLEntryType>());
        return CHIP_NO_ERROR;
    }

    void ResetCapturedSyncs()
    {
        hasLastEndpointGroupSync = false;
        hasLastNodeKeySetSync    = false;
        hasLastBindingSync       = false;
        hasLastAclSync           = false;
    }

    std::map<SyncKind, CHIP_ERROR> completeWith;  // result for the next completion of that kind (consumed)
    std::map<SyncKind, CHIP_ERROR> failStartWith; // SyncNode returns this synchronously (consumed)
    std::optional<SyncKind> deferKind;            // completions of this kind are captured, not run
    std::vector<std::function<void(CHIP_ERROR)>> deferred;
    std::vector<std::pair<NodeId, SyncKind>> syncCalls;
    std::vector<std::pair<NodeId, std::vector<datastore::AccessControlEntryStruct>>> aclListSyncs;
    std::vector<std::pair<NodeId, std::vector<BindingEntryType>>> bindingListSyncs;
    std::vector<BindingEntryType> bindingsToFetch;
    CHIP_ERROR fetchAclListResult   = CHIP_NO_ERROR;
    CHIP_ERROR fetchGroupListResult = CHIP_NO_ERROR;
    std::vector<datastore::ACLEntryStruct> aclListToFetch;
    std::map<EndpointId, std::vector<GroupId>> groupsToFetch;
    std::vector<EndpointEntryType> endpointsToFetch;
    std::vector<uint16_t> fetchedGroupKeySetIDs;
    GroupKeySetType fetchedGroupKeySet;
    uint8_t epochKey0[3]          = { 0x10, 0x11, 0x12 };
    uint8_t epochKey1[2]          = { 0x20, 0x21 };
    int fetchEndpointListCalls    = 0;
    int fetchGroupKeySetListCalls = 0;
    int fetchGroupKeySetCalls     = 0;
    int fetchAclListCalls         = 0;
    EndpointGroupIdEntryType lastEndpointGroupSync;
    NodeKeySetEntryType lastNodeKeySetSync;
    BindingEntryType lastBindingSync;
    ACLEntryType lastAclSync; // views the datastore's storage; valid only until the datastore changes
    std::optional<datastore::AccessControlEntryStruct> lastAclSyncOwned;
    std::optional<datastore::AccessControlEntryStruct> lastAclSuperseded;
    State lastAclSyncState        = State::kUnknownEnumValue;
    bool hasLastEndpointGroupSync = false;
    bool hasLastNodeKeySetSync    = false;
    bool hasLastBindingSync       = false;
    bool hasLastAclSync           = false;
};

TEST(JointFabricDatastoreTest, AddPendingNodeNotifiesListener)
{
    JointFabricDatastore store;
    DummyListener listener;

    store.AddListener(listener);

    CHIP_ERROR err = store.AddPendingNode(123, "controller-a"_span);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(listener.mNotified);
}

TEST(JointFabricDatastoreTest, RemoveListenerPreventsNotification)
{
    JointFabricDatastore store;
    DummyListener listener;

    store.AddListener(listener);
    store.RemoveListener(listener);
    listener.Reset();

    CHIP_ERROR err = store.AddPendingNode(456, "controller-b"_span);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(listener.mNotified);
}

TEST(JointFabricDatastoreTest, RefreshNonExistentNodeFails)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;

    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);

    CHIP_ERROR err = store.RefreshNode(999);
    EXPECT_NE(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, UpdateNodeChangesFriendlyNameAndNotifiesListener)
{
    JointFabricDatastore store;
    DummyListener listener;
    TrackingDelegate delegate;

    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    store.AddListener(listener);

    ASSERT_EQ(store.AddPendingNode(123, "original-name"_span), CHIP_NO_ERROR);
    listener.Reset();

    ASSERT_EQ(store.UpdateNode(123, "updated-name"_span), CHIP_NO_ERROR);
    EXPECT_TRUE(listener.mNotified);

    ASSERT_EQ(store.GetNodeInformationEntries().size(), 1u);
    ExpectCharSpanEquals(store.GetNodeInformationEntries()[0].friendlyName, "updated-name");
}

TEST(JointFabricDatastoreTest, AddGroupKeySetEntryOwnsSpanData)
{
    JointFabricDatastore store;

    uint8_t originalEpochKey0[] = { 0x01, 0x02, 0x03 };
    uint8_t originalEpochKey1[] = { 0x11, 0x12 };
    uint8_t expectedEpochKey0[] = { 0x01, 0x02, 0x03 };
    uint8_t expectedEpochKey1[] = { 0x11, 0x12 };

    GroupKeySetType keySet;
    keySet.groupKeySetID = 11;
    keySet.epochKey0.SetNonNull(ByteSpan(originalEpochKey0));
    keySet.epochKey1.SetNonNull(ByteSpan(originalEpochKey1));
    keySet.epochKey2.SetNull();

    ASSERT_EQ(store.AddGroupKeySetEntry(keySet), CHIP_NO_ERROR);

    originalEpochKey0[0] = 0xEE;
    originalEpochKey1[1] = 0xFF;

    ASSERT_EQ(store.GetGroupKeySetList().size(), 1u);
    const auto & stored = store.GetGroupKeySetList()[0];
    ExpectNullableByteSpanEquals(stored.epochKey0, ByteSpan(expectedEpochKey0));
    ExpectNullableByteSpanEquals(stored.epochKey1, ByteSpan(expectedEpochKey1));
    EXPECT_TRUE(stored.epochKey2.IsNull());
}

// UpdateKeySet: the stored key set is updated, then each node entry for it is marked Pending and synced, and
// Committed once the node has it.
TEST(JointFabricDatastoreTest, UpdateKeySetStoresKeySetThenSyncsNodeEntries)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);

    uint8_t epochKey0[]    = { 0x01, 0x02, 0x03 };
    uint8_t newEpochKey0[] = { 0x04, 0x05, 0x06 };
    GroupKeySetType keySet;
    keySet.groupKeySetID = 11;
    keySet.epochKey0.SetNonNull(ByteSpan(epochKey0));
    keySet.epochKey1.SetNull();
    keySet.epochKey2.SetNull();
    ASSERT_EQ(store.AddGroupKeySetEntry(keySet), CHIP_NO_ERROR);
    ASSERT_EQ(store.ForceAddNodeKeySetEntry(11, 123), CHIP_NO_ERROR);

    keySet.epochKey0.SetNonNull(ByteSpan(newEpochKey0));
    delegate.deferKind = SyncKind::kNodeKeySet;
    ASSERT_EQ(store.UpdateGroupKeySetEntry(keySet), CHIP_NO_ERROR);

    ExpectNullableByteSpanEquals(store.GetGroupKeySetList()[0].epochKey0, ByteSpan(newEpochKey0));
    EXPECT_EQ(store.GetNodeKeySetList()[0].statusEntry.state, State::kPending);
    ASSERT_EQ(delegate.deferred.size(), 1u);
    EXPECT_EQ(delegate.lastNodeKeySetSync.groupKeySetID, 11u);
    EXPECT_EQ(delegate.lastNodeKeySetSync.statusEntry.state, State::kPending);

    delegate.RunDeferred();
    EXPECT_EQ(store.GetNodeKeySetList()[0].statusEntry.state, State::kCommitted);
}

TEST(JointFabricDatastoreTest, AddAndUpdateAdminOwnsFriendlyNameAndIcac)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;

    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);

    char initialFriendlyName[]    = "admin-one";
    uint8_t initialIcac[]         = { 0x01, 0x02, 0x03 };
    uint8_t expectedInitialIcac[] = { 0x01, 0x02, 0x03 };

    AdminEntryType admin;
    admin.nodeID       = 100;
    admin.vendorID     = static_cast<VendorId>(55);
    admin.friendlyName = CharSpan(initialFriendlyName, sizeof(initialFriendlyName) - 1);
    admin.icac         = ByteSpan(initialIcac);

    ASSERT_EQ(store.AddAdmin(admin), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetAdminEntries().size(), 1u);

    initialFriendlyName[0] = 'x';
    initialIcac[0]         = 0xAA;

    const auto & storedAdmin = store.GetAdminEntries()[0];
    ExpectCharSpanEquals(storedAdmin.friendlyName, "admin-one");
    ExpectByteSpanEquals(storedAdmin.icac, ByteSpan(expectedInitialIcac));

    char updatedFriendlyName[]    = "admin-two";
    uint8_t updatedIcac[]         = { 0x0A, 0x0B };
    uint8_t expectedUpdatedIcac[] = { 0x0A, 0x0B };

    ASSERT_EQ(store.UpdateAdmin(100, MakeOptional(CharSpan(updatedFriendlyName, sizeof(updatedFriendlyName) - 1)),
                                MakeOptional(ByteSpan(updatedIcac))),
              CHIP_NO_ERROR);

    updatedFriendlyName[0] = 'y';
    updatedIcac[0]         = 0xCC;

    ExpectCharSpanEquals(store.GetAdminEntries()[0].friendlyName, "admin-two");
    ExpectByteSpanEquals(store.GetAdminEntries()[0].icac, ByteSpan(expectedUpdatedIcac));
}

TEST(JointFabricDatastoreTest, AddGroupAndUpdateEndpointOwnBufferBackedFriendlyNames)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;

    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);

    char groupName[] = "living-room";
    JointFabricCluster::Commands::AddGroup::DecodableType addGroup;
    addGroup.groupID      = 10;
    addGroup.friendlyName = CharSpan(groupName, sizeof(groupName) - 1);
    addGroup.groupKeySetID.SetNonNull(99);
    addGroup.groupPermission = JointFabricCluster::DatastoreAccessControlEntryPrivilegeEnum::kView;

    ASSERT_EQ(store.AddGroup(addGroup), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetGroupEntries().size(), 1u);

    groupName[0] = 'x';
    ExpectCharSpanEquals(store.GetGroupEntries()[0].friendlyName, "living-room");

    char endpointName[] = "switch-1";
    ASSERT_EQ(store.TestAddEndpointEntry(2, 123, CharSpan(endpointName, sizeof(endpointName) - 1)), CHIP_NO_ERROR);

    endpointName[0] = 'y';
    ASSERT_EQ(store.GetNodeEndpointList().size(), 1u);
    ExpectCharSpanEquals(store.GetNodeEndpointList()[0].friendlyName, "switch-1");

    char updatedEndpointName[] = "switch-main";
    ASSERT_EQ(store.UpdateEndpointForNode(123, 2, CharSpan(updatedEndpointName, sizeof(updatedEndpointName) - 1)), CHIP_NO_ERROR);

    updatedEndpointName[0] = 'z';
    ExpectCharSpanEquals(store.GetNodeEndpointList()[0].friendlyName, "switch-main");
}

// Regression for UpdateGroup dereferencing a stored null Nullable.
//
// AddGroup copies the request's groupKeySetID / groupCAT / groupCATVersion into the stored entry
// verbatim, with no non-null requirement, so any of them can be persisted as a null Nullable. When a
// later UpdateGroup supplies a non-null value for that same field, it enters the "value changed"
// branch and reads the stored Nullable with .Value() without first checking IsNull(), which throws
// std::bad_optional_access (or std::terminate under -fno-exceptions). The fix guards each stored read
// with an IsNull() || prefix so a null stored field is treated as changed.
//
// friendlyName is left null in these tests so UpdateGroup does not enter the delegate SyncNode path;
// a minimal delegate (the group is referenced by no endpoints) is sufficient.
TEST(JointFabricDatastoreTest, UpdateGroupWithNullStoredGroupKeySetIDSucceeds)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);

    JointFabricCluster::Commands::AddGroup::DecodableType addGroup;
    addGroup.groupID      = 10;
    addGroup.friendlyName = "group-a"_span;
    // groupKeySetID left default-constructed == null.
    addGroup.groupPermission = JointFabricCluster::DatastoreAccessControlEntryPrivilegeEnum::kView;
    ASSERT_EQ(store.AddGroup(addGroup), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetGroupEntries().size(), 1u);
    ASSERT_TRUE(store.GetGroupEntries()[0].groupKeySetID.IsNull());

    JointFabricCluster::Commands::UpdateGroup::DecodableType updateGroup;
    updateGroup.groupID = 10;
    updateGroup.groupKeySetID.SetNonNull(static_cast<uint16_t>(5));
    EXPECT_EQ(store.UpdateGroup(updateGroup), CHIP_NO_ERROR);

    ASSERT_EQ(store.GetGroupEntries().size(), 1u);
    ASSERT_FALSE(store.GetGroupEntries()[0].groupKeySetID.IsNull());
    EXPECT_EQ(store.GetGroupEntries()[0].groupKeySetID.Value(), 5u);
}

TEST(JointFabricDatastoreTest, UpdateGroupWithNullStoredGroupCATSucceeds)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);

    JointFabricCluster::Commands::AddGroup::DecodableType addGroup;
    addGroup.groupID      = 11;
    addGroup.friendlyName = "group-b"_span;
    // groupCAT left default-constructed == null.
    addGroup.groupPermission = JointFabricCluster::DatastoreAccessControlEntryPrivilegeEnum::kView;
    ASSERT_EQ(store.AddGroup(addGroup), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetGroupEntries().size(), 1u);
    ASSERT_TRUE(store.GetGroupEntries()[0].groupCAT.IsNull());

    JointFabricCluster::Commands::UpdateGroup::DecodableType updateGroup;
    updateGroup.groupID = 11;
    updateGroup.groupCAT.SetNonNull(static_cast<uint16_t>(0x1234));
    EXPECT_EQ(store.UpdateGroup(updateGroup), CHIP_NO_ERROR);

    ASSERT_EQ(store.GetGroupEntries().size(), 1u);
    ASSERT_FALSE(store.GetGroupEntries()[0].groupCAT.IsNull());
    EXPECT_EQ(store.GetGroupEntries()[0].groupCAT.Value(), 0x1234u);
}

TEST(JointFabricDatastoreTest, UpdateGroupWithNullStoredGroupCATVersionSucceeds)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);

    JointFabricCluster::Commands::AddGroup::DecodableType addGroup;
    addGroup.groupID      = 12;
    addGroup.friendlyName = "group-c"_span;
    // groupCATVersion left default-constructed == null.
    addGroup.groupPermission = JointFabricCluster::DatastoreAccessControlEntryPrivilegeEnum::kView;
    ASSERT_EQ(store.AddGroup(addGroup), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetGroupEntries().size(), 1u);
    ASSERT_TRUE(store.GetGroupEntries()[0].groupCATVersion.IsNull());

    JointFabricCluster::Commands::UpdateGroup::DecodableType updateGroup;
    updateGroup.groupID = 12;
    updateGroup.groupCATVersion.SetNonNull(static_cast<uint16_t>(7));
    EXPECT_EQ(store.UpdateGroup(updateGroup), CHIP_NO_ERROR);

    ASSERT_EQ(store.GetGroupEntries().size(), 1u);
    ASSERT_FALSE(store.GetGroupEntries()[0].groupCATVersion.IsNull());
    EXPECT_EQ(store.GetGroupEntries()[0].groupCATVersion.Value(), 7u);
}

TEST(JointFabricDatastoreTest, RefreshNodeFetchesGroupKeySetsAndCommitsNode)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;

    delegate.fetchedGroupKeySetIDs = { 77 };

    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "controller-a"_span), CHIP_NO_ERROR);

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.fetchEndpointListCalls, 1);
    EXPECT_EQ(delegate.fetchGroupKeySetListCalls, 1);
    EXPECT_EQ(delegate.fetchGroupKeySetCalls, 1);
    EXPECT_EQ(delegate.fetchAclListCalls, 1);

    ASSERT_EQ(store.GetGroupKeySetList().size(), 1u);
    uint8_t expectedEpochKey0[] = { 0x10, 0x11, 0x12 };
    uint8_t expectedEpochKey1[] = { 0x20, 0x21 };

    delegate.epochKey0[0] = 0xAA;
    delegate.epochKey1[0] = 0xBB;

    const auto & storedKeySet = store.GetGroupKeySetList()[0];
    EXPECT_EQ(storedKeySet.groupKeySetID, 77);
    ExpectNullableByteSpanEquals(storedKeySet.epochKey0, ByteSpan(expectedEpochKey0));
    ExpectNullableByteSpanEquals(storedKeySet.epochKey1, ByteSpan(expectedEpochKey1));

    ASSERT_EQ(store.GetNodeInformationEntries().size(), 1u);
    EXPECT_EQ(store.GetNodeInformationEntries()[0].commissioningStatusEntry.state,
              JointFabricCluster::DatastoreStateEnum::kCommitted);
}

TEST(JointFabricDatastoreTest, AddGroupIDToEndpointForNodeAddsMissingNodeKeySet)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;

    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "controller-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.TestAddEndpointEntry(1, 123, "endpoint-a"_span), CHIP_NO_ERROR);

    JointFabricCluster::Commands::AddGroup::DecodableType addGroup;
    addGroup.groupID      = 10;
    addGroup.friendlyName = "group-a"_span;
    addGroup.groupKeySetID.SetNonNull(55);
    addGroup.groupPermission = JointFabricCluster::DatastoreAccessControlEntryPrivilegeEnum::kView;

    ASSERT_EQ(store.AddGroup(addGroup), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);

    ASSERT_EQ(store.GetEndpointGroupIDList().size(), 1u);
    ASSERT_EQ(store.GetNodeKeySetList().size(), 1u);

    const auto & nodeKeySet = store.GetNodeKeySetList()[0];
    EXPECT_EQ(nodeKeySet.nodeID, 123u);
    EXPECT_EQ(nodeKeySet.groupKeySetID, 55u);
    // AddGroupIDToEndpointForNode: "If this succeeds, update the new KeySet entry in the Datastore to Committed."
    EXPECT_EQ(nodeKeySet.statusEntry.state, JointFabricCluster::DatastoreStateEnum::kCommitted);

    ASSERT_TRUE(delegate.hasLastNodeKeySetSync);
    EXPECT_EQ(delegate.lastNodeKeySetSync.nodeID, 123u);
    EXPECT_EQ(delegate.lastNodeKeySetSync.groupKeySetID, 55u);
    EXPECT_EQ(delegate.lastNodeKeySetSync.statusEntry.state, JointFabricCluster::DatastoreStateEnum::kPending);

    ASSERT_TRUE(delegate.hasLastEndpointGroupSync);
    EXPECT_EQ(delegate.lastEndpointGroupSync.nodeID, 123u);
    EXPECT_EQ(delegate.lastEndpointGroupSync.endpointID, 1u);
    EXPECT_EQ(delegate.lastEndpointGroupSync.groupID, 10u);
    EXPECT_EQ(delegate.lastEndpointGroupSync.statusEntry.state, JointFabricCluster::DatastoreStateEnum::kPending);
}

TEST(JointFabricDatastoreTest, AddGroupIDToEndpointForNodeIsIdempotentForDuplicateAssignments)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;

    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "controller-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.TestAddEndpointEntry(1, 123, "endpoint-a"_span), CHIP_NO_ERROR);

    JointFabricCluster::Commands::AddGroup::DecodableType addGroup;
    addGroup.groupID      = 10;
    addGroup.friendlyName = "group-a"_span;
    addGroup.groupKeySetID.SetNonNull(55);
    addGroup.groupPermission = JointFabricCluster::DatastoreAccessControlEntryPrivilegeEnum::kView;

    ASSERT_EQ(store.AddGroup(addGroup), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetEndpointGroupIDList().size(), 1u);
    ASSERT_EQ(store.GetNodeKeySetList().size(), 1u);

    delegate.ResetCapturedSyncs();

    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);

    EXPECT_EQ(store.GetEndpointGroupIDList().size(), 1u);
    EXPECT_EQ(store.GetNodeKeySetList().size(), 1u);
    EXPECT_FALSE(delegate.hasLastEndpointGroupSync);
    EXPECT_FALSE(delegate.hasLastNodeKeySetSync);
}

TEST(JointFabricDatastoreTest, RemoveGroupIdFromEndpointSyncsDeletePendingEntries)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;

    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "controller-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.TestAddEndpointEntry(1, 123, "endpoint-a"_span), CHIP_NO_ERROR);

    JointFabricCluster::Commands::AddGroup::DecodableType addGroup;
    addGroup.groupID      = 10;
    addGroup.friendlyName = "group-a"_span;
    addGroup.groupKeySetID.SetNonNull(55);
    addGroup.groupPermission = JointFabricCluster::DatastoreAccessControlEntryPrivilegeEnum::kView;

    ASSERT_EQ(store.AddGroup(addGroup), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetEndpointGroupIDList().size(), 1u);
    ASSERT_EQ(store.GetNodeKeySetList().size(), 1u);

    delegate.ResetCapturedSyncs();

    ASSERT_EQ(store.RemoveGroupIDFromEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    ASSERT_TRUE(delegate.hasLastEndpointGroupSync);
    ASSERT_TRUE(delegate.hasLastNodeKeySetSync);

    EXPECT_EQ(delegate.lastEndpointGroupSync.nodeID, 123u);
    EXPECT_EQ(delegate.lastEndpointGroupSync.endpointID, 1u);
    EXPECT_EQ(delegate.lastEndpointGroupSync.groupID, 10u);
    EXPECT_EQ(delegate.lastEndpointGroupSync.statusEntry.state, JointFabricCluster::DatastoreStateEnum::kDeletePending);

    EXPECT_EQ(delegate.lastNodeKeySetSync.nodeID, 123u);
    EXPECT_EQ(delegate.lastNodeKeySetSync.groupKeySetID, 55u);
    EXPECT_EQ(delegate.lastNodeKeySetSync.statusEntry.state, JointFabricCluster::DatastoreStateEnum::kDeletePending);

    EXPECT_TRUE(store.GetEndpointGroupIDList().empty());
    EXPECT_TRUE(store.GetNodeKeySetList().empty());
}

TEST(JointFabricDatastoreTest, AddBindingAssignsListIdAndStoresCommittedEntry)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;

    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "controller-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.TestAddEndpointEntry(1, 123, "endpoint-a"_span), CHIP_NO_ERROR);

    JointFabricCluster::Structs::DatastoreBindingTargetStruct::Type binding;
    binding.node.SetValue(0x1111);
    binding.endpoint.SetValue(2);

    ASSERT_EQ(store.AddBindingToEndpointForNode(123, 1, binding), CHIP_NO_ERROR);
    ASSERT_TRUE(delegate.hasLastBindingSync);

    ASSERT_EQ(store.GetEndpointBindingList().size(), 1u);
    const auto & storedBinding = store.GetEndpointBindingList()[0];
    EXPECT_EQ(storedBinding.nodeID, 123u);
    EXPECT_EQ(storedBinding.endpointID, 1u);
    EXPECT_TRUE(storedBinding.binding.node.HasValue());
    EXPECT_EQ(storedBinding.binding.node.Value(), 0x1111u);
    EXPECT_EQ(storedBinding.statusEntry.state, JointFabricCluster::DatastoreStateEnum::kCommitted);
}

TEST(JointFabricDatastoreTest, RemoveBindingSyncsDeletePendingPayload)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;

    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "controller-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.TestAddEndpointEntry(1, 123, "endpoint-a"_span), CHIP_NO_ERROR);

    JointFabricCluster::Structs::DatastoreBindingTargetStruct::Type binding;
    binding.group.SetValue(10);

    ASSERT_EQ(store.AddBindingToEndpointForNode(123, 1, binding), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetEndpointBindingList().size(), 1u);

    const uint16_t listId = store.GetEndpointBindingList()[0].listID;
    delegate.ResetCapturedSyncs();

    ASSERT_EQ(store.RemoveBindingFromEndpointForNode(listId, 123, 1), CHIP_NO_ERROR);
    ASSERT_TRUE(delegate.hasLastBindingSync);
    EXPECT_EQ(delegate.lastBindingSync.nodeID, 123u);
    EXPECT_EQ(delegate.lastBindingSync.endpointID, 1u);
    EXPECT_EQ(delegate.lastBindingSync.listID, listId);
    EXPECT_EQ(delegate.lastBindingSync.statusEntry.state, JointFabricCluster::DatastoreStateEnum::kDeletePending);
    EXPECT_TRUE(store.GetEndpointBindingList().empty());
}

TEST(JointFabricDatastoreTest, AddAclDeduplicatesAndRemoveAclSyncsDeletePayload)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;

    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "controller-a"_span), CHIP_NO_ERROR);

    JointFabricCluster::Structs::DatastoreAccessControlEntryStruct::DecodableType aclEntry;
    aclEntry.privilege = JointFabricCluster::DatastoreAccessControlEntryPrivilegeEnum::kView;
    aclEntry.authMode  = JointFabricCluster::DatastoreAccessControlEntryAuthModeEnum::kCase;

    ASSERT_EQ(store.AddACLToNode(123, aclEntry), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetNodeACLList().size(), 1u);

    const uint16_t listId = store.GetNodeACLList()[0].listID;
    EXPECT_EQ(store.GetNodeACLList()[0].statusEntry.state, JointFabricCluster::DatastoreStateEnum::kCommitted);

    delegate.ResetCapturedSyncs();
    ASSERT_EQ(store.AddACLToNode(123, aclEntry), CHIP_NO_ERROR);
    EXPECT_EQ(store.GetNodeACLList().size(), 1u);
    EXPECT_FALSE(delegate.hasLastAclSync);

    ASSERT_EQ(store.RemoveACLFromNode(listId, 123), CHIP_NO_ERROR);
    ASSERT_TRUE(delegate.hasLastAclSync);
    EXPECT_EQ(delegate.lastAclSync.nodeID, 123u);
    EXPECT_EQ(delegate.lastAclSync.listID, listId);
    EXPECT_EQ(delegate.lastAclSync.statusEntry.state, JointFabricCluster::DatastoreStateEnum::kDeletePending);
    EXPECT_TRUE(store.GetNodeACLList().empty());
}
// Round-trips `value` through TLV, as the cluster receives it. The decoded lists read from `buffer`.
CHIP_ERROR DecodeAcl(const AclType & value, uint8_t * buffer, size_t bufferSize,
                     JointFabricCluster::Structs::DatastoreAccessControlEntryStruct::DecodableType & decoded)
{
    TLV::TLVWriter writer;
    writer.Init(buffer, bufferSize);
    ReturnErrorOnFailure(DataModel::Encode(writer, TLV::AnonymousTag(), value));
    ReturnErrorOnFailure(writer.Finalize());

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    ReturnErrorOnFailure(reader.Next());
    return DataModel::Decode(reader, decoded);
}

// The node matches ACL entries with subjects in any order, so the datastore does too: two such entries
// would be one on the node, and removing either would remove both.
TEST(JointFabricDatastoreTest, AddAclDeduplicatesRegardlessOfSubjectOrder)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);

    const uint64_t subjectsAB[] = { 0x1111, 0x2222 };
    const uint64_t subjectsBA[] = { 0x2222, 0x1111 };
    AclType value;
    value.privilege = Privilege::kView;
    value.authMode  = AuthMode::kCase;
    value.targets.SetNull();

    uint8_t bufferAB[64];
    JointFabricCluster::Structs::DatastoreAccessControlEntryStruct::DecodableType aclAB;
    value.subjects.SetNonNull(Span<const uint64_t>(subjectsAB));
    ASSERT_EQ(DecodeAcl(value, bufferAB, sizeof(bufferAB), aclAB), CHIP_NO_ERROR);

    uint8_t bufferBA[64];
    JointFabricCluster::Structs::DatastoreAccessControlEntryStruct::DecodableType aclBA;
    value.subjects.SetNonNull(Span<const uint64_t>(subjectsBA));
    ASSERT_EQ(DecodeAcl(value, bufferBA, sizeof(bufferBA), aclBA), CHIP_NO_ERROR);

    ASSERT_EQ(store.AddACLToNode(123, aclAB), CHIP_NO_ERROR);
    delegate.ResetCapturedSyncs();
    ASSERT_EQ(store.AddACLToNode(123, aclBA), CHIP_NO_ERROR);

    EXPECT_EQ(store.GetNodeACLList().size(), 1u);
    EXPECT_FALSE(delegate.hasLastAclSync);
}

// Regression for the JointFabricDatastore async-callback iterator use-after-free. Run under ASan
// (the existing out/asan unit-test config) to catch the heap-use-after-free in the unpatched code.

// Repro: RemoveBindingFromEndpointForNode captures a raw iterator into the async SyncNode
// completion. SyncNode is an async CASE round-trip; the completion fires later. If a second
// Invoke (AddBindingToEndpointForNode) grows mEndpointBindingEntries and the std::vector
// reallocates in the meantime, the captured iterator dangles and erase(it) is a heap UAF.
// The delegate below defers the completion (modelling the round-trip); the test forces a
// reallocation and then replays it.
class DeferringBindingDelegate : public TrackingDelegate
{
public:
    bool deferNext   = false;
    bool hasDeferred = false;
    std::function<void(CHIP_ERROR)> deferred;

    CHIP_ERROR SyncNode(NodeId nodeId, const BindingEntryType & bindingEntry, std::function<void(CHIP_ERROR)> onSuccess) override
    {
        if (deferNext)
        {
            deferNext   = false;
            hasDeferred = true;
            deferred    = std::move(onSuccess);
            return CHIP_NO_ERROR;
        }
        if (onSuccess)
        {
            onSuccess(CHIP_NO_ERROR);
        }
        return CHIP_NO_ERROR;
    }

    void RunDeferred()
    {
        if (hasDeferred && deferred)
        {
            deferred(CHIP_NO_ERROR);
        }
    }
};

TEST(JointFabricDatastoreTest, RemoveBindingIteratorUseAfterFree)
{
    JointFabricDatastore store;
    DeferringBindingDelegate delegate;

    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "controller-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.TestAddEndpointEntry(1, 123, "endpoint-a"_span), CHIP_NO_ERROR);

    // Binding that will be removed (entry 0).
    JointFabricCluster::Structs::DatastoreBindingTargetStruct::Type b0;
    b0.node.SetValue(0x1000);
    b0.endpoint.SetValue(2);
    ASSERT_EQ(store.AddBindingToEndpointForNode(123, 1, b0), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetEndpointBindingList().size(), 1u);
    const uint16_t listId0 = store.GetEndpointBindingList()[0].listID;

    // Remove it, but defer the erase callback (models the async SyncNode round-trip).
    delegate.deferNext = true;
    ASSERT_EQ(store.RemoveBindingFromEndpointForNode(listId0, 123, 1), CHIP_NO_ERROR);
    ASSERT_TRUE(delegate.hasDeferred);

    // A second admin Invoke arrives during the round-trip: add more bindings, forcing
    // mEndpointBindingEntries to reallocate. The captured iterator now dangles.
    for (uint16_t i = 1; i <= 12; ++i)
    {
        JointFabricCluster::Structs::DatastoreBindingTargetStruct::Type bi;
        bi.node.SetValue(static_cast<NodeId>(0x2000 + i));
        bi.endpoint.SetValue(2);
        ASSERT_EQ(store.AddBindingToEndpointForNode(123, 1, bi), CHIP_NO_ERROR);
    }

    // The SyncNode response finally arrives: erase(it) on the dangling iterator.
    // RED  -> ASan heap-use-after-free at the erase(it) site in RemoveBindingFromEndpointForNode.
    // GREEN (stable-key re-resolution fix) -> entry listId0 erased correctly, no UAF.
    delegate.RunDeferred();

    for (const auto & e : store.GetEndpointBindingList())
    {
        EXPECT_NE(e.listID, listId0);
    }
}

// Regression for the CRITICAL site: RemoveACLFromNode captured a raw iterator
// [this, it] into the async SyncNode completion and called mACLEntries.erase(it). An interleaved
// Invoke that reallocates mACLEntries leaves that iterator dangling -> heap use-after-free in
// erase(). The fix re-resolves the entry by stable key (nodeID + listID) inside the completion.
class DeferringAclDelegate : public TrackingDelegate
{
public:
    bool deferNext   = false;
    bool hasDeferred = false;
    std::function<void(CHIP_ERROR)> deferred;

    CHIP_ERROR SyncNode(NodeId nodeId, const ACLEntryType & aclEntry, const std::optional<AclType> & superseded,
                        std::function<void(CHIP_ERROR)> onSuccess) override
    {
        if (deferNext)
        {
            deferNext   = false;
            hasDeferred = true;
            deferred    = std::move(onSuccess);
            return CHIP_NO_ERROR;
        }
        if (onSuccess)
        {
            onSuccess(CHIP_NO_ERROR);
        }
        return CHIP_NO_ERROR;
    }

    void RunDeferred()
    {
        if (hasDeferred && deferred)
        {
            deferred(CHIP_NO_ERROR);
        }
    }
};

TEST(JointFabricDatastoreTest, RemoveAclIteratorUseAfterFree)
{
    JointFabricDatastore store;
    DeferringAclDelegate delegate;

    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "controller-a"_span), CHIP_NO_ERROR);

    JointFabricCluster::Structs::DatastoreAccessControlEntryStruct::DecodableType aclEntry;
    aclEntry.privilege = JointFabricCluster::DatastoreAccessControlEntryPrivilegeEnum::kView;
    aclEntry.authMode  = JointFabricCluster::DatastoreAccessControlEntryAuthModeEnum::kCase;

    // ACL that will be removed (entry 0, on node 123).
    ASSERT_EQ(store.AddACLToNode(123, aclEntry), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetNodeACLList().size(), 1u);
    const uint16_t listId0 = store.GetNodeACLList()[0].listID;

    // Remove it, but defer the erase callback (models the async SyncNode round-trip).
    delegate.deferNext = true;
    ASSERT_EQ(store.RemoveACLFromNode(listId0, 123), CHIP_NO_ERROR);
    ASSERT_TRUE(delegate.hasDeferred);

    // A second admin Invoke arrives during the round-trip: register more nodes and add ACLs,
    // forcing mACLEntries to reallocate. The captured iterator now dangles.
    for (uint16_t i = 1; i <= 12; ++i)
    {
        const NodeId extraNode = static_cast<NodeId>(200 + i);
        ASSERT_EQ(store.AddPendingNode(extraNode, "controller-b"_span), CHIP_NO_ERROR);
        ASSERT_EQ(store.AddACLToNode(extraNode, aclEntry), CHIP_NO_ERROR);
    }

    // The SyncNode response finally arrives: erase(it) on the dangling iterator.
    // RED  -> ASan heap-use-after-free at the erase(it) site in RemoveACLFromNode.
    // GREEN (stable-key re-resolution fix) -> entry listId0 erased correctly, no UAF.
    delegate.RunDeferred();

    for (const auto & e : store.GetNodeACLList())
    {
        EXPECT_FALSE(e.nodeID == 123u && e.listID == listId0);
    }
}

// Regression for a back() site: AddBindingToEndpointForNode marked
// mEndpointBindingEntries.back() as kCommitted in its async completion. If an interleaved Invoke
// appends more bindings before the completion fires, back() now refers to the wrong (newest) entry
// and the binding that was actually synced is left stuck Pending. The fix re-resolves by stable key
// (nodeID + endpointID + listID). This site is in-bounds (logic corruption), so the assertion checks
// the correct entry is committed rather than relying on ASan.
TEST(JointFabricDatastoreTest, AddBindingBackReferenceMarksWrongEntry)
{
    JointFabricDatastore store;
    DeferringBindingDelegate delegate;

    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "controller-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.TestAddEndpointEntry(1, 123, "endpoint-a"_span), CHIP_NO_ERROR);

    // Add the binding whose commit we defer (models the async SyncNode round-trip).
    JointFabricCluster::Structs::DatastoreBindingTargetStruct::Type b1;
    b1.node.SetValue(0x1000);
    b1.endpoint.SetValue(2);
    delegate.deferNext = true;
    ASSERT_EQ(store.AddBindingToEndpointForNode(123, 1, b1), CHIP_NO_ERROR);
    ASSERT_TRUE(delegate.hasDeferred);
    const uint16_t listId1 = store.GetEndpointBindingList().back().listID;

    // A second admin Invoke appends more bindings, reallocating the vector and moving back().
    for (uint16_t i = 2; i <= 13; ++i)
    {
        JointFabricCluster::Structs::DatastoreBindingTargetStruct::Type bi;
        bi.node.SetValue(static_cast<NodeId>(0x2000 + i));
        bi.endpoint.SetValue(2);
        ASSERT_EQ(store.AddBindingToEndpointForNode(123, 1, bi), CHIP_NO_ERROR);
    }

    // The deferred commit arrives. RED (back()) would commit the newest entry, leaving listId1 Pending.
    // GREEN (stable-key) commits listId1 correctly.
    delegate.RunDeferred();

    bool found = false;
    for (const auto & e : store.GetEndpointBindingList())
    {
        if (e.listID == listId1)
        {
            found = true;
            EXPECT_EQ(e.statusEntry.state, JointFabricCluster::DatastoreStateEnum::kCommitted);
        }
    }
    EXPECT_TRUE(found);
}

// When removing the anchor fabric, every record must be wiped and the anchor identity must be reset.
// When other fabrics are removed, nothing should happen (the datastore holds no records owned by other fabrics.)
// statusEntry.failureCode holds an IM status code, so the triage compares it as one. ConstraintError
// is unrecoverable: the entry is dropped.
TEST(JointFabricDatastoreTest, RefreshDropsUnrecoverableCommitFailedAcl)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);

    SeedAcl(store, 123, 7, Privilege::kView, AuthMode::kCase, { 0x1111 }, State::kCommitFailed);
    store.GetNodeACLList().back().statusEntry.failureCode = to_underlying(Protocols::InteractionModel::Status::ConstraintError);

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    EXPECT_EQ(FindAcl(store, 123, 7), nullptr);
}

// TIMEOUT is recoverable: the entry survives the refresh and is retried.
TEST(JointFabricDatastoreTest, RefreshRetriesRecoverableCommitFailedAcl)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);

    SeedAcl(store, 123, 7, Privilege::kView, AuthMode::kCase, { 0x1111 }, State::kCommitFailed);
    store.GetNodeACLList().back().statusEntry.failureCode = to_underlying(Protocols::InteractionModel::Status::Timeout);

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    ASSERT_NE(FindAcl(store, 123, 7), nullptr);
    EXPECT_EQ(FindAcl(store, 123, 7)->statusEntry.state, State::kCommitted);
}

// ResourceExhausted is unrecoverable: the binding entry is dropped.
TEST(JointFabricDatastoreTest, RefreshDropsUnrecoverableCommitFailedBinding)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.TestAddEndpointEntry(1, 123, "ep"_span), CHIP_NO_ERROR);

    EndpointEntryType endpoint;
    endpoint.endpointID = 1;
    endpoint.nodeID     = 123;
    delegate.endpointsToFetch.push_back(endpoint);

    BindingEntryType bindingEntry;
    bindingEntry.nodeID     = 123;
    bindingEntry.endpointID = 1;
    bindingEntry.listID     = 9;
    bindingEntry.binding.group.SetValue(10);
    bindingEntry.statusEntry.state       = State::kCommitFailed;
    bindingEntry.statusEntry.failureCode = to_underlying(Protocols::InteractionModel::Status::ResourceExhausted);
    store.GetEndpointBindingList().push_back({ bindingEntry });

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    EXPECT_TRUE(store.GetEndpointBindingList().empty());
}

// A failed ACL removal records CommitFailed and the failure code, and the entry stays marked for
// removal.
TEST(JointFabricDatastoreTest, FailedAclRemovalRecordsCommitFailedAndKeepsRemovalIntent)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 7, Privilege::kManage, AuthMode::kCase, { 0x1111 }, State::kCommitted);

    delegate.completeWith[SyncKind::kAcl] = CHIP_IM_GLOBAL_STATUS(Timeout);
    EXPECT_EQ(store.RemoveACLFromNode(7, 123), CHIP_NO_ERROR);

    const auto * entry = FindAcl(store, 123, 7);
    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(entry->statusEntry.state, State::kCommitFailed);
    EXPECT_EQ(entry->statusEntry.failureCode, to_underlying(Protocols::InteractionModel::Status::Timeout));
    EXPECT_TRUE(entry->pendingRemoval);
}

// RefreshNode retries a failed removal as a removal: the entry is left out of the ACL write and
// erased once the write succeeds.
TEST(JointFabricDatastoreTest, FailedAclRemovalIsRemovedByNextRefresh)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 7, Privilege::kManage, AuthMode::kCase, { 0x1111 }, State::kCommitted);

    delegate.completeWith[SyncKind::kAcl] = CHIP_IM_GLOBAL_STATUS(Timeout);
    ASSERT_EQ(store.RemoveACLFromNode(7, 123), CHIP_NO_ERROR);
    ASSERT_NE(FindAcl(store, 123, 7), nullptr);

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    ASSERT_FALSE(delegate.aclListSyncs.empty());
    for (const auto & written : delegate.aclListSyncs.back().second)
    {
        EXPECT_FALSE(written.subjects == std::vector<uint64_t>{ 0x1111 });
    }
    EXPECT_EQ(FindAcl(store, 123, 7), nullptr);
}

// A failed add records CommitFailed with the code, as the specification requires.
TEST(JointFabricDatastoreTest, FailedAclAddRecordsCommitFailed)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);

    JointFabricCluster::Structs::DatastoreAccessControlEntryStruct::DecodableType aclEntry;
    aclEntry.privilege = Privilege::kView;
    aclEntry.authMode  = AuthMode::kCase;

    delegate.completeWith[SyncKind::kAcl] = CHIP_IM_GLOBAL_STATUS(Busy);
    ASSERT_EQ(store.AddACLToNode(123, aclEntry), CHIP_NO_ERROR);

    ASSERT_EQ(store.GetNodeACLList().size(), 1u);
    EXPECT_EQ(store.GetNodeACLList()[0].statusEntry.state, State::kCommitFailed);
    EXPECT_EQ(store.GetNodeACLList()[0].statusEntry.failureCode, to_underlying(Protocols::InteractionModel::Status::Busy));
}

TEST(JointFabricDatastoreTest, SuccessfulAclRemovalStillErasesEntry)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 7, Privilege::kManage, AuthMode::kCase, { 0x1111 }, State::kCommitted);

    ASSERT_EQ(store.RemoveACLFromNode(7, 123), CHIP_NO_ERROR);

    EXPECT_EQ(FindAcl(store, 123, 7), nullptr);
}

// When SyncNode fails to start, the entry records CommitFailed and the call returns the start error.
TEST(JointFabricDatastoreTest, AclSyncStartFailureIsRecorded)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);

    JointFabricCluster::Structs::DatastoreAccessControlEntryStruct::DecodableType aclEntry;
    aclEntry.privilege = Privilege::kView;
    aclEntry.authMode  = AuthMode::kCase;

    delegate.failStartWith[SyncKind::kAcl] = CHIP_ERROR_CONNECTION_ABORTED;
    EXPECT_EQ(store.AddACLToNode(123, aclEntry), CHIP_ERROR_CONNECTION_ABORTED);

    ASSERT_EQ(store.GetNodeACLList().size(), 1u);
    EXPECT_EQ(store.GetNodeACLList()[0].statusEntry.state, State::kCommitFailed);
    EXPECT_EQ(store.GetNodeACLList()[0].statusEntry.failureCode, to_underlying(Protocols::InteractionModel::Status::Failure));
}

TEST(JointFabricDatastoreTest, FailedBindingRemovalRecordsCommitFailed)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.TestAddEndpointEntry(1, 123, "ep"_span), CHIP_NO_ERROR);

    JointFabricCluster::Structs::DatastoreBindingTargetStruct::Type binding;
    binding.group.SetValue(10);
    ASSERT_EQ(store.AddBindingToEndpointForNode(123, 1, binding), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetEndpointBindingList().size(), 1u);
    const uint16_t listId = store.GetEndpointBindingList()[0].listID;

    delegate.completeWith[SyncKind::kBinding] = CHIP_IM_GLOBAL_STATUS(Timeout);
    EXPECT_EQ(store.RemoveBindingFromEndpointForNode(listId, 123, 1), CHIP_NO_ERROR);

    ASSERT_EQ(store.GetEndpointBindingList().size(), 1u);
    EXPECT_EQ(store.GetEndpointBindingList()[0].statusEntry.state, State::kCommitFailed);
    EXPECT_EQ(store.GetEndpointBindingList()[0].statusEntry.failureCode,
              to_underlying(Protocols::InteractionModel::Status::Timeout));
}

// The node still holds the binding after the failed removal. RefreshNode leaves it out of the binding
// write and erases it once the write succeeds.
TEST(JointFabricDatastoreTest, FailedBindingRemovalIsRemovedByNextRefresh)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.TestAddEndpointEntry(1, 123, "ep"_span), CHIP_NO_ERROR);

    JointFabricCluster::Structs::DatastoreBindingTargetStruct::Type binding;
    binding.group.SetValue(10);
    ASSERT_EQ(store.AddBindingToEndpointForNode(123, 1, binding), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetEndpointBindingList().size(), 1u);
    const auto heldBinding = store.GetEndpointBindingList()[0];

    delegate.completeWith[SyncKind::kBinding] = CHIP_IM_GLOBAL_STATUS(Timeout);
    ASSERT_EQ(store.RemoveBindingFromEndpointForNode(heldBinding.listID, 123, 1), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetEndpointBindingList().size(), 1u);

    EndpointEntryType endpoint;
    endpoint.nodeID     = 123;
    endpoint.endpointID = 1;
    delegate.endpointsToFetch.push_back(endpoint);
    delegate.bindingsToFetch.push_back(heldBinding);

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    ASSERT_FALSE(delegate.bindingListSyncs.empty());
    for (const auto & written : delegate.bindingListSyncs.back().second)
    {
        EXPECT_FALSE(chip::app::detail::BindingEntryValueEquals(written, heldBinding));
    }
    EXPECT_TRUE(store.GetEndpointBindingList().empty());
}

// Binding list IDs can be reused. An add clears any removal intent left for its key, so the new
// entry is written by RefreshNode instead of being removed.
TEST(JointFabricDatastoreTest, ReAddedBindingIsNotTreatedAsRemoval)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.TestAddEndpointEntry(1, 123, "ep"_span), CHIP_NO_ERROR);

    JointFabricCluster::Structs::DatastoreBindingTargetStruct::Type binding;
    binding.group.SetValue(10);
    ASSERT_EQ(store.AddBindingToEndpointForNode(123, 1, binding), CHIP_NO_ERROR);
    const uint16_t listId = store.GetEndpointBindingList()[0].listID;

    delegate.completeWith[SyncKind::kBinding] = CHIP_IM_GLOBAL_STATUS(Timeout);
    ASSERT_EQ(store.RemoveBindingFromEndpointForNode(listId, 123, 1), CHIP_NO_ERROR);
    store.GetEndpointBindingList().clear();

    delegate.completeWith[SyncKind::kBinding] = CHIP_IM_GLOBAL_STATUS(Timeout);
    ASSERT_EQ(store.AddBindingToEndpointForNode(123, 1, binding), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetEndpointBindingList().size(), 1u);
    ASSERT_EQ(store.GetEndpointBindingList()[0].listID, listId);

    EndpointEntryType endpoint;
    endpoint.nodeID     = 123;
    endpoint.endpointID = 1;
    delegate.endpointsToFetch.push_back(endpoint);

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    ASSERT_FALSE(delegate.bindingListSyncs.empty());
    ASSERT_EQ(delegate.bindingListSyncs.back().second.size(), 1u);
    EXPECT_EQ(delegate.bindingListSyncs.back().second[0].listID, listId);
    EXPECT_EQ(store.GetEndpointBindingList().size(), 1u);
}

TEST(JointFabricDatastoreTest, FailedGroupIdRemovalRecordsCommitFailed)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.TestAddEndpointEntry(1, 123, "ep"_span), CHIP_NO_ERROR);

    JointFabricCluster::Commands::AddGroup::DecodableType addGroup;
    addGroup.groupID         = 10;
    addGroup.friendlyName    = "group-a"_span;
    addGroup.groupPermission = Privilege::kView;
    ASSERT_EQ(store.AddGroup(addGroup), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetEndpointGroupIDList().size(), 1u);

    delegate.completeWith[SyncKind::kEndpointGroup] = CHIP_IM_GLOBAL_STATUS(Timeout);
    EXPECT_EQ(store.RemoveGroupIDFromEndpointForNode(123, 1, 10), CHIP_NO_ERROR);

    ASSERT_EQ(store.GetEndpointGroupIDList().size(), 1u);
    EXPECT_EQ(store.GetEndpointGroupIDList()[0].statusEntry.state, State::kCommitFailed);
    EXPECT_EQ(store.GetEndpointGroupIDList()[0].statusEntry.failureCode,
              to_underlying(Protocols::InteractionModel::Status::Timeout));
}

// Refreshing node A and then node B sends B only B's ACL entries.
TEST(JointFabricDatastoreTest, RefreshDoesNotLeakAclEntriesAcrossNodes)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(0xA, "node-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(0xB, "node-b"_span), CHIP_NO_ERROR);
    SeedAcl(store, 0xA, 1, Privilege::kAdminister, AuthMode::kCase, { 0xAAAA }, State::kPending);
    SeedAcl(store, 0xB, 2, Privilege::kView, AuthMode::kCase, { 0xBBBB }, State::kPending);

    ASSERT_EQ(store.RefreshNode(0xA), CHIP_NO_ERROR);
    ASSERT_EQ(store.RefreshNode(0xB), CHIP_NO_ERROR);

    ASSERT_EQ(delegate.aclListSyncs.size(), 2u);
    const auto & [nodeId, entries] = delegate.aclListSyncs[1];
    EXPECT_EQ(nodeId, 0xBu);
    ASSERT_EQ(entries.size(), 1u);
    EXPECT_TRUE(entries[0].subjects == std::vector<uint64_t>{ 0xBBBB });
}

// The node stays Pending until the final ACL write completes, and that completion commits the
// entries.
TEST(JointFabricDatastoreTest, RefreshCommitsNodeOnlyAfterFinalAclSync)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 7, Privilege::kView, AuthMode::kCase, { 0x1111 }, State::kPending);

    delegate.deferKind = SyncKind::kAclList;
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    EXPECT_EQ(store.GetNodeInformationEntries()[0].commissioningStatusEntry.state, State::kPending);
    ASSERT_EQ(delegate.deferred.size(), 1u);

    delegate.RunDeferred();

    EXPECT_EQ(store.GetNodeInformationEntries()[0].commissioningStatusEntry.state, State::kCommitted);
    EXPECT_EQ(FindAcl(store, 123, 7)->statusEntry.state, State::kCommitted);
}

TEST(JointFabricDatastoreTest, FailedFinalAclSyncLeavesNodePendingAndReleasesGuard)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 7, Privilege::kView, AuthMode::kCase, { 0x1111 }, State::kPending);

    delegate.deferKind = SyncKind::kAclList;
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    ASSERT_EQ(delegate.deferred.size(), 1u);
    delegate.RunDeferred(0, CHIP_IM_GLOBAL_STATUS(Timeout));

    EXPECT_EQ(store.GetNodeInformationEntries()[0].commissioningStatusEntry.state, State::kPending);
    EXPECT_EQ(FindAcl(store, 123, 7)->statusEntry.state, State::kCommitFailed);
    EXPECT_EQ(FindAcl(store, 123, 7)->statusEntry.failureCode, to_underlying(Protocols::InteractionModel::Status::Timeout));

    delegate.deferKind.reset();
    EXPECT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
}

// A removal that fails, then fails again in a refresh, keeps its removal intent and is removed by a
// later successful refresh.
TEST(JointFabricDatastoreTest, FailedRemovalWhoseRefreshWriteFailsIsRetriedLater)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 7, Privilege::kManage, AuthMode::kCase, { 0x1111 }, State::kCommitted);
    delegate.aclListToFetch = store.GetNodeACLList(); // the node keeps the entry until a write removes it

    delegate.completeWith[SyncKind::kAcl] = CHIP_IM_GLOBAL_STATUS(Timeout);
    ASSERT_EQ(store.RemoveACLFromNode(7, 123), CHIP_NO_ERROR);

    delegate.completeWith[SyncKind::kAclList] = CHIP_IM_GLOBAL_STATUS(Busy);
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    const auto * entry = FindAcl(store, 123, 7);
    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(entry->statusEntry.state, State::kCommitFailed);
    EXPECT_EQ(entry->statusEntry.failureCode, to_underlying(Protocols::InteractionModel::Status::Busy));
    EXPECT_TRUE(entry->pendingRemoval);

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    ASSERT_FALSE(delegate.aclListSyncs.empty());
    for (const auto & written : delegate.aclListSyncs.back().second)
    {
        EXPECT_FALSE(written.subjects == std::vector<uint64_t>{ 0x1111 });
    }
    EXPECT_EQ(FindAcl(store, 123, 7), nullptr);
}

TEST(JointFabricDatastoreTest, RefreshReleasesGuardOnFetchFailure)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);

    delegate.fetchAclListResult = CHIP_IM_GLOBAL_STATUS(Timeout);
    static_cast<void>(store.RefreshNode(123));
    EXPECT_EQ(store.GetNodeInformationEntries()[0].commissioningStatusEntry.state, State::kPending);

    delegate.fetchAclListResult = CHIP_NO_ERROR;
    EXPECT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    EXPECT_EQ(store.GetNodeInformationEntries()[0].commissioningStatusEntry.state, State::kCommitted);
}

TEST(JointFabricDatastoreTest, RefreshReleasesGuardOnSyncStartFailure)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 7, Privilege::kView, AuthMode::kCase, { 0x1111 }, State::kPending);

    delegate.failStartWith[SyncKind::kAclList] = CHIP_ERROR_CONNECTION_ABORTED;
    static_cast<void>(store.RefreshNode(123));
    EXPECT_EQ(store.GetNodeInformationEntries()[0].commissioningStatusEntry.state, State::kPending);
    EXPECT_EQ(FindAcl(store, 123, 7)->statusEntry.state, State::kCommitFailed);

    EXPECT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    EXPECT_EQ(store.GetNodeInformationEntries()[0].commissioningStatusEntry.state, State::kCommitted);
}

TEST(JointFabricDatastoreTest, RefreshOfUnknownNodeReleasesGuard)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);

    EXPECT_NE(store.RefreshNode(999), CHIP_NO_ERROR);
    EXPECT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
}

// A failed binding write is recorded, and the refresh still writes the ACLs. The node stays Pending.
TEST(JointFabricDatastoreTest, BindingStageFailureStillRunsAclStage)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 7, Privilege::kView, AuthMode::kCase, { 0x1111 }, State::kPending);

    delegate.completeWith[SyncKind::kBindingList] = CHIP_IM_GLOBAL_STATUS(Timeout);
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    EXPECT_EQ(delegate.aclListSyncs.size(), 1u);
    EXPECT_EQ(FindAcl(store, 123, 7)->statusEntry.state, State::kCommitted);
    EXPECT_EQ(store.GetNodeInformationEntries()[0].commissioningStatusEntry.state, State::kPending);
    EXPECT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
}

// A binding whose earlier write failed is retried by RefreshNode and committed when the write succeeds.
TEST(JointFabricDatastoreTest, RefreshCommitsSuccessfullyRetriedBinding)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.TestAddEndpointEntry(1, 123, "ep"_span), CHIP_NO_ERROR);

    EndpointEntryType endpoint;
    endpoint.nodeID     = 123;
    endpoint.endpointID = 1;
    delegate.endpointsToFetch.push_back(endpoint);

    BindingEntryType bindingEntry;
    bindingEntry.nodeID     = 123;
    bindingEntry.endpointID = 1;
    bindingEntry.listID     = 9;
    bindingEntry.binding.group.SetValue(10);
    bindingEntry.statusEntry.state       = State::kCommitFailed;
    bindingEntry.statusEntry.failureCode = to_underlying(Protocols::InteractionModel::Status::Timeout);
    store.GetEndpointBindingList().push_back({ bindingEntry });
    delegate.bindingsToFetch.push_back(bindingEntry);

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    ASSERT_EQ(store.GetEndpointBindingList().size(), 1u);
    EXPECT_EQ(store.GetEndpointBindingList()[0].statusEntry.state, State::kCommitted);
    EXPECT_EQ(store.GetEndpointBindingList()[0].statusEntry.failureCode, 0u);
}

// TIMEOUT is recoverable: the binding survives the refresh and is retried.
TEST(JointFabricDatastoreTest, RefreshRetriesRecoverableCommitFailedBinding)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.TestAddEndpointEntry(1, 123, "ep"_span), CHIP_NO_ERROR);

    EndpointEntryType endpoint;
    endpoint.nodeID     = 123;
    endpoint.endpointID = 1;
    delegate.endpointsToFetch.push_back(endpoint);

    BindingEntryType bindingEntry;
    bindingEntry.nodeID     = 123;
    bindingEntry.endpointID = 1;
    bindingEntry.listID     = 9;
    bindingEntry.binding.group.SetValue(10);
    bindingEntry.statusEntry.state       = State::kCommitFailed;
    bindingEntry.statusEntry.failureCode = to_underlying(Protocols::InteractionModel::Status::Timeout);
    store.GetEndpointBindingList().push_back({ bindingEntry });

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    ASSERT_FALSE(delegate.bindingListSyncs.empty());
    ASSERT_EQ(delegate.bindingListSyncs.back().second.size(), 1u);
    EXPECT_EQ(delegate.bindingListSyncs.back().second[0].listID, 9u);
    ASSERT_EQ(store.GetEndpointBindingList().size(), 1u);
    EXPECT_EQ(store.GetEndpointBindingList()[0].statusEntry.state, State::kCommitted);
}

// ACL entries read from a node carry no listID, so the refresh matches them to datastore entries by
// value.
TEST(JointFabricDatastoreTest, RefreshMatchesFetchedAclsByValue)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 5, Privilege::kAdminister, AuthMode::kCase, { 0x1111 }, State::kCommitted);
    SeedAcl(store, 123, 6, Privilege::kView, AuthMode::kCase, { 0x2222 }, State::kCommitted);
    delegate.aclListToFetch = store.GetNodeACLList(); // node holds exactly what the datastore holds

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    ASSERT_EQ(store.GetNodeACLList().size(), 2u);
    EXPECT_NE(FindAcl(store, 123, 5), nullptr);
    EXPECT_NE(FindAcl(store, 123, 6), nullptr);
    EXPECT_EQ(delegate.aclListSyncs.back().second.size(), 2u);
}

// An entry on the node that the datastore does not hold is adopted as Committed, with a new listID.
TEST(JointFabricDatastoreTest, RefreshAdoptsOutOfBandAclWithFreshListId)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 5, Privilege::kAdminister, AuthMode::kCase, { 0x1111 }, State::kCommitted);
    delegate.aclListToFetch = store.GetNodeACLList();

    datastore::ACLEntryStruct extra;
    extra.ACLEntry.privilege = Privilege::kView;
    extra.ACLEntry.authMode  = AuthMode::kCase;
    extra.ACLEntry.subjects  = { 0x3333 };
    delegate.aclListToFetch.push_back(extra);

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    ASSERT_EQ(store.GetNodeACLList().size(), 2u);
    const auto & adopted = store.GetNodeACLList()[1];
    EXPECT_TRUE(adopted.ACLEntry.subjects == std::vector<uint64_t>{ 0x3333 });
    EXPECT_NE(adopted.listID, 5u);
    EXPECT_EQ(adopted.statusEntry.state, State::kCommitted);
    EXPECT_EQ(delegate.aclListSyncs.back().second.size(), 2u);
}

// The node still holds the value an entry had before its Pending update. It is not adopted, so the
// write replaces it.
TEST(JointFabricDatastoreTest, RefreshDoesNotReadoptSupersededValue)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 5, Privilege::kManage, AuthMode::kCase, { 0x1111 }, State::kCommitted);
    delegate.aclListToFetch = store.GetNodeACLList();

    auto & entry             = store.GetNodeACLList()[0];
    entry.supersededValue    = entry.ACLEntry;
    entry.ACLEntry.privilege = Privilege::kView;
    entry.statusEntry.state  = State::kPending;

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    for (const auto & held : store.GetNodeACLList())
    {
        EXPECT_NE(held.ACLEntry.privilege, Privilege::kManage);
    }
    ASSERT_EQ(delegate.aclListSyncs.back().second.size(), 1u);
    EXPECT_EQ(delegate.aclListSyncs.back().second[0].privilege, Privilege::kView);
    EXPECT_EQ(FindAcl(store, 123, 5)->statusEntry.state, State::kCommitted);
    EXPECT_FALSE(FindAcl(store, 123, 5)->supersededValue.has_value());
}

TEST(JointFabricDatastoreTest, RefreshTreatsNullTargetsAsEmptyWhenMatching)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 5, Privilege::kView, AuthMode::kCase, { 0x1111 }, State::kCommitted);
    delegate.aclListToFetch = store.GetNodeACLList(); // no targets: the mock reports them as null

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    EXPECT_EQ(store.GetNodeACLList().size(), 1u);
    EXPECT_EQ(delegate.aclListSyncs.back().second.size(), 1u);
}

// The node no longer holds an entry whose removal failed earlier: the removal has taken effect.
TEST(JointFabricDatastoreTest, RefreshErasesRemovalTheNodeAlreadyCompleted)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 5, Privilege::kManage, AuthMode::kCase, { 0x1111 }, State::kCommitted);

    delegate.completeWith[SyncKind::kAcl] = CHIP_IM_GLOBAL_STATUS(Timeout);
    ASSERT_EQ(store.RemoveACLFromNode(5, 123), CHIP_NO_ERROR);
    ASSERT_TRUE(FindAcl(store, 123, 5)->pendingRemoval);

    delegate.deferKind = SyncKind::kAclList;
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    EXPECT_EQ(FindAcl(store, 123, 5), nullptr);
    delegate.RunDeferred();
}

// The node still holds an entry whose removal failed earlier. It is matched, left out of the write,
// and erased once the write succeeds.
TEST(JointFabricDatastoreTest, RefreshKeepsRemovalTheNodeStillHolds)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 5, Privilege::kManage, AuthMode::kCase, { 0x1111 }, State::kCommitted);
    delegate.aclListToFetch = store.GetNodeACLList();

    delegate.completeWith[SyncKind::kAcl] = CHIP_IM_GLOBAL_STATUS(Timeout);
    ASSERT_EQ(store.RemoveACLFromNode(5, 123), CHIP_NO_ERROR);

    delegate.deferKind = SyncKind::kAclList;
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetNodeACLList().size(), 1u);
    ASSERT_NE(FindAcl(store, 123, 5), nullptr);
    EXPECT_TRUE(delegate.aclListSyncs.back().second.empty());

    delegate.RunDeferred();
    EXPECT_TRUE(store.GetNodeACLList().empty());
}

void AddEndpointOneToRefresh(JointFabricDatastore & store, TrackingDelegate & delegate)
{
    ASSERT_EQ(store.TestAddEndpointEntry(1, 123, "ep"_span), CHIP_NO_ERROR);
    EndpointEntryType endpoint;
    endpoint.nodeID     = 123;
    endpoint.endpointID = 1;
    delegate.endpointsToFetch.push_back(endpoint);
}

void AddGroupTen(JointFabricDatastore & store, std::optional<uint16_t> groupKeySetId)
{
    JointFabricCluster::Commands::AddGroup::DecodableType addGroup;
    addGroup.groupID      = 10;
    addGroup.friendlyName = "group-a"_span;
    if (groupKeySetId.has_value())
    {
        addGroup.groupKeySetID.SetNonNull(*groupKeySetId);
    }
    addGroup.groupPermission = Privilege::kView;
    ASSERT_EQ(store.AddGroup(addGroup), CHIP_NO_ERROR);
}

datastore::EndpointGroupIDEntryStruct MakeEndpointGroupEntry(State state, uint8_t failureCode = 0)
{
    datastore::EndpointGroupIDEntryStruct entry;
    entry.nodeID                  = 123;
    entry.endpointID              = 1;
    entry.groupID                 = 10;
    entry.statusEntry.state       = state;
    entry.statusEntry.failureCode = failureCode;
    return entry;
}

// A failed group removal is retried by RefreshNode as a removal of the real entry.
TEST(JointFabricDatastoreTest, RefreshRetriesFailedEndpointGroupRemovalAsRemoval)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddEndpointOneToRefresh(store, delegate);
    AddGroupTen(store, std::nullopt);
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);

    delegate.completeWith[SyncKind::kEndpointGroup] = CHIP_IM_GLOBAL_STATUS(Timeout);
    ASSERT_EQ(store.RemoveGroupIDFromEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetEndpointGroupIDList()[0].statusEntry.state, State::kCommitFailed);

    delegate.groupsToFetch[1] = { 10 };
    delegate.ResetCapturedSyncs();
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    ASSERT_TRUE(delegate.hasLastEndpointGroupSync);
    EXPECT_EQ(delegate.lastEndpointGroupSync.groupID, 10u);
    EXPECT_EQ(delegate.lastEndpointGroupSync.endpointID, 1u);
    EXPECT_EQ(delegate.lastEndpointGroupSync.statusEntry.state, State::kDeletePending);
    EXPECT_TRUE(store.GetEndpointGroupIDList().empty());
}

TEST(JointFabricDatastoreTest, RefreshRetriesRecoverableEndpointGroupAdd)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddEndpointOneToRefresh(store, delegate);
    store.GetEndpointGroupIDList().push_back(
        MakeEndpointGroupEntry(State::kCommitFailed, to_underlying(Protocols::InteractionModel::Status::Timeout)));

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    ASSERT_TRUE(delegate.hasLastEndpointGroupSync);
    EXPECT_EQ(delegate.lastEndpointGroupSync.groupID, 10u);
    EXPECT_EQ(delegate.lastEndpointGroupSync.statusEntry.state, State::kPending);
    ASSERT_EQ(store.GetEndpointGroupIDList().size(), 1u);
    EXPECT_EQ(store.GetEndpointGroupIDList()[0].statusEntry.state, State::kCommitted);
}

TEST(JointFabricDatastoreTest, RefreshDropsUnrecoverableEndpointGroupFailure)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddEndpointOneToRefresh(store, delegate);
    store.GetEndpointGroupIDList().push_back(
        MakeEndpointGroupEntry(State::kCommitFailed, to_underlying(Protocols::InteractionModel::Status::ConstraintError)));

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    EXPECT_FALSE(delegate.hasLastEndpointGroupSync);
    EXPECT_TRUE(store.GetEndpointGroupIDList().empty());
}

// A DeletePending group entry the node still holds is sent as a removal of that entry.
TEST(JointFabricDatastoreTest, RefreshRetriesEndpointGroupRemovalAsRemoval)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddEndpointOneToRefresh(store, delegate);
    store.GetEndpointGroupIDList().push_back(MakeEndpointGroupEntry(State::kDeletePending));
    delegate.groupsToFetch[1] = { 10 };

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    ASSERT_TRUE(delegate.hasLastEndpointGroupSync);
    EXPECT_EQ(delegate.lastEndpointGroupSync.groupID, 10u);
    EXPECT_EQ(delegate.lastEndpointGroupSync.statusEntry.state, State::kDeletePending);
    EXPECT_TRUE(store.GetEndpointGroupIDList().empty());
}

// A failed key set removal is retried by RefreshNode as a removal of the real entry.
TEST(JointFabricDatastoreTest, RefreshRetriesFailedKeySetRemovalAsRemoval)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddEndpointOneToRefresh(store, delegate);
    AddGroupTen(store, 55);
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetNodeKeySetList().size(), 1u);

    delegate.completeWith[SyncKind::kNodeKeySet] = CHIP_IM_GLOBAL_STATUS(Timeout);
    ASSERT_EQ(store.RemoveGroupIDFromEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetNodeKeySetList().size(), 1u);
    ASSERT_EQ(store.GetNodeKeySetList()[0].statusEntry.state, State::kCommitFailed);

    delegate.fetchedGroupKeySetIDs = { 55 };
    delegate.ResetCapturedSyncs();
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    ASSERT_TRUE(delegate.hasLastNodeKeySetSync);
    EXPECT_EQ(delegate.lastNodeKeySetSync.groupKeySetID, 55u);
    EXPECT_EQ(delegate.lastNodeKeySetSync.statusEntry.state, State::kDeletePending);
    EXPECT_TRUE(store.GetNodeKeySetList().empty());
}

TEST(JointFabricDatastoreTest, RefreshRetriesKeySetRemovalAsRemoval)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);

    NodeKeySetEntryType keySetEntry;
    keySetEntry.nodeID            = 123;
    keySetEntry.groupKeySetID     = 55;
    keySetEntry.statusEntry.state = State::kDeletePending;
    store.GetNodeKeySetList().push_back({ keySetEntry });
    delegate.fetchedGroupKeySetIDs = { 55 };

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    ASSERT_TRUE(delegate.hasLastNodeKeySetSync);
    EXPECT_EQ(delegate.lastNodeKeySetSync.nodeID, 123u);
    EXPECT_EQ(delegate.lastNodeKeySetSync.groupKeySetID, 55u);
    EXPECT_EQ(delegate.lastNodeKeySetSync.statusEntry.state, State::kDeletePending);
    EXPECT_TRUE(store.GetNodeKeySetList().empty());
}

// Changing a group's key set removes the old key set from the group's nodes.
TEST(JointFabricDatastoreTest, UpdateGroupKeySetChangeSendsRemovalOfOldKeySet)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    AddGroupTen(store, 5);
    store.GetEndpointGroupIDList().push_back(MakeEndpointGroupEntry(State::kCommitted));
    ASSERT_EQ(store.TestAddNodeKeySetEntry(10, 5, 123), CHIP_NO_ERROR);

    JointFabricCluster::Commands::UpdateGroup::DecodableType updateGroup;
    updateGroup.groupID = 10;
    updateGroup.groupKeySetID.SetNonNull(static_cast<uint16_t>(6));
    ASSERT_EQ(store.UpdateGroup(updateGroup), CHIP_NO_ERROR);

    ASSERT_TRUE(delegate.hasLastNodeKeySetSync);
    EXPECT_EQ(delegate.lastNodeKeySetSync.nodeID, 123u);
    EXPECT_EQ(delegate.lastNodeKeySetSync.groupKeySetID, 5u);
    EXPECT_EQ(delegate.lastNodeKeySetSync.statusEntry.state, State::kDeletePending);
    ASSERT_EQ(store.GetNodeKeySetList().size(), 1u);
    EXPECT_EQ(store.GetNodeKeySetList()[0].groupKeySetID, 6u);
}

// An unrecoverable CommitFailure is dropped, as the specification requires. If the entry was being
// removed and the node still holds it, a later refresh removes it instead of adopting it.
TEST(JointFabricDatastoreTest, UnrecoverableAclRemovalFailureIsRemovedAtNextRefresh)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 7, Privilege::kManage, AuthMode::kCase, { 0x1111 }, State::kCommitted);
    delegate.aclListToFetch = store.GetNodeACLList(); // the node keeps the entry throughout

    delegate.completeWith[SyncKind::kAcl] = CHIP_IM_GLOBAL_STATUS(ConstraintError);
    ASSERT_EQ(store.RemoveACLFromNode(7, 123), CHIP_NO_ERROR);

    // First refresh: triage drops the entry, as specified.
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    EXPECT_EQ(FindAcl(store, 123, 7), nullptr);

    // Second refresh: the node still reports the value, so it is removed.
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    for (const auto & written : delegate.aclListSyncs.back().second)
    {
        EXPECT_FALSE(written.subjects == std::vector<uint64_t>{ 0x1111 });
    }
    for (const auto & entry : store.GetNodeACLList())
    {
        EXPECT_FALSE(entry.ACLEntry.subjects == std::vector<uint64_t>{ 0x1111 });
    }
}

// Once the node no longer holds a tombstoned value, the tombstone is dropped: the value is adopted if
// it is later added to the node outside the datastore.
TEST(JointFabricDatastoreTest, UnrecoverableAclRemovalTombstoneDroppedWhenNodeIsClean)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 7, Privilege::kManage, AuthMode::kCase, { 0x1111 }, State::kCommitted);
    const auto heldAcls     = store.GetNodeACLList();
    delegate.aclListToFetch = heldAcls;

    delegate.completeWith[SyncKind::kAcl] = CHIP_IM_GLOBAL_STATUS(ConstraintError);
    ASSERT_EQ(store.RemoveACLFromNode(7, 123), CHIP_NO_ERROR);
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    delegate.aclListToFetch.clear();
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    delegate.aclListToFetch = heldAcls;
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetNodeACLList().size(), 1u);
    EXPECT_TRUE(store.GetNodeACLList()[0].ACLEntry.subjects == std::vector<uint64_t>{ 0x1111 });
    EXPECT_EQ(store.GetNodeACLList()[0].statusEntry.state, State::kCommitted);
}

TEST(JointFabricDatastoreTest, UnrecoverableBindingRemovalFailureIsReaddedAsDeletePending)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddEndpointOneToRefresh(store, delegate);

    JointFabricCluster::Structs::DatastoreBindingTargetStruct::Type binding;
    binding.group.SetValue(10);
    ASSERT_EQ(store.AddBindingToEndpointForNode(123, 1, binding), CHIP_NO_ERROR);
    const auto heldBinding = store.GetEndpointBindingList()[0];
    delegate.bindingsToFetch.push_back(heldBinding);

    delegate.completeWith[SyncKind::kBinding] = CHIP_IM_GLOBAL_STATUS(ConstraintError);
    ASSERT_EQ(store.RemoveBindingFromEndpointForNode(heldBinding.listID, 123, 1), CHIP_NO_ERROR);

    // First refresh: triage drops the entry, as specified.
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    EXPECT_TRUE(store.GetEndpointBindingList().empty());

    // Second refresh: the node still holds the binding, so it is added back as DeletePending.
    delegate.deferKind = SyncKind::kBindingList;
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetEndpointBindingList().size(), 1u);
    EXPECT_EQ(store.GetEndpointBindingList()[0].statusEntry.state, State::kDeletePending);
    for (const auto & written : delegate.bindingListSyncs.back().second)
    {
        EXPECT_FALSE(chip::app::detail::BindingEntryValueEquals(written, heldBinding));
    }

    delegate.RunDeferred();
    EXPECT_TRUE(store.GetEndpointBindingList().empty());
}

TEST(JointFabricDatastoreTest, UnrecoverableEndpointGroupRemovalFailureIsReaddedAsDeletePending)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddEndpointOneToRefresh(store, delegate);
    AddGroupTen(store, std::nullopt);
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    delegate.groupsToFetch[1] = { 10 };

    delegate.completeWith[SyncKind::kEndpointGroup] = CHIP_IM_GLOBAL_STATUS(ConstraintError);
    ASSERT_EQ(store.RemoveGroupIDFromEndpointForNode(123, 1, 10), CHIP_NO_ERROR);

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    EXPECT_TRUE(store.GetEndpointGroupIDList().empty());

    delegate.ResetCapturedSyncs();
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    ASSERT_TRUE(delegate.hasLastEndpointGroupSync);
    EXPECT_EQ(delegate.lastEndpointGroupSync.groupID, 10u);
    EXPECT_EQ(delegate.lastEndpointGroupSync.statusEntry.state, State::kDeletePending);
    EXPECT_TRUE(store.GetEndpointGroupIDList().empty());
}

TEST(JointFabricDatastoreTest, UnrecoverableKeySetRemovalFailureIsRemovedAtNextRefresh)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddEndpointOneToRefresh(store, delegate);
    AddGroupTen(store, 55);
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    delegate.fetchedGroupKeySetIDs = { 55 };

    delegate.completeWith[SyncKind::kNodeKeySet] = CHIP_IM_GLOBAL_STATUS(ConstraintError);
    ASSERT_EQ(store.RemoveGroupIDFromEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetNodeKeySetList().size(), 1u);

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    EXPECT_TRUE(store.GetNodeKeySetList().empty());

    delegate.ResetCapturedSyncs();
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    ASSERT_TRUE(delegate.hasLastNodeKeySetSync);
    EXPECT_EQ(delegate.lastNodeKeySetSync.groupKeySetID, 55u);
    EXPECT_EQ(delegate.lastNodeKeySetSync.statusEntry.state, State::kDeletePending);
    EXPECT_TRUE(store.GetNodeKeySetList().empty());
}

// Adding an entry whose removal failed cancels the removal and syncs the entry as an add.
TEST(JointFabricDatastoreTest, AddCancelsPendingAclRemoval)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 7, Privilege::kView, AuthMode::kCase, {}, State::kCommitted);

    delegate.completeWith[SyncKind::kAcl] = CHIP_IM_GLOBAL_STATUS(Timeout);
    ASSERT_EQ(store.RemoveACLFromNode(7, 123), CHIP_NO_ERROR);
    ASSERT_TRUE(FindAcl(store, 123, 7)->pendingRemoval);

    JointFabricCluster::Structs::DatastoreAccessControlEntryStruct::DecodableType aclEntry;
    aclEntry.privilege = Privilege::kView;
    aclEntry.authMode  = AuthMode::kCase;
    delegate.ResetCapturedSyncs();
    ASSERT_EQ(store.AddACLToNode(123, aclEntry), CHIP_NO_ERROR);

    ASSERT_TRUE(delegate.hasLastAclSync);
    EXPECT_EQ(delegate.lastAclSync.listID, 7u);
    EXPECT_EQ(delegate.lastAclSync.statusEntry.state, State::kPending);
    ASSERT_EQ(store.GetNodeACLList().size(), 1u);
    EXPECT_FALSE(FindAcl(store, 123, 7)->pendingRemoval);
    EXPECT_EQ(FindAcl(store, 123, 7)->statusEntry.state, State::kCommitted);
}

TEST(JointFabricDatastoreTest, AddOfTombstonedAclValueDropsTombstone)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 7, Privilege::kView, AuthMode::kCase, {}, State::kCommitted);
    delegate.aclListToFetch = store.GetNodeACLList();

    delegate.completeWith[SyncKind::kAcl] = CHIP_IM_GLOBAL_STATUS(ConstraintError);
    ASSERT_EQ(store.RemoveACLFromNode(7, 123), CHIP_NO_ERROR);
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    ASSERT_TRUE(store.GetNodeACLList().empty());

    JointFabricCluster::Structs::DatastoreAccessControlEntryStruct::DecodableType aclEntry;
    aclEntry.privilege = Privilege::kView;
    aclEntry.authMode  = AuthMode::kCase;
    ASSERT_EQ(store.AddACLToNode(123, aclEntry), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetNodeACLList().size(), 1u);

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetNodeACLList().size(), 1u);
    EXPECT_EQ(store.GetNodeACLList()[0].statusEntry.state, State::kCommitted);
    EXPECT_FALSE(store.GetNodeACLList()[0].pendingRemoval);
    EXPECT_EQ(delegate.aclListSyncs.back().second.size(), 1u);
}

TEST(JointFabricDatastoreTest, AddCancelsPendingBindingRemoval)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddEndpointOneToRefresh(store, delegate);

    JointFabricCluster::Structs::DatastoreBindingTargetStruct::Type binding;
    binding.group.SetValue(10);
    ASSERT_EQ(store.AddBindingToEndpointForNode(123, 1, binding), CHIP_NO_ERROR);
    const auto heldBinding = store.GetEndpointBindingList()[0];
    delegate.bindingsToFetch.push_back(heldBinding);

    delegate.completeWith[SyncKind::kBinding] = CHIP_IM_GLOBAL_STATUS(Timeout);
    ASSERT_EQ(store.RemoveBindingFromEndpointForNode(heldBinding.listID, 123, 1), CHIP_NO_ERROR);

    delegate.ResetCapturedSyncs();
    ASSERT_EQ(store.AddBindingToEndpointForNode(123, 1, binding), CHIP_NO_ERROR);
    ASSERT_TRUE(delegate.hasLastBindingSync);
    EXPECT_EQ(delegate.lastBindingSync.statusEntry.state, State::kPending);
    ASSERT_EQ(store.GetEndpointBindingList().size(), 1u);
    EXPECT_EQ(store.GetEndpointBindingList()[0].statusEntry.state, State::kCommitted);

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    ASSERT_EQ(delegate.bindingListSyncs.back().second.size(), 1u);
    EXPECT_EQ(store.GetEndpointBindingList().size(), 1u);
}

TEST(JointFabricDatastoreTest, AddCancelsPendingEndpointGroupRemoval)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddEndpointOneToRefresh(store, delegate);
    AddGroupTen(store, std::nullopt);
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    delegate.groupsToFetch[1] = { 10 };

    delegate.completeWith[SyncKind::kEndpointGroup] = CHIP_IM_GLOBAL_STATUS(Timeout);
    ASSERT_EQ(store.RemoveGroupIDFromEndpointForNode(123, 1, 10), CHIP_NO_ERROR);

    delegate.ResetCapturedSyncs();
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    ASSERT_TRUE(delegate.hasLastEndpointGroupSync);
    EXPECT_EQ(delegate.lastEndpointGroupSync.statusEntry.state, State::kPending);
    ASSERT_EQ(store.GetEndpointGroupIDList().size(), 1u);
    EXPECT_EQ(store.GetEndpointGroupIDList()[0].statusEntry.state, State::kCommitted);

    delegate.ResetCapturedSyncs();
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.hasLastEndpointGroupSync);
    EXPECT_EQ(store.GetEndpointGroupIDList().size(), 1u);
}

// An add that cancels an in-flight removal waits for it, and the removal's result no longer applies to the entry.
TEST(JointFabricDatastoreTest, ReAddDuringEndpointGroupRemovalRunsAfterIt)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddEndpointOneToRefresh(store, delegate);
    AddGroupTen(store, std::nullopt);
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);

    delegate.deferKind = SyncKind::kEndpointGroup;
    ASSERT_EQ(store.RemoveGroupIDFromEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    ASSERT_EQ(delegate.deferred.size(), 1u);

    delegate.RunDeferred(); // the removal
    ASSERT_EQ(store.GetEndpointGroupIDList().size(), 1u);
    EXPECT_EQ(store.GetEndpointGroupIDList()[0].statusEntry.state, State::kPending);
    ASSERT_EQ(delegate.deferred.size(), 1u);
    EXPECT_EQ(delegate.lastEndpointGroupSync.statusEntry.state, State::kPending);

    delegate.RunDeferred(); // the add
    ASSERT_EQ(store.GetEndpointGroupIDList().size(), 1u);
    EXPECT_EQ(store.GetEndpointGroupIDList()[0].statusEntry.state, State::kCommitted);
}

TEST(JointFabricDatastoreTest, FailedRemovalCancelledByReAddDoesNotMarkEndpointGroup)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddEndpointOneToRefresh(store, delegate);
    AddGroupTen(store, std::nullopt);
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);

    delegate.deferKind = SyncKind::kEndpointGroup;
    ASSERT_EQ(store.RemoveGroupIDFromEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);

    delegate.RunDeferred(0, CHIP_IM_GLOBAL_STATUS(Timeout)); // the removal
    ASSERT_EQ(store.GetEndpointGroupIDList().size(), 1u);
    EXPECT_EQ(store.GetEndpointGroupIDList()[0].statusEntry.state, State::kPending);

    delegate.RunDeferred(); // the add
    EXPECT_EQ(store.GetEndpointGroupIDList()[0].statusEntry.state, State::kCommitted);
}

TEST(JointFabricDatastoreTest, ReAddDuringNodeKeySetRemovalRunsAfterIt)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddEndpointOneToRefresh(store, delegate);
    AddGroupTen(store, 55);
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetNodeKeySetList().size(), 1u);

    delegate.deferKind = SyncKind::kNodeKeySet;
    ASSERT_EQ(store.RemoveGroupIDFromEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    ASSERT_EQ(delegate.deferred.size(), 1u);

    delegate.RunDeferred(); // the key set removal
    ASSERT_EQ(store.GetNodeKeySetList().size(), 1u);
    EXPECT_EQ(store.GetNodeKeySetList()[0].statusEntry.state, State::kPending);
    ASSERT_EQ(delegate.deferred.size(), 1u);

    delegate.RunDeferred(); // the key set add, after which the group is added
    ASSERT_EQ(store.GetNodeKeySetList().size(), 1u);
    EXPECT_EQ(store.GetNodeKeySetList()[0].statusEntry.state, State::kCommitted);
    ASSERT_EQ(store.GetEndpointGroupIDList().size(), 1u);
    EXPECT_EQ(store.GetEndpointGroupIDList()[0].statusEntry.state, State::kCommitted);
}

// Group and key set syncs share the node's queue with ACL and binding syncs.
TEST(JointFabricDatastoreTest, GroupSyncWaitsForAclSyncOnSameNode)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddEndpointOneToRefresh(store, delegate);
    AddGroupTen(store, std::nullopt);
    SeedAcl(store, 123, 5, Privilege::kView, AuthMode::kCase, { 0x1111 }, State::kCommitted);

    delegate.deferKind = SyncKind::kAcl;
    ASSERT_EQ(store.RemoveACLFromNode(5, 123), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);
    EXPECT_FALSE(delegate.hasLastEndpointGroupSync);
    EXPECT_EQ(store.GetEndpointGroupIDList()[0].statusEntry.state, State::kPending);

    delegate.RunDeferred();
    EXPECT_TRUE(delegate.hasLastEndpointGroupSync);
    EXPECT_EQ(store.GetEndpointGroupIDList()[0].statusEntry.state, State::kCommitted);
}

// A group whose key set failed to reach the node is not added: the node would reject the group's key map.
TEST(JointFabricDatastoreTest, GroupAddWaitsForFailedKeySet)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddEndpointOneToRefresh(store, delegate);
    AddGroupTen(store, 55);

    delegate.completeWith[SyncKind::kNodeKeySet] = CHIP_IM_GLOBAL_STATUS(Timeout);
    ASSERT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_NO_ERROR);

    EXPECT_EQ(store.GetNodeKeySetList()[0].statusEntry.state, State::kCommitFailed);
    EXPECT_FALSE(delegate.hasLastEndpointGroupSync);
    EXPECT_EQ(store.GetEndpointGroupIDList()[0].statusEntry.state, State::kPending);
}

// Seeds `count` Committed ACL entries on node 123, with list IDs from 1.
void SeedAcls(JointFabricDatastore & store, uint16_t count)
{
    for (uint16_t listId = 1; listId <= count; ++listId)
    {
        SeedAcl(store, 123, listId, Privilege::kView, AuthMode::kCase, { listId }, State::kCommitted);
    }
}

// Fills node 123's sync queue: one removal in flight and the rest waiting.
void FillSyncQueue(JointFabricDatastore & store, TrackingDelegate & delegate)
{
    delegate.deferKind = SyncKind::kAcl;
    for (uint16_t listId = 1; listId <= JointFabricDatastore::kMaxQueuedNodeSyncs + 1; ++listId)
    {
        ASSERT_EQ(store.RemoveACLFromNode(listId, 123), CHIP_NO_ERROR);
    }
    ASSERT_EQ(delegate.deferred.size(), 1u);
}

// BUSY is a temporary condition that commissioners retry. The rejected command changes nothing, so the retry
// repeats it in full.
TEST(JointFabricDatastoreTest, RemovalWhenSyncQueueFullIsBusyAndChangesNothing)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    const uint16_t lastListId = JointFabricDatastore::kMaxQueuedNodeSyncs + 2;
    SeedAcls(store, lastListId);
    FillSyncQueue(store, delegate);

    EXPECT_EQ(store.RemoveACLFromNode(lastListId, 123), CHIP_IM_GLOBAL_STATUS(Busy));
    EXPECT_EQ(FindAcl(store, 123, lastListId)->statusEntry.state, State::kCommitted);
    EXPECT_FALSE(FindAcl(store, 123, lastListId)->pendingRemoval);

    delegate.RunDeferred();
    EXPECT_EQ(store.RemoveACLFromNode(lastListId, 123), CHIP_NO_ERROR);
    EXPECT_EQ(FindAcl(store, 123, lastListId)->statusEntry.state, State::kDeletePending);
}

TEST(JointFabricDatastoreTest, AddWhenSyncQueueFullIsBusyAndChangesNothing)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddEndpointOneToRefresh(store, delegate);
    AddGroupTen(store, std::nullopt);
    SeedAcls(store, JointFabricDatastore::kMaxQueuedNodeSyncs + 1);
    FillSyncQueue(store, delegate);
    const size_t aclCount = store.GetNodeACLList().size();

    JointFabricCluster::Structs::DatastoreAccessControlEntryStruct::DecodableType aclEntry;
    aclEntry.privilege = Privilege::kManage;
    aclEntry.authMode  = AuthMode::kCase;
    EXPECT_EQ(store.AddACLToNode(123, aclEntry), CHIP_IM_GLOBAL_STATUS(Busy));
    EXPECT_EQ(store.GetNodeACLList().size(), aclCount);

    EXPECT_EQ(store.AddGroupIDToEndpointForNode(123, 1, 10), CHIP_IM_GLOBAL_STATUS(Busy));
    EXPECT_TRUE(store.GetEndpointGroupIDList().empty());

    JointFabricCluster::Structs::DatastoreBindingTargetStruct::Type binding;
    binding.group.SetValue(10);
    EXPECT_EQ(store.AddBindingToEndpointForNode(123, 1, binding), CHIP_IM_GLOBAL_STATUS(Busy));
    EXPECT_TRUE(store.GetEndpointBindingList().empty());

    delegate.RunDeferred();
    EXPECT_EQ(store.AddBindingToEndpointForNode(123, 1, binding), CHIP_NO_ERROR);
    ASSERT_EQ(store.GetEndpointBindingList().size(), 1u);
    EXPECT_EQ(store.GetEndpointBindingList()[0].statusEntry.state, State::kPending);
}

// Each endpoint's fetched binding list only replaces that endpoint's bindings.
TEST(JointFabricDatastoreTest, RefreshKeepsBindingsOnOtherEndpoints)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    for (EndpointId endpointId : { EndpointId(1), EndpointId(2) })
    {
        ASSERT_EQ(store.TestAddEndpointEntry(endpointId, 123, "ep"_span), CHIP_NO_ERROR);
        EndpointEntryType endpoint;
        endpoint.nodeID     = 123;
        endpoint.endpointID = endpointId;
        delegate.endpointsToFetch.push_back(endpoint);

        JointFabricCluster::Structs::DatastoreBindingTargetStruct::Type binding;
        binding.group.SetValue(static_cast<GroupId>(10 + endpointId));
        ASSERT_EQ(store.AddBindingToEndpointForNode(123, endpointId, binding), CHIP_NO_ERROR);
    }
    ASSERT_EQ(store.GetEndpointBindingList().size(), 2u);
    const uint16_t listId1 = store.GetEndpointBindingList()[0].listID;
    const uint16_t listId2 = store.GetEndpointBindingList()[1].listID;
    delegate.bindingsToFetch.assign(store.GetEndpointBindingList().begin(), store.GetEndpointBindingList().end());

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    ASSERT_EQ(store.GetEndpointBindingList().size(), 2u);
    EXPECT_EQ(store.GetEndpointBindingList()[0].listID, listId1);
    EXPECT_EQ(store.GetEndpointBindingList()[1].listID, listId2);
    EXPECT_EQ(delegate.bindingListSyncs.back().second.size(), 2u);
}

// Single-entry syncs rewrite the whole attribute on the node, so the datastore issues them one at a
// time per node.
TEST(JointFabricDatastoreTest, SecondAclSyncOnSameNodeWaitsForFirst)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 5, Privilege::kView, AuthMode::kCase, { 0x1111 }, State::kCommitted);
    SeedAcl(store, 123, 6, Privilege::kView, AuthMode::kCase, { 0x2222 }, State::kCommitted);

    delegate.deferKind = SyncKind::kAcl;
    ASSERT_EQ(store.RemoveACLFromNode(5, 123), CHIP_NO_ERROR);
    ASSERT_EQ(store.RemoveACLFromNode(6, 123), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.deferred.size(), 1u);

    delegate.RunDeferred();
    EXPECT_EQ(delegate.deferred.size(), 1u); // the second one has now started

    delegate.RunDeferred();
    EXPECT_TRUE(store.GetNodeACLList().empty());
}

TEST(JointFabricDatastoreTest, AclSyncsOnDifferentNodesRunConcurrently)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(0xA, "node-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(0xB, "node-b"_span), CHIP_NO_ERROR);
    SeedAcl(store, 0xA, 5, Privilege::kView, AuthMode::kCase, { 0x1111 }, State::kCommitted);
    SeedAcl(store, 0xB, 6, Privilege::kView, AuthMode::kCase, { 0x2222 }, State::kCommitted);

    delegate.deferKind = SyncKind::kAcl;
    ASSERT_EQ(store.RemoveACLFromNode(5, 0xA), CHIP_NO_ERROR);
    ASSERT_EQ(store.RemoveACLFromNode(6, 0xB), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.deferred.size(), 2u);

    delegate.RunDeferred();
    delegate.RunDeferred();
    EXPECT_TRUE(store.GetNodeACLList().empty());
}

TEST(JointFabricDatastoreTest, RefreshNodeIsBusyWhileSyncQueued)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 5, Privilege::kView, AuthMode::kCase, { 0x1111 }, State::kCommitted);

    delegate.deferKind = SyncKind::kAcl;
    ASSERT_EQ(store.RemoveACLFromNode(5, 123), CHIP_NO_ERROR);
    EXPECT_EQ(store.RefreshNode(123), CHIP_IM_GLOBAL_STATUS(Busy));

    delegate.RunDeferred();
    EXPECT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, SyncQueuedDuringRefreshRunsAfterIt)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 5, Privilege::kView, AuthMode::kCase, { 0x1111 }, State::kCommitted);
    delegate.aclListToFetch = store.GetNodeACLList();

    delegate.deferKind = SyncKind::kAclList;
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    ASSERT_EQ(store.RemoveACLFromNode(5, 123), CHIP_NO_ERROR);

    auto aclSyncCount = [&delegate]() {
        return std::count_if(delegate.syncCalls.begin(), delegate.syncCalls.end(),
                             [](const auto & call) { return call.second == SyncKind::kAcl; });
    };
    EXPECT_EQ(aclSyncCount(), 0);

    delegate.RunDeferred();
    EXPECT_EQ(aclSyncCount(), 1);
    EXPECT_TRUE(store.GetNodeACLList().empty());
}

TEST(JointFabricDatastoreTest, QueuedSyncForErasedEntryFinishesQuietly)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 5, Privilege::kView, AuthMode::kCase, { 0x1111 }, State::kCommitted);
    SeedAcl(store, 123, 6, Privilege::kView, AuthMode::kCase, { 0x2222 }, State::kCommitted);

    delegate.deferKind = SyncKind::kAcl;
    ASSERT_EQ(store.RemoveACLFromNode(5, 123), CHIP_NO_ERROR);
    ASSERT_EQ(store.RemoveACLFromNode(6, 123), CHIP_NO_ERROR);
    auto & acls = store.GetNodeACLList();
    acls.erase(std::remove_if(acls.begin(), acls.end(), [](const auto & entry) { return entry.listID == 6; }), acls.end());

    delegate.RunDeferred();
    EXPECT_TRUE(delegate.deferred.empty());
    EXPECT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, BindingSyncsSerializedPerNode)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.TestAddEndpointEntry(1, 123, "ep"_span), CHIP_NO_ERROR);

    JointFabricCluster::Structs::DatastoreBindingTargetStruct::Type first;
    first.group.SetValue(10);
    JointFabricCluster::Structs::DatastoreBindingTargetStruct::Type second;
    second.group.SetValue(11);

    delegate.deferKind = SyncKind::kBinding;
    ASSERT_EQ(store.AddBindingToEndpointForNode(123, 1, first), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddBindingToEndpointForNode(123, 1, second), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.deferred.size(), 1u);

    delegate.RunDeferred();
    EXPECT_EQ(delegate.deferred.size(), 1u);

    delegate.RunDeferred();
    ASSERT_EQ(store.GetEndpointBindingList().size(), 2u);
    EXPECT_EQ(store.GetEndpointBindingList()[0].statusEntry.state, State::kCommitted);
    EXPECT_EQ(store.GetEndpointBindingList()[1].statusEntry.state, State::kCommitted);
}

// An add of an entry whose removal is in flight cancels the removal: the removal's completion does not
// erase it, and the add runs next.
TEST(JointFabricDatastoreTest, AddDuringInFlightAclRemovalKeepsEntry)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 5, Privilege::kView, AuthMode::kCase, {}, State::kCommitted);

    delegate.deferKind = SyncKind::kAcl;
    ASSERT_EQ(store.RemoveACLFromNode(5, 123), CHIP_NO_ERROR);

    JointFabricCluster::Structs::DatastoreAccessControlEntryStruct::DecodableType aclEntry;
    aclEntry.privilege = Privilege::kView;
    aclEntry.authMode  = AuthMode::kCase;
    ASSERT_EQ(store.AddACLToNode(123, aclEntry), CHIP_NO_ERROR);

    delegate.RunDeferred(); // the removal
    ASSERT_NE(FindAcl(store, 123, 5), nullptr);
    ASSERT_EQ(delegate.deferred.size(), 1u);
    delegate.RunDeferred(); // the add
    EXPECT_EQ(FindAcl(store, 123, 5)->statusEntry.state, State::kCommitted);
}

// A removal sends the full stored value: entries read from the node carry no nodeID or listID to
// match on.
TEST(JointFabricDatastoreTest, AclRemovalSendsFullEntryValue)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 7, Privilege::kOperate, AuthMode::kCase, { 0x1111 }, State::kCommitted);

    ASSERT_EQ(store.RemoveACLFromNode(7, 123), CHIP_NO_ERROR);

    ASSERT_TRUE(delegate.lastAclSyncOwned.has_value());
    EXPECT_EQ(delegate.lastAclSyncOwned->privilege, Privilege::kOperate);
    EXPECT_TRUE(delegate.lastAclSyncOwned->subjects == std::vector<uint64_t>{ 0x1111 });
    EXPECT_EQ(delegate.lastAclSyncState, State::kDeletePending);
    EXPECT_FALSE(delegate.lastAclSuperseded.has_value());
}

TEST(JointFabricDatastoreTest, AclAddSendsNoSupersededValue)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);

    JointFabricCluster::Structs::DatastoreAccessControlEntryStruct::DecodableType aclEntry;
    aclEntry.privilege = Privilege::kView;
    aclEntry.authMode  = AuthMode::kCase;
    ASSERT_EQ(store.AddACLToNode(123, aclEntry), CHIP_NO_ERROR);

    ASSERT_TRUE(delegate.lastAclSyncOwned.has_value());
    EXPECT_EQ(delegate.lastAclSyncOwned->privilege, Privilege::kView);
    EXPECT_EQ(delegate.lastAclSyncState, State::kPending);
    EXPECT_FALSE(delegate.lastAclSuperseded.has_value());
}

// Group 10 with View permission, and a Group-auth ACL entry on node 123 for it.
void SetUpGroupAcl(JointFabricDatastore & store, TrackingDelegate & delegate)
{
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddGroupTen(store, std::nullopt);
    SeedAcl(store, 123, 7, Privilege::kView, AuthMode::kGroup, { 10 }, State::kCommitted);
}

CHIP_ERROR SetGroupTenPermission(JointFabricDatastore & store, Privilege privilege)
{
    JointFabricCluster::Commands::UpdateGroup::DecodableType updateGroup;
    updateGroup.groupID = 10;
    updateGroup.groupPermission.SetNonNull(privilege);
    return store.UpdateGroup(updateGroup);
}

// A permission change replaces the value the node holds: the sync carries the old value as superseded.
TEST(JointFabricDatastoreTest, AclUpdateSendsSupersededValue)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    SetUpGroupAcl(store, delegate);

    ASSERT_EQ(SetGroupTenPermission(store, Privilege::kManage), CHIP_NO_ERROR);

    ASSERT_TRUE(delegate.lastAclSyncOwned.has_value());
    EXPECT_EQ(delegate.lastAclSyncOwned->privilege, Privilege::kManage);
    ASSERT_TRUE(delegate.lastAclSuperseded.has_value());
    EXPECT_EQ(delegate.lastAclSuperseded->privilege, Privilege::kView);
    EXPECT_TRUE(delegate.lastAclSuperseded->subjects == std::vector<uint64_t>{ 10 });
}

// The first of two permission changes fails: the node still holds the original value, so the second
// change replaces that one.
TEST(JointFabricDatastoreTest, BackToBackUpdatesKeepOldestSupersededValue)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    SetUpGroupAcl(store, delegate);

    delegate.deferKind = SyncKind::kAcl;
    ASSERT_EQ(SetGroupTenPermission(store, Privilege::kManage), CHIP_NO_ERROR);
    ASSERT_EQ(SetGroupTenPermission(store, Privilege::kAdminister), CHIP_NO_ERROR);
    ASSERT_EQ(delegate.deferred.size(), 1u);

    delegate.RunDeferred(0, CHIP_IM_GLOBAL_STATUS(Timeout));

    ASSERT_EQ(delegate.deferred.size(), 1u);
    EXPECT_EQ(delegate.lastAclSyncOwned->privilege, Privilege::kAdminister);
    ASSERT_TRUE(delegate.lastAclSuperseded.has_value());
    EXPECT_EQ(delegate.lastAclSuperseded->privilege, Privilege::kView);
}

// The first of two permission changes succeeds: the node now holds the intermediate value, so the second
// change replaces that one.
TEST(JointFabricDatastoreTest, BackToBackUpdatesReplaceTheValueTheNodeHolds)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    SetUpGroupAcl(store, delegate);

    delegate.deferKind = SyncKind::kAcl;
    ASSERT_EQ(SetGroupTenPermission(store, Privilege::kManage), CHIP_NO_ERROR);
    ASSERT_EQ(SetGroupTenPermission(store, Privilege::kAdminister), CHIP_NO_ERROR);

    delegate.RunDeferred();

    ASSERT_EQ(delegate.deferred.size(), 1u);
    EXPECT_EQ(delegate.lastAclSyncOwned->privilege, Privilege::kAdminister);
    ASSERT_TRUE(delegate.lastAclSuperseded.has_value());
    EXPECT_EQ(delegate.lastAclSuperseded->privilege, Privilege::kManage);

    delegate.RunDeferred();
    EXPECT_EQ(FindAcl(store, 123, 7)->statusEntry.state, State::kCommitted);
    EXPECT_EQ(FindAcl(store, 123, 7)->ACLEntry.privilege, Privilege::kAdminister);
}

TEST(JointFabricDatastoreTest, CommitClearsSupersededValue)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    SetUpGroupAcl(store, delegate);

    ASSERT_EQ(SetGroupTenPermission(store, Privilege::kManage), CHIP_NO_ERROR);

    EXPECT_EQ(FindAcl(store, 123, 7)->statusEntry.state, State::kCommitted);
    EXPECT_FALSE(FindAcl(store, 123, 7)->supersededValue.has_value());
}

// A permission change made while RefreshNode's ACL write is in flight: the write carried the old value,
// so the entry is not Committed by it, and the queued sync replaces the value the write left on the node.
TEST(JointFabricDatastoreTest, RefreshAclWriteDoesNotCommitUpdateMadeDuringIt)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    SetUpGroupAcl(store, delegate);
    delegate.aclListToFetch = store.GetNodeACLList();

    delegate.deferKind = SyncKind::kAclList;
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    ASSERT_EQ(delegate.deferred.size(), 1u);
    ASSERT_EQ(SetGroupTenPermission(store, Privilege::kManage), CHIP_NO_ERROR);

    delegate.ResetCapturedSyncs();
    delegate.RunDeferred();

    ASSERT_TRUE(delegate.hasLastAclSync);
    EXPECT_EQ(delegate.lastAclSyncOwned->privilege, Privilege::kManage);
    ASSERT_TRUE(delegate.lastAclSuperseded.has_value());
    EXPECT_EQ(delegate.lastAclSuperseded->privilege, Privilege::kView);
    EXPECT_EQ(FindAcl(store, 123, 7)->statusEntry.state, State::kCommitted);
    EXPECT_EQ(FindAcl(store, 123, 7)->ACLEntry.privilege, Privilege::kManage);
}

// UpdateGroup changes entries on every node in the group: it is rejected whole if any node's queue is full.
TEST(JointFabricDatastoreTest, UpdateGroupWhenSyncQueueFullIsBusyAndChangesNothing)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddGroupTen(store, std::nullopt);
    SeedAcls(store, JointFabricDatastore::kMaxQueuedNodeSyncs + 1);
    SeedAcl(store, 123, 100, Privilege::kView, AuthMode::kGroup, { 10 }, State::kCommitted);
    FillSyncQueue(store, delegate);

    EXPECT_EQ(SetGroupTenPermission(store, Privilege::kManage), CHIP_IM_GLOBAL_STATUS(Busy));
    EXPECT_EQ(store.GetGroupEntries()[0].groupPermission, Privilege::kView);
    EXPECT_EQ(FindAcl(store, 123, 100)->ACLEntry.privilege, Privilege::kView);
    EXPECT_EQ(FindAcl(store, 123, 100)->statusEntry.state, State::kCommitted);
}

// An entry removed while its update has not reached the node: the removal removes the old value too.
TEST(JointFabricDatastoreTest, RemovingEntryWithPendingUpdateSendsSupersededValue)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    SetUpGroupAcl(store, delegate);

    delegate.completeWith[SyncKind::kAcl] = CHIP_IM_GLOBAL_STATUS(Timeout);
    ASSERT_EQ(SetGroupTenPermission(store, Privilege::kManage), CHIP_NO_ERROR);
    ASSERT_EQ(FindAcl(store, 123, 7)->statusEntry.state, State::kCommitFailed);

    ASSERT_EQ(store.RemoveACLFromNode(7, 123), CHIP_NO_ERROR);

    EXPECT_EQ(delegate.lastAclSyncState, State::kDeletePending);
    EXPECT_EQ(delegate.lastAclSyncOwned->privilege, Privilege::kManage);
    ASSERT_TRUE(delegate.lastAclSuperseded.has_value());
    EXPECT_EQ(delegate.lastAclSuperseded->privilege, Privilege::kView);
    EXPECT_EQ(FindAcl(store, 123, 7), nullptr);
}

// Endpoints without a Groups cluster fail the group fetch. The merge for that endpoint is skipped, and
// the refresh still commits the node.
TEST(JointFabricDatastoreTest, RefreshCommitsNodeWhenGroupFetchFails)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    AddEndpointOneToRefresh(store, delegate);
    store.GetEndpointGroupIDList().push_back(MakeEndpointGroupEntry(State::kCommitted));

    delegate.fetchGroupListResult = CHIP_IM_GLOBAL_STATUS(UnsupportedCluster);
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    EXPECT_EQ(store.GetNodeInformationEntries()[0].commissioningStatusEntry.state, State::kCommitted);
    EXPECT_EQ(store.GetEndpointGroupIDList().size(), 1u);
}

// AddGroup 0x000A with CAT 0x2345 v1 and permission Operate; node 123 exists.
void SetUpCatGroup(JointFabricDatastore & store)
{
    JointFabricCluster::Commands::AddGroup::DecodableType addGroup;
    addGroup.groupID      = 0x000A;
    addGroup.friendlyName = "cat-group"_span;
    addGroup.groupCAT.SetNonNull(static_cast<uint16_t>(0x2345));
    addGroup.groupCATVersion.SetNonNull(static_cast<uint16_t>(1));
    addGroup.groupPermission = Privilege::kOperate;
    ASSERT_EQ(store.AddGroup(addGroup), CHIP_NO_ERROR);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
}

CHIP_ERROR UpdateCatGroup(JointFabricDatastore & store, std::optional<uint16_t> cat, std::optional<uint16_t> version,
                          std::optional<Privilege> permission)
{
    JointFabricCluster::Commands::UpdateGroup::DecodableType update;
    update.groupID = 0x000A;
    if (cat.has_value())
    {
        update.groupCAT.SetNonNull(*cat);
    }
    if (version.has_value())
    {
        update.groupCATVersion.SetNonNull(*version);
    }
    if (permission.has_value())
    {
        update.groupPermission.SetNonNull(*permission);
    }
    return store.UpdateGroup(update);
}

// A CAT subject is kMinCASEAuthTag | (cat << 16) | version. UpdateGroup selects it by CAT identifier
// and rewrites it to the new version.
TEST(JointFabricDatastoreTest, UpdateGroupRewritesCatSubjectToNewVersion)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    SetUpCatGroup(store);
    const NodeId oldSubject = NodeIdFromCASEAuthTag(0x2345'0001);
    const NodeId newSubject = NodeIdFromCASEAuthTag(0x2345'0002);
    SeedAcl(store, 123, 5, Privilege::kOperate, AuthMode::kCase, { oldSubject }, State::kCommitted);

    ASSERT_EQ(UpdateCatGroup(store, std::nullopt, 2, std::nullopt), CHIP_NO_ERROR);

    ASSERT_TRUE(delegate.lastAclSyncOwned.has_value());
    EXPECT_TRUE(delegate.lastAclSyncOwned->subjects == std::vector<uint64_t>{ newSubject });
    ASSERT_TRUE(delegate.lastAclSuperseded.has_value());
    EXPECT_TRUE(delegate.lastAclSuperseded->subjects == std::vector<uint64_t>{ oldSubject });
    EXPECT_EQ(delegate.lastAclSyncOwned->privilege, Privilege::kOperate); // version-only bump
    EXPECT_EQ(FindAcl(store, 123, 5)->statusEntry.state, State::kCommitted);
}

// A CASE subject 0x000A is node ID 10, not group 0x000A.
TEST(JointFabricDatastoreTest, UpdateGroupIgnoresCaseSubjectEqualToGroupId)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    SetUpCatGroup(store);
    SeedAcl(store, 123, 5, Privilege::kView, AuthMode::kCase, { 0x000A }, State::kCommitted);

    ASSERT_EQ(UpdateCatGroup(store, std::nullopt, std::nullopt, Privilege::kAdminister), CHIP_NO_ERROR);

    EXPECT_FALSE(delegate.hasLastAclSync);
    EXPECT_EQ(FindAcl(store, 123, 5)->ACLEntry.privilege, Privilege::kView);
}

TEST(JointFabricDatastoreTest, UpdateGroupRewritesOnCatIdentifierChange)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    SetUpCatGroup(store);
    SeedAcl(store, 123, 5, Privilege::kOperate, AuthMode::kCase, { NodeIdFromCASEAuthTag(0x2345'0001) }, State::kCommitted);

    ASSERT_EQ(UpdateCatGroup(store, 0x3456, std::nullopt, std::nullopt), CHIP_NO_ERROR);

    EXPECT_TRUE(FindAcl(store, 123, 5)->ACLEntry.subjects == std::vector<uint64_t>{ NodeIdFromCASEAuthTag(0x3456'0001) });
    ASSERT_TRUE(delegate.lastAclSuperseded.has_value());
    EXPECT_TRUE(delegate.lastAclSuperseded->subjects == std::vector<uint64_t>{ NodeIdFromCASEAuthTag(0x2345'0001) });
}

// Matching by CAT identifier also catches an entry left at an older version.
TEST(JointFabricDatastoreTest, UpdateGroupRewritesEntryLeftAtOlderVersion)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    SetUpCatGroup(store);
    ASSERT_EQ(UpdateCatGroup(store, std::nullopt, 2, std::nullopt), CHIP_NO_ERROR);
    SeedAcl(store, 123, 5, Privilege::kOperate, AuthMode::kCase, { NodeIdFromCASEAuthTag(0x2345'0001) }, State::kCommitted);

    ASSERT_EQ(UpdateCatGroup(store, std::nullopt, 3, std::nullopt), CHIP_NO_ERROR);

    EXPECT_TRUE(FindAcl(store, 123, 5)->ACLEntry.subjects == std::vector<uint64_t>{ NodeIdFromCASEAuthTag(0x2345'0003) });
}

TEST(JointFabricDatastoreTest, UpdateGroupLeavesUnrelatedSubjectsOnVersionBump)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    SetUpCatGroup(store);
    SeedAcl(store, 123, 5, Privilege::kView, AuthMode::kCase, { NodeIdFromCASEAuthTag(0x2345'0001), 0xDEADBEEF },
            State::kCommitted);

    ASSERT_EQ(UpdateCatGroup(store, std::nullopt, 2, std::nullopt), CHIP_NO_ERROR);

    const auto * entry = FindAcl(store, 123, 5);
    EXPECT_TRUE(entry->ACLEntry.subjects == (std::vector<uint64_t>{ NodeIdFromCASEAuthTag(0x2345'0002), 0xDEADBEEF }));
    EXPECT_EQ(entry->ACLEntry.privilege, Privilege::kView);
    EXPECT_EQ(store.GetNodeACLList().size(), 1u);
}

// A permission change applies only to the group's subjects: an entry that also grants other subjects is split.
TEST(JointFabricDatastoreTest, UpdateGroupSplitsMixedEntryOnPermissionChange)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    SetUpCatGroup(store);
    const NodeId catSubject = NodeIdFromCASEAuthTag(0x2345'0001);
    SeedAcl(store, 123, 5, Privilege::kOperate, AuthMode::kCase, { catSubject, 0xDEADBEEF }, State::kCommitted);

    delegate.deferKind = SyncKind::kAcl;
    ASSERT_EQ(UpdateCatGroup(store, std::nullopt, std::nullopt, Privilege::kManage), CHIP_NO_ERROR);

    // The replace of the original entry is issued first.
    ASSERT_EQ(delegate.deferred.size(), 1u);
    EXPECT_TRUE(delegate.lastAclSyncOwned->subjects == std::vector<uint64_t>{ 0xDEADBEEF });
    EXPECT_EQ(delegate.lastAclSyncOwned->privilege, Privilege::kOperate);
    ASSERT_TRUE(delegate.lastAclSuperseded.has_value());
    EXPECT_TRUE(delegate.lastAclSuperseded->subjects == (std::vector<uint64_t>{ catSubject, 0xDEADBEEF }));

    delegate.RunDeferred();
    ASSERT_EQ(delegate.deferred.size(), 1u);
    EXPECT_TRUE(delegate.lastAclSyncOwned->subjects == std::vector<uint64_t>{ catSubject });
    EXPECT_EQ(delegate.lastAclSyncOwned->privilege, Privilege::kManage);
    EXPECT_FALSE(delegate.lastAclSuperseded.has_value());
    delegate.RunDeferred();

    ASSERT_EQ(store.GetNodeACLList().size(), 2u);
    const auto & added = store.GetNodeACLList()[1];
    EXPECT_NE(added.listID, 5u);
    EXPECT_EQ(added.statusEntry.state, State::kCommitted);
    EXPECT_EQ(FindAcl(store, 123, 5)->statusEntry.state, State::kCommitted);
}

TEST(JointFabricDatastoreTest, UpdateGroupStillUpdatesGroupAuthPrivilege)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    SetUpCatGroup(store);
    SeedAcl(store, 123, 5, Privilege::kOperate, AuthMode::kGroup, { 0x000A }, State::kCommitted);

    ASSERT_EQ(UpdateCatGroup(store, std::nullopt, std::nullopt, Privilege::kManage), CHIP_NO_ERROR);

    EXPECT_EQ(FindAcl(store, 123, 5)->ACLEntry.privilege, Privilege::kManage);
    EXPECT_TRUE(FindAcl(store, 123, 5)->ACLEntry.subjects == std::vector<uint64_t>{ 0x000A });
}

TEST(JointFabricDatastoreTest, UpdateGroupVersionBumpLeavesGroupAuthEntryAlone)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    SetUpCatGroup(store);
    SeedAcl(store, 123, 5, Privilege::kView, AuthMode::kGroup, { 0x000A }, State::kCommitted);

    ASSERT_EQ(UpdateCatGroup(store, std::nullopt, 2, std::nullopt), CHIP_NO_ERROR);

    EXPECT_FALSE(delegate.hasLastAclSync);
    EXPECT_EQ(FindAcl(store, 123, 5)->ACLEntry.privilege, Privilege::kView);
}

TEST(JointFabricDatastoreTest, UpdateGroupRejectsCatVersionZero)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    SetUpCatGroup(store);

    EXPECT_EQ(UpdateCatGroup(store, 0x3456, 0, Privilege::kManage), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    const auto & group = store.GetGroupEntries()[0];
    EXPECT_EQ(group.groupCAT.Value(), 0x2345u);
    EXPECT_EQ(group.groupCATVersion.Value(), 1u);
    EXPECT_EQ(group.groupPermission, Privilege::kOperate);
}

// A rewrite whose sync failed is retried by RefreshNode as a replace: the node's old subject is not adopted.
TEST(JointFabricDatastoreTest, FailedCatRewriteIsRetriedAsReplaceAtRefresh)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    SetUpCatGroup(store);
    const NodeId oldSubject = NodeIdFromCASEAuthTag(0x2345'0001);
    const NodeId newSubject = NodeIdFromCASEAuthTag(0x2345'0002);
    SeedAcl(store, 123, 5, Privilege::kOperate, AuthMode::kCase, { oldSubject }, State::kCommitted);
    delegate.aclListToFetch = store.GetNodeACLList(); // the node holds v1 throughout

    delegate.completeWith[SyncKind::kAcl] = CHIP_IM_GLOBAL_STATUS(Timeout);
    ASSERT_EQ(UpdateCatGroup(store, std::nullopt, 2, std::nullopt), CHIP_NO_ERROR);
    ASSERT_EQ(FindAcl(store, 123, 5)->statusEntry.state, State::kCommitFailed);
    ASSERT_TRUE(FindAcl(store, 123, 5)->supersededValue.has_value());

    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);

    const auto & written = delegate.aclListSyncs.back().second;
    ASSERT_EQ(written.size(), 1u);
    EXPECT_TRUE(written[0].subjects == std::vector<uint64_t>{ newSubject });
    ASSERT_EQ(store.GetNodeACLList().size(), 1u);
    EXPECT_EQ(FindAcl(store, 123, 5)->statusEntry.state, State::kCommitted);
}

// Removing the joint fabric drops queued syncs. A sync in flight still completes, and its completion changes
// nothing: it must not start an operation queued since, which would then run alongside the one in flight.
TEST(JointFabricDatastoreTest, FabricRemovalIgnoresSyncCompletingAfterIt)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    store.SetAnchorFabricIndex(1);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 5, Privilege::kView, AuthMode::kCase, { 0x5 }, State::kCommitted);
    SeedAcl(store, 123, 6, Privilege::kView, AuthMode::kCase, { 0x6 }, State::kCommitted);

    auto aclSyncCount = [&delegate]() {
        return std::count_if(delegate.syncCalls.begin(), delegate.syncCalls.end(),
                             [](const auto & call) { return call.second == SyncKind::kAcl; });
    };

    delegate.deferKind = SyncKind::kAcl;
    ASSERT_EQ(store.RemoveACLFromNode(5, 123), CHIP_NO_ERROR);
    ASSERT_EQ(store.RemoveACLFromNode(6, 123), CHIP_NO_ERROR);
    ASSERT_EQ(aclSyncCount(), 1);

    store.OnFabricRemoved(1);

    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    SeedAcl(store, 123, 7, Privilege::kView, AuthMode::kCase, { 0x7 }, State::kCommitted);
    SeedAcl(store, 123, 8, Privilege::kView, AuthMode::kCase, { 0x8 }, State::kCommitted);
    ASSERT_EQ(store.RemoveACLFromNode(7, 123), CHIP_NO_ERROR);
    ASSERT_EQ(store.RemoveACLFromNode(8, 123), CHIP_NO_ERROR);
    EXPECT_EQ(aclSyncCount(), 2);

    delegate.RunDeferred(); // the removal of 5, from before the fabric was removed
    EXPECT_EQ(aclSyncCount(), 2);
    EXPECT_NE(FindAcl(store, 123, 7), nullptr);

    delegate.RunDeferred(); // the removal of 7
    EXPECT_EQ(FindAcl(store, 123, 7), nullptr);
    EXPECT_EQ(aclSyncCount(), 3);
}

// A refresh in flight when the joint fabric is removed no longer blocks refreshes, and its completion does not end
// a later one.
TEST(JointFabricDatastoreTest, FabricRemovalDuringRefreshReleasesIt)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);
    store.SetAnchorFabricIndex(1);
    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);

    delegate.deferKind = SyncKind::kAclList;
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    ASSERT_EQ(delegate.deferred.size(), 1u);

    store.OnFabricRemoved(1);

    ASSERT_EQ(store.AddPendingNode(123, "node-a"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.RefreshNode(123), CHIP_NO_ERROR);
    ASSERT_EQ(delegate.deferred.size(), 2u);

    delegate.RunDeferred(); // the first refresh's ACL write
    EXPECT_EQ(store.GetNodeInformationEntries()[0].commissioningStatusEntry.state, State::kPending);

    delegate.RunDeferred(); // the second refresh's ACL write
    EXPECT_EQ(store.GetNodeInformationEntries()[0].commissioningStatusEntry.state, State::kCommitted);
}

TEST(JointFabricDatastoreTest, OnFabricRemovedWipesDatastoreOnlyForAnchorFabric)
{
    JointFabricDatastore store;
    TrackingDelegate delegate;
    ASSERT_EQ(store.SetDelegate(&delegate), CHIP_NO_ERROR);

    constexpr FabricIndex kAnchorFabric = 1;
    constexpr FabricIndex kOtherFabric  = 2;
    store.SetAnchorFabricIndex(kAnchorFabric);
    ASSERT_EQ(store.SetAnchorNodeId(0xABCD), CHIP_NO_ERROR);

    // A spread of records, including the secret-bearing group-key-set and admin (ICAC) entries.
    uint8_t epoch0[] = { 0x01, 0x02, 0x03 };
    GroupKeySetType keySet;
    keySet.groupKeySetID = 11;
    keySet.epochKey0.SetNonNull(ByteSpan(epoch0));
    keySet.epochKey1.SetNull();
    keySet.epochKey2.SetNull();
    ASSERT_EQ(store.AddGroupKeySetEntry(keySet), CHIP_NO_ERROR);

    char adminName[] = "admin";
    uint8_t icac[]   = { 0x0A, 0x0B };
    AdminEntryType admin;
    admin.nodeID       = 100;
    admin.vendorID     = static_cast<VendorId>(7);
    admin.friendlyName = CharSpan(adminName, sizeof(adminName) - 1);
    admin.icac         = ByteSpan(icac);
    ASSERT_EQ(store.AddAdmin(admin), CHIP_NO_ERROR);

    ASSERT_EQ(store.AddPendingNode(123, "controller"_span), CHIP_NO_ERROR);
    ASSERT_EQ(store.TestAddEndpointEntry(1, 123, "endpoint"_span), CHIP_NO_ERROR);

    JointFabricCluster::Structs::DatastoreBindingTargetStruct::Type binding;
    binding.group.SetValue(10);
    ASSERT_EQ(store.AddBindingToEndpointForNode(123, 1, binding), CHIP_NO_ERROR);

    JointFabricCluster::Structs::DatastoreAccessControlEntryStruct::DecodableType aclEntry;
    aclEntry.privilege = JointFabricCluster::DatastoreAccessControlEntryPrivilegeEnum::kView;
    aclEntry.authMode  = JointFabricCluster::DatastoreAccessControlEntryAuthModeEnum::kCase;
    ASSERT_EQ(store.AddACLToNode(123, aclEntry), CHIP_NO_ERROR);

    // Removing a non-anchor fabric leaves everything intact.
    store.OnFabricRemoved(kOtherFabric);
    EXPECT_EQ(store.GetGroupKeySetList().size(), 1u);
    EXPECT_EQ(store.GetAdminEntries().size(), 1u);
    EXPECT_EQ(store.GetNodeInformationEntries().size(), 1u);
    EXPECT_EQ(store.GetNodeACLList().size(), 1u);
    EXPECT_EQ(store.GetEndpointBindingList().size(), 1u);
    EXPECT_EQ(store.GetNodeEndpointList().size(), 1u);
    EXPECT_EQ(store.GetAnchorFabricIndex(), kAnchorFabric);

    // Removing the joint (anchor) fabric wipes the datastore and resets anchor identity.
    store.OnFabricRemoved(kAnchorFabric);
    EXPECT_TRUE(store.GetGroupKeySetList().empty());
    EXPECT_TRUE(store.GetAdminEntries().empty());
    EXPECT_TRUE(store.GetNodeInformationEntries().empty());
    EXPECT_TRUE(store.GetNodeACLList().empty());
    EXPECT_TRUE(store.GetEndpointBindingList().empty());
    EXPECT_TRUE(store.GetNodeEndpointList().empty());
    EXPECT_EQ(store.GetAnchorFabricIndex(), kUndefinedFabricIndex);
    EXPECT_EQ(store.GetAnchorNodeId(), kUndefinedNodeId);
}

// Direct unit tests for the detail::MarkEntryCommittedIfFound helper. It re-resolves an entry by a
// stable-key predicate (rather than a captured iterator/index) inside an async SyncNode completion
// and marks the matched entry kCommitted. A minimal fake entry mirrors the only field the helper
// touches (statusEntry.state), keeping these tests focused on the generic lookup/mutate contract.
namespace {

struct FakeEntry
{
    int key;
    struct
    {
        JointFabricCluster::DatastoreStateEnum state;
    } statusEntry;
};

FakeEntry MakePendingEntry(int key)
{
    return FakeEntry{ key, { JointFabricCluster::DatastoreStateEnum::kPending } };
}

auto MatchKey(int key)
{
    return [key](const FakeEntry & e) { return e.key == key; };
}

} // namespace

TEST(MarkEntryCommittedIfFoundTest, MarksMatchingEntryCommitted)
{
    std::vector<FakeEntry> entries{ MakePendingEntry(1) };

    chip::app::detail::MarkEntryCommittedIfFound(entries, MatchKey(1));

    EXPECT_EQ(entries[0].statusEntry.state, JointFabricCluster::DatastoreStateEnum::kCommitted);
}

TEST(MarkEntryCommittedIfFoundTest, LeavesNonMatchingEntriesUntouched)
{
    std::vector<FakeEntry> entries{ MakePendingEntry(1), MakePendingEntry(2), MakePendingEntry(3) };

    chip::app::detail::MarkEntryCommittedIfFound(entries, MatchKey(2));

    EXPECT_EQ(entries[0].statusEntry.state, JointFabricCluster::DatastoreStateEnum::kPending);
    EXPECT_EQ(entries[1].statusEntry.state, JointFabricCluster::DatastoreStateEnum::kCommitted);
    EXPECT_EQ(entries[2].statusEntry.state, JointFabricCluster::DatastoreStateEnum::kPending);
}

TEST(MarkEntryCommittedIfFoundTest, NoMatchIsNoOp)
{
    std::vector<FakeEntry> entries{ MakePendingEntry(1), MakePendingEntry(2) };

    chip::app::detail::MarkEntryCommittedIfFound(entries, MatchKey(99));

    EXPECT_EQ(entries[0].statusEntry.state, JointFabricCluster::DatastoreStateEnum::kPending);
    EXPECT_EQ(entries[1].statusEntry.state, JointFabricCluster::DatastoreStateEnum::kPending);
}

TEST(MarkEntryCommittedIfFoundTest, EmptyVectorIsNoOp)
{
    std::vector<FakeEntry> entries;

    // Must not dereference end(); simply does nothing on an empty vector.
    chip::app::detail::MarkEntryCommittedIfFound(entries, MatchKey(1));

    EXPECT_TRUE(entries.empty());
}

TEST(MarkEntryCommittedIfFoundTest, MarksOnlyFirstMatch)
{
    // find_if stops at the first match: a duplicate key only commits the earliest entry. Callers are
    // expected to pass predicates that uniquely identify the synced entry.
    std::vector<FakeEntry> entries{ MakePendingEntry(7), MakePendingEntry(7) };

    chip::app::detail::MarkEntryCommittedIfFound(entries, MatchKey(7));

    EXPECT_EQ(entries[0].statusEntry.state, JointFabricCluster::DatastoreStateEnum::kCommitted);
    EXPECT_EQ(entries[1].statusEntry.state, JointFabricCluster::DatastoreStateEnum::kPending);
}

// Nodes report an empty subject or target list as null (AclStorage.cpp); the datastore sends an
// empty non-null list. The two compare equal.
TEST(AclEntryValueEqualsTest, NullListEqualsEmptyList)
{
    AclType a;
    a.privilege = Privilege::kView;
    a.authMode  = AuthMode::kCase;
    a.subjects.SetNull();
    a.targets.SetNull();

    AclType b = a;
    b.subjects.SetNonNull();
    b.targets.SetNonNull();

    EXPECT_TRUE(chip::app::detail::AclEntryValueEquals(a, b));
}

TEST(AclEntryValueEqualsTest, SubjectOrderDoesNotMatter)
{
    const uint64_t s1[] = { 0x1111, 0x2222 };
    const uint64_t s2[] = { 0x2222, 0x1111 };
    AclType a;
    a.privilege = Privilege::kView;
    a.authMode  = AuthMode::kCase;
    a.subjects.SetNonNull(s1);
    a.targets.SetNull();
    AclType b = a;
    b.subjects.SetNonNull(s2);

    EXPECT_TRUE(chip::app::detail::AclEntryValueEquals(a, b));
}

TEST(AclEntryValueEqualsTest, TargetOrderDoesNotMatter)
{
    TargetType t1[2];
    t1[0].cluster.SetNonNull(6u);
    t1[0].endpoint.SetNull();
    t1[0].deviceType.SetNull();
    t1[1].cluster.SetNull();
    t1[1].endpoint.SetNonNull(1);
    t1[1].deviceType.SetNull();
    const TargetType t2[] = { t1[1], t1[0] };

    AclType a;
    a.privilege = Privilege::kView;
    a.authMode  = AuthMode::kCase;
    a.subjects.SetNull();
    a.targets.SetNonNull(t1);
    AclType b = a;
    b.targets.SetNonNull(t2);

    EXPECT_TRUE(chip::app::detail::AclEntryValueEquals(a, b));

    b.targets.SetNonNull(Span<const TargetType>(t2, 1));
    EXPECT_FALSE(chip::app::detail::AclEntryValueEquals(a, b));
}

TEST(AclEntryValueEqualsTest, PrivilegeMatters)
{
    AclType a;
    a.privilege = Privilege::kManage;
    a.authMode  = AuthMode::kCase;
    a.subjects.SetNull();
    a.targets.SetNull();
    AclType b   = a;
    b.privilege = Privilege::kView;

    EXPECT_FALSE(chip::app::detail::AclEntryValueEquals(a, b));
}

TEST(AclEntryValueEqualsTest, SubjectsMatter)
{
    const uint64_t s1[] = { 0x1111 };
    const uint64_t s2[] = { 0x1111, 0x2222 };
    AclType a;
    a.privilege = Privilege::kView;
    a.authMode  = AuthMode::kCase;
    a.subjects.SetNonNull(s1);
    a.targets.SetNull();
    AclType b = a;
    b.subjects.SetNonNull(s2);

    EXPECT_FALSE(chip::app::detail::AclEntryValueEquals(a, b));
}

const uint64_t kSubjectX[] = { 0x1111 };
const uint64_t kSubjectY[] = { 0x2222 };
const Span<const uint64_t> kSubjectsX(kSubjectX);
const Span<const uint64_t> kSubjectsY(kSubjectY);

// Builds an ACL entry as the datastore sends it: an empty target list is non-null.
ACLEntryType MakeAclEntry(Privilege privilege, Span<const uint64_t> subjects, State state = State::kPending)
{
    ACLEntryType entry;
    entry.nodeID             = 123;
    entry.ACLEntry.privilege = privilege;
    entry.ACLEntry.authMode  = AuthMode::kCase;
    entry.ACLEntry.subjects.SetNonNull(subjects);
    entry.ACLEntry.targets.SetNonNull();
    entry.statusEntry.state = state;
    return entry;
}

AclType MakeAcl(Privilege privilege, Span<const uint64_t> subjects)
{
    return MakeAclEntry(privilege, subjects).ACLEntry;
}

TEST(ApplyAclEditTest, AddAppendsWhenAbsent)
{
    const std::vector<ACLEntryType> current{ MakeAclEntry(Privilege::kView, kSubjectsX) };

    const auto result = chip::app::detail::ApplyAclEdit(current, MakeAclEntry(Privilege::kManage, kSubjectsY), std::nullopt);

    ASSERT_EQ(result.size(), 2u);
    EXPECT_TRUE(chip::app::detail::AclEntryValueEquals(result[0].ACLEntry, MakeAcl(Privilege::kView, kSubjectsX)));
    EXPECT_TRUE(chip::app::detail::AclEntryValueEquals(result[1].ACLEntry, MakeAcl(Privilege::kManage, kSubjectsY)));
}

TEST(ApplyAclEditTest, AddIsIdempotent)
{
    const std::vector<ACLEntryType> current{ MakeAclEntry(Privilege::kView, kSubjectsX) };

    const auto result = chip::app::detail::ApplyAclEdit(current, MakeAclEntry(Privilege::kView, kSubjectsX), std::nullopt);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_TRUE(chip::app::detail::AclEntryValueEquals(result[0].ACLEntry, MakeAcl(Privilege::kView, kSubjectsX)));
}

TEST(ApplyAclEditTest, ReplaceSwapsSupersededForNew)
{
    const std::vector<ACLEntryType> current{ MakeAclEntry(Privilege::kManage, kSubjectsX) };

    const auto result = chip::app::detail::ApplyAclEdit(current, MakeAclEntry(Privilege::kView, kSubjectsX),
                                                        std::make_optional(MakeAcl(Privilege::kManage, kSubjectsX)));

    ASSERT_EQ(result.size(), 1u);
    EXPECT_TRUE(chip::app::detail::AclEntryValueEquals(result[0].ACLEntry, MakeAcl(Privilege::kView, kSubjectsX)));
}

TEST(ApplyAclEditTest, ReplaceWithSupersededAbsentStillEnsuresNew)
{
    const std::vector<ACLEntryType> current{ MakeAclEntry(Privilege::kView, kSubjectsY) };

    const auto result = chip::app::detail::ApplyAclEdit(current, MakeAclEntry(Privilege::kView, kSubjectsX),
                                                        std::make_optional(MakeAcl(Privilege::kManage, kSubjectsX)));

    ASSERT_EQ(result.size(), 2u);
    EXPECT_TRUE(chip::app::detail::AclEntryValueEquals(result[0].ACLEntry, MakeAcl(Privilege::kView, kSubjectsY)));
    EXPECT_TRUE(chip::app::detail::AclEntryValueEquals(result[1].ACLEntry, MakeAcl(Privilege::kView, kSubjectsX)));
}

TEST(ApplyAclEditTest, RemoveErasesAllEqualEntries)
{
    const std::vector<ACLEntryType> current{ MakeAclEntry(Privilege::kView, kSubjectsX),
                                             MakeAclEntry(Privilege::kView, kSubjectsX) };

    const auto result =
        chip::app::detail::ApplyAclEdit(current, MakeAclEntry(Privilege::kView, kSubjectsX, State::kDeletePending), std::nullopt);

    EXPECT_TRUE(result.empty());
}

TEST(ApplyAclEditTest, RemoveOfAbsentEntryIsNoOp)
{
    const std::vector<ACLEntryType> current{ MakeAclEntry(Privilege::kView, kSubjectsY) };

    const auto result =
        chip::app::detail::ApplyAclEdit(current, MakeAclEntry(Privilege::kView, kSubjectsX, State::kDeletePending), std::nullopt);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_TRUE(chip::app::detail::AclEntryValueEquals(result[0].ACLEntry, MakeAcl(Privilege::kView, kSubjectsY)));
}

TEST(ApplyAclEditTest, RemoveMatchesNullTargetsAgainstEmpty)
{
    auto fetched = MakeAclEntry(Privilege::kView, kSubjectsX);
    fetched.ACLEntry.targets.SetNull();
    const std::vector<ACLEntryType> current{ fetched };

    const auto result =
        chip::app::detail::ApplyAclEdit(current, MakeAclEntry(Privilege::kView, kSubjectsX, State::kDeletePending), std::nullopt);

    EXPECT_TRUE(result.empty());
}

TEST(ApplyAclEditTest, RemoveLeavesUnrelatedEntries)
{
    const std::vector<ACLEntryType> current{ MakeAclEntry(Privilege::kView, kSubjectsX),
                                             MakeAclEntry(Privilege::kManage, kSubjectsY),
                                             MakeAclEntry(Privilege::kView, kSubjectsY) };

    const auto result =
        chip::app::detail::ApplyAclEdit(current, MakeAclEntry(Privilege::kView, kSubjectsX, State::kDeletePending), std::nullopt);

    ASSERT_EQ(result.size(), 2u);
    EXPECT_TRUE(chip::app::detail::AclEntryValueEquals(result[0].ACLEntry, MakeAcl(Privilege::kManage, kSubjectsY)));
    EXPECT_TRUE(chip::app::detail::AclEntryValueEquals(result[1].ACLEntry, MakeAcl(Privilege::kView, kSubjectsY)));
}

// An entry removed while an update was Pending: the node may still hold the superseded value.
TEST(ApplyAclEditTest, RemoveAlsoErasesSupersededValue)
{
    const std::vector<ACLEntryType> current{ MakeAclEntry(Privilege::kManage, kSubjectsX) };

    const auto result = chip::app::detail::ApplyAclEdit(current, MakeAclEntry(Privilege::kView, kSubjectsX, State::kDeletePending),
                                                        std::make_optional(MakeAcl(Privilege::kManage, kSubjectsX)));

    EXPECT_TRUE(result.empty());
}

BindingEntryType MakeBindingEntry(EndpointId endpointId, GroupId groupId, State state = State::kPending)
{
    BindingEntryType entry;
    entry.nodeID     = 123;
    entry.endpointID = endpointId;
    entry.binding.group.SetValue(groupId);
    entry.statusEntry.state = state;
    return entry;
}

TEST(ApplyBindingEditTest, AddAppendsWhenAbsent)
{
    const std::vector<BindingEntryType> current{ MakeBindingEntry(1, 10) };

    const auto result = chip::app::detail::ApplyBindingEdit(current, MakeBindingEntry(1, 11));

    ASSERT_EQ(result.size(), 2u);
    EXPECT_TRUE(chip::app::detail::BindingEntryValueEquals(result[1], MakeBindingEntry(1, 11)));
}

TEST(ApplyBindingEditTest, AddIsIdempotent)
{
    const std::vector<BindingEntryType> current{ MakeBindingEntry(1, 10) };

    const auto result = chip::app::detail::ApplyBindingEdit(current, MakeBindingEntry(1, 10));

    EXPECT_EQ(result.size(), 1u);
}

// listID does not survive the wire, so removal matches on endpointID and the binding target only.
TEST(ApplyBindingEditTest, RemoveMatchesByValue)
{
    auto fetched   = MakeBindingEntry(1, 10);
    fetched.listID = 5;
    const std::vector<BindingEntryType> current{ fetched, MakeBindingEntry(2, 10), MakeBindingEntry(1, 11) };

    auto removal      = MakeBindingEntry(1, 10, State::kDeletePending);
    removal.listID    = 9;
    const auto result = chip::app::detail::ApplyBindingEdit(current, removal);

    ASSERT_EQ(result.size(), 2u);
    EXPECT_TRUE(chip::app::detail::BindingEntryValueEquals(result[0], MakeBindingEntry(2, 10)));
    EXPECT_TRUE(chip::app::detail::BindingEntryValueEquals(result[1], MakeBindingEntry(1, 11)));
}

TEST(ApplyBindingEditTest, RemoveOfAbsentEntryIsNoOp)
{
    const std::vector<BindingEntryType> current{ MakeBindingEntry(1, 10) };

    const auto result = chip::app::detail::ApplyBindingEdit(current, MakeBindingEntry(1, 11, State::kDeletePending));

    ASSERT_EQ(result.size(), 1u);
    EXPECT_TRUE(chip::app::detail::BindingEntryValueEquals(result[0], MakeBindingEntry(1, 10)));
}

} // namespace
