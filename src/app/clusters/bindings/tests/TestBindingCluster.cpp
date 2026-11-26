/*
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
#include <pw_unit_test/framework.h>

#include <app/clusters/bindings/BindingCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/Binding/Enums.h>
#include <clusters/Binding/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/GroupId.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

namespace {

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Binding;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;
using chip::Protocols::InteractionModel::Status;
using chip::Testing::IsAttributesListEqualTo;

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestBindingCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

BindingCluster::Context CreateStandardContext()
{
    return BindingCluster::Context{
        .bindingTable    = Binding::Table::GetInstance(),
        .bindingManager  = Binding::Manager::GetInstance(),
        .platformManager = chip::DeviceLayer::PlatformMgr(),
    };
}

TEST_F(TestBindingCluster, TestAttributes)
{
    BindingCluster cluster(CreateStandardContext(), 1);

    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Binding::Attributes::Binding::kMetadataEntry,
                                        }));
}

TEST_F(TestBindingCluster, RejectsReservedGroupId)
{
    for (const auto pattern :
         { chip::Testing::ListWritingPattern::ReplaceAll, chip::Testing::ListWritingPattern::ClearAllThenAppendItems })
    {
        TestPersistentStorageDelegate storage;
        Binding::Manager manager;
        manager.GetBindingTable().SetPersistentStorage(&storage);
        BindingCluster cluster(BindingCluster::Context{ .bindingTable    = manager.GetBindingTable(),
                                                        .bindingManager  = manager,
                                                        .platformManager = DeviceLayer::PlatformMgr() },
                               1);
        chip::Testing::ClusterTester tester(cluster);

        TargetStructType target{ .group = MakeOptional(kUndefinedGroupId), .fabricIndex = chip::Testing::kTestFabricIndex };
        app::DataModel::List<const TargetStructType> bindings(&target, 1);

        EXPECT_EQ(tester.WriteAttribute(Binding::Attributes::Binding::Id, bindings, pattern), Status::ConstraintError);
        EXPECT_EQ(manager.GetBindingTable().Size(), 0u);

        target.group = MakeOptional(kMinApplicationGroupId);
        EXPECT_EQ(tester.WriteAttribute(Binding::Attributes::Binding::Id, bindings, pattern), Status::Success);
        ASSERT_EQ(manager.GetBindingTable().Size(), 1u);
        EXPECT_EQ(manager.GetBindingTable().begin()->groupId, kMinApplicationGroupId);
    }
}

TEST_F(TestBindingCluster, NotifiesAttributeChangedOnWrite)
{
    constexpr EndpointId kEndpoint = 1;
    const app::ConcreteClusterPath clusterPath(kEndpoint, Binding::Id);
    const app::ConcreteAttributePath bindingAttrPath(kEndpoint, Binding::Id, Binding::Attributes::Binding::Id);

    TestPersistentStorageDelegate storage;
    Binding::Manager manager;
    manager.GetBindingTable().SetPersistentStorage(&storage);
    BindingCluster cluster(BindingCluster::Context{ .bindingTable    = manager.GetBindingTable(),
                                                    .bindingManager  = manager,
                                                    .platformManager = DeviceLayer::PlatformMgr() },
                           kEndpoint);
    chip::Testing::ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

    // Failed ReplaceAll must not mark attribute dirty or advance DataVersion.
    {
        tester.GetDirtyList().clear();
        const DataVersion versionBefore = cluster.GetDataVersion(clusterPath);

        TargetStructType invalid{ .group = MakeOptional(kUndefinedGroupId), .fabricIndex = chip::Testing::kTestFabricIndex };
        app::DataModel::List<const TargetStructType> bindings(&invalid, 1);

        EXPECT_EQ(tester.WriteAttribute(Binding::Attributes::Binding::Id, bindings, chip::Testing::ListWritingPattern::ReplaceAll),
                  Status::ConstraintError);
        EXPECT_TRUE(tester.GetDirtyList().empty());
        EXPECT_EQ(cluster.GetDataVersion(clusterPath), versionBefore);
    }

    // Successful ReplaceAll and ClearAllThenAppendItems must mark attribute dirty and advance DataVersion.
    for (const auto pattern :
         { chip::Testing::ListWritingPattern::ReplaceAll, chip::Testing::ListWritingPattern::ClearAllThenAppendItems })
    {
        tester.GetDirtyList().clear();
        const DataVersion versionBefore = cluster.GetDataVersion(clusterPath);

        TargetStructType valid{ .group = MakeOptional(kMinApplicationGroupId), .fabricIndex = chip::Testing::kTestFabricIndex };
        app::DataModel::List<const TargetStructType> bindings(&valid, 1);

        EXPECT_EQ(tester.WriteAttribute(Binding::Attributes::Binding::Id, bindings, pattern), Status::Success);
        EXPECT_TRUE(tester.IsAttributeDirty(Binding::Attributes::Binding::Id));
        EXPECT_NE(cluster.GetDataVersion(clusterPath), versionBefore);
    }

    // Standalone valid AppendItem must mark attribute dirty and advance DataVersion.
    {
        tester.GetDirtyList().clear();
        const DataVersion versionBefore = cluster.GetDataVersion(clusterPath);

        chip::Testing::WriteOperation writeOp(bindingAttrPath);
        const Access::SubjectDescriptor subjectDescriptor = tester.GetCommandHandler().GetSubjectDescriptor();
        writeOp.SetSubjectDescriptor(subjectDescriptor);
        writeOp.SetListOperation(app::ConcreteDataAttributePath::ListOperation::AppendItem);
        TargetStructType valid{ .group       = MakeOptional(static_cast<GroupId>(kMinApplicationGroupId + 1)),
                                .fabricIndex = chip::Testing::kTestFabricIndex };
        auto decoder = writeOp.DecoderFor(valid);

        EXPECT_EQ(cluster.WriteAttribute(writeOp.GetRequest(), decoder), Status::Success);
        EXPECT_TRUE(tester.IsAttributeDirty(Binding::Attributes::Binding::Id));
        EXPECT_NE(cluster.GetDataVersion(clusterPath), versionBefore);
    }

    // Failed AppendItem must not mark attribute dirty or advance DataVersion.
    {
        tester.GetDirtyList().clear();
        const DataVersion versionBefore = cluster.GetDataVersion(clusterPath);

        chip::Testing::WriteOperation writeOp(bindingAttrPath);
        const Access::SubjectDescriptor subjectDescriptor = tester.GetCommandHandler().GetSubjectDescriptor();
        writeOp.SetSubjectDescriptor(subjectDescriptor);
        writeOp.SetListOperation(app::ConcreteDataAttributePath::ListOperation::AppendItem);
        TargetStructType invalid{ .group = MakeOptional(kUndefinedGroupId), .fabricIndex = chip::Testing::kTestFabricIndex };
        auto decoder = writeOp.DecoderFor(invalid);

        EXPECT_EQ(cluster.WriteAttribute(writeOp.GetRequest(), decoder), Status::ConstraintError);
        EXPECT_TRUE(tester.GetDirtyList().empty());
        EXPECT_EQ(cluster.GetDataVersion(clusterPath), versionBefore);
    }

    // Clearing the list must mark attribute dirty and advance DataVersion.
    {
        tester.GetDirtyList().clear();
        const DataVersion versionBefore = cluster.GetDataVersion(clusterPath);

        app::DataModel::List<const TargetStructType> empty;
        EXPECT_EQ(tester.WriteAttribute(Binding::Attributes::Binding::Id, empty, chip::Testing::ListWritingPattern::ReplaceAll),
                  Status::Success);
        EXPECT_TRUE(tester.IsAttributeDirty(Binding::Attributes::Binding::Id));
        EXPECT_NE(cluster.GetDataVersion(clusterPath), versionBefore);
    }

    // Replacing an empty list with an empty list must not mark attribute dirty or advance DataVersion.
    {
        ASSERT_EQ(manager.GetBindingTable().Size(), 0u);
        tester.GetDirtyList().clear();
        const DataVersion versionBefore = cluster.GetDataVersion(clusterPath);

        app::DataModel::List<const TargetStructType> empty;
        EXPECT_EQ(tester.WriteAttribute(Binding::Attributes::Binding::Id, empty, chip::Testing::ListWritingPattern::ReplaceAll),
                  Status::Success);
        EXPECT_TRUE(tester.GetDirtyList().empty());
        EXPECT_EQ(cluster.GetDataVersion(clusterPath), versionBefore);
    }

    // Storage failure before any table mutation must not mark attribute dirty or advance DataVersion.
    {
        TargetStructType initial{ .group = MakeOptional(kMinApplicationGroupId), .fabricIndex = chip::Testing::kTestFabricIndex };
        app::DataModel::List<const TargetStructType> initialList(&initial, 1);
        ASSERT_EQ(
            tester.WriteAttribute(Binding::Attributes::Binding::Id, initialList, chip::Testing::ListWritingPattern::ReplaceAll),
            Status::Success);
        ASSERT_EQ(manager.GetBindingTable().Size(), 1u);

        tester.GetDirtyList().clear();
        const DataVersion versionBefore = cluster.GetDataVersion(clusterPath);
        storage.SetRejectWrites(true);

        app::DataModel::List<const TargetStructType> empty;
        EXPECT_FALSE(tester.WriteAttribute(Binding::Attributes::Binding::Id, empty, chip::Testing::ListWritingPattern::ReplaceAll)
                         .IsSuccess());
        storage.SetRejectWrites(false);

        EXPECT_EQ(manager.GetBindingTable().Size(), 1u);
        EXPECT_TRUE(tester.GetDirtyList().empty());
        EXPECT_EQ(cluster.GetDataVersion(clusterPath), versionBefore);
    }

    // Mid-write storage failure after removing an existing entry must still mark attribute dirty and advance DataVersion.
    {
        ASSERT_EQ(manager.GetBindingTable().Size(), 1u);
        tester.GetDirtyList().clear();
        const DataVersion versionBefore = cluster.GetDataVersion(clusterPath);

        // Allow RemoveAt to delete entry 0, then fail SaveEntryToStorage when writing the replacement entry 0.
        storage.AddPoisonKey(DefaultStorageKeyAllocator::BindingTableEntry(0).KeyName(), -1, 0b1);

        TargetStructType replacement{ .group       = MakeOptional(static_cast<GroupId>(kMinApplicationGroupId + 1)),
                                      .fabricIndex = chip::Testing::kTestFabricIndex };
        app::DataModel::List<const TargetStructType> replacementList(&replacement, 1);
        EXPECT_FALSE(
            tester.WriteAttribute(Binding::Attributes::Binding::Id, replacementList, chip::Testing::ListWritingPattern::ReplaceAll)
                .IsSuccess());
        storage.ClearPoisonKeys();

        EXPECT_EQ(manager.GetBindingTable().Size(), 0u);
        EXPECT_TRUE(tester.IsAttributeDirty(Binding::Attributes::Binding::Id));
        EXPECT_NE(cluster.GetDataVersion(clusterPath), versionBefore);
    }

    cluster.Shutdown(app::ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestBindingCluster, NotifiesAttributeChangedOnExternalMutations)
{
    constexpr EndpointId kEndpoint1 = 1;
    constexpr EndpointId kEndpoint2 = 2;
    const app::ConcreteClusterPath clusterPath1(kEndpoint1, Binding::Id);
    const app::ConcreteClusterPath clusterPath2(kEndpoint2, Binding::Id);

    TestPersistentStorageDelegate storage;
    Binding::Manager manager;
    Binding::Table & table = manager.GetBindingTable();
    table.SetPersistentStorage(&storage);

    BindingCluster cluster1(
        BindingCluster::Context{ .bindingTable = table, .bindingManager = manager, .platformManager = DeviceLayer::PlatformMgr() },
        kEndpoint1);
    BindingCluster cluster2(
        BindingCluster::Context{ .bindingTable = table, .bindingManager = manager, .platformManager = DeviceLayer::PlatformMgr() },
        kEndpoint2);

    chip::Testing::ClusterTester tester1(cluster1);
    chip::Testing::ClusterTester tester2(cluster2);
    ASSERT_SUCCESS(cluster1.Startup(tester1.GetServerClusterContext()));
    ASSERT_SUCCESS(cluster2.Startup(tester2.GetServerClusterContext()));

    // Binding::Manager::AddBindingEntry notifies only the matching endpoint.
    {
        tester1.GetDirtyList().clear();
        tester2.GetDirtyList().clear();
        const DataVersion version1Before = cluster1.GetDataVersion(clusterPath1);
        const DataVersion version2Before = cluster2.GetDataVersion(clusterPath2);

        ASSERT_SUCCESS(manager.AddBindingEntry(
            Binding::TableEntry::ForGroup(chip::Testing::kTestFabricIndex, kMinApplicationGroupId, kEndpoint1, std::nullopt)));

        EXPECT_TRUE(tester1.IsAttributeDirty(Binding::Attributes::Binding::Id));
        EXPECT_NE(cluster1.GetDataVersion(clusterPath1), version1Before);
        EXPECT_TRUE(tester2.GetDirtyList().empty());
        EXPECT_EQ(cluster2.GetDataVersion(clusterPath2), version2Before);
    }

    // Direct Binding::Table::Add notifies only the matching endpoint.
    {
        tester1.GetDirtyList().clear();
        tester2.GetDirtyList().clear();
        const DataVersion version1Before = cluster1.GetDataVersion(clusterPath1);
        const DataVersion version2Before = cluster2.GetDataVersion(clusterPath2);

        ASSERT_SUCCESS(table.Add(Binding::TableEntry::ForGroup(
            chip::Testing::kTestFabricIndex, static_cast<GroupId>(kMinApplicationGroupId + 1), kEndpoint2, std::nullopt)));

        EXPECT_TRUE(tester1.GetDirtyList().empty());
        EXPECT_EQ(cluster1.GetDataVersion(clusterPath1), version1Before);
        EXPECT_TRUE(tester2.IsAttributeDirty(Binding::Attributes::Binding::Id));
        EXPECT_NE(cluster2.GetDataVersion(clusterPath2), version2Before);
    }

    // Direct Binding::Table::RemoveAt notifies only the endpoint of the removed entry.
    {
        tester1.GetDirtyList().clear();
        tester2.GetDirtyList().clear();
        const DataVersion version1Before = cluster1.GetDataVersion(clusterPath1);
        const DataVersion version2Before = cluster2.GetDataVersion(clusterPath2);

        auto iter = table.begin();
        ASSERT_NE(iter, table.end());
        ASSERT_EQ(iter->local, kEndpoint1);
        ASSERT_SUCCESS(table.RemoveAt(iter));

        EXPECT_TRUE(tester1.IsAttributeDirty(Binding::Attributes::Binding::Id));
        EXPECT_NE(cluster1.GetDataVersion(clusterPath1), version1Before);
        EXPECT_TRUE(tester2.GetDirtyList().empty());
        EXPECT_EQ(cluster2.GetDataVersion(clusterPath2), version2Before);
    }

    // Cluster WriteAttribute on endpoint 1 does not notify endpoint 2.
    {
        tester1.GetDirtyList().clear();
        tester2.GetDirtyList().clear();
        const DataVersion version1Before = cluster1.GetDataVersion(clusterPath1);
        const DataVersion version2Before = cluster2.GetDataVersion(clusterPath2);

        TargetStructType valid{ .group = MakeOptional(kMinApplicationGroupId), .fabricIndex = chip::Testing::kTestFabricIndex };
        app::DataModel::List<const TargetStructType> bindings(&valid, 1);
        EXPECT_EQ(tester1.WriteAttribute(Binding::Attributes::Binding::Id, bindings, chip::Testing::ListWritingPattern::ReplaceAll),
                  Status::Success);

        EXPECT_TRUE(tester1.IsAttributeDirty(Binding::Attributes::Binding::Id));
        EXPECT_NE(cluster1.GetDataVersion(clusterPath1), version1Before);
        EXPECT_TRUE(tester2.GetDirtyList().empty());
        EXPECT_EQ(cluster2.GetDataVersion(clusterPath2), version2Before);
    }

    // Shutdown unregisters the listener; a subsequent Startup re-registers it.
    cluster1.Shutdown(app::ClusterShutdownType::kClusterShutdown);
    {
        tester1.GetDirtyList().clear();
        const DataVersion version1Before = cluster1.GetDataVersion(clusterPath1);

        ASSERT_SUCCESS(table.Add(Binding::TableEntry::ForGroup(
            chip::Testing::kTestFabricIndex, static_cast<GroupId>(kMinApplicationGroupId + 2), kEndpoint1, std::nullopt)));

        EXPECT_TRUE(tester1.GetDirtyList().empty());
        EXPECT_EQ(cluster1.GetDataVersion(clusterPath1), version1Before);
    }

    ASSERT_SUCCESS(cluster1.Startup(tester1.GetServerClusterContext()));
    {
        tester1.GetDirtyList().clear();
        const DataVersion version1Before = cluster1.GetDataVersion(clusterPath1);

        ASSERT_SUCCESS(table.Add(Binding::TableEntry::ForGroup(
            chip::Testing::kTestFabricIndex, static_cast<GroupId>(kMinApplicationGroupId + 3), kEndpoint1, std::nullopt)));

        EXPECT_TRUE(tester1.IsAttributeDirty(Binding::Attributes::Binding::Id));
        EXPECT_NE(cluster1.GetDataVersion(clusterPath1), version1Before);
    }

    cluster1.Shutdown(app::ClusterShutdownType::kClusterShutdown);
    cluster2.Shutdown(app::ClusterShutdownType::kClusterShutdown);
}

} // namespace
