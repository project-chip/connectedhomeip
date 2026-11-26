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
#include <app/data-model-provider/AttributeChangeListener.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/data-model/Decode.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/Binding/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/GroupId.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/Defer.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <protocols/interaction_model/StatusCode.h>

#include <optional>
#include <vector>

namespace {

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Binding;

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

void ExpectBindingRead(BindingCluster & cluster, FabricIndex fabric, bool filtered, DataVersion expectedVersion,
                       const std::vector<TableEntry> & expected)
{
    chip::Testing::ReadOperation read(1, Binding::Id, Binding::Attributes::Binding::Id);
    auto subject        = chip::Testing::kAdminSubjectDescriptor;
    subject.fabricIndex = fabric;
    read.SetSubjectDescriptor(subject);
    const app::ConcreteClusterPath path(1, Binding::Id);
    chip::Testing::ReadOperation::EncodingParams params;
    params.SetDataVersion(cluster.GetDataVersion(path)).SetIsFabricFiltered(filtered);
    auto encoder = read.StartEncoding(params);
    ASSERT_NE(encoder, nullptr);
    ASSERT_TRUE(cluster.ReadAttribute(read.GetRequest(), *encoder).IsSuccess());
    ASSERT_SUCCESS(read.FinishEncoding());
    std::vector<chip::Testing::DecodedAttributeData> reports;
    ASSERT_SUCCESS(read.GetEncodedIBs().Decode(reports));
    ASSERT_EQ(reports.size(), 1u);
    EXPECT_EQ(reports[0].dataVersion, expectedVersion);
    DecodableBindingListType bindings;
    ASSERT_SUCCESS(app::DataModel::Decode(reports[0].dataReader, bindings));
    std::vector<TableEntry> actual;
    auto it = bindings.begin();
    while (it.Next())
    {
        const auto & binding = it.GetValue();
        if (binding.group.HasValue())
        {
            actual.push_back(TableEntry::ForGroup(binding.fabricIndex, binding.group.Value(), 1, binding.cluster.std_optional()));
        }
        else
        {
            ASSERT_TRUE(binding.node.HasValue());
            ASSERT_TRUE(binding.endpoint.HasValue());
            actual.push_back(TableEntry::ForNode(binding.fabricIndex, binding.node.Value(), 1, binding.endpoint.Value(),
                                                 binding.cluster.std_optional()));
        }
    }
    ASSERT_SUCCESS(it.GetStatus());
    EXPECT_EQ(actual, expected);
}

class ReadBindingOnChange : public app::DataModel::AttributeChangeListener
{
public:
    ReadBindingOnChange(BindingCluster & cluster, FabricIndex fabric, DataVersion version,
                        const std::vector<TableEntry> & expected) :
        mCluster(cluster),
        mFabric(fabric), mVersion(version), mExpected(expected)
    {}
    void OnAttributeChanged(const app::ConcreteAttributePath &, app::DataModel::AttributeChangeType) override
    {
        called = true;
        ExpectBindingRead(mCluster, mFabric, true, mVersion, mExpected);
    }
    bool called = false;

private:
    BindingCluster & mCluster;
    FabricIndex mFabric;
    DataVersion mVersion;
    const std::vector<TableEntry> & mExpected;
};

TEST_F(TestBindingCluster, ReadsBetweenChunksKeepOriginalValueAndVersionUntilCompletion)
{
    enum class Completion
    {
        kUnchanged,
        kChanged,
        kFailure
    };
    for (auto completion : { Completion::kUnchanged, Completion::kChanged, Completion::kFailure })
    {
        for (bool initiallyEmpty : { false, true })
        {
            TestPersistentStorageDelegate storage;
            Binding::Manager manager;
            auto & table = manager.GetBindingTable();
            table.SetPersistentStorage(&storage);
            constexpr auto fabric      = chip::Testing::kTestFabricIndex;
            constexpr auto otherFabric = static_cast<FabricIndex>(fabric + 1);
            std::vector<TableEntry> original;
            const auto group = TableEntry::ForGroup(fabric, 1, 1, std::nullopt);
            const auto node  = TableEntry::ForNode(fabric, 123, 1, 42, std::nullopt);
            const auto other = TableEntry::ForGroup(otherFabric, 9, 1, std::nullopt);
            if (!initiallyEmpty)
            {
                ASSERT_SUCCESS(table.Add(group));
                original.push_back(group);
                original.push_back(node);
            }
            ASSERT_SUCCESS(table.Add(other));
            if (!initiallyEmpty)
            {
                ASSERT_SUCCESS(table.Add(node));
            }
            ASSERT_SUCCESS(table.Add(TableEntry::ForGroup(fabric, 88, 2, std::nullopt)));
            BindingCluster cluster({ table, manager, DeviceLayer::PlatformMgr() }, 1);
            chip::Testing::ClusterTester tester(cluster);
            ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
            auto shutdown = MakeDefer([&] { cluster.Shutdown(app::ClusterShutdownType::kClusterShutdown); });
            const app::ConcreteClusterPath clusterPath(1, Binding::Id);
            const app::ConcreteAttributePath attributePath(1, Binding::Id, Binding::Attributes::Binding::Id);
            const auto version  = cluster.GetDataVersion(clusterPath);
            auto expectOriginal = [&] {
                ExpectBindingRead(cluster, fabric, true, version, original);
                ExpectBindingRead(cluster, otherFabric, true, version, { other });
                auto unfiltered = original;
                unfiltered.push_back(other);
                ExpectBindingRead(cluster, fabric, false, version, unfiltered);
            };
            expectOriginal();
            cluster.ListAttributeWriteNotification(attributePath, app::DataModel::ListWriteOperation::kListWriteBegin, fabric);
            chip::Testing::WriteOperation write(attributePath);
            write.SetSubjectDescriptor(chip::Testing::kAdminSubjectDescriptor);
            write.SetListOperation(app::ConcreteDataAttributePath::ListOperation::ReplaceAll);
            auto clear = write.DecoderFor(app::DataModel::List<const TargetStructType>());
            ASSERT_TRUE(cluster.WriteAttribute(write.GetRequest(), clear).IsSuccess());
            expectOriginal();
            write.SetListOperation(app::ConcreteDataAttributePath::ListOperation::AppendItem);
            std::vector<TableEntry> finalBindings;
            if (completion == Completion::kUnchanged)
            {
                if (!initiallyEmpty)
                {
                    TargetStructType target{ .group = MakeOptional(GroupId{ 1 }), .fabricIndex = fabric };
                    auto appendGroup = write.DecoderFor(target);
                    ASSERT_TRUE(cluster.WriteAttribute(write.GetRequest(), appendGroup).IsSuccess());
                    expectOriginal();
                    target          = TargetStructType{ .node        = MakeOptional(NodeId{ 123 }),
                                                        .endpoint    = MakeOptional(EndpointId{ 42 }),
                                                        .fabricIndex = fabric };
                    auto appendNode = write.DecoderFor(target);
                    ASSERT_TRUE(cluster.WriteAttribute(write.GetRequest(), appendNode).IsSuccess());
                    finalBindings = original;
                }
            }
            else
            {
                TargetStructType target{ .group = MakeOptional(GroupId{ 3 }), .fabricIndex = fabric };
                auto append = write.DecoderFor(target);
                ASSERT_TRUE(cluster.WriteAttribute(write.GetRequest(), append).IsSuccess());
                finalBindings.push_back(TableEntry::ForGroup(fabric, 3, 1, std::nullopt));
                if (completion == Completion::kFailure)
                {
                    target.group.SetValue(kUndefinedGroupId);
                    auto invalid = write.DecoderFor(target);
                    EXPECT_EQ(cluster.WriteAttribute(write.GetRequest(), invalid),
                              Protocols::InteractionModel::Status::ConstraintError);
                }
            }
            expectOriginal();
            const bool changed      = completion != Completion::kUnchanged;
            const auto finalVersion = version + (changed ? 1u : 0u);
            ReadBindingOnChange listener(cluster, fabric, finalVersion, finalBindings);
            auto & provider = tester.GetServerClusterContext().provider;
            provider.RegisterAttributeChangeListener(listener);
            auto unregister = MakeDefer([&] { provider.UnregisterAttributeChangeListener(listener); });
            cluster.ListAttributeWriteNotification(attributePath,
                                                   completion == Completion::kFailure
                                                       ? app::DataModel::ListWriteOperation::kListWriteFailure
                                                       : app::DataModel::ListWriteOperation::kListWriteSuccess,
                                                   fabric);
            EXPECT_EQ(listener.called, changed);
            EXPECT_EQ(tester.IsAttributeDirty(Binding::Attributes::Binding::Id), changed);
            ExpectBindingRead(cluster, fabric, true, finalVersion, finalBindings);
            ExpectBindingRead(cluster, otherFabric, true, finalVersion, { other });
            finalBindings.push_back(other);
            ExpectBindingRead(cluster, fabric, false, finalVersion, finalBindings);
        }
    }
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
        EXPECT_TRUE(tester.WriteAttribute(Binding::Attributes::Binding::Id, empty, chip::Testing::ListWritingPattern::ReplaceAll)
                        .IsNoOpSuccess());
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

} // namespace
