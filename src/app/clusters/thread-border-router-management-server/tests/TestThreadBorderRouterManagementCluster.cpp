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

#include <app/FailSafeContext.h>
#include <app/clusters/general-commissioning-server/BreadCrumbTracker.h>
#include <app/clusters/thread-border-router-management-server/ThreadBorderRouterManagementDelegate.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/ThreadBorderRouterManagement/Metadata.h>
#include <iterator>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ThreadBorderRouterManagement;

namespace {

constexpr EndpointId kTestEndpointId           = 1;
constexpr uint16_t kMockThreadVersion          = 1;
constexpr uint64_t kMockActiveDatasetTimestamp = 12345ULL;
constexpr FabricIndex kTestFabricIndex         = 1;
constexpr uint16_t kTestFailSafeTimeout        = 60;

class MockDelegate : public ThreadBorderRouterManagementDelegate
{
public:
    bool mPanChangeSupported        = true;
    size_t mMockBorderAgentIdLength = 16;

    CHIP_ERROR Init(AttributeChangeCallback * attributeChangeCallback) override { return CHIP_NO_ERROR; }
    bool GetPanChangeSupported() override { return mPanChangeSupported; }
    void GetBorderRouterName(MutableCharSpan & borderRouterName) override
    {
        EXPECT_EQ(CopyCharSpanToMutableCharSpan("MockBR"_span, borderRouterName), CHIP_NO_ERROR);
    }
    CHIP_ERROR GetBorderAgentId(MutableByteSpan & borderAgentId) override
    {
        borderAgentId.reduce_size(mMockBorderAgentIdLength);
        return CHIP_NO_ERROR;
    }
    uint16_t GetThreadVersion() override { return kMockThreadVersion; }
    bool GetInterfaceEnabled() override { return true; }
    CHIP_ERROR GetDataset(Thread::OperationalDataset & dataset, DatasetType type) override
    {
        if (mReturnNotFoundForDataset)
        {
            return CHIP_ERROR_NOT_FOUND;
        }
        if (type == DatasetType::kActive)
        {
            EXPECT_EQ(dataset.SetActiveTimestamp(kMockActiveDatasetTimestamp), CHIP_NO_ERROR);
        }
        return CHIP_NO_ERROR;
    }
    void SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t sequenceNum,
                          ActivateDatasetCallback * callback) override
    {
        if (callback != nullptr)
        {
            callback->OnActivateDatasetComplete(sequenceNum, CHIP_NO_ERROR);
        }
    }
    CHIP_ERROR CommitActiveDataset() override
    {
        mCommitCalled = true;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR RevertActiveDataset() override
    {
        mRevertCalled = true;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetPendingDataset(const Thread::OperationalDataset & pendingDataset) override { return CHIP_NO_ERROR; }

    bool mRevertCalled             = false;
    bool mReturnNotFoundForDataset = false;
    bool mCommitCalled             = false;
};

class MockBreadcrumbTracker : public BreadCrumbTracker
{
public:
    void SetBreadCrumb(uint64_t v) override
    {
        mBreadcrumb = v;
        mCalled     = true;
    }
    uint64_t mBreadcrumb = 0;
    bool mCalled         = false;
};

struct TestThreadBorderRouterManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        DeviceLayer::PlatformMgr().Shutdown();
        Platform::MemoryShutdown();
    }

    TestThreadBorderRouterManagementCluster() :
        config(delegate, failSafeContext, breadcrumbTracker, DeviceLayer::PlatformMgr()), cluster(kTestEndpointId, config)
    {}

    void TearDown() override { cluster.Shutdown(ClusterShutdownType::kClusterShutdown); }

    FailSafeContext failSafeContext;
    MockBreadcrumbTracker breadcrumbTracker;
    MockDelegate delegate;
    ThreadBorderRouterManagementCluster::Config config;
    ThreadBorderRouterManagementCluster cluster;
};

TEST_F(TestThreadBorderRouterManagementCluster, TestReadClusterRevision)
{

    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint16_t clusterRevision{};
    EXPECT_TRUE(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, clusterRevision).IsSuccess());
    EXPECT_EQ(clusterRevision, ThreadBorderRouterManagement::kRevision);
}

TEST_F(TestThreadBorderRouterManagementCluster, TestAttributesList)
{
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    std::vector<app::DataModel::AttributeEntry> expected(Attributes::kMandatoryMetadata.begin(),
                                                         Attributes::kMandatoryMetadata.end());

    EXPECT_TRUE(chip::Testing::IsAttributesListEqualTo(cluster, expected));
}

TEST_F(TestThreadBorderRouterManagementCluster, TestGeneratedCommandsList)
{
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(chip::Testing::IsGeneratedCommandsListEqualTo(cluster, { Commands::DatasetResponse::Id }));
}

TEST_F(TestThreadBorderRouterManagementCluster, TestReadAttributes)
{

    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    chip::CharSpan name;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::BorderRouterName::Id, name).IsSuccess());
    EXPECT_EQ(std::string_view(name.data(), name.size()), std::string_view("MockBR"));

    chip::ByteSpan agentId;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::BorderAgentID::Id, agentId).IsSuccess());

    uint16_t version;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::ThreadVersion::Id, version).IsSuccess());
    EXPECT_EQ(version, kMockThreadVersion);

    bool enabled;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::InterfaceEnabled::Id, enabled).IsSuccess());
    EXPECT_TRUE(enabled);

    DataModel::Nullable<uint64_t> activeTimestamp;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::ActiveDatasetTimestamp::Id, activeTimestamp).IsSuccess());
    EXPECT_FALSE(activeTimestamp.IsNull());
    EXPECT_EQ(activeTimestamp.Value(), kMockActiveDatasetTimestamp);

    DataModel::Nullable<uint64_t> pendingTimestamp;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::PendingDatasetTimestamp::Id, pendingTimestamp).IsSuccess());
    EXPECT_TRUE(pendingTimestamp.IsNull());
}

TEST_F(TestThreadBorderRouterManagementCluster, TestReadBorderAgentID_InvalidLength)
{
    delegate.mMockBorderAgentIdLength = 10; // Invalid length!
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    chip::ByteSpan agentId;
    EXPECT_FALSE(tester.ReadAttribute(Attributes::BorderAgentID::Id, agentId).IsSuccess());
}

TEST_F(TestThreadBorderRouterManagementCluster, TestReadBorderAgentID_ValidLength)
{
    delegate.mMockBorderAgentIdLength = 16; // Valid length!
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    chip::ByteSpan agentId;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::BorderAgentID::Id, agentId).IsSuccess());
    EXPECT_EQ(agentId.size(), 16u);
}

TEST_F(TestThreadBorderRouterManagementCluster, TestFeatureMap_PanChangeSupported)
{
    delegate.mPanChangeSupported = true;
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint32_t featureMap;
    EXPECT_TRUE(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap).IsSuccess());
    EXPECT_EQ(featureMap, static_cast<uint32_t>(Feature::kPANChange));
}

TEST_F(TestThreadBorderRouterManagementCluster, TestFeatureMap_PanChangeNotSupported)
{
    delegate.mPanChangeSupported = false;

    ThreadBorderRouterManagementCluster::Config localConfig(delegate, failSafeContext, breadcrumbTracker,
                                                            DeviceLayer::PlatformMgr());
    ThreadBorderRouterManagementCluster localCluster(kTestEndpointId, localConfig);

    chip::Testing::ClusterTester tester(localCluster);
    EXPECT_EQ(localCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint32_t featureMap;
    EXPECT_TRUE(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap).IsSuccess());
    EXPECT_EQ(featureMap, 0u);

    localCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestThreadBorderRouterManagementCluster, TestGetActiveDatasetRequest)
{

    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::GetActiveDatasetRequest::Type request;
    auto result = tester.Invoke(request);

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.response.has_value());
}

TEST_F(TestThreadBorderRouterManagementCluster, TestGetPendingDatasetRequest)
{

    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::GetPendingDatasetRequest::Type request;
    auto result = tester.Invoke(request);

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.response.has_value());
}

TEST_F(TestThreadBorderRouterManagementCluster, TestSetActiveDatasetRequest)
{
    delegate.mReturnNotFoundForDataset = true;
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Arm FailSafe (required by spec)
    EXPECT_EQ(failSafeContext.ArmFailSafe(kTestFabricIndex, System::Clock::Seconds16(kTestFailSafeTimeout)), CHIP_NO_ERROR);

    Commands::SetActiveDatasetRequest::Type request;
    uint8_t validDataset[] = { 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x0b, 0x35, 0x06,
                               0x00, 0x04, 0x00, 0x1f, 0xff, 0xe0, 0x02, 0x08, 0xde, 0xaa, 0x00, 0xbe, 0xef, 0x00, 0xca, 0xef, 0x07,
                               0x08, 0xfd, 0xde, 0xad, 0x00, 0xbe, 0xef, 0x00, 0x00, 0x05, 0x10, 0xb7, 0x28, 0x08, 0x04, 0x85, 0xcf,
                               0xc5, 0x25, 0x7f, 0x68, 0x4c, 0x54, 0x9d, 0x6a, 0x57, 0x5e, 0x03, 0x0a, 0x4f, 0x70, 0x65, 0x6e, 0x54,
                               0x68, 0x72, 0x65, 0x61, 0x64, 0x01, 0x02, 0xc1, 0x15, 0x04, 0x10, 0xcb, 0x13, 0x47, 0xeb, 0x0c, 0xd4,
                               0xb3, 0x5c, 0xd1, 0x42, 0xda, 0x5e, 0x6d, 0xf1, 0x8b, 0x88, 0x0c, 0x04, 0x02, 0xa0, 0xf7, 0xf8 };
    request.activeDataset  = ByteSpan(validDataset);
    request.breadcrumb.SetValue(1);

    auto result = tester.Invoke(request);

    EXPECT_TRUE(result.IsSuccess());
}

TEST_F(TestThreadBorderRouterManagementCluster, TestSetPendingDatasetRequest)
{

    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::SetPendingDatasetRequest::Type request;
    uint8_t validDataset[] = { 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x0b, 0x35, 0x06,
                               0x00, 0x04, 0x00, 0x1f, 0xff, 0xe0, 0x02, 0x08, 0xde, 0xaa, 0x00, 0xbe, 0xef, 0x00, 0xca, 0xef, 0x07,
                               0x08, 0xfd, 0xde, 0xad, 0x00, 0xbe, 0xef, 0x00, 0x00, 0x05, 0x10, 0xb7, 0x28, 0x08, 0x04, 0x85, 0xcf,
                               0xc5, 0x25, 0x7f, 0x68, 0x4c, 0x54, 0x9d, 0x6a, 0x57, 0x5e, 0x03, 0x0a, 0x4f, 0x70, 0x65, 0x6e, 0x54,
                               0x68, 0x72, 0x65, 0x61, 0x64, 0x01, 0x02, 0xc1, 0x15, 0x04, 0x10, 0xcb, 0x13, 0x47, 0xeb, 0x0c, 0xd4,
                               0xb3, 0x5c, 0xd1, 0x42, 0xda, 0x5e, 0x6d, 0xf1, 0x8b, 0x88, 0x0c, 0x04, 0x02, 0xa0, 0xf7, 0xf8 };
    request.pendingDataset = ByteSpan(validDataset);

    auto result = tester.Invoke(request);

    EXPECT_TRUE(result.IsSuccess());
}

TEST_F(TestThreadBorderRouterManagementCluster, TestAcceptedCommandsList_PanChangeSupported)
{
    delegate.mPanChangeSupported = true;
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(chip::Testing::IsAcceptedCommandsListEqualTo(
        cluster,
        { Commands::GetActiveDatasetRequest::kMetadataEntry, Commands::GetPendingDatasetRequest::kMetadataEntry,
          Commands::SetActiveDatasetRequest::kMetadataEntry, Commands::SetPendingDatasetRequest::kMetadataEntry }));
}

TEST_F(TestThreadBorderRouterManagementCluster, TestAcceptedCommandsList_PanChangeNotSupported)
{
    delegate.mPanChangeSupported = false;
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(chip::Testing::IsAcceptedCommandsListEqualTo(cluster,
                                                             { Commands::GetActiveDatasetRequest::kMetadataEntry,
                                                               Commands::GetPendingDatasetRequest::kMetadataEntry,
                                                               Commands::SetActiveDatasetRequest::kMetadataEntry }));
}

TEST_F(TestThreadBorderRouterManagementCluster, TestSetActiveDatasetRequestFailsafeRequired)
{

    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::SetActiveDatasetRequest::Type request;
    request.activeDataset = ByteSpan();
    request.breadcrumb.SetValue(1);

    auto result = tester.Invoke(request);

    // Expect failure because FailSafe is not armed!
    EXPECT_FALSE(result.IsSuccess());
}

TEST_F(TestThreadBorderRouterManagementCluster, TestFailSafeTimerExpired)
{

    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DeviceLayer::ChipDeviceEvent event;
    event.Type = DeviceLayer::DeviceEventType::kFailSafeTimerExpired;

    // Call handler directly (we will need to make it accessible or friend it)
    cluster.OnPlatformEventHandler(&event, reinterpret_cast<intptr_t>(&cluster));

    EXPECT_TRUE(delegate.mRevertCalled);
}

TEST_F(TestThreadBorderRouterManagementCluster, TestBreadcrumbHandling)
{
    delegate.mReturnNotFoundForDataset = true;
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Arm FailSafe (required for SetActiveDataset)
    EXPECT_EQ(failSafeContext.ArmFailSafe(1, System::Clock::Seconds16(60)), CHIP_NO_ERROR);

    Commands::SetActiveDatasetRequest::Type request;
    request.activeDataset = ByteSpan();
    request.breadcrumb.SetValue(42);

    auto result = tester.Invoke(request);
    EXPECT_TRUE(result.IsSuccess());

    // Simulate async completion
    cluster.OnActivateDatasetComplete(1, CHIP_NO_ERROR);

    // Verify breadcrumb was applied to tracker!
    EXPECT_TRUE(breadcrumbTracker.mCalled);
    EXPECT_EQ(breadcrumbTracker.mBreadcrumb, 42uLL);
}

TEST_F(TestThreadBorderRouterManagementCluster, TestGetActiveDatasetRequestRequiresCASE)
{

    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Explicitly set AuthMode to None to simulate non-CASE session!
    Access::SubjectDescriptor subjectDescriptor;
    subjectDescriptor.authMode = Access::AuthMode::kNone;
    tester.SetSubjectDescriptor(subjectDescriptor);

    Commands::GetActiveDatasetRequest::Type request;
    auto result = tester.Invoke(request);

    // Expect failure with UnsupportedAccess!
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(
        result.GetStatusCode(),
        std::make_optional(Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::UnsupportedAccess)));
}

TEST_F(TestThreadBorderRouterManagementCluster, TestGetActiveDatasetRequestNotFoundReturnsEmpty)
{
    delegate.mReturnNotFoundForDataset = true;
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::GetActiveDatasetRequest::Type request;
    auto result = tester.Invoke(request);

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.response.has_value());
    if (result.response.has_value())
    {
        EXPECT_TRUE(result.response->dataset.empty());
    }
}

TEST_F(TestThreadBorderRouterManagementCluster, TestSetActiveDatasetRequestInvalidInState)
{

    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(failSafeContext.ArmFailSafe(1, System::Clock::Seconds16(60)), CHIP_NO_ERROR);

    Commands::SetActiveDatasetRequest::Type request;
    uint8_t validDataset[] = { 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x0b, 0x35, 0x06,
                               0x00, 0x04, 0x00, 0x1f, 0xff, 0xe0, 0x02, 0x08, 0xde, 0xaa, 0x00, 0xbe, 0xef, 0x00, 0xca, 0xef, 0x07,
                               0x08, 0xfd, 0xde, 0xad, 0x00, 0xbe, 0xef, 0x00, 0x00, 0x05, 0x10, 0xb7, 0x28, 0x08, 0x04, 0x85, 0xcf,
                               0xc5, 0x25, 0x7f, 0x68, 0x4c, 0x54, 0x9d, 0x6a, 0x57, 0x5e, 0x03, 0x0a, 0x4f, 0x70, 0x65, 0x6e, 0x54,
                               0x68, 0x72, 0x65, 0x61, 0x64, 0x01, 0x02, 0xc1, 0x15, 0x04, 0x10, 0xcb, 0x13, 0x47, 0xeb, 0x0c, 0xd4,
                               0xb3, 0x5c, 0xd1, 0x42, 0xda, 0x5e, 0x6d, 0xf1, 0x8b, 0x88, 0x0c, 0x04, 0x02, 0xa0, 0xf7, 0xf8 };
    request.activeDataset  = ByteSpan(validDataset);

    auto result = tester.Invoke(request);

    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(
        result.GetStatusCode(),
        std::make_optional(Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::InvalidInState)));
}

TEST_F(TestThreadBorderRouterManagementCluster, TestCommissioningComplete)
{

    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DeviceLayer::ChipDeviceEvent event;
    event.Type = DeviceLayer::DeviceEventType::kCommissioningComplete;

    // Call handler directly
    ThreadBorderRouterManagementCluster::OnPlatformEventHandler(&event, reinterpret_cast<intptr_t>(&cluster));

    EXPECT_TRUE(delegate.mCommitCalled);
}

} // namespace
