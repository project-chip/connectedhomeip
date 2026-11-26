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
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/Binding/Enums.h>
#include <clusters/Binding/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/GroupId.h>
#include <lib/support/Defer.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

namespace {

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Binding;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;
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

TEST_F(TestBindingCluster, NonListWriteReportsBindingAttributeChange)
{
    TestPersistentStorageDelegate storage;
    Binding::Manager manager;
    manager.GetBindingTable().SetPersistentStorage(&storage);
    BindingCluster cluster({ manager.GetBindingTable(), manager, DeviceLayer::PlatformMgr() }, 1);
    chip::Testing::ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
    auto shutdown = MakeDefer([&] { cluster.Shutdown(app::ClusterShutdownType::kClusterShutdown); });
    const app::ConcreteClusterPath clusterPath(1, Binding::Id);
    const app::ConcreteAttributePath attributePath(1, Binding::Id, Binding::Attributes::Binding::Id);
    const auto version = cluster.GetDataVersion(clusterPath);
    TargetStructType target{ .group = MakeOptional(kMinApplicationGroupId), .fabricIndex = chip::Testing::kTestFabricIndex };
    app::DataModel::List<const TargetStructType> bindings(&target, 1);
    chip::Testing::WriteOperation write(attributePath);
    write.SetSubjectDescriptor(chip::Testing::kAdminSubjectDescriptor);
    auto decoder = write.DecoderFor(bindings);

    ASSERT_TRUE(cluster.WriteAttribute(write.GetRequest(), decoder).IsSuccess());
    EXPECT_EQ(manager.GetBindingTable().Size(), 1u);
    ASSERT_EQ(tester.GetDirtyList().size(), 1u);
    EXPECT_EQ(tester.GetDirtyList()[0], attributePath);
    EXPECT_EQ(cluster.GetDataVersion(clusterPath), version + 1);
}

TEST_F(TestBindingCluster, CompletedListWritesReportBindingChangesIncludingClear)
{
    for (const auto pattern :
         { chip::Testing::ListWritingPattern::ReplaceAll, chip::Testing::ListWritingPattern::ClearAllThenAppendItems })
    {
        TestPersistentStorageDelegate storage;
        Binding::Manager manager;
        manager.GetBindingTable().SetPersistentStorage(&storage);
        BindingCluster cluster({ manager.GetBindingTable(), manager, DeviceLayer::PlatformMgr() }, 1);
        chip::Testing::ClusterTester tester(cluster);
        ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
        auto shutdown = MakeDefer([&] { cluster.Shutdown(app::ClusterShutdownType::kClusterShutdown); });
        const app::ConcreteClusterPath clusterPath(1, Binding::Id);
        const app::ConcreteAttributePath attributePath(1, Binding::Id, Binding::Attributes::Binding::Id);
        TargetStructType target{ .group = MakeOptional(kMinApplicationGroupId), .fabricIndex = chip::Testing::kTestFabricIndex };
        app::DataModel::List<const TargetStructType> bindings(&target, 1);

        for (bool clear : { false, true })
        {
            tester.GetDirtyList().clear();
            const auto version = cluster.GetDataVersion(clusterPath);
            if (clear)
            {
                bindings = app::DataModel::List<const TargetStructType>();
            }
            cluster.ListAttributeWriteNotification(attributePath, app::DataModel::ListWriteOperation::kListWriteBegin,
                                                   chip::Testing::kTestFabricIndex);
            ASSERT_TRUE(tester.WriteAttribute(Binding::Attributes::Binding::Id, bindings, pattern).IsSuccess());
            EXPECT_EQ(manager.GetBindingTable().Size(), clear ? 0u : 1u);
            EXPECT_TRUE(tester.GetDirtyList().empty());
            EXPECT_EQ(cluster.GetDataVersion(clusterPath), version);

            cluster.ListAttributeWriteNotification(attributePath, app::DataModel::ListWriteOperation::kListWriteSuccess,
                                                   chip::Testing::kTestFabricIndex);
            ASSERT_EQ(tester.GetDirtyList().size(), 1u);
            EXPECT_EQ(tester.GetDirtyList()[0], attributePath);
            EXPECT_EQ(cluster.GetDataVersion(clusterPath), version + 1);
        }
    }
}

TEST_F(TestBindingCluster, UnchangedWritesDoNotReportOrIncrementDataVersion)
{
    using Pattern                           = chip::Testing::ListWritingPattern;
    const std::optional<Pattern> patterns[] = { std::nullopt, Pattern::ReplaceAll, Pattern::ClearAllThenAppendItems };
    for (const auto pattern : patterns)
    {
        for (bool empty : { true, false })
        {
            TestPersistentStorageDelegate storage;
            Binding::Manager manager;
            auto & table = manager.GetBindingTable();
            table.SetPersistentStorage(&storage);
            BindingCluster cluster({ table, manager, DeviceLayer::PlatformMgr() }, 1);
            chip::Testing::ClusterTester tester(cluster);
            ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
            auto shutdown = MakeDefer([&] { cluster.Shutdown(app::ClusterShutdownType::kClusterShutdown); });
            const app::ConcreteClusterPath clusterPath(1, Binding::Id);
            const app::ConcreteAttributePath attributePath(1, Binding::Id, Binding::Attributes::Binding::Id);
            constexpr auto fabric    = chip::Testing::kTestFabricIndex;
            const auto otherEndpoint = TableEntry::ForGroup(fabric, 123, 2, std::nullopt);
            const auto otherFabric   = TableEntry::ForGroup(static_cast<FabricIndex>(fabric + 1), 456, 1, std::nullopt);
            ASSERT_SUCCESS(table.Add(otherEndpoint));
            ASSERT_SUCCESS(table.Add(otherFabric));
            TargetStructType targets[] = {
                { .group = MakeOptional(GroupId{ 1 }), .cluster = MakeOptional(ClusterId{ 6 }), .fabricIndex = fabric },
                { .group = MakeOptional(GroupId{ 2 }), .fabricIndex = fabric },
            };
            if (!empty)
            {
                ASSERT_SUCCESS(table.Add(TableEntry::ForGroup(fabric, 1, 1, 6)));
                ASSERT_SUCCESS(table.Add(TableEntry::ForGroup(fabric, 2, 1, std::nullopt)));
            }
            const app::DataModel::List<const TargetStructType> bindings(targets, empty ? 0u : 2u);
            const auto version = cluster.GetDataVersion(clusterPath);
            if (pattern.has_value())
            {
                cluster.ListAttributeWriteNotification(attributePath, app::DataModel::ListWriteOperation::kListWriteBegin, fabric);
                ASSERT_TRUE(tester.WriteAttribute(Binding::Attributes::Binding::Id, bindings, *pattern).IsSuccess());
                cluster.ListAttributeWriteNotification(attributePath, app::DataModel::ListWriteOperation::kListWriteSuccess,
                                                       fabric);
            }
            else
            {
                chip::Testing::WriteOperation write(attributePath);
                write.SetSubjectDescriptor(chip::Testing::kAdminSubjectDescriptor);
                auto decoder = write.DecoderFor(bindings);
                ASSERT_TRUE(cluster.WriteAttribute(write.GetRequest(), decoder).IsSuccess());
            }
            EXPECT_TRUE(tester.GetDirtyList().empty());
            EXPECT_EQ(cluster.GetDataVersion(clusterPath), version);
            EXPECT_EQ(table.Size(), empty ? 2u : 4u);
            EXPECT_EQ(*table.begin(), otherEndpoint);
            EXPECT_EQ(*(++table.begin()), otherFabric);
        }
    }
}

TEST_F(TestBindingCluster, ReorderedBindingsReportChange)
{
    TestPersistentStorageDelegate storage;
    Binding::Manager manager;
    auto & table = manager.GetBindingTable();
    table.SetPersistentStorage(&storage);
    constexpr auto fabric = chip::Testing::kTestFabricIndex;
    ASSERT_SUCCESS(table.Add(TableEntry::ForGroup(fabric, 1, 1, std::nullopt)));
    ASSERT_SUCCESS(table.Add(TableEntry::ForGroup(fabric, 2, 1, std::nullopt)));
    BindingCluster cluster({ table, manager, DeviceLayer::PlatformMgr() }, 1);
    chip::Testing::ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
    auto shutdown = MakeDefer([&] { cluster.Shutdown(app::ClusterShutdownType::kClusterShutdown); });
    const app::ConcreteClusterPath clusterPath(1, Binding::Id);
    const app::ConcreteAttributePath attributePath(1, Binding::Id, Binding::Attributes::Binding::Id);
    const auto version         = cluster.GetDataVersion(clusterPath);
    TargetStructType targets[] = {
        { .group = MakeOptional(GroupId{ 2 }), .fabricIndex = fabric },
        { .group = MakeOptional(GroupId{ 1 }), .fabricIndex = fabric },
    };
    const app::DataModel::List<const TargetStructType> bindings(targets);
    cluster.ListAttributeWriteNotification(attributePath, app::DataModel::ListWriteOperation::kListWriteBegin, fabric);
    ASSERT_TRUE(tester.WriteAttribute(Binding::Attributes::Binding::Id, bindings, chip::Testing::ListWritingPattern::ReplaceAll)
                    .IsSuccess());
    cluster.ListAttributeWriteNotification(attributePath, app::DataModel::ListWriteOperation::kListWriteSuccess, fabric);
    EXPECT_TRUE(tester.IsAttributeDirty(Binding::Attributes::Binding::Id));
    EXPECT_EQ(cluster.GetDataVersion(clusterPath), version + 1);
    EXPECT_EQ(table.begin()->groupId, 2);
}

TEST_F(TestBindingCluster, FailedListWriteReportsPartialChangesAndReleasesSnapshot)
{
    TestPersistentStorageDelegate storage;
    Binding::Manager manager;
    auto & table = manager.GetBindingTable();
    table.SetPersistentStorage(&storage);
    constexpr auto fabric = chip::Testing::kTestFabricIndex;
    ASSERT_SUCCESS(table.Add(TableEntry::ForGroup(fabric, 1, 1, std::nullopt)));
    BindingCluster cluster({ table, manager, DeviceLayer::PlatformMgr() }, 1);
    chip::Testing::ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
    auto shutdown = MakeDefer([&] { cluster.Shutdown(app::ClusterShutdownType::kClusterShutdown); });
    const app::ConcreteAttributePath attributePath(1, Binding::Id, Binding::Attributes::Binding::Id);
    TargetStructType invalid{ .group = MakeOptional(kUndefinedGroupId), .fabricIndex = fabric };
    const app::DataModel::List<const TargetStructType> bindings(&invalid, 1);
    cluster.ListAttributeWriteNotification(attributePath, app::DataModel::ListWriteOperation::kListWriteBegin, fabric);
    // Clearing succeeds before the reserved group ID is rejected, leaving an observable empty list.
    EXPECT_EQ(tester.WriteAttribute(Binding::Attributes::Binding::Id, bindings,
                                    chip::Testing::ListWritingPattern::ClearAllThenAppendItems),
              Protocols::InteractionModel::Status::ConstraintError);
    cluster.ListAttributeWriteNotification(attributePath, app::DataModel::ListWriteOperation::kListWriteFailure, fabric);
    EXPECT_TRUE(tester.IsAttributeDirty(Binding::Attributes::Binding::Id));
    EXPECT_EQ(table.Size(), 0u);
    tester.GetDirtyList().clear();
    cluster.ListAttributeWriteNotification(attributePath, app::DataModel::ListWriteOperation::kListWriteBegin, fabric);
    ASSERT_TRUE(tester
                    .WriteAttribute(Binding::Attributes::Binding::Id, app::DataModel::List<const TargetStructType>(),
                                    chip::Testing::ListWritingPattern::ReplaceAll)
                    .IsSuccess());
    cluster.ListAttributeWriteNotification(attributePath, app::DataModel::ListWriteOperation::kListWriteSuccess, fabric);
    EXPECT_TRUE(tester.GetDirtyList().empty());
}

TEST_F(TestBindingCluster, RejectedReplacementDoesNotReportAttributeChange)
{
    TestPersistentStorageDelegate storage;
    Binding::Manager manager;
    manager.GetBindingTable().SetPersistentStorage(&storage);
    BindingCluster cluster({ manager.GetBindingTable(), manager, DeviceLayer::PlatformMgr() }, 1);
    chip::Testing::ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
    auto shutdown = MakeDefer([&] { cluster.Shutdown(app::ClusterShutdownType::kClusterShutdown); });
    const app::ConcreteClusterPath clusterPath(1, Binding::Id);
    const auto version = cluster.GetDataVersion(clusterPath);
    TargetStructType target{ .group = MakeOptional(kUndefinedGroupId), .fabricIndex = chip::Testing::kTestFabricIndex };
    app::DataModel::List<const TargetStructType> bindings(&target, 1);

    EXPECT_EQ(tester.WriteAttribute(Binding::Attributes::Binding::Id, bindings, chip::Testing::ListWritingPattern::ReplaceAll),
              Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_TRUE(tester.GetDirtyList().empty());
    EXPECT_EQ(cluster.GetDataVersion(clusterPath), version);
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

        EXPECT_EQ(tester.WriteAttribute(Binding::Attributes::Binding::Id, bindings, pattern),
                  Protocols::InteractionModel::Status::ConstraintError);
        EXPECT_EQ(manager.GetBindingTable().Size(), 0u);

        target.group = MakeOptional(kMinApplicationGroupId);
        EXPECT_EQ(tester.WriteAttribute(Binding::Attributes::Binding::Id, bindings, pattern),
                  Protocols::InteractionModel::Status::Success);
        ASSERT_EQ(manager.GetBindingTable().Size(), 1u);
        EXPECT_EQ(manager.GetBindingTable().begin()->groupId, kMinApplicationGroupId);
    }
}

} // namespace
