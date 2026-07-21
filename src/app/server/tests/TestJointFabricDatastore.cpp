#include "app/server/JointFabricDatastore.h"

#include <pw_unit_test/framework.h>

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

class TrackingDelegate : public JointFabricDatastore::Delegate
{
public:
    CHIP_ERROR SyncNode(NodeId nodeId, const EndpointGroupIdEntryType & endpointGroupIDEntry,
                        std::function<void()> onSuccess) override
    {
        lastEndpointGroupSync    = endpointGroupIDEntry;
        hasLastEndpointGroupSync = true;
        if (onSuccess)
        {
            onSuccess();
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SyncNode(NodeId nodeId, const NodeKeySetEntryType & nodeKeySetEntry, std::function<void()> onSuccess) override
    {
        lastNodeKeySetSync    = nodeKeySetEntry;
        hasLastNodeKeySetSync = true;
        if (onSuccess)
        {
            onSuccess();
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SyncNode(NodeId nodeId, const BindingEntryType & bindingEntry, std::function<void()> onSuccess) override
    {
        lastBindingSync    = bindingEntry;
        hasLastBindingSync = true;
        if (onSuccess)
        {
            onSuccess();
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SyncNode(NodeId nodeId, std::vector<BindingEntryType> & bindingEntries, std::function<void()> onSuccess) override
    {
        if (onSuccess)
        {
            onSuccess();
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SyncNode(NodeId nodeId, const ACLEntryType & aclEntry, std::function<void()> onSuccess) override
    {
        lastAclSync    = aclEntry;
        hasLastAclSync = true;
        if (onSuccess)
        {
            onSuccess();
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SyncNode(NodeId nodeId, const std::vector<ACLEntryType> & aclEntries, std::function<void()> onSuccess) override
    {
        if (onSuccess)
        {
            onSuccess();
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SyncNode(NodeId nodeId, const GroupKeySetType & groupKeySet, std::function<void()> onSuccess) override
    {
        if (onSuccess)
        {
            onSuccess();
        }
        return CHIP_NO_ERROR;
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
        onSuccess(CHIP_NO_ERROR, {});
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FetchEndpointBindingList(NodeId nodeId, EndpointId endpointId,
                                        std::function<void(CHIP_ERROR, const std::vector<BindingEntryType> &)> onSuccess) override
    {
        onSuccess(CHIP_NO_ERROR, {});
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
        onSuccess(CHIP_NO_ERROR, {});
        return CHIP_NO_ERROR;
    }

    void ResetCapturedSyncs()
    {
        hasLastEndpointGroupSync = false;
        hasLastNodeKeySetSync    = false;
        hasLastBindingSync       = false;
        hasLastAclSync           = false;
    }

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
    ACLEntryType lastAclSync;
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
    std::function<void()> deferred;

    CHIP_ERROR SyncNode(NodeId nodeId, const BindingEntryType & bindingEntry, std::function<void()> onSuccess) override
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
            onSuccess();
        }
        return CHIP_NO_ERROR;
    }

    void RunDeferred()
    {
        if (hasDeferred && deferred)
        {
            deferred();
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
    std::function<void()> deferred;

    CHIP_ERROR SyncNode(NodeId nodeId, const ACLEntryType & aclEntry, std::function<void()> onSuccess) override
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
            onSuccess();
        }
        return CHIP_NO_ERROR;
    }

    void RunDeferred()
    {
        if (hasDeferred && deferred)
        {
            deferred();
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

} // namespace
