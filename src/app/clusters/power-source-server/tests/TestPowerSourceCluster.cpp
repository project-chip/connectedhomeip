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

#include <app/clusters/power-source-server/PowerSourceCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/PowerSource/Metadata.h>
#include <include/platform/CHIPDeviceLayer.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerSource;
using namespace chip::app::Clusters::PowerSource::Attributes;
using namespace chip::Testing;

struct TestPowerSourceCluster : public ::testing::Test
{
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        DeviceLayer::PlatformMgr().Shutdown();
        chip::Platform::MemoryShutdown();
    }

    TestPowerSourceCluster() {}

    TestServerClusterContext testContext;

    void TestReadAllAttributes(ClusterTester & tester, bool battery, bool replaceable = false, bool rechargeable = false)
    {
#define Read(attr_name)                                                                                                            \
    {                                                                                                                              \
        attr_name::TypeInfo::DecodableType val;                                                                                    \
        EXPECT_EQ(tester.ReadAttribute(attr_name::Id, val), CHIP_NO_ERROR);                                                        \
    }

        Read(Status);
        Read(Order);
        Read(Description);
        Read(EndpointList);

        if (!battery)
        {
            Read(WiredAssessedInputVoltage);
            Read(WiredAssessedInputFrequency);
            Read(WiredCurrentType);
            Read(WiredAssessedCurrent);
            Read(WiredNominalVoltage);
            Read(WiredMaximumCurrent);
            Read(WiredPresent);
            Read(ActiveWiredFaults);
        }

        if (battery)
        {
            Read(BatVoltage);
            Read(BatPercentRemaining);
            Read(BatTimeRemaining);
            Read(BatChargeLevel);
            Read(BatReplacementNeeded);
            Read(BatReplaceability);
            Read(BatPresent);
            Read(ActiveBatFaults);

            if (replaceable)
            {
                Read(BatReplacementDescription);
                Read(BatCommonDesignation);
                Read(BatANSIDesignation);
                Read(BatIECDesignation);
                Read(BatApprovedChemistry);
                Read(BatQuantity);
            }

            if (rechargeable)
            {
                Read(BatChargeState);
                Read(BatTimeToFullCharge);
                Read(BatFunctionalWhileCharging);
                Read(BatChargingCurrent);
                Read(ActiveBatChargeFaults);
            }

            if (replaceable || rechargeable)
            {
                Read(BatCapacity);
            }
        }
#undef Read
    }

    void RunAllSupportedGetters(const PowerSourceCluster & cluster, bool battery, bool replaceable = false,
                                bool rechargeable = false)
    {
        cluster.GetStatus();
        cluster.GetOrder();
        cluster.GetDescription();
        cluster.GetEndpointList();

        if (!battery)
        {
            cluster.GetWiredAssessedInputVoltage();
            cluster.GetWiredAssessedInputFrequency();
            cluster.GetWiredCurrentType();
            cluster.GetWiredAssessedCurrent();
            cluster.GetWiredNominalVoltage();
            cluster.GetWiredMaximumCurrent();
            cluster.GetWiredPresent();
            cluster.GetActiveWiredFaults();
        }

        if (battery)
        {
            cluster.GetBatVoltage();
            cluster.GetBatPercentRemaining();
            cluster.GetBatTimeRemaining();
            cluster.GetBatChargeLevel();
            cluster.GetBatReplacementNeeded();
            cluster.GetBatReplaceability();
            cluster.GetBatPresent();
            cluster.GetActiveBatFaults();

            if (replaceable)
            {
                cluster.GetBatReplacementDescription();
                cluster.GetBatCommonDesignation();
                cluster.GetBatANSIDesignation();
                cluster.GetBatIECDesignation();
                cluster.GetBatApprovedChemistry();
                cluster.GetBatQuantity();
            }

            if (rechargeable)
            {
                cluster.GetBatChargeState();
                cluster.GetBatTimeToFullCharge();
                cluster.GetBatFunctionalWhileCharging();
                cluster.GetBatChargingCurrent();
                cluster.GetActiveBatChargeFaults();
            }

            if (replaceable || rechargeable)
            {
                cluster.GetBatCapacity();
            }
        }
    }
};

TEST_F(TestPowerSourceCluster, AttributeTest)
{
    // Wired
    {
        PowerSourceCluster::WiredConfiguration config(CharSpan{}, WiredCurrentTypeEnum::kAc);
        PowerSourceCluster cluster(kRootEndpointId, DeviceLayer::SystemLayer(), config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(
            cluster,
            { // Mandatory
              Status::kMetadataEntry, Order::kMetadataEntry, Description::kMetadataEntry, EndpointList::kMetadataEntry,
              // Wired specific
              WiredAssessedInputVoltage::kMetadataEntry, WiredAssessedInputFrequency::kMetadataEntry,
              WiredCurrentType::kMetadataEntry, WiredAssessedCurrent::kMetadataEntry, WiredNominalVoltage::kMetadataEntry,
              WiredMaximumCurrent::kMetadataEntry, WiredPresent::kMetadataEntry, ActiveWiredFaults::kMetadataEntry }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Battery
    for (bool replaceable : { false, true })
    {
        for (bool rechargeable : { false, true })
        {
            PowerSourceCluster::BatteryConfiguration config(CharSpan{}, BatReplaceabilityEnum::kUnspecified);
            if (replaceable)
            {
                config.MakeReplaceable(CharSpan{}, 0);
            }
            if (rechargeable)
            {
                config.MakeRechargeable();
            }
            PowerSourceCluster cluster(kRootEndpointId, DeviceLayer::SystemLayer(), config);
            ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

            ReadOnlyBufferBuilder<const DataModel::AttributeEntry> expected;
            ASSERT_EQ(expected.EnsureAppendCapacity(32), CHIP_NO_ERROR); // attribute count of this cluster
            for (const auto & el :
                 { // Mandatory
                   Status::kMetadataEntry, Order::kMetadataEntry, Description::kMetadataEntry, EndpointList::kMetadataEntry,
                   // Battery specific
                   BatVoltage::kMetadataEntry, BatPercentRemaining::kMetadataEntry, BatTimeRemaining::kMetadataEntry,
                   BatChargeLevel::kMetadataEntry, BatReplacementNeeded::kMetadataEntry, BatReplaceability::kMetadataEntry,
                   BatPresent::kMetadataEntry, ActiveBatFaults::kMetadataEntry })
            {
                ASSERT_EQ(expected.Append(el), CHIP_NO_ERROR);
            }

            if (replaceable)
            {
                for (const auto & el : { // replaceable battery specific
                                         BatReplacementDescription::kMetadataEntry, BatCommonDesignation::kMetadataEntry,
                                         BatANSIDesignation::kMetadataEntry, BatIECDesignation::kMetadataEntry,
                                         BatApprovedChemistry::kMetadataEntry, BatQuantity::kMetadataEntry })
                {
                    ASSERT_EQ(expected.Append(el), CHIP_NO_ERROR);
                }
            }
            if (rechargeable)
            {
                for (const auto & el : { // rechargeable battery specific
                                         BatChargeState::kMetadataEntry, BatTimeToFullCharge::kMetadataEntry,
                                         BatFunctionalWhileCharging::kMetadataEntry, BatChargingCurrent::kMetadataEntry,
                                         ActiveBatChargeFaults::kMetadataEntry })
                {
                    ASSERT_EQ(expected.Append(el), CHIP_NO_ERROR);
                }
            }
            if (replaceable || rechargeable)
            {
                ASSERT_EQ(expected.Append(BatCapacity::kMetadataEntry), CHIP_NO_ERROR);
            }
            EXPECT_TRUE(IsAttributesListEqualTo(cluster, expected.TakeBuffer()));

            cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        }
    }
}

TEST_F(TestPowerSourceCluster, ReadAttributeTest)
{
    // Wired
    {
        PowerSourceCluster::WiredConfiguration config(CharSpan{}, WiredCurrentTypeEnum::kDc);
        PowerSourceCluster cluster(kRootEndpointId, DeviceLayer::SystemLayer(), config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);
        TestReadAllAttributes(tester, false);
        EXPECT_FALSE(HasFailure());

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Battery
    for (bool replaceable : { false, true })
    {
        for (bool rechargeable : { false, true })
        {
            PowerSourceCluster::BatteryConfiguration config(CharSpan{}, BatReplaceabilityEnum::kUnspecified);
            if (replaceable)
            {
                config.MakeReplaceable(CharSpan{}, 0);
            }
            if (rechargeable)
            {
                config.MakeRechargeable();
            }
            PowerSourceCluster cluster(kRootEndpointId, DeviceLayer::SystemLayer(), config);
            ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

            ClusterTester tester(cluster);
            TestReadAllAttributes(tester, true, replaceable, rechargeable);
            EXPECT_FALSE(HasFailure());

            cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        }
    }
}

TEST_F(TestPowerSourceCluster, TestSetters)
{
    // Rechargeable battery
    PowerSourceCluster::BatteryConfiguration config(CharSpan{}, BatReplaceabilityEnum::kUnspecified);
    config.MakeRechargeable();

    PowerSourceCluster cluster(kRootEndpointId, optSet, DeviceLayer::SystemLayer(), config);

    // These are mandatory, so always supported
    EXPECT_EQ(cluster.SetStatus({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetOrder({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetEndpointList({}), CHIP_NO_ERROR);

    // These are wired feature only so they are unsupported for this cluster
    EXPECT_EQ(cluster.SetWiredAssessedInputVoltage({}), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.SetWiredAssessedInputFrequency({}), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.SetWiredAssessedCurrent({}), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.SetWiredPresent({}), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.SetActiveWiredFaults({}), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.AddActiveWiredFault({}), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.RemoveActiveWiredFault({}), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    // These are supported when battery feature is set
    EXPECT_EQ(cluster.SetBatChargeLevel({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatReplacementNeeded({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatVoltage({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatPercentRemaining({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatTimeRemaining({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetActiveBatFaults({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.AddActiveBatFault({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.RemoveActiveBatFault({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatPresent({}), CHIP_NO_ERROR);

    // These are supported when rechargeable feature is set besides battery
    EXPECT_EQ(cluster.SetBatChargeState({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatFunctionalWhileCharging({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatTimeToFullCharge({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatChargingCurrent({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetActiveBatChargeFaults({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.AddActiveBatChargeFault({}), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.RemoveActiveBatChargeFault({}), CHIP_NO_ERROR);
}

TEST_F(TestPowerSourceCluster, TestGetters)
{
    // Wired
    {
        PowerSourceCluster::WiredConfiguration config(CharSpan{}, WiredCurrentTypeEnum::kDc);
        PowerSourceCluster cluster(kRootEndpointId, DeviceLayer::SystemLayer(), config);

        // if a getter is called when not supported, the test will die.
        RunAllSupportedGetters(cluster, false);
    }

    // Battery
    for (bool replaceable : { false, true })
    {
        for (bool rechargeable : { false, true })
        {
            PowerSourceCluster::BatteryConfiguration config(CharSpan{}, BatReplaceabilityEnum::kUnspecified);
            if (replaceable)
            {
                config.MakeReplaceable(CharSpan{}, 0);
            }
            if (rechargeable)
            {
                config.MakeRechargeable();
            }
            PowerSourceCluster cluster(kRootEndpointId, DeviceLayer::SystemLayer(), config);

            // if a getter is called when not supported, the test will die.
            RunAllSupportedGetters(cluster, true, replaceable, rechargeable);
        }
    }
}

TEST_F(TestPowerSourceCluster, TestSetterBounds)
{
    // Constraints on string sizes can't be tested for failing cases,
    // because they are set in the constructor which when failing will die.
    // So we can at least test that the maximum allowed length works.

    char longTestText[] =
        "Very very long text used for descriptions and designations, totally longer than one hundred bytes. For testing purposes";

    CharSpan description            = CharSpan(longTestText, Description::TypeInfo::MaxLength());
    CharSpan replacementDescription = CharSpan(longTestText, BatReplacementDescription::TypeInfo::MaxLength());
    CharSpan ansiDesignation        = CharSpan(longTestText, BatANSIDesignation::TypeInfo::MaxLength());
    CharSpan iecDesignation         = CharSpan(longTestText, BatIECDesignation::TypeInfo::MaxLength());

    PowerSourceCluster::BatteryConfiguration config(description, BatReplaceabilityEnum::kUnspecified);
    config.MakeReplaceable(replacementDescription, 0);
    config.ansiDesignation = ansiDesignation;
    config.iecDesignation  = iecDesignation;

    // This should not fail
    PowerSourceCluster cluster(kRootEndpointId, DeviceLayer::SystemLayer(), config);

    // Test BatPercentRemaining, which can be only from 0 to 200, or null.

    EXPECT_EQ(cluster.SetBatPercentRemaining(NullOptional), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatPercentRemaining(MakeOptional(uint8_t{ 0 })), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatPercentRemaining(MakeOptional(uint8_t{ 50 })), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatPercentRemaining(MakeOptional(uint8_t{ 150 })), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetBatPercentRemaining(MakeOptional(uint8_t{ 200 })), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetBatPercentRemaining(MakeOptional(uint8_t{ 201 })), CHIP_ERROR_INVALID_INTEGER_VALUE);
}

TEST_F(TestPowerSourceCluster, TestPersistence)
{
    // Order attribute is marked with `Persistent` marker.

    uint8_t testOrder = 3;

    PowerSourceCluster::WiredConfiguration config(CharSpan(), WiredCurrentTypeEnum::kAc);

    {
        PowerSourceCluster cluster(kRootEndpointId, DeviceLayer::SystemLayer(), config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        EXPECT_EQ(cluster.SetOrder(testOrder), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetOrder(), testOrder);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        PowerSourceCluster cluster(kRootEndpointId, DeviceLayer::SystemLayer(), config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        uint8_t readOrder{};
        ASSERT_EQ(tester.ReadAttribute(Order::Id, readOrder), CHIP_NO_ERROR);
        EXPECT_EQ(readOrder, testOrder);
        EXPECT_EQ(cluster.GetOrder(), testOrder);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

} // namespace
