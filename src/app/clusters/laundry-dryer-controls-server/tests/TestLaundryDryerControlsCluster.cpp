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

#include <app/clusters/laundry-dryer-controls-server/LaundryDryerControlsCluster.h>
#include <clusters/LaundryDryerControls/Metadata.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LaundryDryerControls;
using namespace chip::Testing;

using chip::Testing::IsAttributesListEqualTo;
using Status = Protocols::InteractionModel::Status;

namespace {

constexpr EndpointId kTestEndpointId = 1;

class MockLaundryDryerControlsDelegate : public LaundryDryerControls::Delegate
{
public:
    // Intentionally omits kExtra so it can be used as an unsupported value in tests.
    std::vector<DrynessLevelEnum> supportedLevels = { DrynessLevelEnum::kLow, DrynessLevelEnum::kNormal, DrynessLevelEnum::kMax };
    CHIP_ERROR getSupportedError                  = CHIP_NO_ERROR;

    CHIP_ERROR GetSupportedDrynessLevelAtIndex(size_t index, DrynessLevelEnum & supportedDryness) override
    {
        if (getSupportedError != CHIP_NO_ERROR)
        {
            return getSupportedError;
        }
        if (index < supportedLevels.size())
        {
            supportedDryness = supportedLevels[index];
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
};

struct TestLaundryDryerControlsCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override { EXPECT_EQ(mCluster.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR); }

    MockLaundryDryerControlsDelegate mMockDelegate;

    LaundryDryerControlsCluster mCluster{ kTestEndpointId, mMockDelegate };

    ClusterTester mClusterTester{ mCluster };
};

TEST_F(TestLaundryDryerControlsCluster, TestAttributesList)
{
    std::vector<DataModel::AttributeEntry> mandatoryAttributes(LaundryDryerControls::Attributes::kMandatoryMetadata.begin(),
                                                               LaundryDryerControls::Attributes::kMandatoryMetadata.end());
    EXPECT_TRUE(IsAttributesListEqualTo(mCluster, mandatoryAttributes));
}

TEST_F(TestLaundryDryerControlsCluster, TestReadClusterRevision)
{
    uint16_t clusterRevision = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, kRevision);
}

TEST_F(TestLaundryDryerControlsCluster, TestReadFeatureMap)
{
    uint32_t featureMap = 1;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);
}

TEST_F(TestLaundryDryerControlsCluster, TestReadSupportedDrynessLevels)
{
    Attributes::SupportedDrynessLevels::TypeInfo::DecodableType list;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SupportedDrynessLevels::Id, list), CHIP_NO_ERROR);

    auto it = list.begin();
    ASSERT_TRUE(it.Next());
    EXPECT_EQ(it.GetValue(), DrynessLevelEnum::kLow);
    ASSERT_TRUE(it.Next());
    EXPECT_EQ(it.GetValue(), DrynessLevelEnum::kNormal);
    ASSERT_TRUE(it.Next());
    EXPECT_EQ(it.GetValue(), DrynessLevelEnum::kMax);
    ASSERT_FALSE(it.Next());
}

TEST_F(TestLaundryDryerControlsCluster, TestReadSupportedDrynessLevelsDelegateError)
{
    mMockDelegate.getSupportedError = CHIP_ERROR_INTERNAL;
    Attributes::SupportedDrynessLevels::TypeInfo::DecodableType list;
    EXPECT_NE(mClusterTester.ReadAttribute(Attributes::SupportedDrynessLevels::Id, list), CHIP_NO_ERROR);
}

TEST_F(TestLaundryDryerControlsCluster, TestReadSelectedDrynessLevelDefaultsToNull)
{
    DataModel::Nullable<DrynessLevelEnum> selectedDrynessLevel = DataModel::MakeNullable(DrynessLevelEnum::kNormal);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SelectedDrynessLevel::Id, selectedDrynessLevel), CHIP_NO_ERROR);
    EXPECT_TRUE(selectedDrynessLevel.IsNull());
}

TEST_F(TestLaundryDryerControlsCluster, TestWriteSupportedValue)
{
    EXPECT_EQ(
        mClusterTester.WriteAttribute(Attributes::SelectedDrynessLevel::Id, DataModel::MakeNullable(DrynessLevelEnum::kNormal)),
        CHIP_NO_ERROR);

    DataModel::Nullable<DrynessLevelEnum> selectedDrynessLevel;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SelectedDrynessLevel::Id, selectedDrynessLevel), CHIP_NO_ERROR);
    ASSERT_FALSE(selectedDrynessLevel.IsNull());
    EXPECT_EQ(selectedDrynessLevel.Value(), DrynessLevelEnum::kNormal);
}

TEST_F(TestLaundryDryerControlsCluster, TestWriteNullClearsSelection)
{
    // First set a value, then clear it.
    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SelectedDrynessLevel::Id, DataModel::MakeNullable(DrynessLevelEnum::kMax)),
              CHIP_NO_ERROR);

    EXPECT_EQ(mClusterTester.WriteAttribute(Attributes::SelectedDrynessLevel::Id, DataModel::Nullable<DrynessLevelEnum>()),
              CHIP_NO_ERROR);

    DataModel::Nullable<DrynessLevelEnum> selectedDrynessLevel = DataModel::MakeNullable(DrynessLevelEnum::kNormal);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SelectedDrynessLevel::Id, selectedDrynessLevel), CHIP_NO_ERROR);
    EXPECT_TRUE(selectedDrynessLevel.IsNull());
}

TEST_F(TestLaundryDryerControlsCluster, TestWriteUnsupportedValueIsRejected)
{
    // kExtra is a valid enum value but is not advertised by the delegate, so it must be rejected.
    EXPECT_EQ(
        mClusterTester.WriteAttribute(Attributes::SelectedDrynessLevel::Id, DataModel::MakeNullable(DrynessLevelEnum::kExtra)),
        Status::ConstraintError);

    // The attribute must remain unchanged (null).
    DataModel::Nullable<DrynessLevelEnum> selectedDrynessLevel = DataModel::MakeNullable(DrynessLevelEnum::kNormal);
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::SelectedDrynessLevel::Id, selectedDrynessLevel), CHIP_NO_ERROR);
    EXPECT_TRUE(selectedDrynessLevel.IsNull());
}

TEST_F(TestLaundryDryerControlsCluster, TestNoOpWrites)
{
    // 1. Write an initial value (a change).
    EXPECT_EQ(
        mClusterTester.WriteAttribute(Attributes::SelectedDrynessLevel::Id, DataModel::MakeNullable(DrynessLevelEnum::kNormal)),
        CHIP_NO_ERROR);

    auto & dirtyList = mClusterTester.GetDirtyList();
    EXPECT_EQ(dirtyList.size(), 1u);
    dirtyList.clear();

    // 2. Write the same value (no-op, no report).
    EXPECT_EQ(
        mClusterTester.WriteAttribute(Attributes::SelectedDrynessLevel::Id, DataModel::MakeNullable(DrynessLevelEnum::kNormal)),
        CHIP_NO_ERROR);
    EXPECT_EQ(dirtyList.size(), 0u);
}

TEST_F(TestLaundryDryerControlsCluster, TestPersistence)
{
    TestServerClusterContext context;
    MockLaundryDryerControlsDelegate mockDelegate;

    // 1. Initial startup: default value is null; modify it.
    {
        LaundryDryerControlsCluster cluster(kTestEndpointId, mockDelegate);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);

        DataModel::Nullable<DrynessLevelEnum> selectedDrynessLevel = DataModel::MakeNullable(DrynessLevelEnum::kNormal);
        EXPECT_EQ(tester.ReadAttribute(Attributes::SelectedDrynessLevel::Id, selectedDrynessLevel), CHIP_NO_ERROR);
        EXPECT_TRUE(selectedDrynessLevel.IsNull());

        EXPECT_EQ(tester.WriteAttribute(Attributes::SelectedDrynessLevel::Id, DataModel::MakeNullable(DrynessLevelEnum::kMax)),
                  CHIP_NO_ERROR);
    }

    // 2. Restart (new cluster instance): the modified value must be loaded from storage.
    {
        LaundryDryerControlsCluster cluster(kTestEndpointId, mockDelegate);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);

        DataModel::Nullable<DrynessLevelEnum> selectedDrynessLevel;
        EXPECT_EQ(tester.ReadAttribute(Attributes::SelectedDrynessLevel::Id, selectedDrynessLevel), CHIP_NO_ERROR);
        ASSERT_FALSE(selectedDrynessLevel.IsNull());
        EXPECT_EQ(selectedDrynessLevel.Value(), DrynessLevelEnum::kMax);
    }
}

} // namespace
