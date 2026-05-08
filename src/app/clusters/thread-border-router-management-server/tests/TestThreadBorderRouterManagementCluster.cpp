/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/thread-border-router-management-server/ThreadBorderRouterManagementCluster.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/clusters/thread-border-router-management-server/ThreadBorderRouterManagementDelegate.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/ThreadBorderRouterManagement/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ThreadBorderRouterManagement;

namespace {

constexpr EndpointId kTestEndpointId = 1;

class MockDelegate : public Delegate
{
public:
    bool mPanChangeSupported = true;

    CHIP_ERROR Init(AttributeChangeCallback * attributeChangeCallback) override { return CHIP_NO_ERROR; }
    bool GetPanChangeSupported() override { return mPanChangeSupported; }
    void GetBorderRouterName(MutableCharSpan & borderRouterName) override { (void) CopyCharSpanToMutableCharSpan("MockBR"_span, borderRouterName); }
    CHIP_ERROR GetBorderAgentId(MutableByteSpan & borderAgentId) override { return CHIP_NO_ERROR; }
    uint16_t GetThreadVersion() override { return 1; }
    bool GetInterfaceEnabled() override { return true; }
    CHIP_ERROR GetDataset(Thread::OperationalDataset & dataset, DatasetType type) override
    {
        if (type == DatasetType::kActive)
        {
            (void) dataset.SetActiveTimestamp(12345ULL);
        }
        return CHIP_NO_ERROR;
    }
    void SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t sequenceNum, ActivateDatasetCallback * callback) override {}
    CHIP_ERROR CommitActiveDataset() override { return CHIP_NO_ERROR; }
    CHIP_ERROR RevertActiveDataset() override { return CHIP_NO_ERROR; }
    CHIP_ERROR SetPendingDataset(const Thread::OperationalDataset & pendingDataset) override { return CHIP_NO_ERROR; }
};

struct TestThreadBorderRouterManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST_F(TestThreadBorderRouterManagementCluster, TestReadClusterRevision)
{
    MockDelegate delegate;
    ThreadBorderRouterManagementCluster::Config config(delegate);
    ThreadBorderRouterManagementCluster cluster(kTestEndpointId, config);
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint16_t clusterRevision{};
    EXPECT_TRUE(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, clusterRevision).IsSuccess());
    EXPECT_EQ(clusterRevision, 1u);
}

TEST_F(TestThreadBorderRouterManagementCluster, TestAttributesList)
{
    MockDelegate delegate;
    ThreadBorderRouterManagementCluster::Config config(delegate);
    ThreadBorderRouterManagementCluster cluster(kTestEndpointId, config);
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(chip::Testing::IsAttributesListEqualTo(cluster, {
        Attributes::BorderRouterName::kMetadataEntry,
        Attributes::BorderAgentID::kMetadataEntry,
        Attributes::ThreadVersion::kMetadataEntry,
        Attributes::InterfaceEnabled::kMetadataEntry,
        Attributes::ActiveDatasetTimestamp::kMetadataEntry,
        Attributes::PendingDatasetTimestamp::kMetadataEntry
    }));
}

TEST_F(TestThreadBorderRouterManagementCluster, TestReadAttributes)
{
    MockDelegate delegate;
    ThreadBorderRouterManagementCluster::Config config(delegate);
    ThreadBorderRouterManagementCluster cluster(kTestEndpointId, config);
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    chip::CharSpan name;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::BorderRouterName::Id, name).IsSuccess());
    EXPECT_EQ(std::string_view(name.data(), name.size()), std::string_view("MockBR"));

    chip::ByteSpan agentId;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::BorderAgentID::Id, agentId).IsSuccess());

    uint16_t version;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::ThreadVersion::Id, version).IsSuccess());
    EXPECT_EQ(version, 1u);

    bool enabled;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::InterfaceEnabled::Id, enabled).IsSuccess());
    EXPECT_TRUE(enabled);

    DataModel::Nullable<uint64_t> activeTimestamp;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::ActiveDatasetTimestamp::Id, activeTimestamp).IsSuccess());
    EXPECT_FALSE(activeTimestamp.IsNull());
    EXPECT_EQ(activeTimestamp.Value(), 12345ULL);

    DataModel::Nullable<uint64_t> pendingTimestamp;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::PendingDatasetTimestamp::Id, pendingTimestamp).IsSuccess());
    EXPECT_TRUE(pendingTimestamp.IsNull());
}

TEST_F(TestThreadBorderRouterManagementCluster, TestFeatureMap)
{
    {
        MockDelegate delegate;
        delegate.mPanChangeSupported = true;
        ThreadBorderRouterManagementCluster::Config config(delegate);
        ThreadBorderRouterManagementCluster cluster(kTestEndpointId, config);
        chip::Testing::ClusterTester tester(cluster);
        EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

        uint32_t featureMap;
        EXPECT_TRUE(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap).IsSuccess());
        EXPECT_EQ(featureMap, 1u);
    }
    {
        MockDelegate delegate;
        delegate.mPanChangeSupported = false;
        ThreadBorderRouterManagementCluster::Config config(delegate);
        ThreadBorderRouterManagementCluster cluster(kTestEndpointId, config);
        chip::Testing::ClusterTester tester(cluster);
        EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

        uint32_t featureMap;
        EXPECT_TRUE(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap).IsSuccess());
        EXPECT_EQ(featureMap, 0u);
    }
}

} // namespace
