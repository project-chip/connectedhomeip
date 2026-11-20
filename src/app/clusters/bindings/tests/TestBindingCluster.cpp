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
#include <lib/support/TestPersistentStorageDelegate.h>

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
