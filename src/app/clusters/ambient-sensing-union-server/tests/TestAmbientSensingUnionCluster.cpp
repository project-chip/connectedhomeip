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

#include <app/clusters/ambient-sensing-union-server/AmbientSensingUnionCluster.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/AmbientSensingUnion/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AmbientSensingUnion;
using namespace chip::Testing;

namespace {

constexpr EndpointId kTestEndpointId = 1;
constexpr NodeId kTestNodeId1        = 0x1234567890ABCDEF;
constexpr NodeId kTestNodeId2        = 0xFEDCBA0987654321;
constexpr EndpointId kContributorEp1 = 1;
constexpr EndpointId kContributorEp2 = 2;

// Test delegate to track callbacks
class TestAmbientSensingUnionDelegate : public AmbientSensingUnionDelegate
{
public:
    void Reset()
    {
        mUnionNameChangedCalled   = false;
        mUnionHealthChangedCalled = false;
        mLastUnionName.clear();
        mLastUnionHealth = UnionHealthEnum::kFullyFunctional;
    }

    void OnUnionNameChanged(const CharSpan & unionName) override
    {
        mUnionNameChangedCalled = true;
        mLastUnionName          = std::string(unionName.data(), unionName.size());
    }

    void OnUnionHealthChanged(UnionHealthEnum unionHealth) override
    {
        mUnionHealthChangedCalled = true;
        mLastUnionHealth          = unionHealth;
    }

    bool mUnionNameChangedCalled   = false;
    bool mUnionHealthChangedCalled = false;
    std::string mLastUnionName;
    UnionHealthEnum mLastUnionHealth = UnionHealthEnum::kFullyFunctional;
};

// Test persistence delegate
class TestPersistenceDelegate : public AmbientSensingUnionPersistenceDelegate
{
public:
    CHIP_ERROR LoadUnionName(char * buffer, size_t bufferSize, size_t & outLength) override
    {
        if (!mHasStoredName)
        {
            return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
        }

        outLength = std::min(mStoredName.size(), bufferSize);
        memcpy(buffer, mStoredName.data(), outLength);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SaveUnionName(const CharSpan & unionName) override
    {
        mStoredName    = std::string(unionName.data(), unionName.size());
        mHasStoredName = true;
        mSaveCount++;
        return CHIP_NO_ERROR;
    }

    void Reset()
    {
        mStoredName.clear();
        mHasStoredName = false;
        mSaveCount     = 0;
    }

    std::string mStoredName;
    bool mHasStoredName = false;
    int mSaveCount      = 0;
};

struct TestAmbientSensingUnionCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        mDelegate.Reset();
        mPersistence.Reset();
    }

    TestAmbientSensingUnionDelegate mDelegate;
    TestPersistenceDelegate mPersistence;
};

// =============================================================================
// Basic Attribute Tests
// =============================================================================

TEST_F(TestAmbientSensingUnionCluster, TestReadClusterRevision)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    uint16_t clusterRevision;
    EXPECT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, AmbientSensingUnion::kRevision);
}

TEST_F(TestAmbientSensingUnionCluster, TestReadFeatureMap)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    uint32_t featureMap;
    EXPECT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    // No features defined for this cluster
    EXPECT_EQ(featureMap, 0u);
}

TEST_F(TestAmbientSensingUnionCluster, TestReadUnionName)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    constexpr char kTestName[] = "LivingRoomUnion";

    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithUnionName(
        CharSpan::fromCharString(kTestName)) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    CharSpan unionName;
    EXPECT_EQ(tester.ReadAttribute(Attributes::UnionName::Id, unionName), CHIP_NO_ERROR);
    EXPECT_TRUE(unionName.data_equal(CharSpan::fromCharString(kTestName)));

    // Also verify via getter
    EXPECT_TRUE(cluster.GetUnionName().data_equal(CharSpan::fromCharString(kTestName)));
}

TEST_F(TestAmbientSensingUnionCluster, TestReadUnionHealth)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();

    // Test health with no contributors (NonFunctional)
    {
        AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
        EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);

        UnionHealthEnum health;
        EXPECT_EQ(tester.ReadAttribute(Attributes::UnionHealth::Id, health), CHIP_NO_ERROR);
        EXPECT_EQ(health, UnionHealthEnum::kNonFunctional);
    }

    // Test health with all online contributors (FullyFunctional)
    {
        AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
        EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

        EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
                  CHIP_NO_ERROR);

        chip::Testing::ClusterTester tester(cluster);

        UnionHealthEnum health;
        EXPECT_EQ(tester.ReadAttribute(Attributes::UnionHealth::Id, health), CHIP_NO_ERROR);
        EXPECT_EQ(health, UnionHealthEnum::kFullyFunctional);
    }

    // Test health with mixed online/offline contributors (LimitedDegraded)
    {
        AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
        EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

        EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
                  CHIP_NO_ERROR);
        EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId2, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOffline),
                  CHIP_NO_ERROR);

        chip::Testing::ClusterTester tester(cluster);

        UnionHealthEnum health;
        EXPECT_EQ(tester.ReadAttribute(Attributes::UnionHealth::Id, health), CHIP_NO_ERROR);
        EXPECT_EQ(health, UnionHealthEnum::kLimitedDegraded);
    }
}

TEST_F(TestAmbientSensingUnionCluster, TestReadEmptyContributorList)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetContributorCount(), 0u);
}

// =============================================================================
// UnionName Write Tests
// =============================================================================

TEST_F(TestAmbientSensingUnionCluster, TestWriteUnionName)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    constexpr char kNewName[] = "BedroomUnion";

    EXPECT_EQ(tester.WriteAttribute(Attributes::UnionName::Id, CharSpan::fromCharString(kNewName)), CHIP_NO_ERROR);

    CharSpan unionName;
    EXPECT_EQ(tester.ReadAttribute(Attributes::UnionName::Id, unionName), CHIP_NO_ERROR);
    EXPECT_TRUE(unionName.data_equal(CharSpan::fromCharString(kNewName)));

    EXPECT_TRUE(mDelegate.mUnionNameChangedCalled);
    EXPECT_EQ(mDelegate.mLastUnionName, kNewName);
}

TEST_F(TestAmbientSensingUnionCluster, TestWriteUnionNameViaSetMethod)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    constexpr char kNewName[] = "KitchenUnion";

    EXPECT_EQ(cluster.SetUnionName(CharSpan::fromCharString(kNewName)), CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.GetUnionName().data_equal(CharSpan::fromCharString(kNewName)));
    EXPECT_TRUE(mDelegate.mUnionNameChangedCalled);
}

TEST_F(TestAmbientSensingUnionCluster, TestWriteUnionNameSameValueNoOp)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    constexpr char kTestName[] = "TestUnion";

    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }
                                            .WithUnionName(CharSpan::fromCharString(kTestName))
                                            .WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    // Writing the same value should not trigger delegate
    EXPECT_EQ(cluster.SetUnionName(CharSpan::fromCharString(kTestName)), CHIP_NO_ERROR);
    EXPECT_FALSE(mDelegate.mUnionNameChangedCalled);
}

TEST_F(TestAmbientSensingUnionCluster, TestWriteUnionNameMaxLength)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    // Create a 128-character string (max allowed per spec)
    char maxLengthName[129];
    memset(maxLengthName, 'A', 128);
    maxLengthName[128] = '\0';

    EXPECT_EQ(tester.WriteAttribute(Attributes::UnionName::Id, CharSpan(maxLengthName, 128)), CHIP_NO_ERROR);

    CharSpan unionName;
    EXPECT_EQ(tester.ReadAttribute(Attributes::UnionName::Id, unionName), CHIP_NO_ERROR);
    EXPECT_EQ(unionName.size(), 128u);
}

TEST_F(TestAmbientSensingUnionCluster, TestWriteUnionNameTooLong)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    // Create a 129-character string (exceeds max)
    char tooLongName[130];
    memset(tooLongName, 'A', 129);
    tooLongName[129] = '\0';

    EXPECT_EQ(tester.WriteAttribute(Attributes::UnionName::Id, CharSpan(tooLongName, 129)), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestAmbientSensingUnionCluster, TestWriteUnionNameEmptyString)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithUnionName(
        CharSpan::fromCharString("InitialName")) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    // Empty string should be valid
    EXPECT_EQ(tester.WriteAttribute(Attributes::UnionName::Id, CharSpan()), CHIP_NO_ERROR);

    CharSpan unionName;
    EXPECT_EQ(tester.ReadAttribute(Attributes::UnionName::Id, unionName), CHIP_NO_ERROR);
    EXPECT_EQ(unionName.size(), 0u);
}

// =============================================================================
// Matter Contributor Tests
// =============================================================================

TEST_F(TestAmbientSensingUnionCluster, TestAddMatterContributor)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetContributorCount(), 1u);
    EXPECT_TRUE(mDelegate.mUnionHealthChangedCalled);
}

TEST_F(TestAmbientSensingUnionCluster, TestAddMatterContributorDuplicate)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1), CHIP_ERROR_DUPLICATE_KEY_ID);

    EXPECT_EQ(cluster.GetContributorCount(), 1u);
}

TEST_F(TestAmbientSensingUnionCluster, TestAddMultipleMatterContributors)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp2), CHIP_NO_ERROR); // Same node, different endpoint
    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId2, kContributorEp1), CHIP_NO_ERROR); // Different node

    EXPECT_EQ(cluster.GetContributorCount(), 3u);
}

TEST_F(TestAmbientSensingUnionCluster, TestRemoveMatterContributor)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetContributorCount(), 1u);

    mDelegate.Reset();
    EXPECT_EQ(cluster.RemoveMatterContributor(kTestNodeId1, kContributorEp1), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetContributorCount(), 0u);
    EXPECT_TRUE(mDelegate.mUnionHealthChangedCalled);
}

TEST_F(TestAmbientSensingUnionCluster, TestRemoveMatterContributorNotFound)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.RemoveMatterContributor(kTestNodeId1, kContributorEp1), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestAmbientSensingUnionCluster, TestUpdateMatterContributorStatus)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);

    mDelegate.Reset();

    EXPECT_EQ(
        cluster.UpdateMatterContributorStatus(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOffline),
        CHIP_NO_ERROR);

    EXPECT_TRUE(mDelegate.mUnionHealthChangedCalled);
    EXPECT_EQ(mDelegate.mLastUnionHealth, UnionHealthEnum::kNonFunctional);
}

TEST_F(TestAmbientSensingUnionCluster, TestUpdateMatterContributorStatusSameValue)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);

    mDelegate.Reset();

    // Update to same value - should not trigger health change callback
    EXPECT_EQ(
        cluster.UpdateMatterContributorStatus(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
        CHIP_NO_ERROR);

    EXPECT_FALSE(mDelegate.mUnionHealthChangedCalled);
}

TEST_F(TestAmbientSensingUnionCluster, TestUpdateMatterContributorStatusNotFound)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(
        cluster.UpdateMatterContributorStatus(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOffline),
        CHIP_ERROR_NOT_FOUND);
}

// =============================================================================
// Non-Matter Contributor Tests
// =============================================================================

TEST_F(TestAmbientSensingUnionCluster, TestAddNonMatterContributor)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    constexpr char kContributorName[] = "ZigbeeSensor1";

    EXPECT_EQ(cluster.AddNonMatterContributor(CharSpan::fromCharString(kContributorName),
                                              UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetContributorCount(), 1u);
    EXPECT_TRUE(mDelegate.mUnionHealthChangedCalled);
}

TEST_F(TestAmbientSensingUnionCluster, TestAddNonMatterContributorEmptyName)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    // Per spec: ContributorName is mandatory when NodeID is NULL
    EXPECT_EQ(cluster.AddNonMatterContributor(CharSpan(), UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_ERROR_INVALID_ARGUMENT);

    EXPECT_EQ(cluster.GetContributorCount(), 0u);
}

TEST_F(TestAmbientSensingUnionCluster, TestAddNonMatterContributorDuplicate)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    constexpr char kContributorName[] = "LegacySensor";

    EXPECT_EQ(cluster.AddNonMatterContributor(CharSpan::fromCharString(kContributorName)), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.AddNonMatterContributor(CharSpan::fromCharString(kContributorName)), CHIP_ERROR_DUPLICATE_KEY_ID);

    EXPECT_EQ(cluster.GetContributorCount(), 1u);
}

TEST_F(TestAmbientSensingUnionCluster, TestRemoveNonMatterContributor)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    constexpr char kContributorName[] = "BluetoothSensor";

    EXPECT_EQ(cluster.AddNonMatterContributor(CharSpan::fromCharString(kContributorName)), CHIP_NO_ERROR);

    // Consume the add event
    (void) context->EventsGenerator().GetNextEvent();

    mDelegate.Reset();

    EXPECT_EQ(cluster.RemoveNonMatterContributor(CharSpan::fromCharString(kContributorName)), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetContributorCount(), 0u);
    EXPECT_TRUE(mDelegate.mUnionHealthChangedCalled);
}

TEST_F(TestAmbientSensingUnionCluster, TestUpdateNonMatterContributorStatus)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    constexpr char kContributorName[] = "WifiSensor";

    EXPECT_EQ(cluster.AddNonMatterContributor(CharSpan::fromCharString(kContributorName),
                                              UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);
    mDelegate.Reset();

    EXPECT_EQ(cluster.UpdateNonMatterContributorStatus(CharSpan::fromCharString(kContributorName),
                                                       UnionContributorStatusEnum::kUnionContributorOffline),
              CHIP_NO_ERROR);

    EXPECT_TRUE(mDelegate.mUnionHealthChangedCalled);
    EXPECT_EQ(mDelegate.mLastUnionHealth, UnionHealthEnum::kNonFunctional);
}

// =============================================================================
// Mixed Contributor Tests
// =============================================================================

TEST_F(TestAmbientSensingUnionCluster, TestMixedContributors)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    // Add Matter contributors
    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId2, kContributorEp1), CHIP_NO_ERROR);

    // Add non-Matter contributors
    EXPECT_EQ(cluster.AddNonMatterContributor(CharSpan::fromCharString("Zigbee1")), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.AddNonMatterContributor(CharSpan::fromCharString("Zigbee2")), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetContributorCount(), 4u);

    EXPECT_EQ(cluster.RemoveMatterContributor(kTestNodeId1, kContributorEp1), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.RemoveMatterContributor(kTestNodeId2, kContributorEp1), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.RemoveNonMatterContributor(CharSpan::fromCharString("Zigbee1")), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.RemoveNonMatterContributor(CharSpan::fromCharString("Zigbee2")), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetContributorCount(), 0u);
}

// =============================================================================
// Maximum Contributor Count Boundary Test (128 per spec)
// =============================================================================

TEST_F(TestAmbientSensingUnionCluster, TestMaxContributorsBoundary)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    // Fill to spec maximum (128 contributors)
    // Use 100 Matter contributors + 28 non-Matter contributors
    for (uint16_t i = 0; i < 100; i++)
    {
        NodeId nodeId = static_cast<NodeId>(0x1000000000000000ULL + i);
        ASSERT_EQ(cluster.AddMatterContributor(nodeId, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
                  CHIP_NO_ERROR)
            << "Failed to add Matter contributor " << i;
    }

    for (uint8_t i = 0; i < 28; i++)
    {
        char name[32];
        snprintf(name, sizeof(name), "Sensor%u", i);
        ASSERT_EQ(
            cluster.AddNonMatterContributor(CharSpan::fromCharString(name), UnionContributorStatusEnum::kUnionContributorOnline),
            CHIP_NO_ERROR)
            << "Failed to add non-Matter contributor " << static_cast<int>(i);
    }

    // Verify exactly 128 contributors (spec maximum)
    EXPECT_EQ(cluster.GetContributorCount(), 128u);

    // Verify health is FullyFunctional with all online contributors
    EXPECT_EQ(cluster.GetUnionHealth(), UnionHealthEnum::kFullyFunctional);

    // 129th contributor should fail
    NodeId extraNodeId = static_cast<NodeId>(0x2000000000000000ULL);
    EXPECT_EQ(cluster.AddMatterContributor(extraNodeId, kContributorEp1), CHIP_ERROR_NO_MEMORY);

    EXPECT_EQ(cluster.AddNonMatterContributor(CharSpan::fromCharString("ExtraSensor")), CHIP_ERROR_NO_MEMORY);

    // Verify count remains at spec maximum of 128
    EXPECT_EQ(cluster.GetContributorCount(), 128u);
}

TEST_F(TestAmbientSensingUnionCluster, TestContributorSlotReuse)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    // Add and remove contributors, then add new ones to verify slot reuse
    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId2, kContributorEp1), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetContributorCount(), 2u);

    // Remove first contributor
    EXPECT_EQ(cluster.RemoveMatterContributor(kTestNodeId1, kContributorEp1), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetContributorCount(), 1u);

    // Add new contributor - should reuse the freed slot
    NodeId newNodeId = 0x9999999999999999ULL;
    EXPECT_EQ(cluster.AddMatterContributor(newNodeId, kContributorEp1), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetContributorCount(), 2u);

    // Add non-Matter contributor
    EXPECT_EQ(cluster.AddNonMatterContributor(CharSpan::fromCharString("NewSensor")), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetContributorCount(), 3u);
}

// =============================================================================
// Union Health Recalculation Tests
// =============================================================================

TEST_F(TestAmbientSensingUnionCluster, TestUnionHealthAllOnline)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);
    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId2, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetUnionHealth(), UnionHealthEnum::kFullyFunctional);
}

TEST_F(TestAmbientSensingUnionCluster, TestUnionHealthAllOffline)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOffline),
              CHIP_NO_ERROR);
    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId2, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOffline),
              CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetUnionHealth(), UnionHealthEnum::kNonFunctional);
}

TEST_F(TestAmbientSensingUnionCluster, TestUnionHealthPartialOffline)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);
    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId2, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOffline),
              CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetUnionHealth(), UnionHealthEnum::kLimitedDegraded);
}

TEST_F(TestAmbientSensingUnionCluster, TestUnionHealthNoContributors)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.RemoveMatterContributor(kTestNodeId1, kContributorEp1), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetUnionHealth(), UnionHealthEnum::kNonFunctional);
}

TEST_F(TestAmbientSensingUnionCluster, TestUnionHealthTransitions)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    // Start with one online contributor
    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetUnionHealth(), UnionHealthEnum::kFullyFunctional);

    // Add offline contributor - becomes degraded
    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId2, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOffline),
              CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetUnionHealth(), UnionHealthEnum::kLimitedDegraded);

    // Make offline contributor online - becomes fully functional
    EXPECT_EQ(
        cluster.UpdateMatterContributorStatus(kTestNodeId2, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
        CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetUnionHealth(), UnionHealthEnum::kFullyFunctional);

    // Make both offline - becomes non-functional
    EXPECT_EQ(
        cluster.UpdateMatterContributorStatus(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOffline),
        CHIP_NO_ERROR);
    EXPECT_EQ(
        cluster.UpdateMatterContributorStatus(kTestNodeId2, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOffline),
        CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetUnionHealth(), UnionHealthEnum::kNonFunctional);
}

// =============================================================================
// Event Tests
// =============================================================================

TEST_F(TestAmbientSensingUnionCluster, TestUnionContributorAddedEvent)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);

    AmbientSensingUnion::Events::UnionContributorAdded::DecodableType decodedEvent;
    auto eventInfo = context->EventsGenerator().GetNextEvent();
    if (eventInfo.has_value())
    {
        EXPECT_EQ(eventInfo.value().GetEventData(decodedEvent), CHIP_NO_ERROR);

        auto iter = decodedEvent.addedContributor.begin();
        ASSERT_TRUE(iter.Next());
        const auto & contributor = iter.GetValue();
        EXPECT_FALSE(contributor.contributorNodeID.IsNull());
        EXPECT_EQ(contributor.contributorNodeID.Value(), kTestNodeId1);
        EXPECT_FALSE(iter.Next()); // Only one element expected
    }
    else
    {
        FAIL() << "Failed to get event";
    }
}

TEST_F(TestAmbientSensingUnionCluster, TestUnionContributorRemovedEvent)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1), CHIP_NO_ERROR);
    (void) context->EventsGenerator().GetNextEvent(); // Consume add event

    EXPECT_EQ(cluster.RemoveMatterContributor(kTestNodeId1, kContributorEp1), CHIP_NO_ERROR);

    AmbientSensingUnion::Events::UnionContributorRemoved::DecodableType decodedEvent;
    auto eventInfo = context->EventsGenerator().GetNextEvent();
    if (eventInfo.has_value())
    {
        EXPECT_EQ(eventInfo.value().GetEventData(decodedEvent), CHIP_NO_ERROR);

        auto iter = decodedEvent.removedContributor.begin();
        ASSERT_TRUE(iter.Next());
        const auto & contributor = iter.GetValue();
        EXPECT_FALSE(contributor.contributorNodeID.IsNull());
        EXPECT_EQ(contributor.contributorNodeID.Value(), kTestNodeId1);
        EXPECT_FALSE(iter.Next()); // Only one element expected
    }
    else
    {
        FAIL() << "Failed to get event";
    }
}

TEST_F(TestAmbientSensingUnionCluster, TestUnionContributorStatusChangedEvent)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithDelegate(&mDelegate) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);
    (void) context->EventsGenerator().GetNextEvent(); // Consume add event

    EXPECT_EQ(
        cluster.UpdateMatterContributorStatus(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOffline),
        CHIP_NO_ERROR);

    AmbientSensingUnion::Events::UnionContributorStatusChanged::DecodableType decodedEvent;
    auto eventInfo = context->EventsGenerator().GetNextEvent();
    if (eventInfo.has_value())
    {
        EXPECT_EQ(eventInfo.value().GetEventData(decodedEvent), CHIP_NO_ERROR);

        auto iter = decodedEvent.statusChangedContributor.begin();
        ASSERT_TRUE(iter.Next());
        const auto & contributor = iter.GetValue();
        EXPECT_EQ(contributor.contributorHealth, UnionContributorStatusEnum::kUnionContributorOffline);
        EXPECT_FALSE(iter.Next()); // Only one element expected
    }
    else
    {
        FAIL() << "Failed to get event";
    }
}

// =============================================================================
// Persistence Tests
// =============================================================================

TEST_F(TestAmbientSensingUnionCluster, TestUnionNamePersistence)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();

    constexpr char kInitialName[] = "InitialUnion";
    constexpr char kNewName[]     = "UpdatedUnion";

    // First cluster instance - set initial name
    {
        AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }
                                                .WithUnionName(CharSpan::fromCharString(kInitialName))
                                                .WithPersistence(&mPersistence) };
        EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

        EXPECT_EQ(cluster.SetUnionName(CharSpan::fromCharString(kNewName)), CHIP_NO_ERROR);
        EXPECT_EQ(mPersistence.mSaveCount, 1);
        EXPECT_EQ(mPersistence.mStoredName, kNewName);
    }

    // Second cluster instance - should load persisted name
    {
        AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }
                                                .WithUnionName(CharSpan::fromCharString(kInitialName))
                                                .WithPersistence(&mPersistence) };
        EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(cluster.GetUnionName().data_equal(CharSpan::fromCharString(kNewName)));
    }
}

TEST_F(TestAmbientSensingUnionCluster, TestUnionNameNoPersistence)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();

    constexpr char kTestName[] = "TestUnion";

    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }.WithUnionName(
        CharSpan::fromCharString(kTestName)) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetUnionName(CharSpan::fromCharString("NewName")), CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.GetUnionName().data_equal(CharSpan::fromCharString("NewName")));
}

// =============================================================================
// Error Handling Tests
// =============================================================================

TEST_F(TestAmbientSensingUnionCluster, TestSetUnionNameTooLong)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    // Create a 129-character string (exceeds max of 128)
    char tooLongName[130];
    memset(tooLongName, 'A', 129);
    tooLongName[129] = '\0';

    EXPECT_EQ(cluster.SetUnionName(CharSpan(tooLongName, 129)), CHIP_ERROR_INVALID_ARGUMENT);
}

// =============================================================================
// Attribute Notification Tests
// =============================================================================

TEST_F(TestAmbientSensingUnionCluster, TestUnionNameChangeNotification)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_FALSE(context->ChangeListener().IsDirty({ kTestEndpointId, AmbientSensingUnion::Id, Attributes::UnionName::Id }));

    EXPECT_EQ(cluster.SetUnionName(CharSpan::fromCharString("NewName")), CHIP_NO_ERROR);

    EXPECT_TRUE(context->ChangeListener().IsDirty({ kTestEndpointId, AmbientSensingUnion::Id, Attributes::UnionName::Id }));
}

TEST_F(TestAmbientSensingUnionCluster, TestUnionHealthChangeNotification_OnContributorAdd)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);

    EXPECT_TRUE(context->ChangeListener().IsDirty({ kTestEndpointId, AmbientSensingUnion::Id, Attributes::UnionHealth::Id }));
}

TEST_F(TestAmbientSensingUnionCluster, TestContributorListChangeNotification_OnAdd)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_FALSE(
        context->ChangeListener().IsDirty({ kTestEndpointId, AmbientSensingUnion::Id, Attributes::UnionContributorList::Id }));

    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);

    EXPECT_TRUE(
        context->ChangeListener().IsDirty({ kTestEndpointId, AmbientSensingUnion::Id, Attributes::UnionContributorList::Id }));
}

// =============================================================================
// Read-Only Attribute Write Tests
// =============================================================================

TEST_F(TestAmbientSensingUnionCluster, TestWriteReadOnlyUnionHealth)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    EXPECT_EQ(tester.WriteAttribute(Attributes::UnionHealth::Id, UnionHealthEnum::kNonFunctional),
              CHIP_IM_GLOBAL_STATUS(UnsupportedWrite));
}

TEST_F(TestAmbientSensingUnionCluster, TestWriteReadOnlyContributorList)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    Structs::UnionContributorStruct::Type contributor;
    contributor.contributorNodeID.SetNonNull(kTestNodeId1);
    contributor.contributorEndpointID.SetNonNull(kContributorEp1);
    contributor.contributorHealth = UnionContributorStatusEnum::kUnionContributorOnline;

    app::DataModel::List<Structs::UnionContributorStruct::Type> contributorList(&contributor, 1);

    EXPECT_EQ(
        tester.WriteAttribute(Attributes::UnionContributorList::Id, contributorList, chip::Testing::ListWritingPattern::ReplaceAll),
        CHIP_IM_GLOBAL_STATUS(UnsupportedWrite));
}

// =============================================================================
// Shutdown Tests
// =============================================================================

TEST_F(TestAmbientSensingUnionCluster, TestShutdown)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetUnionName(CharSpan::fromCharString("TestUnion")), CHIP_NO_ERROR);

    // Shutdown should not crash
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);

    // State should still be accessible
    EXPECT_EQ(cluster.GetContributorCount(), 1u);
}

// =============================================================================
// Non-Matter Contributor Name Length Tests
// =============================================================================

TEST_F(TestAmbientSensingUnionCluster, TestNonMatterContributorMaxNameLength)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    // Create max length name (128 chars per spec)
    char maxLengthName[129];
    memset(maxLengthName, 'X', 128);
    maxLengthName[128] = '\0';

    EXPECT_EQ(cluster.AddNonMatterContributor(CharSpan(maxLengthName, 128)), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetContributorCount(), 1u);
}

TEST_F(TestAmbientSensingUnionCluster, TestNonMatterContributorNameTooLong)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    // Create 129-character name (exceeds max of 128)
    char tooLongName[130];
    memset(tooLongName, 'X', 129);
    tooLongName[129] = '\0';

    EXPECT_EQ(cluster.AddNonMatterContributor(CharSpan(tooLongName, 129)), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(cluster.GetContributorCount(), 0u);
}

// =============================================================================
// Delegate Not Set Tests
// =============================================================================

TEST_F(TestAmbientSensingUnionCluster, TestOperationsWithoutDelegate)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();

    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    // All operations should succeed without delegate
    EXPECT_EQ(cluster.SetUnionName(CharSpan::fromCharString("TestUnion")), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1), CHIP_NO_ERROR);
    EXPECT_EQ(
        cluster.UpdateMatterContributorStatus(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOffline),
        CHIP_NO_ERROR);
    EXPECT_EQ(cluster.RemoveMatterContributor(kTestNodeId1, kContributorEp1), CHIP_NO_ERROR);
}

// =============================================================================
// Integration Tests
// =============================================================================

TEST_F(TestAmbientSensingUnionCluster, TestFullWorkflow)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }
                                            .WithUnionName(CharSpan::fromCharString("SmartHomeUnion"))
                                            .WithDelegate(&mDelegate)
                                            .WithPersistence(&mPersistence) };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    // 1. Initial state
    EXPECT_EQ(cluster.GetContributorCount(), 0u);
    EXPECT_EQ(cluster.GetUnionHealth(), UnionHealthEnum::kNonFunctional);

    // 2. Add first Matter contributor (online)
    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetContributorCount(), 1u);
    EXPECT_EQ(cluster.GetUnionHealth(), UnionHealthEnum::kFullyFunctional);

    // 3. Add second Matter contributor (offline)
    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId2, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOffline),
              CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetContributorCount(), 2u);
    EXPECT_EQ(cluster.GetUnionHealth(), UnionHealthEnum::kLimitedDegraded);

    // 4. Add non-Matter contributor (online)
    EXPECT_EQ(cluster.AddNonMatterContributor(CharSpan::fromCharString("ZigbeeSensor"),
                                              UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetContributorCount(), 3u);
    EXPECT_EQ(cluster.GetUnionHealth(), UnionHealthEnum::kLimitedDegraded);

    // 5. Bring offline contributor back online
    EXPECT_EQ(
        cluster.UpdateMatterContributorStatus(kTestNodeId2, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
        CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetUnionHealth(), UnionHealthEnum::kFullyFunctional);

    // 6. Update union name
    EXPECT_EQ(cluster.SetUnionName(CharSpan::fromCharString("UpdatedUnion")), CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.GetUnionName().data_equal(CharSpan::fromCharString("UpdatedUnion")));
    EXPECT_EQ(mPersistence.mSaveCount, 1);

    // 7. Remove a contributor
    EXPECT_EQ(cluster.RemoveMatterContributor(kTestNodeId1, kContributorEp1), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetContributorCount(), 2u);

    // 8. Verify events were generated
    int eventCount = 0;
    while (context->EventsGenerator().GetNextEvent().has_value())
    {
        eventCount++;
    }
    EXPECT_GE(eventCount, 5); // At least: 3 adds, 1 health change, 1 remove
}

TEST_F(TestAmbientSensingUnionCluster, TestConfigurationChaining)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();

    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId }
                                            .WithUnionName(CharSpan::fromCharString("ChainedConfig"))
                                            .WithDelegate(&mDelegate)
                                            .WithPersistence(&mPersistence) };

    EXPECT_TRUE(cluster.GetUnionName().data_equal(CharSpan::fromCharString("ChainedConfig")));

    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster.GetUnionName().data_equal(CharSpan::fromCharString("ChainedConfig")));
    EXPECT_EQ(cluster.GetUnionHealth(), UnionHealthEnum::kNonFunctional); // 0 contributors

    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetUnionHealth(), UnionHealthEnum::kFullyFunctional);
}

// =============================================================================
// Direct Contributor Array Access Test
// =============================================================================

TEST_F(TestAmbientSensingUnionCluster, TestGetContributorsArray)
{
    auto context = std::make_unique<chip::Testing::TestServerClusterContext>();
    AmbientSensingUnionCluster cluster{ AmbientSensingUnionCluster::Config{ kTestEndpointId } };
    EXPECT_EQ(cluster.Startup(context->Get()), CHIP_NO_ERROR);

    // Add some contributors
    EXPECT_EQ(cluster.AddMatterContributor(kTestNodeId1, kContributorEp1, UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);
    EXPECT_EQ(cluster.AddNonMatterContributor(CharSpan::fromCharString("TestSensor"),
                                              UnionContributorStatusEnum::kUnionContributorOnline),
              CHIP_NO_ERROR);

    // Access contributor array directly
    const auto * contributors = cluster.GetContributors();
    ASSERT_NE(contributors, nullptr);

    // Count active contributors
    size_t activeCount = 0;
    for (size_t i = 0; i < AmbientSensingUnionCluster::kMaxContributors; i++)
    {
        if (contributors[i].active)
        {
            activeCount++;
        }
    }
    EXPECT_EQ(activeCount, 2u);
}

} // namespace
