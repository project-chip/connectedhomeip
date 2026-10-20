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

#if defined(__unix__) || defined(__APPLE__)
#include <csignal>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalProtectionAlarm;
using namespace chip::app::Clusters::ElectricalProtectionAlarm::Attributes;
using namespace chip::Testing;
using chip::Testing::IsAttributesListEqualTo;

namespace {

constexpr EndpointId kEndpoint = 1;

constexpr Optional<int64_t> Structs::ShortCircuitRatingsStruct::Type::*kShortCircuitCurrentMembers[] = {
    &Structs::ShortCircuitRatingsStruct::Type::tripCurrent,
    &Structs::ShortCircuitRatingsStruct::Type::ultimateMaxCurrent,
    &Structs::ShortCircuitRatingsStruct::Type::serviceMaxCurrent,
    &Structs::ShortCircuitRatingsStruct::Type::maxCurrent,
};

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

TEST_F(TestElectricalProtectionAlarmCluster, ValidateShortCircuitCurrentMinima)
{
    EXPECT_EQ(AllFeaturesConfig().ValidateRatingMinima(), CHIP_NO_ERROR);
    for (auto member : kShortCircuitCurrentMembers)
    {
        for (int64_t value : { -50000, -1, 0, 1, 50000 })
        {
            auto config               = AllFeaturesConfig();
            config.shortCircuitRating = DataModel::MakeNullable(Structs::ShortCircuitRatingsStruct::Type{});
            (config.shortCircuitRating.Value().*member).SetValue(value);
            EXPECT_EQ(config.ValidateRatingMinima(), value >= 1 ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT);
            if (value < 1)
            {
                config.mask.SetRaw(0);
                EXPECT_EQ(config.ValidateRatingMinima(), CHIP_ERROR_INVALID_ARGUMENT);
                config.featureMap.Clear(Feature::kShortCircuit);
                EXPECT_EQ(config.ValidateRatingMinima(), CHIP_NO_ERROR);
                continue;
            }

            TestServerClusterContext context;
            ElectricalProtectionAlarmCluster cluster(kEndpoint, config);
            ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
            ClusterTester tester(cluster);
            ShortCircuitRating::TypeInfo::DecodableType rating;
            ASSERT_EQ(tester.ReadAttribute(ShortCircuitRating::Id, rating), CHIP_NO_ERROR);
            ASSERT_FALSE(rating.IsNull());
            ASSERT_TRUE((rating.Value().*member).HasValue());
            EXPECT_EQ((rating.Value().*member).Value(), value);
            for (auto otherMember : kShortCircuitCurrentMembers)
            {
                EXPECT_EQ((rating.Value().*otherMember).HasValue(), member == otherMember);
            }
            EXPECT_FALSE(rating.Value().tripMechanism.HasValue());
            cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        }
    }
}

TEST_F(TestElectricalProtectionAlarmCluster, ValidateResidualCurrentTrippingCharacteristicMinimum)
{
    for (uint16_t raw : { uint16_t{ 0 }, uint16_t{ 1 }, uint16_t{ 0x8000 } })
    {
        auto config = AllFeaturesConfig();
        Structs::ResidualCurrentFaultRatingsStruct::Type rating;
        rating.trippingCharacteristic.SetValue(BitMask<TrippingCharacteristicsBitmap>(raw));
        config.residualCurrentRating = DataModel::MakeNullable(rating);
        EXPECT_EQ(config.ValidateRatingMinima(), raw >= 1 ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT);
        if (raw == 0)
        {
            config.mask.SetRaw(0);
            EXPECT_EQ(config.ValidateRatingMinima(), CHIP_ERROR_INVALID_ARGUMENT);
            config.featureMap.Clear(Feature::kResidualCurrent);
            EXPECT_EQ(config.ValidateRatingMinima(), CHIP_NO_ERROR);
            continue;
        }

        TestServerClusterContext context;
        ElectricalProtectionAlarmCluster cluster(kEndpoint, config);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        ResidualCurrentRating::TypeInfo::DecodableType readback;
        ASSERT_EQ(tester.ReadAttribute(ResidualCurrentRating::Id, readback), CHIP_NO_ERROR);
        ASSERT_FALSE(readback.IsNull());
        ASSERT_TRUE(readback.Value().trippingCharacteristic.HasValue());
        EXPECT_EQ(readback.Value().trippingCharacteristic.Value().Raw(), raw);
        EXPECT_FALSE(readback.Value().tripMechanism.HasValue());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestElectricalProtectionAlarmCluster, ValidateShortCircuitTripMechanismMinimum)
{
    for (uint8_t raw : { uint8_t{ 0 }, uint8_t{ 1 }, uint8_t{ 0x80 } })
    {
        auto config = AllFeaturesConfig();
        Structs::ShortCircuitRatingsStruct::Type rating;
        rating.tripMechanism.SetValue(BitMask<CurrentTripMechanismBitmap>(raw));
        config.shortCircuitRating = DataModel::MakeNullable(rating);
        EXPECT_EQ(config.ValidateRatingMinima(), raw >= 1 ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT);
        if (raw == 0)
        {
            config.mask.SetRaw(0);
            EXPECT_EQ(config.ValidateRatingMinima(), CHIP_ERROR_INVALID_ARGUMENT);
            config.featureMap.Clear(Feature::kShortCircuit);
            EXPECT_EQ(config.ValidateRatingMinima(), CHIP_NO_ERROR);
            continue;
        }

        TestServerClusterContext context;
        ElectricalProtectionAlarmCluster cluster(kEndpoint, config);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        ShortCircuitRating::TypeInfo::DecodableType readback;
        ASSERT_EQ(tester.ReadAttribute(ShortCircuitRating::Id, readback), CHIP_NO_ERROR);
        ASSERT_FALSE(readback.IsNull());
        ASSERT_TRUE(readback.Value().tripMechanism.HasValue());
        EXPECT_EQ(readback.Value().tripMechanism.Value().Raw(), raw);
        for (auto member : kShortCircuitCurrentMembers)
        {
            EXPECT_FALSE((readback.Value().*member).HasValue());
        }
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestElectricalProtectionAlarmCluster, ValidateSurgeProtectionClassMinimum)
{
    for (uint16_t raw : { uint16_t{ 0 }, uint16_t{ 1 }, uint16_t{ 0x8000 } })
    {
        auto config = AllFeaturesConfig();
        Structs::SurgeProtectionRatingsStruct::Type rating;
        rating.protectionClass.SetValue(BitMask<SurgeProtectionClassBitmap>(raw));
        config.surgeProtectionRating = DataModel::MakeNullable(rating);
        EXPECT_EQ(config.ValidateRatingMinima(), raw >= 1 ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT);
        if (raw == 0)
        {
            config.mask.SetRaw(0);
            EXPECT_EQ(config.ValidateRatingMinima(), CHIP_ERROR_INVALID_ARGUMENT);
            config.featureMap.Clear(Feature::kSurgeProtection);
            EXPECT_EQ(config.ValidateRatingMinima(), CHIP_NO_ERROR);
            continue;
        }

        TestServerClusterContext context;
        ElectricalProtectionAlarmCluster cluster(kEndpoint, config);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        SurgeProtectionRating::TypeInfo::DecodableType readback;
        ASSERT_EQ(tester.ReadAttribute(SurgeProtectionRating::Id, readback), CHIP_NO_ERROR);
        ASSERT_FALSE(readback.IsNull());
        ASSERT_TRUE(readback.Value().protectionClass.HasValue());
        EXPECT_EQ(readback.Value().protectionClass.Value().Raw(), raw);
        EXPECT_FALSE(readback.Value().protectionType.HasValue());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestElectricalProtectionAlarmCluster, ValidateSurgeProtectionTypeMinimum)
{
    for (uint16_t raw : { uint16_t{ 0 }, uint16_t{ 1 }, uint16_t{ 0x8000 } })
    {
        auto config = AllFeaturesConfig();
        Structs::SurgeProtectionRatingsStruct::Type rating;
        rating.protectionType.SetValue(BitMask<SurgeProtectionTypeBitmap>(raw));
        config.surgeProtectionRating = DataModel::MakeNullable(rating);
        EXPECT_EQ(config.ValidateRatingMinima(), raw >= 1 ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT);
        if (raw == 0)
        {
            config.mask.SetRaw(0);
            EXPECT_EQ(config.ValidateRatingMinima(), CHIP_ERROR_INVALID_ARGUMENT);
            config.featureMap.Clear(Feature::kSurgeProtection);
            EXPECT_EQ(config.ValidateRatingMinima(), CHIP_NO_ERROR);
            continue;
        }

        TestServerClusterContext context;
        ElectricalProtectionAlarmCluster cluster(kEndpoint, config);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        SurgeProtectionRating::TypeInfo::DecodableType readback;
        ASSERT_EQ(tester.ReadAttribute(SurgeProtectionRating::Id, readback), CHIP_NO_ERROR);
        ASSERT_FALSE(readback.IsNull());
        ASSERT_TRUE(readback.Value().protectionType.HasValue());
        EXPECT_EQ(readback.Value().protectionType.Value().Raw(), raw);
        EXPECT_FALSE(readback.Value().protectionClass.HasValue());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestElectricalProtectionAlarmCluster, NullRatingsAndAbsentMembersRemainValid)
{
    for (bool present : { false, true })
    {
        auto config = AllFeaturesConfig();
        config.shortCircuitRating.SetNull();
        if (present)
        {
            config.residualCurrentRating = DataModel::MakeNullable(Structs::ResidualCurrentFaultRatingsStruct::Type{});
            config.shortCircuitRating    = DataModel::MakeNullable(Structs::ShortCircuitRatingsStruct::Type{});
            config.surgeProtectionRating = DataModel::MakeNullable(Structs::SurgeProtectionRatingsStruct::Type{});
        }
        ASSERT_EQ(config.ValidateRatingMinima(), CHIP_NO_ERROR);

        TestServerClusterContext context;
        ElectricalProtectionAlarmCluster cluster(kEndpoint, config);
        ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        ResidualCurrentRating::TypeInfo::DecodableType residual;
        ShortCircuitRating::TypeInfo::DecodableType shortCircuit;
        SurgeProtectionRating::TypeInfo::DecodableType surge;
        ASSERT_EQ(tester.ReadAttribute(ResidualCurrentRating::Id, residual), CHIP_NO_ERROR);
        ASSERT_EQ(tester.ReadAttribute(ShortCircuitRating::Id, shortCircuit), CHIP_NO_ERROR);
        ASSERT_EQ(tester.ReadAttribute(SurgeProtectionRating::Id, surge), CHIP_NO_ERROR);
        ASSERT_EQ(residual.IsNull(), !present);
        ASSERT_EQ(shortCircuit.IsNull(), !present);
        ASSERT_EQ(surge.IsNull(), !present);
        if (present)
        {
            EXPECT_FALSE(residual.Value().trippingCharacteristic.HasValue());
            EXPECT_FALSE(shortCircuit.Value().tripMechanism.HasValue());
            for (auto member : kShortCircuitCurrentMembers)
            {
                EXPECT_FALSE((shortCircuit.Value().*member).HasValue());
            }
            EXPECT_FALSE(surge.Value().protectionClass.HasValue());
            EXPECT_FALSE(surge.Value().protectionType.HasValue());
        }
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestElectricalProtectionAlarmCluster, DisabledFeatureRatingsAreOmitted)
{
    auto config = AllFeaturesConfig();
    Structs::ResidualCurrentFaultRatingsStruct::Type residual;
    residual.trippingCharacteristic.SetValue(BitMask<TrippingCharacteristicsBitmap>());
    config.residualCurrentRating = DataModel::MakeNullable(residual);
    for (auto member : kShortCircuitCurrentMembers)
    {
        (config.shortCircuitRating.Value().*member).SetValue(0);
    }
    config.shortCircuitRating.Value().tripMechanism.SetValue(BitMask<CurrentTripMechanismBitmap>());
    Structs::SurgeProtectionRatingsStruct::Type surge;
    surge.protectionClass.SetValue(BitMask<SurgeProtectionClassBitmap>());
    surge.protectionType.SetValue(BitMask<SurgeProtectionTypeBitmap>());
    config.surgeProtectionRating = DataModel::MakeNullable(surge);
    config.featureMap.Clear(Feature::kResidualCurrent).Clear(Feature::kShortCircuit).Clear(Feature::kSurgeProtection);
    ASSERT_EQ(config.ValidateRatingMinima(), CHIP_NO_ERROR);
    for (Feature feature : { Feature::kResidualCurrent, Feature::kShortCircuit, Feature::kSurgeProtection })
    {
        auto enabled = config;
        enabled.featureMap.Set(feature);
        EXPECT_EQ(enabled.ValidateRatingMinima(), CHIP_ERROR_INVALID_ARGUMENT);
    }

    TestServerClusterContext context;
    ElectricalProtectionAlarmCluster cluster(kEndpoint, config);
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Mask::kMetadataEntry,
                                            State::kMetadataEntry,
                                            Supported::kMetadataEntry,
                                            ArcCause::kMetadataEntry,
                                            OverLoadRating::kMetadataEntry,
                                            OverVoltageRating::kMetadataEntry,
                                            ArcFaultRating::kMetadataEntry,
                                        }));
    ClusterTester tester(cluster);
    BitMask<Feature> featureMap;
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap.Raw(), config.featureMap.Raw());
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

#if defined(__unix__) || defined(__APPLE__)
TEST_F(TestElectricalProtectionAlarmCluster, ConstructorRejectsInvalidRatingMinima)
{
    ElectricalProtectionAlarmCluster::StartupConfiguration configurations[12];
    for (auto & config : configurations)
    {
        config = AllFeaturesConfig();
        config.mask.SetRaw(0);
        config.state = AllAlarms();
    }
    size_t configurationIndex = 0;
    for (auto member : kShortCircuitCurrentMembers)
    {
        for (int64_t value : { -1, 0 })
        {
            (configurations[configurationIndex++].shortCircuitRating.Value().*member).SetValue(value);
        }
    }
    Structs::ResidualCurrentFaultRatingsStruct::Type residual;
    residual.trippingCharacteristic.SetValue(BitMask<TrippingCharacteristicsBitmap>());
    configurations[configurationIndex++].residualCurrentRating = DataModel::MakeNullable(residual);
    configurations[configurationIndex++].shortCircuitRating.Value().tripMechanism.SetValue(BitMask<CurrentTripMechanismBitmap>());
    Structs::SurgeProtectionRatingsStruct::Type surgeClass;
    surgeClass.protectionClass.SetValue(BitMask<SurgeProtectionClassBitmap>());
    configurations[configurationIndex++].surgeProtectionRating = DataModel::MakeNullable(surgeClass);
    Structs::SurgeProtectionRatingsStruct::Type surgeType;
    surgeType.protectionType.SetValue(BitMask<SurgeProtectionTypeBitmap>());
    configurations[configurationIndex++].surgeProtectionRating = DataModel::MakeNullable(surgeType);
    ASSERT_EQ(configurationIndex, sizeof(configurations) / sizeof(configurations[0]));
    for (const auto & config : configurations)
    {
        const pid_t child = fork();
        ASSERT_GE(child, 0);
        if (child == 0)
        {
            const rlimit coreLimit{ 0, 0 };
            if (setrlimit(RLIMIT_CORE, &coreLimit) != 0)
            {
                _exit(2);
            }
            ElectricalProtectionAlarmCluster cluster(kEndpoint, config);
            _exit(0);
        }
        int childStatus = 0;
        ASSERT_EQ(waitpid(child, &childStatus, 0), child);
        ASSERT_TRUE(WIFSIGNALED(childStatus));
        EXPECT_EQ(WTERMSIG(childStatus), SIGABRT);
    }
}
#endif

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

TEST_F(TestElectricalProtectionAlarmCluster, StartupStateNormalizedAgainstSupportedAndMask)
{
    // A startup state carrying bits the device cannot raise: ArcFault is not supported at all, and
    // OverVoltage is supported but masked out. Only the supported and unmasked bit may survive.
    ElectricalProtectionAlarmCluster::StartupConfiguration config;
    config.featureMap.Set(Feature::kOverVoltage).Set(Feature::kShortCircuit);
    config.supported.Set(AlarmBitmap::kOverVoltageFault).Set(AlarmBitmap::kShortCircuitFault);
    config.mask.Set(AlarmBitmap::kShortCircuitFault); // OverVoltage omitted from the mask
    config.state
        .Set(AlarmBitmap::kArcFault)           // not supported
        .Set(AlarmBitmap::kOverVoltageFault)   // supported but masked out
        .Set(AlarmBitmap::kShortCircuitFault); // supported and unmasked

    TestServerClusterContext context;
    ElectricalProtectionAlarmCluster cluster(kEndpoint, config);
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_FALSE(cluster.GetState().Has(AlarmBitmap::kArcFault));
    EXPECT_FALSE(cluster.GetState().Has(AlarmBitmap::kOverVoltageFault));
    EXPECT_TRUE(cluster.GetState().Has(AlarmBitmap::kShortCircuitFault));

    // The reported attribute must agree with the accessor, since it is what a client sees.
    ClusterTester tester(cluster);
    State::TypeInfo::DecodableType state{};
    ASSERT_TRUE(tester.ReadAttribute(State::Id, state).IsSuccess());
    EXPECT_EQ(state.Raw(), BitMask<AlarmBitmap>().Set(AlarmBitmap::kShortCircuitFault).Raw());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
