/*
 *
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

#include "PowerSourceClusterTestCommon.h"

#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/PowerSource/Metadata.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerSource;
using namespace chip::app::Clusters::PowerSource::Attributes;
using namespace chip::Testing;
using namespace chip::app::Clusters::PowerSource::TestSupport;

struct TestFullBatteryPowerSourceCluster : public TestBase
{
};

TEST_F(TestFullBatteryPowerSourceCluster, AttributeTest)
{
    FullBatteryPowerSourceConfig config(CharSpan{}, BatReplaceabilityEnum::kUnspecified, timerDelegate);
    config.MakeReplaceable(CharSpan{}, 0);
    config.MakeRechargeable();
    FullBatteryPowerSourceCluster cluster(kTestEndpointId, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        { Status::kMetadataEntry,
                                          Order::kMetadataEntry,
                                          Description::kMetadataEntry,
                                          BatVoltage::kMetadataEntry,
                                          BatPercentRemaining::kMetadataEntry,
                                          BatTimeRemaining::kMetadataEntry,
                                          BatChargeLevel::kMetadataEntry,
                                          BatReplacementNeeded::kMetadataEntry,
                                          BatReplaceability::kMetadataEntry,
                                          BatPresent::kMetadataEntry,
                                          ActiveBatFaults::kMetadataEntry,
                                          BatReplacementDescription::kMetadataEntry,
                                          BatCommonDesignation::kMetadataEntry,
                                          BatANSIDesignation::kMetadataEntry,
                                          BatIECDesignation::kMetadataEntry,
                                          BatApprovedChemistry::kMetadataEntry,
                                          BatCapacity::kMetadataEntry,
                                          BatQuantity::kMetadataEntry,
                                          BatChargeState::kMetadataEntry,
                                          BatTimeToFullCharge::kMetadataEntry,
                                          BatFunctionalWhileCharging::kMetadataEntry,
                                          BatChargingCurrent::kMetadataEntry,
                                          ActiveBatChargeFaults::kMetadataEntry,
                                          EndpointList::kMetadataEntry }));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestFullBatteryPowerSourceCluster, ReadAttributeTest)
{
    FullBatteryPowerSourceConfig config(CharSpan{}, BatReplaceabilityEnum::kUnspecified, timerDelegate);
    config.MakeReplaceable(CharSpan{}, 0);
    config.MakeRechargeable();
    FullBatteryPowerSourceCluster cluster(kTestEndpointId, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    ReadAttribute<Status::TypeInfo>(tester);
    ReadAttribute<Order::TypeInfo>(tester);
    ReadAttribute<Description::TypeInfo>(tester);
    ReadAttribute<BatVoltage::TypeInfo>(tester);
    ReadAttribute<BatPercentRemaining::TypeInfo>(tester);
    ReadAttribute<BatTimeRemaining::TypeInfo>(tester);
    ReadAttribute<BatChargeLevel::TypeInfo>(tester);
    ReadAttribute<BatReplacementNeeded::TypeInfo>(tester);
    ReadAttribute<BatReplaceability::TypeInfo>(tester);
    ReadAttribute<BatPresent::TypeInfo>(tester);
    ReadAttribute<ActiveBatFaults::TypeInfo>(tester);
    ReadAttribute<BatReplacementDescription::TypeInfo>(tester);
    ReadAttribute<BatCommonDesignation::TypeInfo>(tester);
    ReadAttribute<BatANSIDesignation::TypeInfo>(tester);
    ReadAttribute<BatIECDesignation::TypeInfo>(tester);
    ReadAttribute<BatApprovedChemistry::TypeInfo>(tester);
    ReadAttribute<BatCapacity::TypeInfo>(tester);
    ReadAttribute<BatQuantity::TypeInfo>(tester);
    ReadAttribute<BatChargeState::TypeInfo>(tester);
    ReadAttribute<BatTimeToFullCharge::TypeInfo>(tester);
    ReadAttribute<BatFunctionalWhileCharging::TypeInfo>(tester);
    ReadAttribute<BatChargingCurrent::TypeInfo>(tester);
    ReadAttribute<ActiveBatChargeFaults::TypeInfo>(tester);
    ReadAttribute<EndpointList::TypeInfo>(tester);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestFullBatteryPowerSourceCluster, TestGetters)
{
    FullBatteryPowerSourceConfig config(CharSpan{}, BatReplaceabilityEnum::kUnspecified, timerDelegate);
    config.MakeReplaceable(CharSpan{}, 0);
    config.MakeRechargeable();
    FullBatteryPowerSourceCluster cluster(kTestEndpointId, config);

    cluster.GetStatus();
    cluster.GetOrder();
    cluster.GetDescription();
    cluster.GetBatVoltage();
    cluster.GetBatPercentRemaining();
    cluster.GetBatTimeRemaining();
    cluster.GetBatChargeLevel();
    cluster.GetBatReplacementNeeded();
    cluster.GetBatReplaceability();
    cluster.GetBatPresent();
    // this will do nothing, because if the buffer is smaller than needed, it will be filled as much as possible.
    auto noBuf = Span<BatFaultEnum>{};
    cluster.GetActiveBatFaults(noBuf);
    cluster.GetBatReplacementDescription();
    cluster.GetBatCommonDesignation();
    cluster.GetBatANSIDesignation();
    cluster.GetBatIECDesignation();
    cluster.GetBatApprovedChemistry();
    cluster.GetBatCapacity();
    cluster.GetBatQuantity();
    cluster.GetBatChargeState();
    cluster.GetBatTimeToFullCharge();
    cluster.GetBatFunctionalWhileCharging();
    cluster.GetBatChargingCurrent();
    // this will do nothing, because if the buffer is smaller than needed, it will be filled as much as possible.
    auto noChargeBuf = Span<BatChargeFaultEnum>{};
    cluster.GetActiveBatChargeFaults(noChargeBuf);
    cluster.GetEndpointList();
}

TEST_F(TestFullBatteryPowerSourceCluster, TestSetters)
{
    FullBatteryPowerSourceConfig config(CharSpan{}, BatReplaceabilityEnum::kUnspecified, timerDelegate);
    config.MakeReplaceable(CharSpan{}, 0);
    config.MakeRechargeable();
    FullBatteryPowerSourceCluster cluster(kTestEndpointId, config);

    EXPECT_EQ(cluster.SetStatus({}), CHIP_NO_ERROR);
    cluster.SetOrder({});
    cluster.SetBatVoltage({});
    EXPECT_EQ(cluster.SetBatPercentRemaining({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatTimeRemaining({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatChargeLevel({}), CHIP_NO_ERROR);
    cluster.SetBatReplacementNeeded({});
    cluster.SetBatPresent({});
    cluster.SetActiveBatFaults({});
    EXPECT_EQ(cluster.SetBatChargeState({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatTimeToFullCharge({}), CHIP_NO_ERROR);
    cluster.SetBatFunctionalWhileCharging({});
    cluster.SetBatChargingCurrent({});
    cluster.SetActiveBatChargeFaults({});
    EXPECT_EQ(cluster.SetEndpointList({}), CHIP_NO_ERROR);
}

TEST_F(TestFullBatteryPowerSourceCluster, TestBounds)
{
    CharSpan longTestText =
        "Very very long text used for descriptions and designations, totally longer than one hundred bytes. For testing purposes"_span;

    FullBatteryPowerSourceConfig config(longTestText, BatReplaceabilityEnum::kUnspecified, timerDelegate);
    config.MakeReplaceable(longTestText, 0);
    config.MakeRechargeable();
    config.batANSIDesignation = longTestText;
    config.batIECDesignation  = longTestText;

    FullBatteryPowerSourceCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    TestStringAttributeReadLength<Description::TypeInfo>(tester);
    TestStringAttributeReadLength<BatReplacementDescription::TypeInfo>(tester);
    TestStringAttributeReadLength<BatANSIDesignation::TypeInfo>(tester);
    TestStringAttributeReadLength<BatIECDesignation::TypeInfo>(tester);

    // Test BatPercentRemaining, which can be only from 0 to 200, or null.

    EXPECT_EQ(cluster.SetBatPercentRemaining(std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatPercentRemaining(uint8_t{ 0 }), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatPercentRemaining(uint8_t{ 50 }), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatPercentRemaining(uint8_t{ 150 }), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatPercentRemaining(uint8_t{ 200 }), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatPercentRemaining(uint8_t{ 201 }), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

} // namespace
