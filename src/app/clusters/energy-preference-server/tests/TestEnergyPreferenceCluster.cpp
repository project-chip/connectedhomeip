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

#include <app/clusters/energy-preference-server/EnergyPreferenceCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/EnergyPreference/Metadata.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyPreference;
using namespace chip::app::Clusters::EnergyPreference::Attributes;
using namespace chip::Testing;

/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/clusters/energy-preference-server/energy-preference-server.h>

using namespace chip;
using namespace chip::app::Clusters::EnergyPreference;
using namespace chip::app::Clusters::EnergyPreference::Structs;

struct TestEnergyPreferenceDelegate : public Delegate
{
    inline static BalanceStruct::Type energyBalances[] = {
        { .step = 0, .label = Optional<chip::CharSpan>("Efficient"_span) },
        { .step = 50, .label = Optional<chip::CharSpan>() },
        { .step = 100, .label = Optional<chip::CharSpan>("Comfort"_span) },
    };

    inline static BalanceStruct::Type powerBalances[] = {
        { .step = 0, .label = Optional<chip::CharSpan>("1 Minute"_span) },
        { .step = 24, .label = Optional<chip::CharSpan>("10 Minutes"_span) },
        { .step = 48, .label = Optional<chip::CharSpan>("40 Minutes"_span) },
        { .step = 70, .label = Optional<chip::CharSpan>("120 Minutes"_span) },
        { .step = 100, .label = Optional<chip::CharSpan>("Never"_span) },
    };

    CHIP_ERROR GetEnergyBalanceAtIndex(chip::EndpointId aEndpoint, size_t aIndex, chip::Percent & aOutStep,
                                       chip::Optional<chip::MutableCharSpan> & aOutLabel) override
    {
        if (aIndex < GetNumEnergyBalances(aEndpoint))
        {
            aOutStep = energyBalances[aIndex].step;
            if (energyBalances[aIndex].label.HasValue())
            {
                TEMPORARY_RETURN_IGNORED chip::CopyCharSpanToMutableCharSpan(energyBalances[aIndex].label.Value(), aOutLabel.Value());
            }
            else
            {
                aOutLabel.ClearValue();
            }
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_NOT_FOUND;
    }

    CHIP_ERROR GetEnergyPriorityAtIndex(chip::EndpointId aEndpoint, size_t aIndex, EnergyPriorityEnum & priority) override
    {
        static EnergyPriorityEnum priorities[] = { EnergyPriorityEnum::kEfficiency, EnergyPriorityEnum::kComfort };

        if (aIndex < MATTER_ARRAY_SIZE(priorities))
        {
            priority = priorities[aIndex];
            return CHIP_NO_ERROR;
        }

        return CHIP_ERROR_NOT_FOUND;
    }

    CHIP_ERROR GetLowPowerModeSensitivityAtIndex(chip::EndpointId aEndpoint, size_t aIndex, chip::Percent & aOutStep,
                                                 chip::Optional<chip::MutableCharSpan> & aOutLabel) override
    {
        if (aIndex < GetNumLowPowerModeSensitivities(aEndpoint))
        {
            aOutStep = powerBalances[aIndex].step;
            if (powerBalances[aIndex].label.HasValue())
            {
                TEMPORARY_RETURN_IGNORED chip::CopyCharSpanToMutableCharSpan(powerBalances[aIndex].label.Value(), aOutLabel.Value());
            }
            else
            {
                aOutLabel.ClearValue();
            }
            return CHIP_NO_ERROR;
        }

        return CHIP_ERROR_NOT_FOUND;
    }

    size_t GetNumEnergyBalances(chip::EndpointId aEndpoint) override
    {
        return (MATTER_ARRAY_SIZE(energyBalances));
    }

    size_t GetNumLowPowerModeSensitivities(chip::EndpointId aEndpoint) override
    {
        return (MATTER_ARRAY_SIZE(powerBalances));
    }
};


struct TestEnergyPreferenceCluster : public ::testing::Test
{
    inline static TestEnergyPreferenceDelegate testDelegate;
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        EnergyPreferenceCluster::SetDelegate(&testDelegate);
    }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestServerClusterContext testContext;
    BitFlags<Feature> bothFeatures{ Feature::kEnergyBalance, Feature::kLowPowerModeSensitivity };
    EndpointId kTestEndpointId = 1;
};

} // namespace

TEST_F(TestEnergyPreferenceCluster, TestAttributes)
{
    {
        EnergyPreferenceCluster cluster(kTestEndpointId, bothFeatures);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                EnergyBalances::kMetadataEntry,
                                                CurrentEnergyBalance::kMetadataEntry,
                                                EnergyPriorities::kMetadataEntry,
                                                LowPowerModeSensitivities::kMetadataEntry,
                                                CurrentLowPowerModeSensitivity::kMetadataEntry
                                            }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        EnergyPreferenceCluster cluster(kTestEndpointId, BitFlags<Feature>(Feature::kEnergyBalance));
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                EnergyBalances::kMetadataEntry,
                                                CurrentEnergyBalance::kMetadataEntry,
                                                EnergyPriorities::kMetadataEntry
                                            }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        EnergyPreferenceCluster cluster(kTestEndpointId, BitFlags<Feature>(Feature::kLowPowerModeSensitivity));
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                LowPowerModeSensitivities::kMetadataEntry,
                                                CurrentLowPowerModeSensitivity::kMetadataEntry
                                            }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestEnergyPreferenceCluster, TestReadAttribute)
{
    EnergyPreferenceCluster cluster(kTestEndpointId, bothFeatures);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    ClusterRevision::TypeInfo::DecodableType clusterRevision{};
    EXPECT_EQ(tester.ReadAttribute(ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);

    FeatureMap::TypeInfo::DecodableType featureMap{};
    EXPECT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

    EnergyBalances::TypeInfo::DecodableType energyBalances{};
    EXPECT_EQ(tester.ReadAttribute(EnergyBalances::Id, energyBalances), CHIP_NO_ERROR);

    CurrentEnergyBalance::TypeInfo::DecodableType currentEnergyBalance{};
    EXPECT_EQ(tester.ReadAttribute(CurrentEnergyBalance::Id, currentEnergyBalance), CHIP_NO_ERROR);

    EnergyPriorities::TypeInfo::DecodableType energyPriorities{};
    EXPECT_EQ(tester.ReadAttribute(EnergyPriorities::Id, energyPriorities), CHIP_NO_ERROR);

    LowPowerModeSensitivities::TypeInfo::DecodableType lowPowerModeSensitivities{};
    EXPECT_EQ(tester.ReadAttribute(LowPowerModeSensitivities::Id, lowPowerModeSensitivities), CHIP_NO_ERROR);

    CurrentLowPowerModeSensitivity::TypeInfo::DecodableType currentLowPowerModeSensitivity{};
    EXPECT_EQ(tester.ReadAttribute(CurrentLowPowerModeSensitivity::Id, currentLowPowerModeSensitivity ), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestEnergyPreferenceCluster, TestListAttributesRead)
{
    EnergyPreferenceCluster cluster(kTestEndpointId, bothFeatures);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // energyBalances
    {
        EnergyBalances::TypeInfo::DecodableType energyBalances{};
        ASSERT_EQ(tester.ReadAttribute(EnergyBalances::Id, energyBalances), CHIP_NO_ERROR);

        auto it = energyBalances.begin();
        ASSERT_TRUE(it.Next());
        auto energyBalance = it.GetValue();
        EXPECT_EQ(energyBalance.step, 0);
        EXPECT_TRUE(energyBalance.label.HasValue());
        EXPECT_TRUE(energyBalance.label.Value().data_equal("Efficient"_span));
        ASSERT_TRUE(it.Next());
        energyBalance = it.GetValue();
        EXPECT_EQ(energyBalance.step, 50);
        EXPECT_FALSE(energyBalance.label.HasValue());
        ASSERT_TRUE(it.Next());
        energyBalance = it.GetValue();
        EXPECT_EQ(energyBalance.step, 100);
        EXPECT_TRUE(energyBalance.label.HasValue());
        EXPECT_TRUE(energyBalance.label.Value().data_equal("Comfort"_span));
        EXPECT_FALSE(it.Next());
    }

    // energyPriorities
    {
        EnergyPriorities::TypeInfo::DecodableType energyPriorities{};
        ASSERT_EQ(tester.ReadAttribute(EnergyPriorities::Id, energyPriorities), CHIP_NO_ERROR);

        auto it = energyPriorities.begin();
        ASSERT_TRUE(it.Next());
        auto energyPriority = it.GetValue();
        EXPECT_EQ(energyPriority, EnergyPriorityEnum::kEfficiency);
        ASSERT_TRUE(it.Next());
        energyPriority = it.GetValue();
        EXPECT_EQ(energyPriority, EnergyPriorityEnum::kComfort);
        EXPECT_FALSE(it.Next());
    }

    // lowPowerModeSensitivities
    {
        LowPowerModeSensitivities::TypeInfo::DecodableType lowPowerModeSensitivities{};
        ASSERT_EQ(tester.ReadAttribute(LowPowerModeSensitivities::Id, lowPowerModeSensitivities), CHIP_NO_ERROR);

        auto it = lowPowerModeSensitivities.begin();
        ASSERT_TRUE(it.Next());
        auto lowPowerModeSensitivity = it.GetValue();
        EXPECT_EQ(lowPowerModeSensitivity.step, 0);
        EXPECT_TRUE(lowPowerModeSensitivity.label.HasValue());
        EXPECT_TRUE(lowPowerModeSensitivity.label.Value().data_equal("1 Minute"_span));
        ASSERT_TRUE(it.Next());
        lowPowerModeSensitivity = it.GetValue();
        EXPECT_EQ(lowPowerModeSensitivity.step, 24);
        EXPECT_TRUE(lowPowerModeSensitivity.label.HasValue());
        EXPECT_TRUE(lowPowerModeSensitivity.label.Value().data_equal("10 Minutes"_span));
        ASSERT_TRUE(it.Next());
        lowPowerModeSensitivity = it.GetValue();
        EXPECT_EQ(lowPowerModeSensitivity.step, 48);
        EXPECT_TRUE(lowPowerModeSensitivity.label.HasValue());
        EXPECT_TRUE(lowPowerModeSensitivity.label.Value().data_equal("40 Minutes"_span));
        ASSERT_TRUE(it.Next());
        lowPowerModeSensitivity = it.GetValue();
        EXPECT_EQ(lowPowerModeSensitivity.step, 70);
        EXPECT_TRUE(lowPowerModeSensitivity.label.HasValue());
        EXPECT_TRUE(lowPowerModeSensitivity.label.Value().data_equal("120 Minutes"_span));
        ASSERT_TRUE(it.Next());
        lowPowerModeSensitivity = it.GetValue();
        EXPECT_EQ(lowPowerModeSensitivity.step, 100);
        EXPECT_TRUE(lowPowerModeSensitivity.label.HasValue());
        EXPECT_TRUE(lowPowerModeSensitivity.label.Value().data_equal("Never"_span));
        EXPECT_FALSE(it.Next());
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestEnergyPreferenceCluster, TestReadWriteAttributeAndBounds)
{
    EnergyPreferenceCluster cluster(kTestEndpointId, bothFeatures);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // currentEnergyBalance
    // In bounds
    size_t i = 0;
    for (; i < testDelegate.GetNumEnergyBalances(); ++i)
    {
        CurrentEnergyBalance::TypeInfo::Type currentEnergyBalanceWrite = i;
        ASSERT_EQ(tester.WriteAttribute(CurrentEnergyBalance::Id, currentEnergyBalanceWrite), CHIP_NO_ERROR);

        CurrentEnergyBalance::TypeInfo::DecodableType currentEnergyBalanceRead{};
        ASSERT_EQ(tester.ReadAttribute(CurrentEnergyBalance::Id, currentEnergyBalanceRead), CHIP_NO_ERROR);
        EXPECT_EQ(currentEnergyBalanceRead, currentEnergyBalanceWrite);
    }

    // Out of bounds
    CurrentEnergyBalance::TypeInfo::Type currentEnergyBalanceWrite = i;
    EXPECT_EQ(tester.WriteAttribute(CurrentEnergyBalance::Id, currentEnergyBalanceWrite),
              Protocols::InteractionModel::Status::ConstraintError);

    // lowPowerModeSensitivity
    // In bounds
    i = 0;
    for (; i < testDelegate.GetNumLowPowerModeSensitivities(); ++i)
    {
        CurrentLowPowerModeSensitivity::TypeInfo::Type currentLowPowerModeSensitivityWrite = i;
        ASSERT_EQ(tester.WriteAttribute(CurrentLowPowerModeSensitivity::Id, currentLowPowerModeSensitivityWrite), CHIP_NO_ERROR);

        CurrentLowPowerModeSensitivity::TypeInfo::DecodableType currentLowPowerModeSensitivityRead{};
        ASSERT_EQ(tester.ReadAttribute(CurrentLowPowerModeSensitivity::Id, currentLowPowerModeSensitivityRead), CHIP_NO_ERROR);
        EXPECT_EQ(currentLowPowerModeSensitivityRead, currentLowPowerModeSensitivityWrite);
    }

    // Out of bounds
    CurrentLowPowerModeSensitivity::TypeInfo::Type currentLowPowerModeSensitivityWrite = testDelegate.GetNumLowPowerModeSensitivities();
    EXPECT_EQ(tester.WriteAttribute(CurrentLowPowerModeSensitivity::Id, currentLowPowerModeSensitivityWrite), Protocols::InteractionModel::Status::InvalidInState);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestEnergyPreferenceCluster, TestSettersAndGetters)
{
    EnergyPreferenceCluster cluster(kTestEndpointId, bothFeatures);

    // currentEnergyBalance
    for (size_t i = 0; i < testDelegate.GetNumCurrentEnergyBalances(); ++i)
    {
        EXPECT_EQ(cluster.SetCurrentEnergyBalance(i), CHIP_NO_ERROR);
    }

    CurrentEnergyBalance::TypeInfo::DecodableType currentEnergyBalance = cluster.GetCurrentEnergyBalance();
    EXPECT_EQ(currentEnergyBalance, static_cast<uint8_t>(testDelegate.GetNumCurrentEnergyBalances() - 1));
    EXPECT_EQ(cluster.SetCurrentEnergyBalance(testDelegate.GetNumCurrentEnergyBalances()), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // lowPowerModeSensitivity
    for (size_t i = 0; i < testDelegate.GetNumCurrentLowPowerModeSensitivities(); ++i)
    {
        EXPECT_EQ(cluster.SetCurrentLowPowerModeSensitivity(i), CHIP_NO_ERROR);
    }

    CurrentLowPowerModeSensitivity::TypeInfo::DecodableType currentLowPowerModeSensitivity = cluster.GetCurrentLowPowerModeSensitivity();
    EXPECT_EQ(currentLowPowerModeSensitivity, static_cast<uint8_t>(testDelegate.GetNumCurrentLowPowerModeSensitivities() - 1));
    EXPECT_EQ(cluster.SetCurrentLowPowerModeSensitivity(testDelegate.GetNumCurrentLowPowerModeSensitivities()), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestEnergyPreferenceCluster, TestPersistence)
{
    CurrentEnergyBalance::TypeInfo::Type currentEnergyBalanceWrite = 1;
    CurrentLowPowerModeSensitivity::TypeInfo::Type currentLowPowerModeSensitivityWrite = 2;
    // Set the values
    {
        EnergyPreferenceCluster cluster(kTestEndpointId, bothFeatures);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ASSERT_EQ(cluster.SetCurrentEnergyBalance(currentEnergyBalanceWrite), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.SetCurrentLowPowerModeSensitivity(currentLowPowerModeSensitivityWrite), CHIP_NO_ERROR);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Read the values back after shutdown and startup
    {
        EnergyPreferenceCluster cluster(kTestEndpointId, bothFeatures);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        CurrentEnergyBalance::TypeInfo::DecodableType currentEnergyBalanceRead = cluster.GetCurrentEnergyBalance();
        EXPECT_EQ(currentEnergyBalanceRead, currentEnergyBalanceWrite);

        CurrentLowPowerModeSensitivity::TypeInfo::DecodableType currentLowPowerModeSensitivityRead = cluster.GetCurrentLowPowerModeSensitivity();
        EXPECT_EQ(currentLowPowerModeSensitivityRead, currentLowPowerModeSensitivityWrite);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}
