#include "app/server/JointFabricDatastore.h"
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;

namespace {

class DummyListener : public JointFabricDatastore::Listener
{
public:
    void MarkNodeListChanged() override { mNotified = true; }
    void Reset() { mNotified = false; }

    bool mNotified = false;
};

TEST(JointFabricDatastoreTest, AddPendingNodeNotifiesListener)
{
    JointFabricDatastore store;
    DummyListener listener;

    store.AddListener(listener);

    // Add a pending node — should notify the listener via MarkNodeListChange
    CHIP_ERROR err = store.AddPendingNode(123, CharSpan::fromCharString("controller-a"));
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

    CHIP_ERROR err = store.AddPendingNode(456, CharSpan::fromCharString("controller-b"));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_FALSE(listener.mNotified);
}

class DummyDelegate : public JointFabricDatastore::Delegate
{
public:
    CHIP_ERROR
    SyncNode(NodeId nodeId,
             const Clusters::JointFabricDatastore::Structs::DatastoreEndpointGroupIDEntryStruct::Type & endpointGroupIDEntry,
             std::function<void()> onSuccess) override
    {
        onSuccess();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR
    SyncNode(NodeId nodeId, const Clusters::JointFabricDatastore::Structs::DatastoreNodeKeySetEntryStruct::Type & nodeKeySetEntry,
             std::function<void()> onSuccess) override
    {
        onSuccess();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR
    SyncNode(NodeId nodeId, const Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type & bindingEntry,
             std::function<void()> onSuccess) override
    {
        onSuccess();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR
    SyncNode(NodeId nodeId,
             std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> & bindingEntries,
             std::function<void()> onSuccess) override
    {
        onSuccess();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SyncNode(NodeId nodeId, const Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type & aclEntry,
                        std::function<void()> onSuccess) override
    {
        onSuccess();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR
    SyncNode(NodeId nodeId,
             const std::vector<app::Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type> & aclEntries,
             std::function<void()> onSuccess) override
    {
        onSuccess();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SyncNode(NodeId nodeId,
                        const Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type & groupKeySet,
                        std::function<void()> onSuccess) override
    {
        onSuccess();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FetchEndpointList(
        NodeId nodeId,
        std::function<void(CHIP_ERROR,
                           const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointEntryStruct::Type> &)>
            onSuccess) override
    {
        onSuccess(CHIP_NO_ERROR, {});
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FetchEndpointGroupList(
        NodeId nodeId, EndpointId endpointId,
        std::function<void(
            CHIP_ERROR, const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreGroupInformationEntryStruct::Type> &)>
            onSuccess) override
    {
        onSuccess(CHIP_NO_ERROR, {});
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FetchEndpointBindingList(
        NodeId nodeId, EndpointId endpointId,
        std::function<void(CHIP_ERROR,
                           const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreEndpointBindingEntryStruct::Type> &)>
            onSuccess) override
    {
        onSuccess(CHIP_NO_ERROR, {});
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FetchGroupKeySetList(
        NodeId nodeId,
        std::function<void(CHIP_ERROR,
                           const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type> &)>
            onSuccess) override
    {
        onSuccess(CHIP_NO_ERROR, {});
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FetchACLList(
        NodeId nodeId,
        std::function<void(CHIP_ERROR, const std::vector<Clusters::JointFabricDatastore::Structs::DatastoreACLEntryStruct::Type> &)>
            onSuccess) override
    {
        onSuccess(CHIP_NO_ERROR, {});
        return CHIP_NO_ERROR;
    }
};

TEST(JointFabricDatastoreTest, RefreshNodeUpdatesExistingNode)
{
    JointFabricDatastore store;
    DummyListener listener;
    DummyDelegate delegate;

    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    store.AddListener(listener);

    // Add initial pending node
    err = store.AddPendingNode(123, CharSpan::fromCharString("controller-a"));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(listener.mNotified);

    listener.Reset();

    // Refresh the node
    err = store.RefreshNode(123);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(listener.mNotified);
}

TEST(JointFabricDatastoreTest, RefreshNonExistentNodeFails)
{
    JointFabricDatastore store;
    DummyListener listener;
    DummyDelegate delegate;

    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    store.AddListener(listener);

    // Attempt to refresh a node that doesn't exist
    err = store.RefreshNode(999);
    EXPECT_NE(err, CHIP_NO_ERROR);
    EXPECT_FALSE(listener.mNotified);
}

TEST(JointFabricDatastoreTest, UpdateNodeChangesNameAndNotifies)
{
    JointFabricDatastore store;
    DummyListener listener;
    DummyDelegate delegate;

    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    store.AddListener(listener);

    err = store.AddPendingNode(123, CharSpan::fromCharString("original-name"));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    listener.Reset();

    err = store.UpdateNode(123, CharSpan::fromCharString("updated-name"));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(listener.mNotified);
}

TEST(JointFabricDatastoreTest, RemoveNodeDeletesAndNotifies)
{
    JointFabricDatastore store;
    DummyListener listener;
    DummyDelegate delegate;

    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    store.AddListener(listener);

    err = store.AddPendingNode(123, CharSpan::fromCharString("test-node"));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    listener.Reset();

    err = store.RemoveNode(123);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(listener.mNotified);
}

TEST(JointFabricDatastoreTest, AddGroupKeySetEntry)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type keySet;
    keySet.groupKeySetID = 1;

    err = store.AddGroupKeySetEntry(keySet);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, RemoveGroupKeySetEntry)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type keySet;
    keySet.groupKeySetID = 1;
    err                  = store.AddGroupKeySetEntry(keySet);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = store.RemoveGroupKeySetEntry(1);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, UpdateGroupKeySetEntry)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    GroupId groupId        = 1;
    uint16_t groupKeySetId = 1;
    NodeId nodeId          = 123;
    err                    = store.TestAddNodeKeySetEntry(groupId, groupKeySetId, nodeId);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type keySet;
    keySet.groupKeySetID = 1;
    err                  = store.AddGroupKeySetEntry(keySet);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = store.UpdateGroupKeySetEntry(keySet);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, AddAndRemoveAdmin)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    Clusters::JointFabricDatastore::Structs::DatastoreAdministratorInformationEntryStruct::Type admin;
    admin.nodeID = 100;

    err = store.AddAdmin(admin);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = store.RemoveAdmin(100);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, UpdateAdmin)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    Clusters::JointFabricDatastore::Structs::DatastoreAdministratorInformationEntryStruct::Type admin;
    admin.nodeID = 100;
    err          = store.AddAdmin(admin);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    uint8_t icacData[] = { 0x01, 0x02 };
    err                = store.UpdateAdmin(100, CharSpan::fromCharString("new-name"), ByteSpan(icacData));
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, RemoveGroupIDFromEndpoint)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EndpointId endpointId  = 1;
    GroupId groupId        = 10;
    uint16_t groupKeySetId = 1;
    NodeId nodeId          = 123;

    err = store.TestAddNodeKeySetEntry(groupId, groupKeySetId, nodeId);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = store.TestAddEndpointEntry(endpointId, nodeId, CharSpan::fromCharString("test-endpoint"));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    Clusters::JointFabricDatastore::Commands::AddGroup::DecodableType addGroupData;
    addGroupData.groupID       = groupId;
    addGroupData.friendlyName  = CharSpan::fromCharString("test-group");
    addGroupData.groupKeySetID = groupKeySetId;
    err                        = store.AddGroup(addGroupData);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = store.AddPendingNode(nodeId, CharSpan::fromCharString("test"));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = store.AddGroupIDToEndpointForNode(nodeId, endpointId, groupId);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = store.RemoveGroupIDFromEndpointForNode(nodeId, endpointId, groupId);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, AddGroup)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    Clusters::JointFabricDatastore::Commands::AddGroup::DecodableType commandData;
    // Note: You'll need to populate commandData fields based on the actual struct definition

    err = store.AddGroup(commandData);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, UpdateGroup)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    Clusters::JointFabricDatastore::Commands::AddGroup::DecodableType addData;
    err = store.AddGroup(addData);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    Clusters::JointFabricDatastore::Commands::UpdateGroup::DecodableType updateData;
    err = store.UpdateGroup(updateData);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, RemoveGroup)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    Clusters::JointFabricDatastore::Commands::AddGroup::DecodableType addData;
    err = store.AddGroup(addData);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    Clusters::JointFabricDatastore::Commands::RemoveGroup::DecodableType removeData;
    err = store.RemoveGroup(removeData);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, UpdateEndpointForNode)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    NodeId nodeId         = 123;
    EndpointId endpointId = 1;

    err = store.AddPendingNode(nodeId, CharSpan::fromCharString("test-node"));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = store.TestAddEndpointEntry(endpointId, nodeId, CharSpan::fromCharString("initial-endpoint"));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = store.UpdateEndpointForNode(nodeId, endpointId, CharSpan::fromCharString("endpoint-name"));
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, UpdateEndpointForNonExistentNodeFails)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = store.UpdateEndpointForNode(999, 1, CharSpan::fromCharString("endpoint-name"));
    EXPECT_NE(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, AddBindingToEndpointForNode)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    NodeId nodeId         = 123;
    EndpointId endpointId = 1;

    err = store.AddPendingNode(nodeId, CharSpan::fromCharString("test-node"));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = store.TestAddEndpointEntry(endpointId, nodeId, CharSpan::fromCharString("test-endpoint"));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    Clusters::JointFabricDatastore::Structs::DatastoreBindingTargetStruct::Type binding;
    err = store.AddBindingToEndpointForNode(nodeId, endpointId, binding);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, RemoveBindingFromEndpointForNode)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    NodeId nodeId         = 123;
    EndpointId endpointId = 1;
    uint16_t listId       = 0;

    err = store.AddPendingNode(nodeId, CharSpan::fromCharString("test-node"));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = store.TestAddEndpointEntry(endpointId, nodeId, CharSpan::fromCharString("test-endpoint"));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    Clusters::JointFabricDatastore::Structs::DatastoreBindingTargetStruct::Type binding;
    err = store.AddBindingToEndpointForNode(nodeId, endpointId, binding);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = store.RemoveBindingFromEndpointForNode(listId, nodeId, endpointId);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, RemoveBindingFromNonExistentNodeFails)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = store.RemoveBindingFromEndpointForNode(0, 999, 1);
    EXPECT_NE(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, AddACLToNode)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = store.AddPendingNode(123, CharSpan::fromCharString("test-node"));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::DecodableType aclEntry;
    err = store.AddACLToNode(123, aclEntry);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, AddACLToNonExistentNodeFails)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::DecodableType aclEntry;
    err = store.AddACLToNode(999, aclEntry);
    EXPECT_NE(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, RemoveACLFromNode)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = store.AddPendingNode(123, CharSpan::fromCharString("test-node"));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    Clusters::JointFabricDatastore::Structs::DatastoreAccessControlEntryStruct::DecodableType aclEntry;
    err = store.AddACLToNode(123, aclEntry);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = store.RemoveACLFromNode(0, 123);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(JointFabricDatastoreTest, RemoveACLFromNonExistentNodeFails)
{
    JointFabricDatastore store;
    DummyDelegate delegate;
    CHIP_ERROR err = store.SetDelegate(&delegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = store.RemoveACLFromNode(0, 999);
    EXPECT_NE(err, CHIP_NO_ERROR);
}

} // namespace
