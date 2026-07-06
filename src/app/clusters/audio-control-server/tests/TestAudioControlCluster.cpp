/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/audio-control-server/AudioControlCluster.h>
#include <app/clusters/audio-control-server/AudioControlDelegate.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/AudioControl/Commands.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AudioControl;
using namespace chip::app::Clusters::AudioControl::Attributes;
using namespace chip::app::Clusters::AudioControl::Commands;
using namespace chip::Testing;
using chip::Protocols::InteractionModel::Status;

// ---------------------------------------------------------------------------
// Mock delegate
// ---------------------------------------------------------------------------

class MockAudioControlDelegate : public AudioControlDelegate
{
public:
    // Configurable return value for the next delegate call.
    Status nextStatus = Status::Success;

    // Call-tracking for OnStartup.
    int startupCalls = 0;
    AudioControlDelegate::StartupState startupState{};

    // Call-tracking for HandleVolumeAndMuteChange.
    int volumeAndMuteCalls = 0;
    uint16_t lastNewVolume = 0;
    bool lastNewSoftMuted  = false;

    // Call-tracking for BEQ handlers.
    int bassChangedCalls   = 0;
    int midChangedCalls    = 0;
    int trebleChangedCalls = 0;
    int16_t lastBass       = 0;
    int16_t lastMid        = 0;
    int16_t lastTreble     = 0;

    void Reset()
    {
        nextStatus         = Status::Success;
        startupCalls       = 0;
        startupState       = {};
        volumeAndMuteCalls = 0;
        lastNewVolume      = 0;
        lastNewSoftMuted   = false;
        bassChangedCalls   = 0;
        midChangedCalls    = 0;
        trebleChangedCalls = 0;
        lastBass = lastMid = lastTreble = 0;
    }

    void OnStartup(const AudioControlDelegate::StartupState & state) override
    {
        ++startupCalls;
        startupState = state;
    }

    Status HandleVolumeAndMuteChange(uint16_t newVolume, bool newSoftMuted) override
    {
        ++volumeAndMuteCalls;
        lastNewVolume    = newVolume;
        lastNewSoftMuted = newSoftMuted;
        return nextStatus;
    }

    Status HandleBassChanged(int16_t bass) override
    {
        ++bassChangedCalls;
        lastBass = bass;
        return nextStatus;
    }

    Status HandleMidChanged(int16_t mid) override
    {
        ++midChangedCalls;
        lastMid = mid;
        return nextStatus;
    }

    Status HandleTrebleChanged(int16_t treble) override
    {
        ++trebleChangedCalls;
        lastTreble = treble;
        return nextStatus;
    }
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static AudioControlCluster::Config BasicConfig()
{
    return AudioControlCluster::Config{}
        .WithMinDeviceVolume(1)
        .WithMaxDeviceVolume(100)
        .WithInitialVolume(50)
        .WithInitialDefaultStepSize(10);
}

static AudioControlCluster::Config BEQAllBandsConfig()
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Bass::Id>().Set<Mid::Id>().Set<Treble::Id>();
    return BasicConfig()
        .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
        .WithOptionalAttributes(optionalSet)
        .WithCorrectionRange(-5, 5);
}

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------

struct TestAudioControlCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    MockAudioControlDelegate mMockDelegate;
    TestServerClusterContext testContext;
};

// ---------- Attribute list tests ----------

TEST_F(TestAudioControlCluster, AttributeListMandatoryOnly)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig());
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
    ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, AudioControl::Id), attributes), CHIP_NO_ERROR);

    // No features, no optional attrs: mandatory + MaxDeviceVolumeDB (because !DB feature) + globals
    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
    AttributeListBuilder listBuilder(expected);
    const AttributeListBuilder::OptionalAttributeEntry optionals[] = {
        { false, PhysicallyMuted::kMetadataEntry },
        { true, MaxDeviceVolumeDB::kMetadataEntry }, // present when !DB feature
        { false, MaxUserVolume::kMetadataEntry },
        { false, StartUpMuted::kMetadataEntry },
        { false, StartUpVolume::kMetadataEntry },
        { false, Bass::kMetadataEntry },
        { false, Mid::kMetadataEntry },
        { false, Treble::kMetadataEntry },
        { false, MinCorrection::kMetadataEntry },
        { false, MaxCorrection::kMetadataEntry },
    };
    ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionals)), CHIP_NO_ERROR);
    ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, AttributeListWithOptionals)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<PhysicallyMuted::Id>().Set<MaxUserVolume::Id>().Set<StartUpMuted::Id>().Set<StartUpVolume::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig().WithOptionalAttributes(optionalSet).WithInitialMaxUserVolume(80));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
    ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, AudioControl::Id), attributes), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
    AttributeListBuilder listBuilder(expected);
    const AttributeListBuilder::OptionalAttributeEntry expectedOptionals[] = {
        { true, PhysicallyMuted::kMetadataEntry },
        { true, MaxDeviceVolumeDB::kMetadataEntry },
        { true, MaxUserVolume::kMetadataEntry },
        { true, StartUpMuted::kMetadataEntry },
        { true, StartUpVolume::kMetadataEntry },
        { false, Bass::kMetadataEntry },
        { false, Mid::kMetadataEntry },
        { false, Treble::kMetadataEntry },
        { false, MinCorrection::kMetadataEntry },
        { false, MaxCorrection::kMetadataEntry },
    };
    ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(expectedOptionals)), CHIP_NO_ERROR);
    ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, AttributeListWithBEQFeature)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Bass::Id>().Set<Mid::Id>().Set<Treble::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                                    .WithOptionalAttributes(optionalSet)
                                    .WithCorrectionRange(-5, 5));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
    ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, AudioControl::Id), attributes), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
    AttributeListBuilder listBuilder(expected);
    const AttributeListBuilder::OptionalAttributeEntry expectedOptionals[] = {
        { false, PhysicallyMuted::kMetadataEntry },
        { true, MaxDeviceVolumeDB::kMetadataEntry }, // !DB feature
        { false, MaxUserVolume::kMetadataEntry },
        { false, StartUpMuted::kMetadataEntry },
        { false, StartUpVolume::kMetadataEntry },
        { true, Bass::kMetadataEntry },
        { true, Mid::kMetadataEntry },
        { true, Treble::kMetadataEntry },
        { true, MinCorrection::kMetadataEntry },
        { true, MaxCorrection::kMetadataEntry },
    };
    ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(expectedOptionals)), CHIP_NO_ERROR);
    ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, AttributeListDBFeatureHidesMaxDeviceVolumeDB)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithFeatures(BitFlags<Feature>(Feature::kDecibel)));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
    ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, AudioControl::Id), attributes), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
    AttributeListBuilder listBuilder(expected);
    const AttributeListBuilder::OptionalAttributeEntry expectedOptionals[] = {
        { false, PhysicallyMuted::kMetadataEntry },
        { false, MaxDeviceVolumeDB::kMetadataEntry }, // DB feature IS set → hidden
        { false, MaxUserVolume::kMetadataEntry },
        { false, StartUpMuted::kMetadataEntry },
        { false, StartUpVolume::kMetadataEntry },
        { false, Bass::kMetadataEntry },
        { false, Mid::kMetadataEntry },
        { false, Treble::kMetadataEntry },
        { false, MinCorrection::kMetadataEntry },
        { false, MaxCorrection::kMetadataEntry },
    };
    ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(expectedOptionals)), CHIP_NO_ERROR);
    ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, BEQFeatureRequiresAtLeastOneBand)
{
    // Minimum valid BEQ configuration: exactly one band in OptionalAttributeSet.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Treble::Id>(); // only Treble — satisfies the "at least one" constraint

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                                    .WithOptionalAttributes(optionalSet)
                                    .WithCorrectionRange(-5, 5)
                                    .WithInitialTreble(3));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // The single configured band is readable.
    int16_t treble{};
    ASSERT_EQ(tester.ReadAttribute(Treble::Id, treble), CHIP_NO_ERROR);
    EXPECT_EQ(treble, 3);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DBFeatureProhibitsMaxDeviceVolumeDBConfig)
{
    // When DB feature is enabled, MaxDeviceVolumeDB must not be in the optional attribute set
    // (constructor VerifyOrDie); it is hidden from the attribute list either way.
    // BasicConfig() never sets MaxDeviceVolumeDB::Id, so this is the valid configuration.
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithFeatures(BitFlags<Feature>(Feature::kDecibel)));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    // MaxDeviceVolumeDB is absent from the attribute list when DB feature is active.
    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
    ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, AudioControl::Id), attributes), CHIP_NO_ERROR);

    auto buf = attributes.TakeBuffer();
    for (const auto & entry : buf)
    {
        EXPECT_NE(entry.attributeId, MaxDeviceVolumeDB::Id);
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- Read attribute tests ----------

TEST_F(TestAudioControlCluster, ReadMandatoryAttributes)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(50));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, AudioControl::kRevision);

    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);

    bool softMuted{};
    ASSERT_EQ(tester.ReadAttribute(SoftMuted::Id, softMuted), CHIP_NO_ERROR);
    EXPECT_FALSE(softMuted);

    uint16_t volume{};
    ASSERT_EQ(tester.ReadAttribute(Volume::Id, volume), CHIP_NO_ERROR);
    EXPECT_EQ(volume, 50u);

    uint16_t minVol{};
    ASSERT_EQ(tester.ReadAttribute(MinDeviceVolume::Id, minVol), CHIP_NO_ERROR);
    EXPECT_EQ(minVol, 1u);

    uint16_t maxVol{};
    ASSERT_EQ(tester.ReadAttribute(MaxDeviceVolume::Id, maxVol), CHIP_NO_ERROR);
    EXPECT_EQ(maxVol, 100u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, ReadUnsupportedAttribute)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig());
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    uint16_t dummy{};
    EXPECT_NE(tester.ReadAttribute(0xFFFF, dummy), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, ReadMaxDeviceVolumeDB)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithMaxDeviceVolumeDB(1200));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    uint16_t maxDeviceVolumeDB{};
    ASSERT_EQ(tester.ReadAttribute(MaxDeviceVolumeDB::Id, maxDeviceVolumeDB), CHIP_NO_ERROR);
    EXPECT_EQ(maxDeviceVolumeDB, 1200u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, ReadCorrectionRangeAttributes)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Bass::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                                    .WithOptionalAttributes(optionalSet)
                                    .WithCorrectionRange(-8, 6));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    int16_t minCorrection{};
    ASSERT_EQ(tester.ReadAttribute(MinCorrection::Id, minCorrection), CHIP_NO_ERROR);
    EXPECT_EQ(minCorrection, -8);

    int16_t maxCorrection{};
    ASSERT_EQ(tester.ReadAttribute(MaxCorrection::Id, maxCorrection), CHIP_NO_ERROR);
    EXPECT_EQ(maxCorrection, 6);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- Command: Mute / Unmute / ToggleMuted ----------

TEST_F(TestAudioControlCluster, MuteUnmuteToggle)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialSoftMuted(false));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    EXPECT_TRUE(tester.Invoke(Mute::Type()).IsSuccess());
    EXPECT_TRUE(cluster.GetSoftMuted());

    EXPECT_TRUE(tester.Invoke(Unmute::Type()).IsSuccess());
    EXPECT_FALSE(cluster.GetSoftMuted());

    EXPECT_TRUE(tester.Invoke(ToggleMuted::Type()).IsSuccess());
    EXPECT_TRUE(cluster.GetSoftMuted());

    EXPECT_TRUE(tester.Invoke(ToggleMuted::Type()).IsSuccess());
    EXPECT_FALSE(cluster.GetSoftMuted());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- Command: SetVolume ----------

TEST_F(TestAudioControlCluster, SetVolumeNotMuted)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(50));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // Set to valid volume while not muted → fast path
    {
        SetVolume::Type cmd;
        cmd.newVolume = 75;
        EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());
        EXPECT_EQ(cluster.GetVolume(), 75u);
    }

    // Volume=0 → SoftMuted=TRUE
    {
        SetVolume::Type cmd;
        cmd.newVolume = 0;
        EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());
        EXPECT_TRUE(cluster.GetSoftMuted());
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetVolumeConstraintErrors)
{
    // MinDeviceVolume=10, MaxDeviceVolume=100
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                AudioControlCluster::Config{}
                                    .WithMinDeviceVolume(10)
                                    .WithMaxDeviceVolume(100)
                                    .WithInitialVolume(50)
                                    .WithInitialDefaultStepSize(5));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // 1..9 are invalid (below MinDeviceVolume but not 0)
    {
        SetVolume::Type cmd;
        cmd.newVolume = 1;
        EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());
    }
    {
        SetVolume::Type cmd;
        cmd.newVolume = 9;
        EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());
    }

    // 0 is valid (mutes)
    {
        SetVolume::Type cmd;
        cmd.newVolume = 0;
        EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());
    }

    // MinDeviceVolume is valid
    {
        SetVolume::Type cmd;
        cmd.newVolume = 10;
        EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());
        EXPECT_EQ(cluster.GetVolume(), 10u);
    }

    // MaxDeviceVolume is valid
    {
        SetVolume::Type cmd;
        cmd.newVolume = 100;
        EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());
        EXPECT_EQ(cluster.GetVolume(), 100u);
    }

    // Above MaxDeviceVolume is invalid
    {
        SetVolume::Type cmd;
        cmd.newVolume = 101;
        EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- UnmutePolicy table tests ----------

TEST_F(TestAudioControlCluster, UnmutePolicyUnmuteOrChangeVolume)
{
    // SoftMuted=TRUE, PhysicallyMuted=FALSE, policy=UnmuteOrChangeVolume
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig().WithInitialSoftMuted(true).WithInitialVolume(50).WithSetVolumeUnmutePolicy(
                                    UnmutePolicyEnum::kUnmuteOrChangeVolume));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    SetVolume::Type cmd;
    cmd.newVolume = 75;
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 75u);
    EXPECT_FALSE(cluster.GetSoftMuted());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, UnmutePolicyDoNotUnmuteAndDoNotChangeVolume)
{
    // SoftMuted=TRUE, policy=DoNotUnmuteAndDoNotChangeVolume → INVALID_IN_STATE
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig().WithInitialSoftMuted(true).WithInitialVolume(50).WithSetVolumeUnmutePolicy(
                                    UnmutePolicyEnum::kDoNotUnmuteAndDoNotChangeVolume));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    SetVolume::Type cmd;
    cmd.newVolume = 75;
    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());
    // State must be unchanged
    EXPECT_EQ(cluster.GetVolume(), 50u);
    EXPECT_TRUE(cluster.GetSoftMuted());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, UnmutePolicyDoNotUnmuteAndChangeVolume)
{
    // SoftMuted=TRUE, policy=DoNotUnmuteAndChangeVolume → volume changes, stays muted
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig().WithInitialSoftMuted(true).WithInitialVolume(50).WithSetVolumeUnmutePolicy(
                                    UnmutePolicyEnum::kDoNotUnmuteAndChangeVolume));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    SetVolume::Type cmd;
    cmd.newVolume = 75;
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 75u);
    EXPECT_TRUE(cluster.GetSoftMuted()); // still muted

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, UnmutePolicyUnmuteOrDoNotChangeVolumePhysicallyMuted)
{
    // PhysicallyMuted=TRUE, policy=UnmuteOrDoNotChangeVolume → INVALID_IN_STATE
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<PhysicallyMuted::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithInitialSoftMuted(false)
                                    .WithInitialVolume(50)
                                    .WithOptionalAttributes(optionalSet)
                                    .WithSetVolumeUnmutePolicy(UnmutePolicyEnum::kUnmuteOrDoNotChangeVolume));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.SetPhysicallyMuted(true), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    SetVolume::Type cmd;
    cmd.newVolume = 75;
    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 50u); // unchanged

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetVolumePhysicallyMutedUnmuteOrChangeVolume)
{
    // PhysicallyMuted=TRUE, SoftMuted=FALSE, policy=kUnmuteOrChangeVolume
    // → volume changes and SoftMuted is cleared (physical mute is hardware-managed)
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<PhysicallyMuted::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithInitialSoftMuted(false)
                                    .WithInitialVolume(50)
                                    .WithOptionalAttributes(optionalSet)
                                    .WithSetVolumeUnmutePolicy(UnmutePolicyEnum::kUnmuteOrChangeVolume));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.SetPhysicallyMuted(true), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    SetVolume::Type cmd;
    cmd.newVolume = 75;
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 75u);
    EXPECT_FALSE(cluster.GetSoftMuted());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetVolumePhysicallyMutedDoNotUnmuteAndChangeVolume)
{
    // PhysicallyMuted=TRUE, SoftMuted=TRUE, policy=kDoNotUnmuteAndChangeVolume
    // → volume changes, SoftMuted stays true
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<PhysicallyMuted::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithInitialSoftMuted(true)
                                    .WithInitialVolume(50)
                                    .WithOptionalAttributes(optionalSet)
                                    .WithSetVolumeUnmutePolicy(UnmutePolicyEnum::kDoNotUnmuteAndChangeVolume));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.SetPhysicallyMuted(true), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    SetVolume::Type cmd;
    cmd.newVolume = 75;
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 75u);
    EXPECT_TRUE(cluster.GetSoftMuted()); // DoNotUnmute: soft-mute preserved

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- Command: IncreaseVolume ----------

TEST_F(TestAudioControlCluster, IncreaseVolumeNotMuted)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(50).WithInitialDefaultStepSize(10));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // Default step (10): 50 → 60
    EXPECT_TRUE(tester.Invoke(IncreaseVolume::Type{}).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 60u);

    // Explicit step 20: 60 → 80
    IncreaseVolume::Type cmd;
    cmd.stepSize.SetValue(20);
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 80u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, IncreaseVolumeCapsAtEffectiveMax)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(95).WithInitialDefaultStepSize(10));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // 95 + 10 = 105 > 100 → capped at 100
    EXPECT_TRUE(tester.Invoke(IncreaseVolume::Type{}).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 100u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, IncreaseVolumeSoftMutedUnmuteOrChangeWithVolumePlusStep)
{
    // SoftMuted=TRUE, PhysicallyMuted=FALSE, policy=UnmuteOrChangeVolume, unmuteVolume=kVolumePlusStepSize
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithInitialSoftMuted(true)
                                    .WithInitialVolume(40)
                                    .WithInitialDefaultStepSize(10)
                                    .WithIncreaseVolumeUnmutePolicy(UnmutePolicyEnum::kUnmuteOrChangeVolume)
                                    .WithIncreaseVolumeUnmuteVolume(UnmuteVolumeEnum::kVolumePlusStepSize));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // RequestedVolume = Volume + StepSize = 40 + 10 = 50; policy unmutes
    EXPECT_TRUE(tester.Invoke(IncreaseVolume::Type{}).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 50u);
    EXPECT_FALSE(cluster.GetSoftMuted());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, IncreaseVolumeSoftMutedUnmuteOrChangeWithMinDeviceVolume)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                AudioControlCluster::Config{}
                                    .WithMinDeviceVolume(5)
                                    .WithMaxDeviceVolume(100)
                                    .WithInitialVolume(40)
                                    .WithInitialDefaultStepSize(10)
                                    .WithInitialSoftMuted(true)
                                    .WithIncreaseVolumeUnmutePolicy(UnmutePolicyEnum::kUnmuteOrChangeVolume)
                                    .WithIncreaseVolumeUnmuteVolume(UnmuteVolumeEnum::kMinDeviceVolume));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    EXPECT_TRUE(tester.Invoke(IncreaseVolume::Type{}).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 5u); // MinDeviceVolume
    EXPECT_FALSE(cluster.GetSoftMuted());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, IncreaseVolumeSoftMutedUnmuteOrChangeWithVolume)
{
    // unmuteVolume=kVolume: requestedVolume is set to the current Volume (unchanged)
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithInitialSoftMuted(true)
                                    .WithInitialVolume(40)
                                    .WithInitialDefaultStepSize(10)
                                    .WithIncreaseVolumeUnmutePolicy(UnmutePolicyEnum::kUnmuteOrChangeVolume)
                                    .WithIncreaseVolumeUnmuteVolume(UnmuteVolumeEnum::kVolume));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    EXPECT_TRUE(tester.Invoke(IncreaseVolume::Type{}).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 40u);  // volume stays the same
    EXPECT_FALSE(cluster.GetSoftMuted()); // unmuted

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, IncreaseVolumeStepSizeConstraintError)
{
    // StepSize is only constrained to be >= 1; an over-large StepSize is clamped against
    // EffectiveMax rather than rejected.
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(50).WithInitialDefaultStepSize(10));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    IncreaseVolume::Type cmd;
    cmd.stepSize.SetValue(0);
    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 50u); // unchanged

    cmd.stepSize.SetValue(100); // effectiveMax(100) - minVol(1) == 99, so this overshoots
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 100u); // clamped to EffectiveMax, not rejected

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, IncreaseVolumePhysicallyMutedOnlyUsesVolumeStepSize)
{
    // PhysicallyMuted=TRUE, SoftMuted=FALSE: the guard (mSoftMuted && !mPhysicallyMuted) is false,
    // so the unmuteVolume enum is skipped entirely and requestedVolume = Volume + StepSize.
    // Here unmuteVolume=kMinDeviceVolume which would give 1 if applied — proves it is not used.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<PhysicallyMuted::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                AudioControlCluster::Config{}
                                    .WithMinDeviceVolume(1)
                                    .WithMaxDeviceVolume(100)
                                    .WithInitialVolume(40)
                                    .WithInitialDefaultStepSize(10)
                                    .WithInitialSoftMuted(false)
                                    .WithOptionalAttributes(optionalSet)
                                    .WithIncreaseVolumeUnmutePolicy(UnmutePolicyEnum::kUnmuteOrChangeVolume)
                                    .WithIncreaseVolumeUnmuteVolume(UnmuteVolumeEnum::kMinDeviceVolume));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.SetPhysicallyMuted(true), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    EXPECT_TRUE(tester.Invoke(IncreaseVolume::Type{}).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 50u); // 40 + 10, NOT kMinDeviceVolume (1)
    EXPECT_FALSE(cluster.GetSoftMuted());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, IncreaseVolumeBothMutedIgnoresUnmuteVolumeEnum)
{
    // SoftMuted=TRUE, PhysicallyMuted=TRUE: guard (mSoftMuted && !mPhysicallyMuted) is still false,
    // so unmuteVolume enum is skipped even though SoftMuted is true.
    // requestedVolume = Volume + StepSize, then policy applied.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<PhysicallyMuted::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                AudioControlCluster::Config{}
                                    .WithMinDeviceVolume(1)
                                    .WithMaxDeviceVolume(100)
                                    .WithInitialVolume(40)
                                    .WithInitialDefaultStepSize(10)
                                    .WithInitialSoftMuted(true)
                                    .WithOptionalAttributes(optionalSet)
                                    .WithIncreaseVolumeUnmutePolicy(UnmutePolicyEnum::kUnmuteOrChangeVolume)
                                    .WithIncreaseVolumeUnmuteVolume(UnmuteVolumeEnum::kMinDeviceVolume));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.SetPhysicallyMuted(true), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    EXPECT_TRUE(tester.Invoke(IncreaseVolume::Type{}).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 50u);  // 40 + 10, NOT kMinDeviceVolume (1)
    EXPECT_FALSE(cluster.GetSoftMuted()); // kUnmuteOrChangeVolume clears SoftMuted

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- Command: DecreaseVolume ----------

TEST_F(TestAudioControlCluster, DecreaseVolumeNotMuted)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(50).WithInitialDefaultStepSize(10));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    EXPECT_TRUE(tester.Invoke(DecreaseVolume::Type{}).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 40u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DecreaseVolumeBelowMinMutesDevice)
{
    // MinDeviceVolume=10, Volume=15, Step=10 → RequestedVolume=5 < 10 → floor at Min and mute
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                AudioControlCluster::Config{}
                                    .WithMinDeviceVolume(10)
                                    .WithMaxDeviceVolume(100)
                                    .WithInitialVolume(15)
                                    .WithInitialSoftMuted(false)
                                    .WithInitialDefaultStepSize(10));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    EXPECT_TRUE(tester.Invoke(DecreaseVolume::Type{}).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 10u); // clamped to MinDeviceVolume
    EXPECT_TRUE(cluster.GetSoftMuted()); // auto-muted

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DecreaseVolumeSoftMutedPolicyPath)
{
    // SoftMuted=true, Volume=50, Step=10 → requestedVolume=40 >= MinDeviceVolume → ApplyUnmutePolicy
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithInitialSoftMuted(true)
                                    .WithInitialVolume(50)
                                    .WithInitialDefaultStepSize(10)
                                    .WithDecreaseVolumeUnmutePolicy(UnmutePolicyEnum::kUnmuteOrChangeVolume));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    EXPECT_TRUE(tester.Invoke(DecreaseVolume::Type{}).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 40u);
    EXPECT_FALSE(cluster.GetSoftMuted());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DecreaseVolumeStepSizeConstraintError)
{
    // StepSize is only constrained to be >= 1; an over-large StepSize floors at MinDeviceVolume
    // (muting the device) rather than being rejected.
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(50).WithInitialDefaultStepSize(10));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    DecreaseVolume::Type cmd;
    cmd.stepSize.SetValue(0);
    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 50u); // unchanged

    cmd.stepSize.SetValue(100); // overshoots down to below MinDeviceVolume
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 1u); // floored at MinDeviceVolume
    EXPECT_TRUE(cluster.GetSoftMuted());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DecreaseVolumePhysicallyMutedPolicyPath)
{
    // PhysicallyMuted=TRUE, SoftMuted=FALSE, Volume=50, Step=10 → requestedVolume=40 >= min
    // Fast-path guard (!mSoftMuted && !mPhysicallyMuted) is false → goes through policy.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<PhysicallyMuted::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithInitialSoftMuted(false)
                                    .WithInitialVolume(50)
                                    .WithOptionalAttributes(optionalSet)
                                    .WithDecreaseVolumeUnmutePolicy(UnmutePolicyEnum::kDoNotUnmuteAndChangeVolume));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.SetPhysicallyMuted(true), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    EXPECT_TRUE(tester.Invoke(DecreaseVolume::Type{}).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 40u);
    EXPECT_FALSE(cluster.GetSoftMuted()); // kDoNotUnmuteAndChangeVolume leaves SoftMuted unchanged (was false)

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- Write attribute tests ----------

TEST_F(TestAudioControlCluster, WriteDefaultStepSize)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialDefaultStepSize(1));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    uint16_t newStep = 20;
    EXPECT_EQ(tester.WriteAttribute(DefaultStepSize::Id, newStep), CHIP_NO_ERROR);

    uint16_t readback{};
    ASSERT_EQ(tester.ReadAttribute(DefaultStepSize::Id, readback), CHIP_NO_ERROR);
    EXPECT_EQ(readback, 20u);

    // 0 is out of range
    newStep = 0;
    EXPECT_NE(tester.WriteAttribute(DefaultStepSize::Id, newStep), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, WriteMaxUserVolumeClampsVolume)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<MaxUserVolume::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithOptionalAttributes(optionalSet)
                                    .WithInitialMaxUserVolume(100)
                                    .WithInitialVolume(90)
                                    .WithInitialSoftMuted(false));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // Writing MaxUserVolume=70 should clamp Volume from 90 → 70
    uint16_t newMax = 70;
    EXPECT_EQ(tester.WriteAttribute(MaxUserVolume::Id, newMax), CHIP_NO_ERROR);

    uint16_t volume{};
    ASSERT_EQ(tester.ReadAttribute(Volume::Id, volume), CHIP_NO_ERROR);
    EXPECT_EQ(volume, 70u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, WriteStartUpMuted)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<StartUpMuted::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithOptionalAttributes(optionalSet));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // Write non-null value
    bool val = true;
    EXPECT_EQ(tester.WriteAttribute(StartUpMuted::Id, DataModel::MakeNullable(val)), CHIP_NO_ERROR);

    DataModel::Nullable<bool> readback{};
    ASSERT_EQ(tester.ReadAttribute(StartUpMuted::Id, readback), CHIP_NO_ERROR);
    ASSERT_FALSE(readback.IsNull());
    EXPECT_TRUE(readback.Value());

    // Write null
    DataModel::Nullable<bool> nullVal;
    nullVal.SetNull();
    EXPECT_EQ(tester.WriteAttribute(StartUpMuted::Id, nullVal), CHIP_NO_ERROR);
    ASSERT_EQ(tester.ReadAttribute(StartUpMuted::Id, readback), CHIP_NO_ERROR);
    EXPECT_TRUE(readback.IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, WriteBassValidatesRange)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Bass::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                                    .WithOptionalAttributes(optionalSet)
                                    .WithCorrectionRange(-5, 5));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    int16_t val = 3;
    EXPECT_EQ(tester.WriteAttribute(Bass::Id, val), CHIP_NO_ERROR);

    // Out of range
    val = 6;
    EXPECT_NE(tester.WriteAttribute(Bass::Id, val), CHIP_NO_ERROR);

    val = -6;
    EXPECT_NE(tester.WriteAttribute(Bass::Id, val), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, WriteMidValidatesRange)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Mid::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                                    .WithOptionalAttributes(optionalSet)
                                    .WithCorrectionRange(-5, 5));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    int16_t val = 3;
    EXPECT_EQ(tester.WriteAttribute(Mid::Id, val), CHIP_NO_ERROR);

    val = 6;
    EXPECT_NE(tester.WriteAttribute(Mid::Id, val), CHIP_NO_ERROR);

    val = -6;
    EXPECT_NE(tester.WriteAttribute(Mid::Id, val), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, WriteTrebleValidatesRange)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Treble::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                                    .WithOptionalAttributes(optionalSet)
                                    .WithCorrectionRange(-5, 5));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    int16_t val = 3;
    EXPECT_EQ(tester.WriteAttribute(Treble::Id, val), CHIP_NO_ERROR);

    val = 6;
    EXPECT_NE(tester.WriteAttribute(Treble::Id, val), CHIP_NO_ERROR);

    val = -6;
    EXPECT_NE(tester.WriteAttribute(Treble::Id, val), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- Write no-op behavior ----------
//
// For most writable attributes, writing the same value a second time must return
// kWriteSuccessNoOp (spec requirement: no-ops must not trigger delegate callbacks
// or KVS stores). StartUpMuted and StartUpVolume are intentional exceptions: they
// always store to KVS so that an explicit null write establishes a persisted startup
// directive even when the in-memory value is already null.

TEST_F(TestAudioControlCluster, WriteNoOpDefaultStepSize)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialDefaultStepSize(10));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    uint16_t val = 20;
    ASSERT_EQ(tester.WriteAttribute(DefaultStepSize::Id, val), CHIP_NO_ERROR);
    EXPECT_TRUE(tester.WriteAttribute(DefaultStepSize::Id, val).IsNoOpSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, WriteNoOpMaxUserVolume)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<MaxUserVolume::Id>();
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig().WithOptionalAttributes(optionalSet).WithInitialMaxUserVolume(80));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    uint16_t val = 60;
    ASSERT_EQ(tester.WriteAttribute(MaxUserVolume::Id, val), CHIP_NO_ERROR);
    EXPECT_TRUE(tester.WriteAttribute(MaxUserVolume::Id, val).IsNoOpSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, WriteNoOpSetVolumeUnmutePolicy)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig());
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    auto val = UnmutePolicyEnum::kDoNotUnmuteAndDoNotChangeVolume;
    ASSERT_EQ(tester.WriteAttribute(SetVolumeUnmutePolicy::Id, val), CHIP_NO_ERROR);
    EXPECT_TRUE(tester.WriteAttribute(SetVolumeUnmutePolicy::Id, val).IsNoOpSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, WriteNoOpIncreaseVolumeUnmutePolicy)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig());
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    auto val = UnmutePolicyEnum::kDoNotUnmuteAndDoNotChangeVolume;
    ASSERT_EQ(tester.WriteAttribute(IncreaseVolumeUnmutePolicy::Id, val), CHIP_NO_ERROR);
    EXPECT_TRUE(tester.WriteAttribute(IncreaseVolumeUnmutePolicy::Id, val).IsNoOpSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, WriteNoOpIncreaseVolumeUnmuteVolume)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig());
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    auto val = UnmuteVolumeEnum::kMinDeviceVolume;
    ASSERT_EQ(tester.WriteAttribute(IncreaseVolumeUnmuteVolume::Id, val), CHIP_NO_ERROR);
    EXPECT_TRUE(tester.WriteAttribute(IncreaseVolumeUnmuteVolume::Id, val).IsNoOpSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, WriteNoOpDecreaseVolumeUnmutePolicy)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig());
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    auto val = UnmutePolicyEnum::kDoNotUnmuteAndDoNotChangeVolume;
    ASSERT_EQ(tester.WriteAttribute(DecreaseVolumeUnmutePolicy::Id, val), CHIP_NO_ERROR);
    EXPECT_TRUE(tester.WriteAttribute(DecreaseVolumeUnmutePolicy::Id, val).IsNoOpSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, WriteNoOpBassDoesNotCallDelegate)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BEQAllBandsConfig());
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    int16_t val = 3;
    ASSERT_EQ(tester.WriteAttribute(Bass::Id, val), CHIP_NO_ERROR); // first write changes 0→3

    mMockDelegate.Reset();
    EXPECT_TRUE(tester.WriteAttribute(Bass::Id, val).IsNoOpSuccess()); // same value: no-op
    EXPECT_EQ(mMockDelegate.bassChangedCalls, 0);                      // delegate must not be called on no-op

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, WriteNoOpMidDoesNotCallDelegate)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BEQAllBandsConfig());
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    int16_t val = 3;
    ASSERT_EQ(tester.WriteAttribute(Mid::Id, val), CHIP_NO_ERROR);

    mMockDelegate.Reset();
    EXPECT_TRUE(tester.WriteAttribute(Mid::Id, val).IsNoOpSuccess());
    EXPECT_EQ(mMockDelegate.midChangedCalls, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, WriteNoOpTrebleDoesNotCallDelegate)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BEQAllBandsConfig());
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    int16_t val = 3;
    ASSERT_EQ(tester.WriteAttribute(Treble::Id, val), CHIP_NO_ERROR);

    mMockDelegate.Reset();
    EXPECT_TRUE(tester.WriteAttribute(Treble::Id, val).IsNoOpSuccess());
    EXPECT_EQ(mMockDelegate.trebleChangedCalls, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, WriteNoOpStartUpMuted)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<StartUpMuted::Id>();
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithOptionalAttributes(optionalSet));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    // null initial value: writing null is immediately a no-op
    DataModel::Nullable<bool> nullVal;
    nullVal.SetNull();
    EXPECT_TRUE(tester.WriteAttribute(StartUpMuted::Id, nullVal).IsNoOpSuccess());

    // write a non-null value, then the same value again
    bool val = true;
    ASSERT_EQ(tester.WriteAttribute(StartUpMuted::Id, DataModel::MakeNullable(val)), CHIP_NO_ERROR);
    EXPECT_TRUE(tester.WriteAttribute(StartUpMuted::Id, DataModel::MakeNullable(val)).IsNoOpSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, WriteNoOpStartUpVolume)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<StartUpVolume::Id>();
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithOptionalAttributes(optionalSet));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    // null initial value: writing null is immediately a no-op
    DataModel::Nullable<uint16_t> nullVal;
    nullVal.SetNull();
    EXPECT_TRUE(tester.WriteAttribute(StartUpVolume::Id, nullVal).IsNoOpSuccess());

    // write a non-null value, then the same value again
    uint16_t val = 50;
    ASSERT_EQ(tester.WriteAttribute(StartUpVolume::Id, DataModel::MakeNullable(val)), CHIP_NO_ERROR);
    EXPECT_TRUE(tester.WriteAttribute(StartUpVolume::Id, DataModel::MakeNullable(val)).IsNoOpSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- Application-facing API ----------

TEST_F(TestAudioControlCluster, SetVolumeAPI)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialVolume(50));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetVolume(80), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetVolume(), 80u);

    // Out of range (0 is not valid for the API since it doesn't implement the "mute" semantic)
    EXPECT_NE(cluster.SetVolume(0), CHIP_NO_ERROR);
    EXPECT_NE(cluster.SetVolume(200), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetVolume(), 80u); // unchanged

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- Application-facing API does NOT call delegate ----------

TEST_F(TestAudioControlCluster, ApplicationSettersDoNotCallDelegate)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialVolume(50));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    mMockDelegate.Reset();

    ASSERT_EQ(cluster.SetVolume(80), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.SetSoftMuted(true), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.SetSoftMuted(false), CHIP_NO_ERROR);

    EXPECT_EQ(mMockDelegate.volumeAndMuteCalls, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetSoftMutedAPI)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialSoftMuted(false));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    ASSERT_EQ(cluster.SetSoftMuted(true), CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.GetSoftMuted());
    bool readback{};
    ASSERT_EQ(tester.ReadAttribute(SoftMuted::Id, readback), CHIP_NO_ERROR);
    EXPECT_TRUE(readback);

    ASSERT_EQ(cluster.SetSoftMuted(false), CHIP_NO_ERROR);
    EXPECT_FALSE(cluster.GetSoftMuted());
    ASSERT_EQ(tester.ReadAttribute(SoftMuted::Id, readback), CHIP_NO_ERROR);
    EXPECT_FALSE(readback);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- Delegate callback verification ----------

TEST_F(TestAudioControlCluster, DelegateMuteCallbackParameters)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(42));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    mMockDelegate.Reset();
    EXPECT_TRUE(tester.Invoke(Mute::Type()).IsSuccess());

    EXPECT_EQ(mMockDelegate.volumeAndMuteCalls, 1);
    EXPECT_EQ(mMockDelegate.lastNewVolume, 42u);
    EXPECT_TRUE(mMockDelegate.lastNewSoftMuted);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DelegateUnmuteCallbackParameters)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialSoftMuted(true).WithInitialVolume(42));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    mMockDelegate.Reset();
    EXPECT_TRUE(tester.Invoke(Unmute::Type()).IsSuccess());

    EXPECT_EQ(mMockDelegate.volumeAndMuteCalls, 1);
    EXPECT_EQ(mMockDelegate.lastNewVolume, 42u);
    EXPECT_FALSE(mMockDelegate.lastNewSoftMuted);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DelegateSetVolumeCallbackParameters)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(50));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    mMockDelegate.Reset();
    SetVolume::Type cmd;
    cmd.newVolume = 70;
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    EXPECT_EQ(mMockDelegate.volumeAndMuteCalls, 1);
    EXPECT_EQ(mMockDelegate.lastNewVolume, 70u);
    EXPECT_FALSE(mMockDelegate.lastNewSoftMuted);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DelegateBassChangedCallback)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Bass::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                                    .WithOptionalAttributes(optionalSet)
                                    .WithCorrectionRange(-5, 5));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    mMockDelegate.Reset();
    int16_t val = 3;
    EXPECT_EQ(tester.WriteAttribute(Bass::Id, val), CHIP_NO_ERROR);

    EXPECT_EQ(mMockDelegate.bassChangedCalls, 1);
    EXPECT_EQ(mMockDelegate.lastBass, 3);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- Delegate veto tests: state must be unchanged on non-Success ----------

TEST_F(TestAudioControlCluster, DelegateMuteVetoLeavesStateUnchanged)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(50));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    mMockDelegate.nextStatus = Status::Busy;
    EXPECT_FALSE(tester.Invoke(Mute::Type()).IsSuccess());

    // SoftMuted must remain false
    EXPECT_FALSE(cluster.GetSoftMuted());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DelegateUnmuteVetoLeavesStateUnchanged)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialSoftMuted(true).WithInitialVolume(50));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    mMockDelegate.nextStatus = Status::Busy;
    EXPECT_FALSE(tester.Invoke(Unmute::Type()).IsSuccess());

    // SoftMuted must remain true
    EXPECT_TRUE(cluster.GetSoftMuted());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DelegateToggleMutedVetoLeavesStateUnchanged)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(50));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    mMockDelegate.nextStatus = Status::Busy;
    EXPECT_FALSE(tester.Invoke(ToggleMuted::Type()).IsSuccess());

    // SoftMuted must remain false
    EXPECT_FALSE(cluster.GetSoftMuted());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DelegateSetVolumeVetoLeavesStateUnchanged)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(50));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    mMockDelegate.nextStatus = Status::Busy;
    SetVolume::Type cmd;
    cmd.newVolume = 75;
    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());

    // Volume and mute state must be unchanged
    EXPECT_EQ(cluster.GetVolume(), 50u);
    EXPECT_FALSE(cluster.GetSoftMuted());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DelegateSetVolumeMuteZeroVetoLeavesStateUnchanged)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(50));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    mMockDelegate.nextStatus = Status::Busy;
    SetVolume::Type cmd;
    cmd.newVolume = 0; // special mute path
    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());

    EXPECT_FALSE(cluster.GetSoftMuted()); // still unmuted

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DelegateIncreaseVolumeVetoLeavesStateUnchanged)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(50).WithInitialDefaultStepSize(10));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    mMockDelegate.nextStatus = Status::Busy;
    EXPECT_FALSE(tester.Invoke(IncreaseVolume::Type{}).IsSuccess());

    EXPECT_EQ(cluster.GetVolume(), 50u); // unchanged

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DelegateDecreaseVolumeVetoLeavesStateUnchanged)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(50).WithInitialDefaultStepSize(10));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    mMockDelegate.nextStatus = Status::Busy;
    EXPECT_FALSE(tester.Invoke(DecreaseVolume::Type{}).IsSuccess());

    EXPECT_EQ(cluster.GetVolume(), 50u); // unchanged
    EXPECT_FALSE(cluster.GetSoftMuted());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DelegateIncreaseVolumePolicyPathVetoLeavesStateUnchanged)
{
    // Start muted so IncreaseVolume goes through ApplyUnmutePolicy, not the fast path.
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig().WithInitialSoftMuted(true).WithInitialVolume(50).WithInitialDefaultStepSize(10));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    mMockDelegate.nextStatus = Status::Busy;
    EXPECT_FALSE(tester.Invoke(IncreaseVolume::Type{}).IsSuccess());

    EXPECT_EQ(cluster.GetVolume(), 50u); // unchanged
    EXPECT_TRUE(cluster.GetSoftMuted()); // still muted

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DelegateBassVetoLeavesStateUnchanged)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Bass::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                                    .WithOptionalAttributes(optionalSet)
                                    .WithCorrectionRange(-5, 5)
                                    .WithInitialBass(0));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    mMockDelegate.nextStatus = Status::Busy;
    int16_t val              = 3;
    EXPECT_NE(tester.WriteAttribute(Bass::Id, val), CHIP_NO_ERROR);

    // Bass must remain 0
    int16_t readback{};
    ASSERT_EQ(tester.ReadAttribute(Bass::Id, readback), CHIP_NO_ERROR);
    EXPECT_EQ(readback, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DelegateMidVetoLeavesStateUnchanged)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Mid::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                                    .WithOptionalAttributes(optionalSet)
                                    .WithCorrectionRange(-5, 5)
                                    .WithInitialMid(0));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    mMockDelegate.nextStatus = Status::Busy;
    int16_t val              = 3;
    EXPECT_NE(tester.WriteAttribute(Mid::Id, val), CHIP_NO_ERROR);

    // Mid must remain 0
    int16_t readback{};
    ASSERT_EQ(tester.ReadAttribute(Mid::Id, readback), CHIP_NO_ERROR);
    EXPECT_EQ(readback, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DelegateTrebleVetoLeavesStateUnchanged)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Treble::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                                    .WithOptionalAttributes(optionalSet)
                                    .WithCorrectionRange(-5, 5)
                                    .WithInitialTreble(0));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    mMockDelegate.nextStatus = Status::Busy;
    int16_t val              = 3;
    EXPECT_NE(tester.WriteAttribute(Treble::Id, val), CHIP_NO_ERROR);

    // Treble must remain 0
    int16_t readback{};
    ASSERT_EQ(tester.ReadAttribute(Treble::Id, readback), CHIP_NO_ERROR);
    EXPECT_EQ(readback, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- Persistence ----------
// Each test starts a cluster, modifies state, shuts down, restarts with the same testContext
// (same backing storage), and verifies the state was restored.

TEST_F(TestAudioControlCluster, PersistenceSoftMuted)
{
    // StartUpMuted must be in the optional set so SoftMuted is loaded from KVS on restart.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<StartUpMuted::Id>();
    auto cfg = BasicConfig().WithOptionalAttributes(optionalSet).WithInitialSoftMuted(false);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.SetSoftMuted(true), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_TRUE(cluster.GetSoftMuted());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceVolume)
{
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialVolume(50));
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.SetVolume(80), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialVolume(50));
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetVolume(), 80u);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceMaxUserVolumeAndVolumeClip)
{
    // SetMaxUserVolume(70) clips Volume 90→70; both values must survive a restart.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<MaxUserVolume::Id>();

    auto cfg = BasicConfig()
                   .WithOptionalAttributes(optionalSet)
                   .WithInitialMaxUserVolume(100)
                   .WithInitialVolume(90)
                   .WithInitialSoftMuted(false);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.SetMaxUserVolume(70), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetMaxUserVolume(), 70u);
        EXPECT_EQ(cluster.GetVolume(), 70u);
        // Persisted MaxUserVolume must constrain SetVolume: 75 > 70 must fail.
        EXPECT_NE(cluster.SetVolume(75), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.SetVolume(65), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceStartUpVolumeClipOnSetMaxUserVolume)
{
    // StartUpVolume=80 written, then MaxUserVolume lowered to 60 clips StartUpVolume to 60.
    // On next boot, Volume must be 60 (from the clipped StartUpVolume).
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<MaxUserVolume::Id>().Set<StartUpVolume::Id>();

    auto cfg = BasicConfig().WithOptionalAttributes(optionalSet).WithInitialMaxUserVolume(100).WithInitialVolume(50);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        uint16_t val = 80;
        ASSERT_EQ(tester.WriteAttribute(StartUpVolume::Id, DataModel::MakeNullable(val)), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.SetMaxUserVolume(60), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetVolume(), 60u);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceStartUpMuted)
{
    // WriteAttribute StartUpMuted=true is persisted; on next boot SoftMuted must be true.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<StartUpMuted::Id>();

    auto cfg = BasicConfig().WithOptionalAttributes(optionalSet).WithInitialSoftMuted(false);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        bool val = true;
        ASSERT_EQ(tester.WriteAttribute(StartUpMuted::Id, DataModel::MakeNullable(val)), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_TRUE(cluster.GetSoftMuted());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceStartUpVolume)
{
    // WriteAttribute StartUpVolume=75 is persisted; on next boot Volume must be 75.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<StartUpVolume::Id>();

    auto cfg = BasicConfig().WithOptionalAttributes(optionalSet).WithInitialVolume(50);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        uint16_t val = 75;
        ASSERT_EQ(tester.WriteAttribute(StartUpVolume::Id, DataModel::MakeNullable(val)), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetVolume(), 75u);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceBass)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Bass::Id>();

    auto cfg = BasicConfig()
                   .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                   .WithOptionalAttributes(optionalSet)
                   .WithCorrectionRange(-5, 5)
                   .WithInitialBass(0);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        int16_t val = 3;
        ASSERT_EQ(tester.WriteAttribute(Bass::Id, val), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        int16_t readback{};
        ASSERT_EQ(tester.ReadAttribute(Bass::Id, readback), CHIP_NO_ERROR);
        EXPECT_EQ(readback, 3);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceMid)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Mid::Id>();

    auto cfg = BasicConfig()
                   .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                   .WithOptionalAttributes(optionalSet)
                   .WithCorrectionRange(-5, 5)
                   .WithInitialMid(0);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        int16_t val = -2;
        ASSERT_EQ(tester.WriteAttribute(Mid::Id, val), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        int16_t readback{};
        ASSERT_EQ(tester.ReadAttribute(Mid::Id, readback), CHIP_NO_ERROR);
        EXPECT_EQ(readback, -2);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceTreble)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Treble::Id>();

    auto cfg = BasicConfig()
                   .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                   .WithOptionalAttributes(optionalSet)
                   .WithCorrectionRange(-5, 5)
                   .WithInitialTreble(0);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        int16_t val = 5;
        ASSERT_EQ(tester.WriteAttribute(Treble::Id, val), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        int16_t readback{};
        ASSERT_EQ(tester.ReadAttribute(Treble::Id, readback), CHIP_NO_ERROR);
        EXPECT_EQ(readback, 5);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// ---------- Command persistence ----------
// Verify every InvokeCommand path that calls Store*() actually survives a restart.

TEST_F(TestAudioControlCluster, PersistenceMuteCommand)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<StartUpMuted::Id>();
    auto cfg = BasicConfig().WithOptionalAttributes(optionalSet).WithInitialSoftMuted(false).WithInitialVolume(50);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_TRUE(ClusterTester(cluster).Invoke(Mute::Type{}).IsSuccess());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_TRUE(cluster.GetSoftMuted());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceUnmuteCommand)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<StartUpMuted::Id>();
    auto cfg = BasicConfig().WithOptionalAttributes(optionalSet).WithInitialSoftMuted(true).WithInitialVolume(50);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_TRUE(ClusterTester(cluster).Invoke(Unmute::Type{}).IsSuccess());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_FALSE(cluster.GetSoftMuted());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceToggleMutedCommand)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<StartUpMuted::Id>();
    auto cfg = BasicConfig().WithOptionalAttributes(optionalSet).WithInitialSoftMuted(false).WithInitialVolume(50);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_TRUE(ClusterTester(cluster).Invoke(ToggleMuted::Type{}).IsSuccess()); // false → true
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_TRUE(cluster.GetSoftMuted());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceSetVolumeCommandFastPath)
{
    // Unmuted fast path → StoreVolume().
    auto cfg = BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(50);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        SetVolume::Type cmd;
        cmd.newVolume = 75;
        ASSERT_TRUE(ClusterTester(cluster).Invoke(cmd).IsSuccess());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetVolume(), 75u);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceSetVolumeCommandMuteZero)
{
    // Volume=0 path → StoreSoftMuted(true).
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<StartUpMuted::Id>();
    auto cfg = BasicConfig().WithOptionalAttributes(optionalSet).WithInitialSoftMuted(false).WithInitialVolume(50);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        SetVolume::Type cmd;
        cmd.newVolume = 0;
        ASSERT_TRUE(ClusterTester(cluster).Invoke(cmd).IsSuccess());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_TRUE(cluster.GetSoftMuted());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceSetVolumeCommandPolicyPath)
{
    // Muted + policy path → ApplyUnmutePolicy → StoreVolume() + StoreSoftMuted().
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<StartUpMuted::Id>();
    auto cfg = BasicConfig()
                   .WithOptionalAttributes(optionalSet)
                   .WithInitialSoftMuted(true)
                   .WithInitialVolume(50)
                   .WithSetVolumeUnmutePolicy(UnmutePolicyEnum::kUnmuteOrChangeVolume);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        SetVolume::Type cmd;
        cmd.newVolume = 75;
        ASSERT_TRUE(ClusterTester(cluster).Invoke(cmd).IsSuccess());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetVolume(), 75u);
        EXPECT_FALSE(cluster.GetSoftMuted());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceIncreaseVolumeCommand)
{
    // Fast path → StoreVolume().
    auto cfg = BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(50).WithInitialDefaultStepSize(10);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_TRUE(ClusterTester(cluster).Invoke(IncreaseVolume::Type{}).IsSuccess()); // 50 → 60
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetVolume(), 60u);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceDecreaseVolumeCommand)
{
    // Fast path → StoreVolume().
    auto cfg = BasicConfig().WithInitialSoftMuted(false).WithInitialVolume(50).WithInitialDefaultStepSize(10);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_TRUE(ClusterTester(cluster).Invoke(DecreaseVolume::Type{}).IsSuccess()); // 50 → 40
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetVolume(), 40u);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceDecreaseVolumeFloorCommand)
{
    // Floor path → StoreVolume(MinDeviceVolume) + StoreSoftMuted(true).
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<StartUpMuted::Id>();
    auto cfg = AudioControlCluster::Config{}
                   .WithOptionalAttributes(optionalSet)
                   .WithMinDeviceVolume(10)
                   .WithMaxDeviceVolume(100)
                   .WithInitialVolume(15)
                   .WithInitialSoftMuted(false)
                   .WithInitialDefaultStepSize(10);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        // 15 - 10 = 5 < MinDeviceVolume(10) → floor
        ASSERT_TRUE(ClusterTester(cluster).Invoke(DecreaseVolume::Type{}).IsSuccess());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetVolume(), 10u);
        EXPECT_TRUE(cluster.GetSoftMuted());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// ---------- Startup: null StartUp* retains last persisted state ----------

TEST_F(TestAudioControlCluster, StartupNullStartUpMutedRetainsPersistedSoftMuted)
{
    // SoftMuted=true is persisted via Mute command; StartUpMuted=null means
    // "use last persisted state", so SoftMuted must remain true after restart.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<StartUpMuted::Id>();
    auto cfg = BasicConfig().WithOptionalAttributes(optionalSet).WithInitialSoftMuted(false);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        ASSERT_TRUE(tester.Invoke(Mute::Type{}).IsSuccess()); // persists SoftMuted=true
        DataModel::Nullable<bool> nullVal;
        nullVal.SetNull();
        ASSERT_TRUE(tester.WriteAttribute(StartUpMuted::Id, nullVal).IsSuccess()); // null→null: no-op, but still succeeds
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_TRUE(cluster.GetSoftMuted()); // retained, not overridden by null StartUpMuted
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, StartupNullStartUpVolumeRetainsPersistedVolume)
{
    // Volume=80 is persisted via command; StartUpVolume=null means "use last persisted Volume".
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<StartUpVolume::Id>();
    auto cfg = BasicConfig().WithOptionalAttributes(optionalSet).WithInitialVolume(50).WithInitialSoftMuted(false);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        SetVolume::Type cmd;
        cmd.newVolume = 80;
        ASSERT_TRUE(tester.Invoke(cmd).IsSuccess()); // persists Volume=80
        DataModel::Nullable<uint16_t> nullVal;
        nullVal.SetNull();
        ASSERT_TRUE(tester.WriteAttribute(StartUpVolume::Id, nullVal).IsSuccess()); // null→null: no-op, but still succeeds
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetVolume(), 80u); // retained, not overridden by null StartUpVolume
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// ---------- App-facing API: SetMaxUserVolume ----------

TEST_F(TestAudioControlCluster, SetMaxUserVolumeConstraintBelowMin)
{
    // MinDeviceVolume=1 (BasicConfig default); 0 is below the minimum.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<MaxUserVolume::Id>();
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig().WithOptionalAttributes(optionalSet).WithInitialMaxUserVolume(100));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_NE(cluster.SetMaxUserVolume(0), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMaxUserVolume(), 100u); // unchanged

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetMaxUserVolumeConstraintAboveMaxDevice)
{
    // MaxDeviceVolume=100 (BasicConfig default); 101 is above the device ceiling.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<MaxUserVolume::Id>();
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig().WithOptionalAttributes(optionalSet).WithInitialMaxUserVolume(100));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_NE(cluster.SetMaxUserVolume(101), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMaxUserVolume(), 100u); // unchanged

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetMaxUserVolumeClipsStartUpVolumeInMemory)
{
    // After SetMaxUserVolume clips StartUpVolume, ReadAttribute in the same session
    // must reflect the clipped in-memory value, not the original write.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<MaxUserVolume::Id>().Set<StartUpVolume::Id>();
    AudioControlCluster cluster(
        kRootEndpointId, mMockDelegate,
        BasicConfig().WithOptionalAttributes(optionalSet).WithInitialMaxUserVolume(100).WithInitialVolume(50));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    uint16_t startupVol = 80;
    ASSERT_EQ(tester.WriteAttribute(StartUpVolume::Id, DataModel::MakeNullable(startupVol)), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.SetMaxUserVolume(60), CHIP_NO_ERROR);

    DataModel::Nullable<uint16_t> readback{};
    ASSERT_EQ(tester.ReadAttribute(StartUpVolume::Id, readback), CHIP_NO_ERROR);
    ASSERT_FALSE(readback.IsNull());
    EXPECT_EQ(readback.Value(), 60u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetMaxUserVolumeDelegateArgsWhenSoftMuted)
{
    // When Volume is clipped while SoftMuted=true, delegate receives (newMax, true).
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<MaxUserVolume::Id>();
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithOptionalAttributes(optionalSet)
                                    .WithInitialMaxUserVolume(100)
                                    .WithInitialVolume(90)
                                    .WithInitialSoftMuted(true));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    mMockDelegate.Reset();
    ASSERT_EQ(cluster.SetMaxUserVolume(70), CHIP_NO_ERROR);

    EXPECT_EQ(mMockDelegate.volumeAndMuteCalls, 1);
    EXPECT_EQ(mMockDelegate.lastNewVolume, 70u);
    EXPECT_TRUE(mMockDelegate.lastNewSoftMuted);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetVolumeRespectEffectiveMaxAfterSetMaxUserVolume)
{
    // After SetMaxUserVolume(70) lowers EffectiveMax, SetVolume(75) must be rejected
    // in the same session — no restart needed.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<MaxUserVolume::Id>();
    AudioControlCluster cluster(
        kRootEndpointId, mMockDelegate,
        BasicConfig().WithOptionalAttributes(optionalSet).WithInitialMaxUserVolume(100).WithInitialVolume(50));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ASSERT_EQ(cluster.SetMaxUserVolume(70), CHIP_NO_ERROR);
    EXPECT_NE(cluster.SetVolume(75), CHIP_NO_ERROR); // 75 > EffectiveMax=70
    EXPECT_EQ(cluster.SetVolume(65), CHIP_NO_ERROR); // 65 ≤ EffectiveMax=70
    EXPECT_EQ(cluster.GetVolume(), 65u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- App-facing API: optional-attribute-absent guard ----------

TEST_F(TestAudioControlCluster, SetPhysicallyMutedAbsentReturnsError)
{
    // PhysicallyMuted not in optional set → setter must fail, no state change, no notification.
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig());
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_NE(cluster.SetPhysicallyMuted(true), CHIP_NO_ERROR);
    EXPECT_FALSE(cluster.GetPhysicallyMuted());
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetMaxUserVolumeAbsentReturnsError)
{
    // MaxUserVolume not in optional set → setter must fail.
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialVolume(50));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_NE(cluster.SetMaxUserVolume(70), CHIP_NO_ERROR);
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, MaxUserVolumeIgnoredWhenAttributeAbsent)
{
    // MaxUserVolume not in optional set: whether or not WithInitialMaxUserVolume was called,
    // EffectiveMaxVolume() must fall back to MaxDeviceVolume — the OptionalAttributeSet, not a
    // zero-vs-nonzero config value, is the source of truth for whether MaxUserVolume applies.
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialVolume(50).WithInitialMaxUserVolume(70));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMaxUserVolume(), 100u); // == MaxDeviceVolume from BasicConfig(), not 70
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetBassAbsentReturnsError)
{
    // Bass not in optional set (BEQ feature present, but only Mid enabled) → setter must fail.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Mid::Id>();
    auto cfg = AudioControlCluster::Config{}
                   .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                   .WithOptionalAttributes(optionalSet)
                   .WithCorrectionRange(-5, 5);
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_NE(cluster.SetBass(2), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetBass(), 0); // unchanged
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetMidAbsentReturnsError)
{
    // Mid not in optional set (BEQ feature present, only Bass enabled) → setter must fail.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Bass::Id>();
    auto cfg = AudioControlCluster::Config{}
                   .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                   .WithOptionalAttributes(optionalSet)
                   .WithCorrectionRange(-5, 5);
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_NE(cluster.SetMid(2), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMid(), 0); // unchanged
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetTrebleAbsentReturnsError)
{
    // Treble not in optional set (BEQ feature present, only Bass enabled) → setter must fail.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Bass::Id>();
    auto cfg = AudioControlCluster::Config{}
                   .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                   .WithOptionalAttributes(optionalSet)
                   .WithCorrectionRange(-5, 5);
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_NE(cluster.SetTreble(2), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetTreble(), 0); // unchanged
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetBassFeatureAbsentReturnsError)
{
    // BEQ feature not enabled → SetBass/SetMid/SetTreble must all fail.
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig());
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_NE(cluster.SetBass(1), CHIP_NO_ERROR);
    EXPECT_NE(cluster.SetMid(1), CHIP_NO_ERROR);
    EXPECT_NE(cluster.SetTreble(1), CHIP_NO_ERROR);
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- App-facing API: SetPhysicallyMuted ----------

TEST_F(TestAudioControlCluster, SetPhysicallyMutedBasic)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<PhysicallyMuted::Id>();
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithOptionalAttributes(optionalSet));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    ASSERT_FALSE(cluster.GetPhysicallyMuted());
    ASSERT_EQ(cluster.SetPhysicallyMuted(true), CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.GetPhysicallyMuted());

    bool readback{};
    ASSERT_EQ(tester.ReadAttribute(PhysicallyMuted::Id, readback), CHIP_NO_ERROR);
    EXPECT_TRUE(readback);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, PhysicallyMutedSeededFromConfig)
{
    // The hardware knows its physical mute state at boot (e.g. a mute switch position), so
    // WithInitialPhysicallyMuted() must take effect immediately — no SetPhysicallyMuted() call
    // needed after Startup().
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<PhysicallyMuted::Id>();
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig().WithOptionalAttributes(optionalSet).WithInitialPhysicallyMuted(true));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    EXPECT_TRUE(cluster.GetPhysicallyMuted());

    bool readback{};
    ASSERT_EQ(tester.ReadAttribute(PhysicallyMuted::Id, readback), CHIP_NO_ERROR);
    EXPECT_TRUE(readback);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetPhysicallyMutedDoesNotCallDelegate)
{
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<PhysicallyMuted::Id>();
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithOptionalAttributes(optionalSet));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    mMockDelegate.Reset();
    ASSERT_EQ(cluster.SetPhysicallyMuted(true), CHIP_NO_ERROR);
    EXPECT_EQ(mMockDelegate.volumeAndMuteCalls, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- App-facing API: SetBass / SetMid / SetTreble ----------

TEST_F(TestAudioControlCluster, SetBassAPI)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BEQAllBandsConfig().WithInitialBass(0));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    EXPECT_EQ(cluster.SetBass(3), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetBass(), 3);
    int16_t readback{};
    ASSERT_EQ(tester.ReadAttribute(Bass::Id, readback), CHIP_NO_ERROR);
    EXPECT_EQ(readback, 3);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetBassConstraintError)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BEQAllBandsConfig().WithInitialBass(0));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_NE(cluster.SetBass(-6), CHIP_NO_ERROR); // below min
    EXPECT_NE(cluster.SetBass(6), CHIP_NO_ERROR);  // above max
    EXPECT_EQ(cluster.GetBass(), 0);               // unchanged

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetBassDoesNotCallDelegate)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BEQAllBandsConfig());
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    mMockDelegate.Reset();
    ASSERT_EQ(cluster.SetBass(2), CHIP_NO_ERROR);
    EXPECT_EQ(mMockDelegate.bassChangedCalls, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetBassPersistence)
{
    auto cfg = BEQAllBandsConfig().WithInitialBass(0);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.SetBass(4), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetBass(), 4);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, SetMidAPI)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BEQAllBandsConfig().WithInitialMid(0));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    EXPECT_EQ(cluster.SetMid(-3), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMid(), -3);
    int16_t readback{};
    ASSERT_EQ(tester.ReadAttribute(Mid::Id, readback), CHIP_NO_ERROR);
    EXPECT_EQ(readback, -3);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetMidConstraintError)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BEQAllBandsConfig().WithInitialMid(0));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_NE(cluster.SetMid(-6), CHIP_NO_ERROR);
    EXPECT_NE(cluster.SetMid(6), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMid(), 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetMidDoesNotCallDelegate)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BEQAllBandsConfig());
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    mMockDelegate.Reset();
    ASSERT_EQ(cluster.SetMid(-1), CHIP_NO_ERROR);
    EXPECT_EQ(mMockDelegate.midChangedCalls, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetMidPersistence)
{
    auto cfg = BEQAllBandsConfig().WithInitialMid(0);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.SetMid(-2), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetMid(), -2);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, SetTrebleAPI)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BEQAllBandsConfig().WithInitialTreble(0));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    EXPECT_EQ(cluster.SetTreble(5), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetTreble(), 5);
    int16_t readback{};
    ASSERT_EQ(tester.ReadAttribute(Treble::Id, readback), CHIP_NO_ERROR);
    EXPECT_EQ(readback, 5);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetTrebleConstraintError)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BEQAllBandsConfig().WithInitialTreble(0));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_NE(cluster.SetTreble(-6), CHIP_NO_ERROR);
    EXPECT_NE(cluster.SetTreble(6), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetTreble(), 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetTrebleDoesNotCallDelegate)
{
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BEQAllBandsConfig());
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    mMockDelegate.Reset();
    ASSERT_EQ(cluster.SetTreble(1), CHIP_NO_ERROR);
    EXPECT_EQ(mMockDelegate.trebleChangedCalls, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetTreblePersistence)
{
    auto cfg = BEQAllBandsConfig().WithInitialTreble(0);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.SetTreble(5), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetTreble(), 5);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// ---------- Non-persisted attributes ----------
// Explicit tests confirming intentionally non-persisted attributes revert to config defaults.

TEST_F(TestAudioControlCluster, PhysicallyMutedNotPersisted)
{
    // PhysicallyMuted is not persisted because hardware re-reports it on each boot.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<PhysicallyMuted::Id>();
    auto cfg = BasicConfig().WithOptionalAttributes(optionalSet);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.SetPhysicallyMuted(true), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_FALSE(cluster.GetPhysicallyMuted()); // reset; hardware will re-report
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceDefaultStepSize)
{
    auto cfg = BasicConfig().WithInitialDefaultStepSize(10);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        uint16_t newStep = 20;
        ASSERT_EQ(tester.WriteAttribute(DefaultStepSize::Id, newStep), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        uint16_t readback{};
        ASSERT_EQ(tester.ReadAttribute(DefaultStepSize::Id, readback), CHIP_NO_ERROR);
        EXPECT_EQ(readback, 20u);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceSetVolumeUnmutePolicy)
{
    auto cfg = BasicConfig().WithSetVolumeUnmutePolicy(UnmutePolicyEnum::kUnmuteOrChangeVolume);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        UnmutePolicyEnum newPolicy = UnmutePolicyEnum::kDoNotUnmuteAndChangeVolume;
        ASSERT_EQ(tester.WriteAttribute(SetVolumeUnmutePolicy::Id, newPolicy), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        UnmutePolicyEnum readback{};
        ASSERT_EQ(tester.ReadAttribute(SetVolumeUnmutePolicy::Id, readback), CHIP_NO_ERROR);
        EXPECT_EQ(readback, UnmutePolicyEnum::kDoNotUnmuteAndChangeVolume);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceIncreaseVolumeUnmutePolicy)
{
    auto cfg = BasicConfig().WithIncreaseVolumeUnmutePolicy(UnmutePolicyEnum::kUnmuteOrChangeVolume);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        UnmutePolicyEnum newPolicy = UnmutePolicyEnum::kDoNotUnmuteAndDoNotChangeVolume;
        ASSERT_EQ(tester.WriteAttribute(IncreaseVolumeUnmutePolicy::Id, newPolicy), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        UnmutePolicyEnum readback{};
        ASSERT_EQ(tester.ReadAttribute(IncreaseVolumeUnmutePolicy::Id, readback), CHIP_NO_ERROR);
        EXPECT_EQ(readback, UnmutePolicyEnum::kDoNotUnmuteAndDoNotChangeVolume);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceIncreaseVolumeUnmuteVolume)
{
    auto cfg = BasicConfig().WithIncreaseVolumeUnmuteVolume(UnmuteVolumeEnum::kVolumePlusStepSize);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        UnmuteVolumeEnum newVal = UnmuteVolumeEnum::kMinDeviceVolume;
        ASSERT_EQ(tester.WriteAttribute(IncreaseVolumeUnmuteVolume::Id, newVal), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        UnmuteVolumeEnum readback{};
        ASSERT_EQ(tester.ReadAttribute(IncreaseVolumeUnmuteVolume::Id, readback), CHIP_NO_ERROR);
        EXPECT_EQ(readback, UnmuteVolumeEnum::kMinDeviceVolume);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, PersistenceDecreaseVolumeUnmutePolicy)
{
    auto cfg = BasicConfig().WithDecreaseVolumeUnmutePolicy(UnmutePolicyEnum::kUnmuteOrChangeVolume);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        UnmutePolicyEnum newPolicy = UnmutePolicyEnum::kDoNotUnmuteAndChangeVolume;
        ASSERT_EQ(tester.WriteAttribute(DecreaseVolumeUnmutePolicy::Id, newPolicy), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        UnmutePolicyEnum readback{};
        ASSERT_EQ(tester.ReadAttribute(DecreaseVolumeUnmutePolicy::Id, readback), CHIP_NO_ERROR);
        EXPECT_EQ(readback, UnmutePolicyEnum::kDoNotUnmuteAndChangeVolume);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// ---------- SetMaxUserVolume delegate behavior ----------

TEST_F(TestAudioControlCluster, SetMaxUserVolumeDelegateCalledOnVolumeClip)
{
    // Volume=90 > new MaxUserVolume=70 → hardware must be notified via delegate.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<MaxUserVolume::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithOptionalAttributes(optionalSet)
                                    .WithInitialMaxUserVolume(100)
                                    .WithInitialVolume(90)
                                    .WithInitialSoftMuted(false));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    mMockDelegate.Reset();
    ASSERT_EQ(cluster.SetMaxUserVolume(70), CHIP_NO_ERROR);

    EXPECT_EQ(mMockDelegate.volumeAndMuteCalls, 1);
    EXPECT_EQ(mMockDelegate.lastNewVolume, 70u);
    EXPECT_FALSE(mMockDelegate.lastNewSoftMuted);
    EXPECT_EQ(cluster.GetVolume(), 70u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetMaxUserVolumeNoDelegateCallWhenNoVolumeClip)
{
    // Volume=50 <= new MaxUserVolume=70 → no clip → delegate NOT called.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<MaxUserVolume::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithOptionalAttributes(optionalSet)
                                    .WithInitialMaxUserVolume(100)
                                    .WithInitialVolume(50)
                                    .WithInitialSoftMuted(false));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    mMockDelegate.Reset();
    ASSERT_EQ(cluster.SetMaxUserVolume(70), CHIP_NO_ERROR);

    EXPECT_EQ(mMockDelegate.volumeAndMuteCalls, 0);
    EXPECT_EQ(cluster.GetVolume(), 50u);
    EXPECT_EQ(cluster.GetMaxUserVolume(), 70u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetMaxUserVolumeDelegateVetoLeavesAllStateUnchanged)
{
    // Delegate rejects the volume clip → MaxUserVolume, Volume, and StartUpVolume must all be unchanged.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<MaxUserVolume::Id>().Set<StartUpVolume::Id>();

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BasicConfig()
                                    .WithOptionalAttributes(optionalSet)
                                    .WithInitialMaxUserVolume(100)
                                    .WithInitialVolume(90)
                                    .WithInitialSoftMuted(false));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    uint16_t startupVol = 80;
    ASSERT_EQ(tester.WriteAttribute(StartUpVolume::Id, DataModel::MakeNullable(startupVol)), CHIP_NO_ERROR);

    mMockDelegate.nextStatus = Status::Busy;
    EXPECT_NE(cluster.SetMaxUserVolume(70), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetMaxUserVolume(), 100u);
    EXPECT_EQ(cluster.GetVolume(), 90u);

    DataModel::Nullable<uint16_t> readback{};
    ASSERT_EQ(tester.ReadAttribute(StartUpVolume::Id, readback), CHIP_NO_ERROR);
    ASSERT_FALSE(readback.IsNull());
    EXPECT_EQ(readback.Value(), 80u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- Optional attribute absent: startup fallback behavior ----------

TEST_F(TestAudioControlCluster, StartUpMutedAbsentRetainsPersistedSoftMuted)
{
    // When StartUpMuted is absent, SoftMuted is still non-volatile per spec: the cluster retains
    // the persisted SoftMuted state on startup, mirroring the behavior of an explicit
    // StartUpMuted=null.
    auto cfg = BasicConfig().WithInitialSoftMuted(false);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        ASSERT_TRUE(tester.Invoke(Mute::Type{}).IsSuccess()); // persists SoftMuted=true
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_TRUE(cluster.GetSoftMuted()); // retained from persistence, not forced to false
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, StartUpVolumeAbsentBehavesAsNull)
{
    // When StartUpVolume is absent, the cluster retains the persisted Volume on startup,
    // mirroring the behavior of an explicit StartUpVolume=null.
    auto cfg = BasicConfig().WithInitialVolume(50).WithInitialSoftMuted(false);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        SetVolume::Type cmd;
        cmd.newVolume = 80;
        ASSERT_TRUE(tester.Invoke(cmd).IsSuccess()); // persists Volume=80
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetVolume(), 80u); // retained — absent StartUpVolume behaves as null
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// ---------- OnStartup delegate notification ----------

TEST_F(TestAudioControlCluster, OnStartupCalledWithConfigDefaultsOnFirstBoot)
{
    // Empty KVS: OnStartup must receive the config defaults unchanged.
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialVolume(50).WithInitialSoftMuted(false));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(mMockDelegate.startupCalls, 1);
    EXPECT_EQ(mMockDelegate.startupState.volume, 50u);
    EXPECT_FALSE(mMockDelegate.startupState.softMuted);
    // No command-driven delegate calls on startup.
    EXPECT_EQ(mMockDelegate.volumeAndMuteCalls, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, OnStartupCalledOncePerStartup)
{
    // Shutdown + re-Startup must trigger OnStartup again (exactly once each time).
    auto cfg = BasicConfig().WithInitialVolume(50).WithInitialSoftMuted(false);

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_EQ(mMockDelegate.startupCalls, 1);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    mMockDelegate.Reset();

    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_EQ(mMockDelegate.startupCalls, 1);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, OnStartupCalledWithPersistedVolume)
{
    // After SetVolume(80) is persisted, a restart must call OnStartup with 80.
    auto cfg = BasicConfig().WithInitialVolume(50).WithInitialSoftMuted(false);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        SetVolume::Type cmd;
        cmd.newVolume = 80;
        ASSERT_TRUE(ClusterTester(cluster).Invoke(cmd).IsSuccess());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    mMockDelegate.Reset();
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(mMockDelegate.startupCalls, 1);
        EXPECT_EQ(mMockDelegate.startupState.volume, 80u);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, OnStartupCalledWithPersistedSoftMuted)
{
    // After Mute is persisted (StartUpMuted=null → retain), restart must call OnStartup with muted=true.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<StartUpMuted::Id>();
    auto cfg = BasicConfig().WithOptionalAttributes(optionalSet).WithInitialVolume(50).WithInitialSoftMuted(false);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_TRUE(ClusterTester(cluster).Invoke(Mute::Type{}).IsSuccess());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    mMockDelegate.Reset();
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(mMockDelegate.startupCalls, 1);
        EXPECT_TRUE(mMockDelegate.startupState.softMuted);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, OnStartupCalledWithStartUpVolumeOverride)
{
    // StartUpVolume=75 overrides persisted Volume=50; OnStartup must see 75.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<StartUpVolume::Id>();
    auto cfg = BasicConfig().WithOptionalAttributes(optionalSet).WithInitialVolume(50).WithInitialSoftMuted(false);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        uint16_t startupVol = 75;
        ASSERT_EQ(tester.WriteAttribute(StartUpVolume::Id, DataModel::MakeNullable(startupVol)), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    mMockDelegate.Reset();
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(mMockDelegate.startupCalls, 1);
        EXPECT_EQ(mMockDelegate.startupState.volume, 75u);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, OnStartupCalledWithStartUpMutedOverride)
{
    // StartUpMuted=true overrides persisted SoftMuted=false; OnStartup must see muted=true.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<StartUpMuted::Id>();
    auto cfg = BasicConfig().WithOptionalAttributes(optionalSet).WithInitialVolume(50).WithInitialSoftMuted(false);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        bool val = true;
        ASSERT_EQ(tester.WriteAttribute(StartUpMuted::Id, DataModel::MakeNullable(val)), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    mMockDelegate.Reset();
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(mMockDelegate.startupCalls, 1);
        EXPECT_TRUE(mMockDelegate.startupState.softMuted);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, OnStartupCalledWithClampedVolume)
{
    // Volume 90 persisted, then MaxUserVolume lowered to 70. OnStartup must see clamped value 70.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<MaxUserVolume::Id>();
    {
        auto cfg = BasicConfig().WithOptionalAttributes(optionalSet).WithInitialMaxUserVolume(100).WithInitialVolume(50);
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        SetVolume::Type cmd;
        cmd.newVolume = 90;
        ASSERT_TRUE(ClusterTester(cluster).Invoke(cmd).IsSuccess()); // persists Volume=90
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    mMockDelegate.Reset();
    {
        // Restart with MaxUserVolume lowered to 70 in config; persisted MaxUserVolume not written
        // so KVS load falls back to new config default.
        auto cfg = BasicConfig().WithOptionalAttributes(optionalSet).WithInitialMaxUserVolume(70).WithInitialVolume(50);
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(mMockDelegate.startupCalls, 1);
        EXPECT_EQ(mMockDelegate.startupState.volume, 70u); // clamped to new MaxUserVolume
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, OnStartupBEQClampedWhenRangeNarrows)
{
    // Bass=4 persisted with range [-5,5]; on restart with narrower range [-3,3] it must be clamped to 3.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Bass::Id>();
    {
        auto cfg = AudioControlCluster::Config{}
                       .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                       .WithOptionalAttributes(optionalSet)
                       .WithCorrectionRange(-5, 5);
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        int16_t val = 4;
        ASSERT_EQ(ClusterTester(cluster).WriteAttribute(Bass::Id, val), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    mMockDelegate.Reset();
    {
        auto cfg = AudioControlCluster::Config{}
                       .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                       .WithOptionalAttributes(optionalSet)
                       .WithCorrectionRange(-3, 3);
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(mMockDelegate.startupState.bass, 3); // clamped to new max
        EXPECT_EQ(cluster.GetBass(), 3);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, OnStartupBEQZeroWhenFeatureDisabled)
{
    // Without BEQ feature, bass/mid/treble in StartupState are the (default-zero) config values,
    // since BasicConfig() never calls WithInitialBass/Mid/Treble. See the StartupState comment:
    // the delegate is constructed alongside the same Config, so it already knows BEQ is disabled
    // and should ignore these fields regardless of what raw value they carry.
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialVolume(50).WithInitialSoftMuted(false));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(mMockDelegate.startupState.bass, 0);
    EXPECT_EQ(mMockDelegate.startupState.mid, 0);
    EXPECT_EQ(mMockDelegate.startupState.treble, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, OnStartupBEQCalledWithConfigDefaults)
{
    // BEQ enabled, no KVS: OnStartup must carry the config-default EQ values.
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                BEQAllBandsConfig().WithInitialBass(2).WithInitialMid(-1).WithInitialTreble(3));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(mMockDelegate.startupState.bass, 2);
    EXPECT_EQ(mMockDelegate.startupState.mid, -1);
    EXPECT_EQ(mMockDelegate.startupState.treble, 3);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, OnStartupBEQCalledWithPersistedValues)
{
    // Write Bass=4, Mid=-3, Treble=2, then restart and verify OnStartup carries them.
    auto cfg = BEQAllBandsConfig();
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ClusterTester tester(cluster);
        int16_t bass = 4, mid = -3, treble = 2;
        ASSERT_EQ(tester.WriteAttribute(Bass::Id, bass), CHIP_NO_ERROR);
        ASSERT_EQ(tester.WriteAttribute(Mid::Id, mid), CHIP_NO_ERROR);
        ASSERT_EQ(tester.WriteAttribute(Treble::Id, treble), CHIP_NO_ERROR);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    mMockDelegate.Reset();
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(mMockDelegate.startupCalls, 1);
        EXPECT_EQ(mMockDelegate.startupState.bass, 4);
        EXPECT_EQ(mMockDelegate.startupState.mid, -3);
        EXPECT_EQ(mMockDelegate.startupState.treble, 2);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAudioControlCluster, OnStartupBEQPartialBands)
{
    // Only Bass in optional set: mid and treble in StartupState are the (default-zero) config
    // values, since this config never calls WithInitialMid/WithInitialTreble. The delegate is
    // constructed alongside the same Config, so it already knows mid/treble are unsupported here.
    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<Bass::Id>();
    auto cfg = AudioControlCluster::Config{}
                   .WithFeatures(BitFlags<Feature>(Feature::kBasicEqualizer))
                   .WithOptionalAttributes(optionalSet)
                   .WithCorrectionRange(-5, 5)
                   .WithInitialBass(3);

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(mMockDelegate.startupState.bass, 3);
    EXPECT_EQ(mMockDelegate.startupState.mid, 0);
    EXPECT_EQ(mMockDelegate.startupState.treble, 0);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- Corrupted KVS: enum fallback to config default on startup ----------

TEST_F(TestAudioControlCluster, StartupCorruptedEnumKvsFallsBackToConfigDefault)
{
    // Simulate corrupted KVS: inject an invalid byte (0xFF → kUnknownEnumValue) for each policy enum.
    // The cluster must fall back to the config default rather than leaving the enum in an invalid
    // state that would break command handling (ApplyUnmutePolicy hits the default case → UnsupportedCommand).
    const uint8_t invalidByte = 0xFF;
    auto writeCorruptByte     = [&](AttributeId attrId) {
        auto key = DefaultStorageKeyAllocator::AttributeValue(kRootEndpointId, AudioControl::Id, attrId);
        ASSERT_EQ(testContext.StorageDelegate().SyncSetKeyValue(key.KeyName(), &invalidByte, 1), CHIP_NO_ERROR);
    };
    writeCorruptByte(SetVolumeUnmutePolicy::Id);
    writeCorruptByte(IncreaseVolumeUnmutePolicy::Id);
    writeCorruptByte(IncreaseVolumeUnmuteVolume::Id);
    writeCorruptByte(DecreaseVolumeUnmutePolicy::Id);

    // Config: DoNotUnmuteAndDoNotChangeVolume for SetVolume, kMinDeviceVolume for IncreaseVolumeUnmuteVolume.
    // After startup with corrupted KVS, these config defaults must be used.
    auto cfg = BasicConfig()
                   .WithSetVolumeUnmutePolicy(UnmutePolicyEnum::kDoNotUnmuteAndDoNotChangeVolume)
                   .WithIncreaseVolumeUnmutePolicy(UnmutePolicyEnum::kDoNotUnmuteAndChangeVolume)
                   .WithIncreaseVolumeUnmuteVolume(UnmuteVolumeEnum::kMinDeviceVolume)
                   .WithDecreaseVolumeUnmutePolicy(UnmutePolicyEnum::kDoNotUnmuteAndChangeVolume)
                   .WithInitialSoftMuted(true)
                   .WithInitialVolume(50);

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    // Verify via ReadAttribute that persisted-but-corrupt values were replaced by config defaults.
    UnmutePolicyEnum svup{};
    ASSERT_EQ(tester.ReadAttribute(SetVolumeUnmutePolicy::Id, svup), CHIP_NO_ERROR);
    EXPECT_EQ(svup, UnmutePolicyEnum::kDoNotUnmuteAndDoNotChangeVolume);

    UnmutePolicyEnum ivup{};
    ASSERT_EQ(tester.ReadAttribute(IncreaseVolumeUnmutePolicy::Id, ivup), CHIP_NO_ERROR);
    EXPECT_EQ(ivup, UnmutePolicyEnum::kDoNotUnmuteAndChangeVolume);

    UnmuteVolumeEnum ivuv{};
    ASSERT_EQ(tester.ReadAttribute(IncreaseVolumeUnmuteVolume::Id, ivuv), CHIP_NO_ERROR);
    EXPECT_EQ(ivuv, UnmuteVolumeEnum::kMinDeviceVolume);

    UnmutePolicyEnum dvup{};
    ASSERT_EQ(tester.ReadAttribute(DecreaseVolumeUnmutePolicy::Id, dvup), CHIP_NO_ERROR);
    EXPECT_EQ(dvup, UnmutePolicyEnum::kDoNotUnmuteAndChangeVolume);

    // Behavioral verify: SetVolume on a muted device with kDoNotUnmuteAndDoNotChangeVolume → rejected and state unchanged.
    // If the corrupt value 0xFF had been used, the policy switch would hit default → UnsupportedCommand,
    // and volume/mute state would still be unchanged — but the wrong status code would be a bug.
    SetVolume::Type cmd;
    cmd.newVolume = 75;
    EXPECT_FALSE(tester.Invoke(cmd).IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 50u); // unchanged
    EXPECT_TRUE(cluster.GetSoftMuted()); // still muted

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ---------- Regression tests for command/persistence hardening fixes ----------

TEST_F(TestAudioControlCluster, StartupCorruptedMaxUserVolumeKvsClampedToValidRange)
{
    // Simulate a corrupted/stale KVS value for MaxUserVolume that is below MinDeviceVolume.
    // Without clamping, EffectiveMaxVolume() would return it verbatim and the later
    // std::clamp(mVolume, mMinDeviceVolume, EffectiveMaxVolume()) call would violate std::clamp's
    // precondition (lower <= upper).
    const uint16_t corruptValue = 0; // BasicConfig() has MinDeviceVolume == 1
    auto key                    = DefaultStorageKeyAllocator::AttributeValue(kRootEndpointId, AudioControl::Id, MaxUserVolume::Id);
    ASSERT_EQ(testContext.StorageDelegate().SyncSetKeyValue(key.KeyName(), &corruptValue, sizeof(corruptValue)), CHIP_NO_ERROR);

    AudioControlCluster::OptionalAttributeSet optionalSet;
    optionalSet.Set<MaxUserVolume::Id>();
    auto cfg = BasicConfig().WithOptionalAttributes(optionalSet).WithInitialMaxUserVolume(100).WithInitialVolume(50);

    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetMaxUserVolume(), 1u); // clamped up to MinDeviceVolume, not left at 0
    EXPECT_EQ(cluster.GetVolume(), 1u);        // Volume clamp then applies against the corrected ceiling

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SetVolumeInvalidUnmutePolicyReturnsConstraintError)
{
    // SoftMuted=TRUE so the command goes through ApplyUnmutePolicy (not the fast path).
    // An UnmutePolicy value outside the defined enumerators is a malformed command field,
    // not a state conflict, so it must be rejected with ConstraintError (not InvalidInState).
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialSoftMuted(true).WithInitialVolume(50));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    SetVolume::Type cmd;
    cmd.newVolume = 75;
    cmd.unmutePolicy.SetValue(static_cast<UnmutePolicyEnum>(99));
    auto result = tester.Invoke(cmd);
    EXPECT_FALSE(result.IsSuccess());
    auto statusCode = result.GetStatusCode();
    // validation to make clang-tidy happy.
    if (statusCode.has_value())
    {
        EXPECT_EQ(statusCode->GetStatus(), Status::ConstraintError);
    }
    else
    {
        FAIL();
    }

    // State unchanged.
    EXPECT_EQ(cluster.GetVolume(), 50u);
    EXPECT_TRUE(cluster.GetSoftMuted());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, IncreaseVolumeDegenerateRangeClampsToNoOpSuccess)
{
    // MinDeviceVolume == MaxDeviceVolume -> EffectiveMax == MinDeviceVolume, so there is no room to
    // increase. StepSize is only checked for being >= 1; the resulting volume is clamped to
    // EffectiveMax, so the command succeeds as a no-op rather than failing with ConstraintError.
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                AudioControlCluster::Config{}
                                    .WithMinDeviceVolume(50)
                                    .WithMaxDeviceVolume(50)
                                    .WithInitialVolume(50)
                                    .WithInitialDefaultStepSize(1));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    auto result = tester.Invoke(IncreaseVolume::Type{});
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 50u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, DecreaseVolumeDegenerateRangeClampsToNoOpSuccess)
{
    // Same degenerate range as IncreaseVolumeDegenerateRangeClampsToNoOpSuccess, but for
    // DecreaseVolume: the result floors at MinDeviceVolume (== MaxDeviceVolume here), so the
    // command succeeds, muting the device rather than failing with ConstraintError.
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate,
                                AudioControlCluster::Config{}
                                    .WithMinDeviceVolume(50)
                                    .WithMaxDeviceVolume(50)
                                    .WithInitialVolume(50)
                                    .WithInitialDefaultStepSize(1));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    auto result = tester.Invoke(DecreaseVolume::Type{});
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(cluster.GetVolume(), 50u);
    EXPECT_TRUE(cluster.GetSoftMuted());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, IncreaseDecreaseVolumeRejectZeroStepSize)
{
    // StepSize is still required to be >= 1 regardless of the volume range.
    AudioControlCluster cluster(kRootEndpointId, mMockDelegate, BasicConfig().WithInitialVolume(50));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    IncreaseVolume::Type increaseCmd;
    increaseCmd.stepSize.SetValue(0);
    auto increaseResult = tester.Invoke(increaseCmd);
    EXPECT_FALSE(increaseResult.IsSuccess());
    auto increaseStatusCode = increaseResult.GetStatusCode();
    // validation to make clang-tidy happy.
    if (increaseStatusCode.has_value())
    {
        EXPECT_EQ(increaseStatusCode->GetStatus(), Status::ConstraintError);
    }
    else
    {
        FAIL();
    }

    DecreaseVolume::Type decreaseCmd;
    decreaseCmd.stepSize.SetValue(0);
    auto decreaseResult = tester.Invoke(decreaseCmd);
    EXPECT_FALSE(decreaseResult.IsSuccess());
    auto decreaseStatusCode = decreaseResult.GetStatusCode();
    // validation to make clang-tidy happy.
    if (decreaseStatusCode.has_value())
    {
        EXPECT_EQ(decreaseStatusCode->GetStatus(), Status::ConstraintError);
    }
    else
    {
        FAIL();
    }

    EXPECT_EQ(cluster.GetVolume(), 50u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAudioControlCluster, SoftMutedPersistedWithoutStartUpMuted)
{
    // SoftMuted is non-volatile per spec and must survive a restart even when StartUpMuted is not
    // in the optional set.
    auto cfg = BasicConfig().WithInitialSoftMuted(false);
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.SetSoftMuted(true), CHIP_NO_ERROR);

        auto key = DefaultStorageKeyAllocator::AttributeValue(kRootEndpointId, AudioControl::Id, SoftMuted::Id);
        EXPECT_TRUE(testContext.StorageDelegate().SyncDoesKeyExist(key.KeyName()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        AudioControlCluster cluster(kRootEndpointId, mMockDelegate, cfg);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        EXPECT_TRUE(cluster.GetSoftMuted()); // restored from persistence, not the config default
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

} // namespace
