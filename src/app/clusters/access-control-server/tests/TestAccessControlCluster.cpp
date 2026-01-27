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

#include <app/clusters/access-control-server/access-control-cluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <clusters/AccessControl/Enums.h>
#include <clusters/AccessControl/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/NetworkCommissioning.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;

struct TestAccessControlCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestAccessControlCluster, CompileTest)
{
    AccessControlCluster cluster;
    ASSERT_EQ(cluster.GetClusterFlags({ kRootEndpointId, AccessControl::Id }), BitFlags<ClusterQualityFlags>());
}

TEST_F(TestAccessControlCluster, CommandsTest)
{
    AccessControlCluster cluster;
    ConcreteClusterPath accessControlPath = ConcreteClusterPath(kRootEndpointId, AccessControl::Id);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> acceptedCommandsBuilder;
    ASSERT_EQ(cluster.AcceptedCommands(accessControlPath, acceptedCommandsBuilder), CHIP_NO_ERROR);
    ReadOnlyBuffer<DataModel::AcceptedCommandEntry> acceptedCommands = acceptedCommandsBuilder.TakeBuffer();

    ReadOnlyBufferBuilder<chip::CommandId> generatedCommandsBuilder;
    ASSERT_EQ(cluster.GeneratedCommands(accessControlPath, generatedCommandsBuilder), CHIP_NO_ERROR);
    ReadOnlyBuffer<chip::CommandId> generatedCommands = generatedCommandsBuilder.TakeBuffer();

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    // Check accepted commands
    ASSERT_EQ(acceptedCommands.size(), AccessControl::Commands::kAcceptedCommandsCount);
    ASSERT_EQ(acceptedCommands[0].commandId, AccessControl::Commands::ReviewFabricRestrictions::Id);
    ASSERT_EQ(acceptedCommands[0].GetInvokePrivilege(),
              AccessControl::Commands::ReviewFabricRestrictions::kMetadataEntry.GetInvokePrivilege());

    // Check generated commands
    ASSERT_EQ(generatedCommands.size(), AccessControl::Commands::kGeneratedCommandsCount);
    ASSERT_EQ(generatedCommands[0], AccessControl::Commands::ReviewFabricRestrictionsResponse::Id);
#else
    ASSERT_EQ(acceptedCommands.size(), (size_t) (0));
    ASSERT_EQ(generatedCommands.size(), (size_t) (0));
#endif
}

TEST_F(TestAccessControlCluster, AttributesTest)
{
    AccessControlCluster cluster;
    ConcreteClusterPath accessControlPath = ConcreteClusterPath(kRootEndpointId, AccessControl::Id);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
    ASSERT_EQ(cluster.Attributes(accessControlPath, attributesBuilder), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedBuilder;
    ASSERT_EQ(expectedBuilder.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);

    ASSERT_EQ(expectedBuilder.AppendElements({
#if CHIP_CONFIG_ENABLE_ACL_EXTENSIONS
        AccessControl::Attributes::Extension::kMetadataEntry,
#endif

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
            AccessControl::Attributes::CommissioningARL::kMetadataEntry, AccessControl::Attributes::Arl::kMetadataEntry
#endif
    }),
              CHIP_NO_ERROR);
    ASSERT_EQ(expectedBuilder.AppendElements(AccessControl::Attributes::kMandatoryMetadata), CHIP_NO_ERROR);
    ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));
}

} // namespace
