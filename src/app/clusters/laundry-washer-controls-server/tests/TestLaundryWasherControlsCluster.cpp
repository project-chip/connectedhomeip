/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/laundry-washer-controls-server/LaundryWasherControlsCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/LaundryWasherControls/Metadata.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LaundryWasherControls;
using namespace chip::app::Clusters::LaundryWasherControls::Attributes;
using namespace chip::Testing;

constexpr CharSpan kSpinSpeedOptions[]                 = { "Low"_span, "Medium"_span, "High"_span };
constexpr NumberOfRinsesEnum kSupportedRinsesOptions[] = { NumberOfRinsesEnum::kNone, NumberOfRinsesEnum::kNormal,
                                                           NumberOfRinsesEnum::kExtra };
constexpr NumberOfRinsesEnum kInvalidRinsesOption      = NumberOfRinsesEnum::kMax;

class TestLaundryWasherControlsDelegate : public Delegate
{
public:
    CHIP_ERROR GetSpinSpeedAtIndex(size_t index, MutableCharSpan & spinSpeed) override
    {
        if (index >= MATTER_ARRAY_SIZE(kSpinSpeedOptions))
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        return CopyCharSpanToMutableCharSpan(kSpinSpeedOptions[index], spinSpeed);
    }

    CHIP_ERROR GetSupportedRinseAtIndex(size_t index, NumberOfRinsesEnum & supportedRinse) override
    {
        if (index >= MATTER_ARRAY_SIZE(kSupportedRinsesOptions))
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        supportedRinse = kSupportedRinsesOptions[index];
        return CHIP_NO_ERROR;
    }
};

TestLaundryWasherControlsDelegate gDelegate;

struct TestLaundryWasherControlsCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestLaundryWasherControlsCluster() {}

    TestServerClusterContext testContext;
    LaundryWasherControlsCluster::Config testConfig{ BitFlags<Feature>(Feature::kSpin, Feature::kRinse), gDelegate };
    EndpointId kTestEndpointId = 1;
};

} // namespace

TEST_F(TestLaundryWasherControlsCluster, TestAttributes)
{
    {
        LaundryWasherControlsCluster cluster(kTestEndpointId, testConfig);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                SpinSpeedCurrent::kMetadataEntry,
                                                SpinSpeeds::kMetadataEntry,
                                                NumberOfRinses::kMetadataEntry,
                                                SupportedRinses::kMetadataEntry,
                                            }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        LaundryWasherControlsCluster::Config configOnlySpin{ BitFlags<Feature>(Feature::kSpin), gDelegate };
        LaundryWasherControlsCluster cluster(kTestEndpointId, configOnlySpin);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                SpinSpeedCurrent::kMetadataEntry,
                                                SpinSpeeds::kMetadataEntry,
                                            }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        LaundryWasherControlsCluster::Config configOnlyRinse{ BitFlags<Feature>(Feature::kRinse), gDelegate };
        LaundryWasherControlsCluster cluster(kTestEndpointId, configOnlyRinse);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster, { NumberOfRinses::kMetadataEntry, SupportedRinses::kMetadataEntry }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestLaundryWasherControlsCluster, TestReadAttribute)
{
    LaundryWasherControlsCluster cluster(kTestEndpointId, testConfig);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    ClusterRevision::TypeInfo::DecodableType clusterRevision{};
    EXPECT_EQ(tester.ReadAttribute(ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);

    FeatureMap::TypeInfo::DecodableType featureMap{};
    EXPECT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

    SpinSpeedCurrent::TypeInfo::DecodableType spinSpeedCurrent{};
    EXPECT_EQ(tester.ReadAttribute(SpinSpeedCurrent::Id, spinSpeedCurrent), CHIP_NO_ERROR);

    SpinSpeeds::TypeInfo::DecodableType spinSpeeds{};
    EXPECT_EQ(tester.ReadAttribute(SpinSpeeds::Id, spinSpeeds), CHIP_NO_ERROR);

    NumberOfRinses::TypeInfo::DecodableType numberOfRinses{};
    EXPECT_EQ(tester.ReadAttribute(NumberOfRinses::Id, numberOfRinses), CHIP_NO_ERROR);

    SupportedRinses::TypeInfo::DecodableType supportedRinses{};
    EXPECT_EQ(tester.ReadAttribute(SupportedRinses::Id, supportedRinses), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestLaundryWasherControlsCluster, TestListAttributesRead)
{
    LaundryWasherControlsCluster cluster(kTestEndpointId, testConfig);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // spinSpeeds
    {
        SpinSpeeds::TypeInfo::DecodableType spinSpeeds{};
        ASSERT_EQ(tester.ReadAttribute(SpinSpeeds::Id, spinSpeeds), CHIP_NO_ERROR);

        auto it = spinSpeeds.begin();
        ASSERT_TRUE(it.Next());
        auto spinSpeed = it.GetValue();
        EXPECT_TRUE(spinSpeed.data_equal("Low"_span));
        ASSERT_TRUE(it.Next());
        spinSpeed = it.GetValue();
        EXPECT_TRUE(spinSpeed.data_equal("Medium"_span));
        ASSERT_TRUE(it.Next());
        spinSpeed = it.GetValue();
        EXPECT_TRUE(spinSpeed.data_equal("High"_span));
        EXPECT_FALSE(it.Next());
    }

    // supportedRinses
    {
        SupportedRinses::TypeInfo::DecodableType supportedRinses{};
        ASSERT_EQ(tester.ReadAttribute(SupportedRinses::Id, supportedRinses), CHIP_NO_ERROR);

        auto it = supportedRinses.begin();
        ASSERT_TRUE(it.Next());
        auto supportedRinse = it.GetValue();
        EXPECT_EQ(supportedRinse, NumberOfRinsesEnum::kNone);
        ASSERT_TRUE(it.Next());
        supportedRinse = it.GetValue();
        EXPECT_EQ(supportedRinse, NumberOfRinsesEnum::kNormal);
        ASSERT_TRUE(it.Next());
        supportedRinse = it.GetValue();
        EXPECT_EQ(supportedRinse, NumberOfRinsesEnum::kExtra);
        EXPECT_FALSE(it.Next());
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestLaundryWasherControlsCluster, TestReadWriteAttributeAndBounds)
{
    LaundryWasherControlsCluster cluster(kTestEndpointId, testConfig);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // spinSpeedCurrent
    // null
    {
        SpinSpeedCurrent::TypeInfo::Type spinSpeedCurrentWrite = DataModel::NullNullable;
        ASSERT_EQ(tester.WriteAttribute(SpinSpeedCurrent::Id, spinSpeedCurrentWrite), CHIP_NO_ERROR);

        SpinSpeedCurrent::TypeInfo::DecodableType spinSpeedCurrentRead{};
        ASSERT_EQ(tester.ReadAttribute(SpinSpeedCurrent::Id, spinSpeedCurrentRead), CHIP_NO_ERROR);
        EXPECT_EQ(spinSpeedCurrentRead, spinSpeedCurrentWrite);
    }

    // In bounds
    size_t i = 0;
    for (; i < MATTER_ARRAY_SIZE(kSpinSpeedOptions); ++i)
    {
        SpinSpeedCurrent::TypeInfo::Type spinSpeedCurrentWrite = i;
        ASSERT_EQ(tester.WriteAttribute(SpinSpeedCurrent::Id, spinSpeedCurrentWrite), CHIP_NO_ERROR);

        SpinSpeedCurrent::TypeInfo::DecodableType spinSpeedCurrentRead{};
        ASSERT_EQ(tester.ReadAttribute(SpinSpeedCurrent::Id, spinSpeedCurrentRead), CHIP_NO_ERROR);
        EXPECT_EQ(spinSpeedCurrentRead, spinSpeedCurrentWrite);
    }

    // Out of bounds
    SpinSpeedCurrent::TypeInfo::Type spinSpeedCurrentWrite = i;
    EXPECT_EQ(tester.WriteAttribute(SpinSpeedCurrent::Id, spinSpeedCurrentWrite),
              Protocols::InteractionModel::Status::ConstraintError);

    // numberOfRinses
    // Supported
    i = 0;
    for (auto supportedRinse : kSupportedRinsesOptions)
    {
        NumberOfRinses::TypeInfo::Type numberOfRinsesWrite = supportedRinse;
        ASSERT_EQ(tester.WriteAttribute(NumberOfRinses::Id, numberOfRinsesWrite), CHIP_NO_ERROR);

        NumberOfRinses::TypeInfo::DecodableType numberOfRinsesRead{};
        ASSERT_EQ(tester.ReadAttribute(NumberOfRinses::Id, numberOfRinsesRead), CHIP_NO_ERROR);
        EXPECT_EQ(numberOfRinsesRead, numberOfRinsesWrite);
    }

    // Unsupported value (not in supported rinses list)
    NumberOfRinses::TypeInfo::Type numberOfRinsesWrite = kInvalidRinsesOption;
    EXPECT_EQ(tester.WriteAttribute(NumberOfRinses::Id, numberOfRinsesWrite), Protocols::InteractionModel::Status::InvalidInState);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestLaundryWasherControlsCluster, TestSettersAndGetters)
{
    LaundryWasherControlsCluster cluster(kTestEndpointId, testConfig);

    // spinSpeedCurrent
    for (size_t i = 0; i < MATTER_ARRAY_SIZE(kSpinSpeedOptions); ++i)
    {
        EXPECT_EQ(cluster.SetSpinSpeedCurrent(i), CHIP_NO_ERROR);
    }

    SpinSpeedCurrent::TypeInfo::DecodableType spinSpeedCurrent = cluster.GetSpinSpeedCurrent();
    EXPECT_EQ(spinSpeedCurrent, static_cast<uint8_t>(MATTER_ARRAY_SIZE(kSpinSpeedOptions) - 1));
    EXPECT_EQ(cluster.SetSpinSpeedCurrent(MATTER_ARRAY_SIZE(kSpinSpeedOptions)), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // numberOfRinses
    for (auto supportedRinse : kSupportedRinsesOptions)
    {
        EXPECT_EQ(cluster.SetNumberOfRinses(supportedRinse), CHIP_NO_ERROR);
    }

    NumberOfRinses::TypeInfo::DecodableType numberOfRinses = cluster.GetNumberOfRinses();
    EXPECT_EQ(numberOfRinses, kSupportedRinsesOptions[MATTER_ARRAY_SIZE(kSupportedRinsesOptions) - 1]);
    EXPECT_EQ(cluster.SetNumberOfRinses(kInvalidRinsesOption), CHIP_IM_GLOBAL_STATUS(InvalidInState));
}

TEST_F(TestLaundryWasherControlsCluster, TestSetDelegate)
{
    LaundryWasherControlsCluster::Config configWithDefaultDelegate{ BitFlags<Feature>(Feature::kSpin, Feature::kRinse) };
    LaundryWasherControlsCluster cluster(kTestEndpointId, configWithDefaultDelegate);

    // If delegate is not set, writing to `spinSpeedCurrent` and `numberOfRinses` should fail.
    // Expect for writing `Null` to `spinSpeedCurrent`.
    EXPECT_EQ(cluster.SetSpinSpeedCurrent(DataModel::NullNullable), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetSpinSpeedCurrent(0), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(cluster.SetNumberOfRinses(NumberOfRinsesEnum::kNone), CHIP_IM_GLOBAL_STATUS(InvalidInState));

    cluster.SetDelegate(gDelegate);
    // After setting the delegate, writing valid values should succeed.
    EXPECT_EQ(cluster.SetSpinSpeedCurrent(0), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetNumberOfRinses(NumberOfRinsesEnum::kNone), CHIP_NO_ERROR);
}
