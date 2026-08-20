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

#include <app/clusters/electrical-alarm-server/ElectricalAlarmCluster.h>
#include <app/clusters/electrical-alarm-server/tests/MockElectricalAlarmDelegate.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/ElectricalAlarm/Attributes.h>
#include <clusters/ElectricalAlarm/Metadata.h>

#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalAlarm;
using namespace chip::app::Clusters::ElectricalAlarm::Attributes;
using namespace chip::Testing;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr EndpointId kTestEndpointId = 1;

// Helper: build a cluster for a given feature set.
ElectricalAlarmCluster MakeCluster(MockElectricalAlarmDelegate & delegate, BitMask<Feature> features)
{
    return ElectricalAlarmCluster(ElectricalAlarmCluster::Config{
        .endpointId = kTestEndpointId,
        .delegate   = delegate,
        .features   = features,
    });
}

struct TestElectricalAlarmCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

// ---------------------------------------------------------------------------
// AttributeList / feature gating
// ---------------------------------------------------------------------------

TEST_F(TestElectricalAlarmCluster, NoFeaturesAttributeList)
{
    TestServerClusterContext context;
    MockElectricalAlarmDelegate delegate;
    auto cluster = MakeCluster(delegate, BitMask<Feature>());

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    EXPECT_TRUE(IsAttributesListEqualTo(
        cluster, { Attributes::Mask::kMetadataEntry, Attributes::State::kMetadataEntry, Attributes::Supported::kMetadataEntry }));
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalAlarmCluster, ResetFeatureAddsLatch)
{
    TestServerClusterContext context;
    MockElectricalAlarmDelegate delegate;
    auto cluster = MakeCluster(delegate, BitMask<Feature>(Feature::kReset));

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    EXPECT_TRUE(IsAttributesListEqualTo(
        cluster, { Attributes::Mask::kMetadataEntry, Attributes::State::kMetadataEntry, Attributes::Supported::kMetadataEntry,
                   Attributes::Latch::kMetadataEntry }));
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalAlarmCluster, OverVoltageFeatureAddsThreshold)
{
    TestServerClusterContext context;
    MockElectricalAlarmDelegate delegate;
    auto cluster = MakeCluster(delegate, BitMask<Feature>(Feature::kOverVoltage));

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    EXPECT_TRUE(IsAttributesListEqualTo(
        cluster, { Attributes::Mask::kMetadataEntry, Attributes::State::kMetadataEntry, Attributes::Supported::kMetadataEntry,
                   Attributes::OverVoltageThreshold::kMetadataEntry }));
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalAlarmCluster, AllFeaturesAttributeList)
{
    TestServerClusterContext context;
    MockElectricalAlarmDelegate delegate;
    BitMask<Feature> all(Feature::kReset, Feature::kAdjustableThresholds, Feature::kOverVoltage, Feature::kUnderVoltage,
                         Feature::kOverFrequency, Feature::kUnderFrequency, Feature::kOverPower, Feature::kUnderPower,
                         Feature::kOverCurrent, Feature::kUnderCurrent, Feature::kPowerImport, Feature::kPowerExport);
    auto cluster = MakeCluster(delegate, all);

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    EXPECT_TRUE(IsAttributesListEqualTo(
        cluster, { Attributes::Mask::kMetadataEntry, Attributes::State::kMetadataEntry, Attributes::Supported::kMetadataEntry,
                   Attributes::Latch::kMetadataEntry, Attributes::OverVoltageThreshold::kMetadataEntry,
                   Attributes::UnderVoltageThreshold::kMetadataEntry, Attributes::OverFrequencyThreshold::kMetadataEntry,
                   Attributes::UnderFrequencyThreshold::kMetadataEntry, Attributes::OverPowerThreshold::kMetadataEntry,
                   Attributes::UnderPowerThreshold::kMetadataEntry, Attributes::OverCurrentThreshold::kMetadataEntry,
                   Attributes::UnderCurrentThreshold::kMetadataEntry, Attributes::PowerImportThreshold::kMetadataEntry,
                   Attributes::PowerExportThreshold::kMetadataEntry }));
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------------------------------------------------------------------------
// Startup validation
// ---------------------------------------------------------------------------

TEST_F(TestElectricalAlarmCluster, AdjustableThresholdsWithoutAlarmClassFails)
{
    TestServerClusterContext context;
    MockElectricalAlarmDelegate delegate;
    // ADJUST alone (no alarm-class feature) must fail.
    auto cluster = MakeCluster(delegate, BitMask<Feature>(Feature::kAdjustableThresholds));

    EXPECT_NE(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalAlarmCluster, AdjustableThresholdsWithAlarmClassSucceeds)
{
    TestServerClusterContext context;
    MockElectricalAlarmDelegate delegate;
    auto cluster = MakeCluster(delegate, BitMask<Feature>(Feature::kAdjustableThresholds, Feature::kOverVoltage));

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------------------------------------------------------------------------
// Attribute reads
// ---------------------------------------------------------------------------

TEST_F(TestElectricalAlarmCluster, ReadMandatoryAttributes)
{
    TestServerClusterContext context;
    MockElectricalAlarmDelegate delegate;
    auto cluster = MakeCluster(delegate, BitMask<Feature>());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    BitMask<AlarmBitmap> mask, state, supported;
    EXPECT_TRUE(tester.ReadAttribute(Mask::Id, mask).IsSuccess());
    EXPECT_TRUE(tester.ReadAttribute(State::Id, state).IsSuccess());
    EXPECT_TRUE(tester.ReadAttribute(Supported::Id, supported).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalAlarmCluster, LatchUnsupportedWithoutReset)
{
    TestServerClusterContext context;
    MockElectricalAlarmDelegate delegate;
    auto cluster = MakeCluster(delegate, BitMask<Feature>());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    BitMask<AlarmBitmap> latch;
    auto status = tester.ReadAttribute(Latch::Id, latch);
    EXPECT_FALSE(status.IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalAlarmCluster, ThresholdUnsupportedWithoutFeature)
{
    TestServerClusterContext context;
    MockElectricalAlarmDelegate delegate;
    auto cluster = MakeCluster(delegate, BitMask<Feature>());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    int64_t val;
    EXPECT_FALSE(tester.ReadAttribute(OverVoltageThreshold::Id, val).IsSuccess());
    EXPECT_FALSE(tester.ReadAttribute(UnderVoltageThreshold::Id, val).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalAlarmCluster, ThresholdReadWithFeature)
{
    TestServerClusterContext context;
    MockElectricalAlarmDelegate delegate;
    auto cluster = MakeCluster(delegate, BitMask<Feature>(Feature::kOverVoltage));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetOverVoltageThreshold(2500), Status::Success);

    ClusterTester tester(cluster);
    int64_t val = 0;
    EXPECT_TRUE(tester.ReadAttribute(OverVoltageThreshold::Id, val).IsSuccess());
    EXPECT_EQ(val, 2500);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------------------------------------------------------------------------
// SetSupportedValue cascade
// ---------------------------------------------------------------------------

TEST_F(TestElectricalAlarmCluster, SetSupportedNarrowsMask)
{
    TestServerClusterContext context;
    MockElectricalAlarmDelegate delegate;
    auto cluster = MakeCluster(delegate, BitMask<Feature>());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    BitMask<AlarmBitmap> supported(AlarmBitmap::kOverVoltage, AlarmBitmap::kUnderVoltage);
    EXPECT_EQ(cluster.SetSupportedValue(supported), Status::Success);

    // Set a mask that includes only supported bits.
    BitMask<AlarmBitmap> mask(AlarmBitmap::kOverVoltage, AlarmBitmap::kUnderVoltage);
    EXPECT_EQ(cluster.SetMaskValue(mask), Status::Success);

    // Remove kUnderVoltage from Supported — Mask must narrow too.
    BitMask<AlarmBitmap> narrowedSupported(AlarmBitmap::kOverVoltage);
    EXPECT_EQ(cluster.SetSupportedValue(narrowedSupported), Status::Success);
    EXPECT_EQ(cluster.GetMask(), narrowedSupported);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalAlarmCluster, SetMaskNarrowsState)
{
    TestServerClusterContext context;
    MockElectricalAlarmDelegate delegate;
    auto cluster = MakeCluster(delegate, BitMask<Feature>());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    BitMask<AlarmBitmap> supported(AlarmBitmap::kOverVoltage, AlarmBitmap::kUnderVoltage);
    EXPECT_EQ(cluster.SetSupportedValue(supported), Status::Success);

    BitMask<AlarmBitmap> mask(AlarmBitmap::kOverVoltage, AlarmBitmap::kUnderVoltage);
    EXPECT_EQ(cluster.SetMaskValue(mask), Status::Success);

    // Activate both alarms in State.
    BitMask<AlarmBitmap> state(AlarmBitmap::kOverVoltage, AlarmBitmap::kUnderVoltage);
    EXPECT_EQ(cluster.SetStateValue(state), Status::Success);

    // Narrow Mask to just kOverVoltage — State must drop kUnderVoltage.
    BitMask<AlarmBitmap> narrowMask(AlarmBitmap::kOverVoltage);
    EXPECT_EQ(cluster.SetMaskValue(narrowMask), Status::Success);
    EXPECT_EQ(cluster.GetState(), narrowMask);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------------------------------------------------------------------------
// SetStateValue latch preservation
// ---------------------------------------------------------------------------

TEST_F(TestElectricalAlarmCluster, LatchedBitsPreserved)
{
    TestServerClusterContext context;
    MockElectricalAlarmDelegate delegate;
    auto cluster = MakeCluster(delegate, BitMask<Feature>(Feature::kReset));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    BitMask<AlarmBitmap> supported(AlarmBitmap::kOverVoltage, AlarmBitmap::kUnderVoltage);
    EXPECT_EQ(cluster.SetSupportedValue(supported), Status::Success);
    EXPECT_EQ(cluster.SetMaskValue(supported), Status::Success);

    // Latch kOverVoltage so it sticks once active.
    BitMask<AlarmBitmap> latch(AlarmBitmap::kOverVoltage);
    EXPECT_EQ(cluster.SetLatchValue(latch), Status::Success);

    // Activate kOverVoltage.
    BitMask<AlarmBitmap> both(AlarmBitmap::kOverVoltage, AlarmBitmap::kUnderVoltage);
    EXPECT_EQ(cluster.SetStateValue(both), Status::Success);

    // Now try to clear everything — kOverVoltage must stay because it's latched+active.
    BitMask<AlarmBitmap> empty;
    EXPECT_EQ(cluster.SetStateValue(empty), Status::Success);
    EXPECT_TRUE(cluster.GetState().Has(AlarmBitmap::kOverVoltage));
    EXPECT_FALSE(cluster.GetState().Has(AlarmBitmap::kUnderVoltage));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalAlarmCluster, ResetClearsLatchedAlarms)
{
    TestServerClusterContext context;
    MockElectricalAlarmDelegate delegate;
    auto cluster = MakeCluster(delegate, BitMask<Feature>(Feature::kReset));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    BitMask<AlarmBitmap> supported(AlarmBitmap::kOverVoltage);
    EXPECT_EQ(cluster.SetSupportedValue(supported), Status::Success);
    EXPECT_EQ(cluster.SetMaskValue(supported), Status::Success);
    EXPECT_EQ(cluster.SetLatchValue(supported), Status::Success);
    EXPECT_EQ(cluster.SetStateValue(supported), Status::Success);

    // Explicitly reset the latched alarm.
    EXPECT_EQ(cluster.ResetLatchedAlarms(supported), Status::Success);
    EXPECT_FALSE(cluster.GetState().Has(AlarmBitmap::kOverVoltage));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------------------------------------------------------------------------
// FeatureMap read
// ---------------------------------------------------------------------------

TEST_F(TestElectricalAlarmCluster, FeatureMapRead)
{
    TestServerClusterContext context;
    MockElectricalAlarmDelegate delegate;
    BitMask<Feature> features(Feature::kReset, Feature::kOverVoltage);
    auto cluster = MakeCluster(delegate, features);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    uint32_t featureMapValue = 0;
    EXPECT_TRUE(tester.ReadAttribute(FeatureMap::Id, featureMapValue).IsSuccess());
    EXPECT_EQ(featureMapValue, features.Raw());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------------------------------------------------------------------------
// Threshold setter guard
// ---------------------------------------------------------------------------

TEST_F(TestElectricalAlarmCluster, ThresholdSetterRejectsWithoutFeature)
{
    TestServerClusterContext context;
    MockElectricalAlarmDelegate delegate;
    auto cluster = MakeCluster(delegate, BitMask<Feature>());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetOverVoltageThreshold(100), Status::UnsupportedAttribute);
    EXPECT_EQ(cluster.SetUnderVoltageThreshold(-100), Status::UnsupportedAttribute);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
