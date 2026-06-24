/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <app/clusters/commissioner-control-server/CommissionerControlCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/CommissionerControl/Metadata.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommissionerControl;
using namespace chip::app::Clusters::CommissionerControl::Attributes;
using namespace chip::Testing;

class CommissionerControlDelegate : public Delegate
{
public:
    CHIP_ERROR HandleCommissioningApprovalRequest(const CommissioningApprovalRequest & request) override { return CHIP_NO_ERROR; }

    CHIP_ERROR ValidateCommissionNodeCommand(NodeId clientNodeId, uint64_t requestId) override { return CHIP_NO_ERROR; }

    CHIP_ERROR GetCommissioningWindowParams(CommissioningWindowParams & outParams) override { return CHIP_NO_ERROR; }

    CHIP_ERROR HandleCommissionNode(const CommissioningWindowParams & params) override { return CHIP_NO_ERROR; }
};

struct TestCommissionerControlCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestCommissionerControlCluster() {}

    TestServerClusterContext testContext;

    CommissionerControlDelegate testDelegate;
};

} // namespace

TEST_F(TestCommissionerControlCluster, AttributeTest)
{
    CommissionerControlCluster cluster(kRootEndpointId, testDelegate);

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::SupportedDeviceCategories::kMetadataEntry,
                                        }));
}

TEST_F(TestCommissionerControlCluster, ReadAttributeTest)
{
    CommissionerControlCluster cluster(kRootEndpointId, testDelegate);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

    BitMask<CommissionerControl::SupportedDeviceCategoryBitmap> supportedDeviceCategories;
    ASSERT_EQ(tester.ReadAttribute(SupportedDeviceCategories::Id, supportedDeviceCategories), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestCommissionerControlCluster, SupportedDeviceCategories)
{
    CommissionerControlCluster cluster(kRootEndpointId, testDelegate);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    BitMask<CommissionerControl::SupportedDeviceCategoryBitmap> supportedDeviceCategories{
        SupportedDeviceCategoryBitmap::kFabricSynchronization
    };
    cluster.SetSupportedDeviceCategories(supportedDeviceCategories);
    EXPECT_EQ(cluster.GetSupportedDeviceCategories(), supportedDeviceCategories);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}
