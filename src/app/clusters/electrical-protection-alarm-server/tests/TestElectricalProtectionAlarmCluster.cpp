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
#include <pw_unit_test/framework.h>

#include <app/clusters/electrical-protection-alarm-server/ElectricalProtectionAlarmCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/ElectricalProtectionAlarm/Attributes.h>
#include <clusters/ElectricalProtectionAlarm/Enums.h>
#include <clusters/ElectricalProtectionAlarm/Metadata.h>
#include <clusters/ElectricalProtectionAlarm/Structs.h>
#include <lib/support/BitMask.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalProtectionAlarm;
using namespace chip::app::Clusters::ElectricalProtectionAlarm::Attributes;
using namespace chip::Testing;
using chip::Testing::IsAttributesListEqualTo;

namespace {

constexpr EndpointId kEndpoint = 1;

BitMask<AlarmBitmap> AllAlarms()
{
    BitMask<AlarmBitmap> bits;
    bits.Set(AlarmBitmap::kShortCircuitFault)
        .Set(AlarmBitmap::kOverLoadFault)
        .Set(AlarmBitmap::kOverVoltageFault)
        .Set(AlarmBitmap::kVoltageSurgeFault)
        .Set(AlarmBitmap::kResidualCurrentFault)
        .Set(AlarmBitmap::kArcFault)
        .Set(AlarmBitmap::kSelfTest);
    return bits;
}

// A config that enables every feature (Supported == all 7 bits), no active alarms, one populated
// rating attribute (ShortCircuitRating) to exercise struct encoding; other ratings null.
ElectricalProtectionAlarmCluster::StartupConfiguration AllFeaturesConfig()
{
    ElectricalProtectionAlarmCluster::StartupConfiguration config;
    config.featureMap.Set(Feature::kShortCircuit)
        .Set(Feature::kOverLoad)
        .Set(Feature::kOverVoltage)
        .Set(Feature::kSurgeProtection)
        .Set(Feature::kResidualCurrent)
        .Set(Feature::kArcFault)
        .Set(Feature::kSelfTest);
    config.supported = AllAlarms();
    config.mask      = AllAlarms();
    // config.state defaults to 0 (no active alarms).

    Structs::ShortCircuitRatingsStruct::Type scr;
    scr.tripCurrent.SetValue(50000); // 50 A in mA
    config.shortCircuitRating = DataModel::MakeNullable(scr);
    return config;
}

struct TestElectricalProtectionAlarmCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

} // namespace

TEST_F(TestElectricalProtectionAlarmCluster, AttributeList_AllFeatures)
{
    ElectricalProtectionAlarmCluster cluster(kEndpoint, AllFeaturesConfig());
    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Mask::kMetadataEntry,
                                            State::kMetadataEntry,
                                            Supported::kMetadataEntry,
                                            ArcCause::kMetadataEntry,
                                            OverLoadRating::kMetadataEntry,
                                            OverVoltageRating::kMetadataEntry,
                                            SurgeProtectionRating::kMetadataEntry,
                                            ShortCircuitRating::kMetadataEntry,
                                            ResidualCurrentRating::kMetadataEntry,
                                            ArcFaultRating::kMetadataEntry,
                                        }));
}

TEST_F(TestElectricalProtectionAlarmCluster, ReadAttributes)
{
    TestServerClusterContext context;
    ElectricalProtectionAlarmCluster cluster(kEndpoint, AllFeaturesConfig());
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);
    ASSERT_EQ(revision, ElectricalProtectionAlarm::kRevision);

    BitMask<AlarmBitmap> supported;
    ASSERT_EQ(tester.ReadAttribute(Supported::Id, supported), CHIP_NO_ERROR);
    ASSERT_EQ(supported.Raw(), AllAlarms().Raw());

    BitMask<AlarmBitmap> state;
    ASSERT_EQ(tester.ReadAttribute(State::Id, state), CHIP_NO_ERROR);
    ASSERT_EQ(state.Raw(), 0u);

    // Populated rating decodes with its field; an unset rating reads back null.
    ShortCircuitRating::TypeInfo::DecodableType shortCircuit;
    ASSERT_EQ(tester.ReadAttribute(ShortCircuitRating::Id, shortCircuit), CHIP_NO_ERROR);
    ASSERT_FALSE(shortCircuit.IsNull());
    ASSERT_TRUE(shortCircuit.Value().tripCurrent.HasValue());
    ASSERT_EQ(shortCircuit.Value().tripCurrent.Value(), 50000);

    OverLoadRating::TypeInfo::DecodableType overLoad;
    ASSERT_EQ(tester.ReadAttribute(OverLoadRating::Id, overLoad), CHIP_NO_ERROR);
    ASSERT_TRUE(overLoad.IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalProtectionAlarmCluster, AlarmStateMachine)
{
    TestServerClusterContext context;
    ElectricalProtectionAlarmCluster cluster(kEndpoint, AllFeaturesConfig());
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ASSERT_EQ(cluster.GetState().Raw(), 0u);

    BitMask<AlarmBitmap> ov;
    ov.Set(AlarmBitmap::kOverVoltageFault);
    ASSERT_EQ(cluster.ActivateAlarms(ov), CHIP_NO_ERROR);
    ASSERT_TRUE(cluster.GetState().Has(AlarmBitmap::kOverVoltageFault));

    BitMask<AlarmBitmap> arc;
    arc.Set(AlarmBitmap::kArcFault);
    ASSERT_EQ(cluster.ActivateAlarms(arc), CHIP_NO_ERROR);
    ASSERT_TRUE(cluster.GetState().Has(AlarmBitmap::kOverVoltageFault));
    ASSERT_TRUE(cluster.GetState().Has(AlarmBitmap::kArcFault));

    ASSERT_EQ(cluster.DeactivateAlarms(ov), CHIP_NO_ERROR);
    ASSERT_FALSE(cluster.GetState().Has(AlarmBitmap::kOverVoltageFault));
    ASSERT_TRUE(cluster.GetState().Has(AlarmBitmap::kArcFault));

    ASSERT_EQ(cluster.ClearAllAlarms(), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.GetState().Raw(), 0u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalProtectionAlarmCluster, UnsupportedAlarmBitsIgnored)
{
    // Only OverVoltage supported; activating an unsupported bit must not change State.
    ElectricalProtectionAlarmCluster::StartupConfiguration config;
    config.featureMap.Set(Feature::kOverVoltage);
    config.supported.Set(AlarmBitmap::kOverVoltageFault);
    config.mask.Set(AlarmBitmap::kOverVoltageFault);

    TestServerClusterContext context;
    ElectricalProtectionAlarmCluster cluster(kEndpoint, config);
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    BitMask<AlarmBitmap> arc;
    arc.Set(AlarmBitmap::kArcFault); // not supported
    ASSERT_EQ(cluster.ActivateAlarms(arc), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.GetState().Raw(), 0u);

    BitMask<AlarmBitmap> ov;
    ov.Set(AlarmBitmap::kOverVoltageFault); // supported
    ASSERT_EQ(cluster.ActivateAlarms(ov), CHIP_NO_ERROR);
    ASSERT_TRUE(cluster.GetState().Has(AlarmBitmap::kOverVoltageFault));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalProtectionAlarmCluster, DisabledAlarmBitsIgnored)
{
    // OverVoltage is supported but disabled (masked out); ArcFault is supported and enabled.
    ElectricalProtectionAlarmCluster::StartupConfiguration config;
    config.featureMap.Set(Feature::kOverVoltage).Set(Feature::kArcFault);
    config.supported.Set(AlarmBitmap::kOverVoltageFault).Set(AlarmBitmap::kArcFault);
    config.mask.Set(AlarmBitmap::kArcFault); // OverVoltage omitted from the mask

    TestServerClusterContext context;
    ElectricalProtectionAlarmCluster cluster(kEndpoint, config);
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // A supported-but-disabled alarm must not transition State to active.
    BitMask<AlarmBitmap> ov;
    ov.Set(AlarmBitmap::kOverVoltageFault);
    ASSERT_EQ(cluster.ActivateAlarms(ov), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.GetState().Raw(), 0u);

    // A supported and enabled alarm still activates (the mask does not over-suppress).
    BitMask<AlarmBitmap> arc;
    arc.Set(AlarmBitmap::kArcFault);
    ASSERT_EQ(cluster.ActivateAlarms(arc), CHIP_NO_ERROR);
    ASSERT_TRUE(cluster.GetState().Has(AlarmBitmap::kArcFault));
    ASSERT_FALSE(cluster.GetState().Has(AlarmBitmap::kOverVoltageFault));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}
