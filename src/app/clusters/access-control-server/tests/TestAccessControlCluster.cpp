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
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/AccessControl/Enums.h>
#include <clusters/AccessControl/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/Span.h>
#include <vector>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;

using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;
using chip::Testing::IsGeneratedCommandsListEqualTo;

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

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    // Check accepted commands
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              {
                                                  AccessControl::Commands::ReviewFabricRestrictions::kMetadataEntry,
                                              }));

    // Check generated commands
    ASSERT_TRUE(IsGeneratedCommandsListEqualTo(cluster,
                                               {
                                                   AccessControl::Commands::ReviewFabricRestrictionsResponse::Id,
                                               }));
#else
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster, {}));
    ASSERT_TRUE(IsGeneratedCommandsListEqualTo(cluster, {}));
#endif
}

TEST_F(TestAccessControlCluster, AttributesTest)
{
    AccessControlCluster cluster;

    std::vector<DataModel::AttributeEntry> expectedAttributes(AccessControl::Attributes::kMandatoryMetadata.begin(),
                                                              AccessControl::Attributes::kMandatoryMetadata.end());

#if CHIP_CONFIG_ENABLE_ACL_EXTENSIONS
    expectedAttributes.push_back(AccessControl::Attributes::Extension::kMetadataEntry);
#endif

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    expectedAttributes.push_back(AccessControl::Attributes::CommissioningARL::kMetadataEntry);
    expectedAttributes.push_back(AccessControl::Attributes::Arl::kMetadataEntry);
#endif

    ASSERT_TRUE(IsAttributesListEqualTo(cluster, expectedAttributes));
}

} // namespace
