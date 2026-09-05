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

#include <algorithm>

#include <app/ConcreteAttributePath.h>
#include <app/clusters/audio-control-server/AudioControlCluster.h>
#include <app/clusters/audio-control-server/SpeakerAudioCoordinator.h>
#include <app/clusters/level-control/LevelControlCluster.h>
#include <app/clusters/on-off-server/OnOffCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <clusters/AudioControl/Commands.h>
#include <lib/support/TimerDelegateMock.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AudioControl::Commands;
using chip::Protocols::InteractionModel::Status;
using chip::Testing::ClusterTester;

constexpr EndpointId kTestEndpointId = 1;

// tester (below) is bound to AudioControlCluster, so its own IsAttributeDirty() only ever checks
// AudioControl's cluster ID. All three clusters in a fixture share one context/dirty list (see
// SpeakerAudioCoordinatorTest::SetUp()), so cross-cluster reportability -- e.g. LevelControl's
// CurrentLevel or OnOff's OnOff going dirty as a result of a coordinator sync -- is checked
// directly against that shared list instead.
bool IsPathDirty(std::vector<ConcreteAttributePath> & dirtyList, const ConcreteAttributePath & path)
{
    return std::find(dirtyList.begin(), dirtyList.end(), path) != dirtyList.end();
}

// SpeakerAudioCoordinator -- not this mock -- is what gets passed as AudioControlCluster's
// delegate (mirroring examples/all-devices-app's Speaker.cpp wiring), so this mock only ever sees
// what SpeakerAudioCoordinator forwards to it: the fixed hardware limits (queried once at
// AudioControlCluster construction time) and HandleVolumeAndMuteChange/HandleBassChanged/
// HandleMidChanged/HandleTrebleChanged/OnStartup.
class MockHardwareDelegate : public AudioControlDelegate
{
public:
    uint16_t minDeviceVolume   = 1;
    uint16_t maxDeviceVolume   = 100;
    uint16_t maxDeviceVolumeDB = 0;
    int16_t minCorrection      = -10;
    int16_t maxCorrection      = 10;

    uint16_t GetMinDeviceVolume() const override { return minDeviceVolume; }
    uint16_t GetMaxDeviceVolume() const override { return maxDeviceVolume; }
    uint16_t GetMaxDeviceVolumeDB() const override { return maxDeviceVolumeDB; }
    int16_t GetMinCorrection() const override { return minCorrection; }
    int16_t GetMaxCorrection() const override { return maxCorrection; }

    Status nextStatus = Status::Success;

    int volumeAndMuteCalls = 0;
    uint16_t lastVolume    = 0;
    bool lastSoftMuted     = false;

    int startupCalls = 0;
    StartupState lastStartupState{};

    int bassChangedCalls   = 0;
    int midChangedCalls    = 0;
    int trebleChangedCalls = 0;

    Status HandleVolumeAndMuteChange(uint16_t newVolume, bool newSoftMuted) override
    {
        ++volumeAndMuteCalls;
        lastVolume    = newVolume;
        lastSoftMuted = newSoftMuted;
        return nextStatus;
    }

    void OnStartup(const StartupState & state) override
    {
        ++startupCalls;
        lastStartupState = state;
    }

    Status HandleBassChanged(int16_t) override
    {
        ++bassChangedCalls;
        return nextStatus;
    }
    Status HandleMidChanged(int16_t) override
    {
        ++midChangedCalls;
        return nextStatus;
    }
    Status HandleTrebleChanged(int16_t) override
    {
        ++trebleChangedCalls;
        return nextStatus;
    }
};

// Wires OnOff + LevelControl + AudioControl through a SpeakerAudioCoordinator, the same way
// examples/all-devices-app's Speaker.cpp does. LevelControl uses the Lighting feature's
// MinLevel=1/MaxLevel=254; AudioControl uses the (default) MinDeviceVolume=1/MaxDeviceVolume=100 --
// so Level and Volume are not on a 1:1 scale, which exercises SpeakerAudioCoordinator's scaling
// on every sync (Volume 1/50/100 <-> Level 1/126/254).
struct SpeakerAudioCoordinatorTest : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    TimerDelegateMock mockTimer;
    MockHardwareDelegate hardwareDelegate;
    SpeakerAudioCoordinator coordinator{ hardwareDelegate };

    // OnOffCluster reads context.defaults.onOff at construction time, so the "starts on" default
    // (matching Speaker.cpp) has to be set here rather than in SetUp().
    OnOffCluster::Context onOffContext{ mockTimer, {}, OnOffCluster::Defaults{ true } };
    OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    LevelControlCluster levelControlCluster{ kTestEndpointId,
                                             LevelControlCluster::Config(mockTimer, coordinator)
                                                 .WithOnOff(onOffCluster)
                                                 .WithLighting(DataModel::NullNullable)
                                                 .WithInitialCurrentLevel(254) };

    AudioControlCluster audioControlCluster{ kTestEndpointId, coordinator,
                                             AudioControlCluster::Config().WithInitialVolume(100).WithInitialSoftMuted(false) };

    // Bound to AudioControlCluster: tester.Invoke() below dispatches AudioControl commands.
    ClusterTester tester{ audioControlCluster };

    void SetUp() override
    {
        // Wiring order mirrors Speaker::Register(): both delegates added to OnOffCluster, then
        // SetClusters() before any cluster's Startup() -- SetClusters() must run first since
        // LevelControlCluster::Startup() below synchronously calls into the coordinator, which
        // dereferences mLevelControlCluster/mAudioControlCluster.
        onOffCluster.AddDelegate(&coordinator);
        onOffCluster.AddDelegate(&levelControlCluster);
        coordinator.SetClusters(levelControlCluster, audioControlCluster);

        // Startup() order here is AudioControl, then LevelControl, then OnOff -- the reverse of
        // Speaker::Register()'s AddCluster() order above -- because ServerClusterInterfaceRegistry
        // starts clusters in the reverse of their registration order (see
        // mSeenFirstLevelChange's declaration in SpeakerAudioCoordinator.h). Matching that real
        // order here matters: it's what lets LevelControlCluster::Startup()'s own resolved-level
        // echo reach the coordinator *after* AudioControlCluster has already resolved Volume, which
        // is exactly the sequencing SpeakerAudioCoordinator::OnLevelChanged's first-call absorption
        // depends on.
        ASSERT_EQ(audioControlCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
        ASSERT_EQ(levelControlCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
        ASSERT_EQ(onOffCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    }
};

// ---------------------------------------------------------------------------
// Startup
// ---------------------------------------------------------------------------

TEST_F(SpeakerAudioCoordinatorTest, StartupForwardsResolvedStateToHardwareDelegate)
{
    // AudioControlCluster::Startup() (the first of the three Startup() calls in SetUp(), matching
    // production ordering) resolves its startup state and hands it to its delegate (the
    // coordinator), which forwards as-is.
    EXPECT_EQ(hardwareDelegate.startupCalls, 1);
    EXPECT_EQ(hardwareDelegate.lastStartupState.volume, audioControlCluster.GetVolume());
    EXPECT_FALSE(hardwareDelegate.lastStartupState.softMuted);

    // Level 254 / Volume 100 (this fixture's initial values) are already each other's scaled
    // equivalent, so LevelControlCluster::Startup()'s resolved-level echo has nothing to
    // reconcile: CurrentLevel is left untouched and no extra hardware notification is sent beyond
    // OnStartup above.
    EXPECT_EQ(levelControlCluster.GetCurrentLevel().Value(), 254u);
    EXPECT_EQ(hardwareDelegate.volumeAndMuteCalls, 0);
}

// AudioControlCluster's own Config().WithInitialVolume(...) is what establishes its Volume,
// independent of LevelControlCluster's Config().WithInitialCurrentLevel(...): the two clusters
// resolve their own initial state independently, and SpeakerAudioCoordinator::OnLevelChanged
// reconciles LevelControlCluster::Startup()'s resolved-level echo (the first OnLevelChanged call)
// to match that Volume, rather than letting it re-derive and overwrite Volume from CurrentLevel.
// Uses its own, deliberately fresh set of objects (not the SpeakerAudioCoordinatorTest fixture's):
// a coordinator only reconciles its very first OnLevelChanged call, which the fixture's own
// SetUp() already consumes, and Level 254 / Volume 100 there happen to be each other's scaled
// equivalent anyway -- neither would exercise or prove this reconciliation. This test pins it with
// genuinely disagreeing initial values instead.
TEST(SpeakerAudioCoordinatorStartupOrderingTest, StartupReconcilesLevelControlsEchoToAudioControlsOwnInitialVolume)
{
    ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);

    TimerDelegateMock mockTimer;
    MockHardwareDelegate hardwareDelegate;
    SpeakerAudioCoordinator coordinator{ hardwareDelegate };

    // Level 1 (MinLevel) would scale to Volume 1 (MinDeviceVolume) -- deliberately disagreeing
    // with AudioControl's own Volume=100 below.
    LevelControlCluster levelControlCluster{
        kTestEndpointId,
        LevelControlCluster::Config(mockTimer, coordinator).WithLighting(DataModel::NullNullable).WithInitialCurrentLevel(1)
    };
    AudioControlCluster audioControlCluster{ kTestEndpointId, coordinator, AudioControlCluster::Config().WithInitialVolume(100) };
    Testing::ClusterTester tester{ audioControlCluster };

    coordinator.SetClusters(levelControlCluster, audioControlCluster);
    // AudioControl first, LevelControl second: matches production ordering (see
    // SpeakerAudioCoordinatorTest::SetUp()'s comment above and mSeenFirstLevelChange's declaration
    // in SpeakerAudioCoordinator.h).
    ASSERT_EQ(audioControlCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    ASSERT_EQ(levelControlCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // AudioControl's own Volume=100 survives; LevelControl's Level=1 disagreed with it (would
    // scale to Volume 1), so it was reconciled up to Level 254 -- Volume 100's scaled equivalent --
    // instead of being left to silently disagree with Volume, per Speaker.adoc's requirement that
    // the two represent the same audio output level.
    EXPECT_EQ(audioControlCluster.GetVolume(), 100u);
    EXPECT_EQ(levelControlCluster.GetCurrentLevel().Value(), 254u);
    // Reconciling CurrentLevel must not re-forward another hardware notification: OnStartup
    // already told hardware the resolved Volume/SoftMuted once.
    EXPECT_EQ(hardwareDelegate.startupCalls, 1);
    EXPECT_EQ(hardwareDelegate.volumeAndMuteCalls, 0);

    Platform::MemoryShutdown();
}

// A raw OnOff command (unlike the coordinator's own ForceOnOff, used by the AudioControl-command
// tests below) is not suppressed on LevelControlCluster: LevelControlCluster is registered as an
// OnOff delegate too (as in Speaker::Register()), and its own On/Off-coupled fade choreography
// (LevelControlCluster.cpp's OnOnOffChanged/MoveToLevelCommand) reacts as well, fading CurrentLevel
// to MinLevel and back since no OnLevel is configured. Each step of that fade routes back through
// SpeakerAudioCoordinator::OnLevelChanged, so this documents the coordinator-mediated notification
// count this causes today (3, not 1) -- this is the report-flooding behavior tracked as a known,
// separate issue; this test pins current behavior rather than endorsing it.
TEST_F(SpeakerAudioCoordinatorTest, RawOnOffCommandAlsoTriggersLevelControlsOwnFadeChoreography)
{
    const int baseline = hardwareDelegate.volumeAndMuteCalls;

    EXPECT_EQ(onOffCluster.SetOnOff(false), CHIP_NO_ERROR);

    // Settles back to where it started: LevelControl's fade-to-MinLevel-and-restore has no net
    // effect on CurrentLevel/Volume, only on how many times hardware was notified along the way.
    EXPECT_EQ(levelControlCluster.GetCurrentLevel().Value(), 254u);
    EXPECT_EQ(audioControlCluster.GetVolume(), 100u);
    EXPECT_TRUE(audioControlCluster.GetSoftMuted());

    EXPECT_EQ(hardwareDelegate.volumeAndMuteCalls, baseline + 3);
    EXPECT_TRUE(hardwareDelegate.lastSoftMuted);
    EXPECT_EQ(hardwareDelegate.lastVolume, 100u);
}

// ---------------------------------------------------------------------------
// LevelControl -> AudioControl (Volume)
// ---------------------------------------------------------------------------

TEST_F(SpeakerAudioCoordinatorTest, LevelChangeScalesToVolumeAndNotifiesHardware)
{
    const int baseline = hardwareDelegate.volumeAndMuteCalls;

    // Level 1 (MinLevel) scales to Volume 1 (MinDeviceVolume).
    EXPECT_EQ(levelControlCluster.ForceCurrentLevel(1), CHIP_NO_ERROR);

    EXPECT_EQ(audioControlCluster.GetVolume(), 1u);
    EXPECT_EQ(hardwareDelegate.volumeAndMuteCalls, baseline + 1);
    EXPECT_EQ(hardwareDelegate.lastVolume, 1u);
    EXPECT_EQ(hardwareDelegate.lastSoftMuted, audioControlCluster.GetSoftMuted());
}

// ---------------------------------------------------------------------------
// AudioControl -> OnOff/LevelControl, with echo absorption
// ---------------------------------------------------------------------------

TEST_F(SpeakerAudioCoordinatorTest, MuteCommandTurnsOffOnOffWithoutTouchingLevelAndNotifiesHardwareOnce)
{
    const int baseline = hardwareDelegate.volumeAndMuteCalls;

    EXPECT_TRUE(tester.Invoke(Mute::Type()).IsSuccess());

    EXPECT_TRUE(audioControlCluster.GetSoftMuted());
    EXPECT_FALSE(onOffCluster.GetOnOff());
    // Volume did not change, so CurrentLevel is left alone (would otherwise drift via lossy
    // Level<->Volume rounding on every mute toggle).
    EXPECT_EQ(levelControlCluster.GetCurrentLevel().Value(), 254u);

    // Exactly one notification: the OnOff/LevelControl echoes this command's own writes trigger
    // are absorbed, not re-forwarded.
    EXPECT_EQ(hardwareDelegate.volumeAndMuteCalls, baseline + 1);
    EXPECT_TRUE(hardwareDelegate.lastSoftMuted);
    EXPECT_EQ(hardwareDelegate.lastVolume, audioControlCluster.GetVolume());
}

TEST_F(SpeakerAudioCoordinatorTest, MuteAtMinLevelNudgesLevelBeforeTurnedOffSoRawOnDoesNotCollapseToOff)
{
    // Drive Volume down to 1, which scales to Level 1 (MinLevel).
    SetVolume::Type setVolume;
    setVolume.newVolume = 1;
    ASSERT_TRUE(tester.Invoke(setVolume).IsSuccess());
    ASSERT_EQ(levelControlCluster.GetCurrentLevel().Value(), 1u);

    // Mute while at MinLevel. Per HandleVolumeAndMuteChange's comment, this nudges
    // LevelControlCluster's "level before off" memory to MinLevel+1 rather than leaving it at
    // MinLevel itself.
    ASSERT_TRUE(tester.Invoke(Mute::Type()).IsSuccess());
    ASSERT_FALSE(onOffCluster.GetOnOff());

    // A later raw On (unlike the coordinator's own ForceOnOff above) isn't suppressed on
    // LevelControlCluster and does derive its restore target from that same memory. Without the
    // nudge, that target would land exactly on MinLevel and MoveToLevelCommand's
    // On-command-to-MinLevel-collapses-to-Off rule (LevelControlCluster.cpp) would flip OnOff
    // straight back to false.
    EXPECT_EQ(onOffCluster.SetOnOff(true), CHIP_NO_ERROR);
    EXPECT_TRUE(onOffCluster.GetOnOff());
}

TEST_F(SpeakerAudioCoordinatorTest, UnmuteCommandTurnsOnOnOffAndNotifiesHardwareOnce)
{
    ASSERT_TRUE(tester.Invoke(Mute::Type()).IsSuccess());
    const int baseline = hardwareDelegate.volumeAndMuteCalls;

    EXPECT_TRUE(tester.Invoke(Unmute::Type()).IsSuccess());

    EXPECT_FALSE(audioControlCluster.GetSoftMuted());
    EXPECT_TRUE(onOffCluster.GetOnOff());
    EXPECT_EQ(hardwareDelegate.volumeAndMuteCalls, baseline + 1);
    EXPECT_FALSE(hardwareDelegate.lastSoftMuted);
}

TEST_F(SpeakerAudioCoordinatorTest, SetVolumeCommandScalesLevelAndNotifiesHardwareOnce)
{
    const int baseline = hardwareDelegate.volumeAndMuteCalls;

    SetVolume::Type cmd;
    cmd.newVolume = 50;
    EXPECT_TRUE(tester.Invoke(cmd).IsSuccess());

    EXPECT_EQ(audioControlCluster.GetVolume(), 50u);
    // Volume 50 scales to Level 126 (Level range [1,254] is wider than Volume range [1,100]).
    EXPECT_EQ(levelControlCluster.GetCurrentLevel().Value(), 126u);
    EXPECT_TRUE(onOffCluster.GetOnOff()); // Unaffected: SoftMuted did not change.

    EXPECT_EQ(hardwareDelegate.volumeAndMuteCalls, baseline + 1);
    EXPECT_EQ(hardwareDelegate.lastVolume, 50u);
    EXPECT_FALSE(hardwareDelegate.lastSoftMuted);
}

// ---------------------------------------------------------------------------
// MaxUserVolume clipping -> LevelControl sync
// ---------------------------------------------------------------------------

// MaxUserVolume is optional; a dedicated fixture enables it so lowering it below the current
// Volume can be exercised, proving that the clip -- delivered via AudioControlCluster::
// SetMaxUserVolume's own HandleVolumeAndMuteChange call, the same delegate hook Mute/Unmute/
// SetVolume use -- reaches LevelControl through the same SyncLevelControlToAudioState path.
struct SpeakerAudioCoordinatorMaxUserVolumeTest : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    TimerDelegateMock mockTimer;
    MockHardwareDelegate hardwareDelegate;
    SpeakerAudioCoordinator coordinator{ hardwareDelegate };

    OnOffCluster::Context onOffContext{ mockTimer, {}, OnOffCluster::Defaults{ true } };
    OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    LevelControlCluster levelControlCluster{ kTestEndpointId,
                                             LevelControlCluster::Config(mockTimer, coordinator)
                                                 .WithOnOff(onOffCluster)
                                                 .WithLighting(DataModel::NullNullable)
                                                 .WithInitialCurrentLevel(254) };

    AudioControlCluster audioControlCluster{
        kTestEndpointId, coordinator,
        AudioControlCluster::Config().WithInitialVolume(100).WithInitialSoftMuted(false).WithOptionalAttributes(
            AudioControlCluster::OptionalAttributeSet().Set<AudioControl::Attributes::MaxUserVolume::Id>())
    };

    ClusterTester tester{ audioControlCluster };

    void SetUp() override
    {
        onOffCluster.AddDelegate(&coordinator);
        onOffCluster.AddDelegate(&levelControlCluster);
        coordinator.SetClusters(levelControlCluster, audioControlCluster);

        ASSERT_EQ(audioControlCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
        ASSERT_EQ(levelControlCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
        ASSERT_EQ(onOffCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    }
};

TEST_F(SpeakerAudioCoordinatorMaxUserVolumeTest, LoweringMaxUserVolumeBelowCurrentVolumeClipsAndSyncsLevelControl)
{
    const int baseline = hardwareDelegate.volumeAndMuteCalls;

    // Volume=100 (fixture default) exceeds the new ceiling of 50, so this must clip Volume down to
    // match -- and, per Speaker.adoc, CurrentLevel must be re-scaled to keep representing it.
    EXPECT_TRUE(tester.WriteAttribute(AudioControl::Attributes::MaxUserVolume::Id, static_cast<uint16_t>(50)).IsSuccess());

    EXPECT_EQ(audioControlCluster.GetMaxUserVolume(), 50u);
    EXPECT_EQ(audioControlCluster.GetVolume(), 50u);
    // Volume 50 scales to Level 126 (Level range [1,254] is wider than Volume range [1,100]).
    EXPECT_EQ(levelControlCluster.GetCurrentLevel().Value(), 126u);
    EXPECT_TRUE(onOffCluster.GetOnOff()); // Unaffected: SoftMuted did not change.

    // Hardware is notified once, with the clipped Volume, via the same HandleVolumeAndMuteChange
    // path Mute/Unmute/SetVolume use -- not the coordinator's own SetVolume, which would be wrong
    // here since this clip is Matter-driven (an app/controller lowering the ceiling), not
    // hardware-driven.
    EXPECT_EQ(hardwareDelegate.volumeAndMuteCalls, baseline + 1);
    EXPECT_EQ(hardwareDelegate.lastVolume, 50u);
}

// ---------------------------------------------------------------------------
// Hardware-originated events (coordinator's own app-facing API)
// ---------------------------------------------------------------------------

// Unlike the AudioControl-command tests above, these go through the coordinator's own
// SetVolume/SetSoftMuted/SetPhysicallyMuted -- the entry point for e.g. a physical volume knob or
// mute button -- so hardwareDelegate.HandleVolumeAndMuteChange must NOT be called (that would be
// circular: hardware is the source of the event here, not its destination).

TEST_F(SpeakerAudioCoordinatorTest, HardwareSetVolumeScalesLevelWithoutNotifyingHardware)
{
    const int baseline = hardwareDelegate.volumeAndMuteCalls;

    EXPECT_EQ(coordinator.SetVolume(50), CHIP_NO_ERROR);

    EXPECT_EQ(audioControlCluster.GetVolume(), 50u);
    // Volume 50 scales to Level 126 (Level range [1,254] is wider than Volume range [1,100]).
    EXPECT_EQ(levelControlCluster.GetCurrentLevel().Value(), 126u);
    EXPECT_TRUE(onOffCluster.GetOnOff()); // Unaffected: SoftMuted did not change.

    EXPECT_EQ(hardwareDelegate.volumeAndMuteCalls, baseline);

    // Bypassing mHardwareDelegate does not mean bypassing IM reporting: this goes through
    // AudioControlCluster::SetVolume/LevelControlCluster::ForceCurrentLevel exactly like the
    // Matter-driven SetVolume command does, so a subscriber sees the same report either way.
    EXPECT_TRUE(tester.IsAttributeDirty(AudioControl::Attributes::Volume::Id));
    auto & dirtyList = tester.GetDirtyList();
    EXPECT_TRUE(IsPathDirty(dirtyList,
                            ConcreteAttributePath(kTestEndpointId, LevelControl::Id, LevelControl::Attributes::CurrentLevel::Id)));
}

TEST_F(SpeakerAudioCoordinatorTest, HardwareSetVolumeRejectsOutOfRangeAndLeavesLevelControlUntouched)
{
    // MaxDeviceVolume (from MockHardwareDelegate) is 100.
    EXPECT_EQ(coordinator.SetVolume(101), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Rejected before any fan-out: LevelControl must not have moved to a level AudioControl's own
    // Volume now disagrees with.
    EXPECT_EQ(audioControlCluster.GetVolume(), 100u);
    EXPECT_EQ(levelControlCluster.GetCurrentLevel().Value(), 254u);
}

TEST_F(SpeakerAudioCoordinatorTest, HardwareSetSoftMutedTurnsOffOnOffWithoutNotifyingHardware)
{
    const int baseline = hardwareDelegate.volumeAndMuteCalls;

    EXPECT_EQ(coordinator.SetSoftMuted(true), CHIP_NO_ERROR);

    EXPECT_TRUE(audioControlCluster.GetSoftMuted());
    EXPECT_FALSE(onOffCluster.GetOnOff());
    // Volume did not change, so CurrentLevel is left alone.
    EXPECT_EQ(levelControlCluster.GetCurrentLevel().Value(), 254u);

    EXPECT_EQ(hardwareDelegate.volumeAndMuteCalls, baseline);

    // As above: bypassing mHardwareDelegate does not mean bypassing IM reporting. SoftMuted goes
    // dirty directly; OnOff goes dirty via ForceOnOff -> OnOffCluster::SetOnOff.
    EXPECT_TRUE(tester.IsAttributeDirty(AudioControl::Attributes::SoftMuted::Id));
    auto & dirtyList = tester.GetDirtyList();
    EXPECT_TRUE(IsPathDirty(dirtyList, ConcreteAttributePath(kTestEndpointId, OnOff::Id, OnOff::Attributes::OnOff::Id)));
}

// PhysicallyMuted is optional and SetPhysicallyMuted never touches LevelControl/OnOff (see its
// header comment), but coordinator.SetPhysicallyMuted() still goes through mAudioControlCluster --
// populated only by SetClusters() -- so, unlike SpeakerAudioCoordinatorBEQTest below (which only
// ever routes through mHardwareDelegate), SetClusters() is still required here.
struct SpeakerAudioCoordinatorPhysicallyMutedTest : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    TimerDelegateMock mockTimer;
    MockHardwareDelegate hardwareDelegate;
    SpeakerAudioCoordinator coordinator{ hardwareDelegate };

    LevelControlCluster levelControlCluster{
        kTestEndpointId,
        LevelControlCluster::Config(mockTimer, coordinator).WithLighting(DataModel::NullNullable).WithInitialCurrentLevel(254)
    };

    AudioControlCluster audioControlCluster{
        kTestEndpointId, coordinator,
        AudioControlCluster::Config().WithInitialVolume(100).WithInitialSoftMuted(false).WithOptionalAttributes(
            AudioControlCluster::OptionalAttributeSet().Set<AudioControl::Attributes::PhysicallyMuted::Id>())
    };

    ClusterTester tester{ audioControlCluster };

    void SetUp() override
    {
        coordinator.SetClusters(levelControlCluster, audioControlCluster);
        ASSERT_EQ(audioControlCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
        ASSERT_EQ(levelControlCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    }
};

TEST_F(SpeakerAudioCoordinatorPhysicallyMutedTest, HardwareSetPhysicallyMutedIsAPureAudioControlPassthrough)
{
    const int baseline = hardwareDelegate.volumeAndMuteCalls;

    EXPECT_EQ(coordinator.SetPhysicallyMuted(true), CHIP_NO_ERROR);

    EXPECT_TRUE(audioControlCluster.GetPhysicallyMuted());
    // Per Speaker.adoc / TC-AUDIOCONTROL-2.8 step 5: SoftMuted is unaffected.
    EXPECT_FALSE(audioControlCluster.GetSoftMuted());
    EXPECT_EQ(hardwareDelegate.volumeAndMuteCalls, baseline);
}

// ---------------------------------------------------------------------------
// OnOff -> AudioControl (SoftMuted), in isolation from LevelControlCluster
// ---------------------------------------------------------------------------

// In production (Speaker::Register()), LevelControlCluster is also a delegate of OnOffCluster, so
// a raw OnOff command triggers LevelControlCluster's own On/Off-coupled fade choreography too --
// each step of which routes back through the coordinator (see
// RawOnOffCommandAlsoTriggersLevelControlsOwnFadeChoreography above for that full, real cascade).
// This fixture instead leaves LevelControlCluster unregistered so the coordinator's own
// OnOff->AudioControl sync can be asserted precisely, independent of that other cascade.
struct SpeakerAudioCoordinatorOnOffSyncTest : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    TimerDelegateMock mockTimer;
    MockHardwareDelegate hardwareDelegate;
    SpeakerAudioCoordinator coordinator{ hardwareDelegate };

    OnOffCluster::Context onOffContext{ mockTimer, {}, OnOffCluster::Defaults{ true } };
    OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    LevelControlCluster levelControlCluster{
        kTestEndpointId,
        LevelControlCluster::Config(mockTimer, coordinator).WithLighting(DataModel::NullNullable).WithInitialCurrentLevel(254)
    };

    AudioControlCluster audioControlCluster{ kTestEndpointId, coordinator,
                                             AudioControlCluster::Config().WithInitialVolume(100).WithInitialSoftMuted(false) };

    ClusterTester tester{ audioControlCluster };

    void SetUp() override
    {
        onOffCluster.AddDelegate(&coordinator); // LevelControlCluster deliberately not added.
        coordinator.SetClusters(levelControlCluster, audioControlCluster);

        // Startup order mirrors production (see SpeakerAudioCoordinatorTest::SetUp()'s comment
        // above): AudioControl, then LevelControl, then OnOff.
        ASSERT_EQ(audioControlCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
        ASSERT_EQ(levelControlCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
        ASSERT_EQ(onOffCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    }
};

TEST_F(SpeakerAudioCoordinatorOnOffSyncTest, OnOffGoingFalseSetsSoftMutedAndNotifiesHardware)
{
    const int baseline = hardwareDelegate.volumeAndMuteCalls;

    EXPECT_EQ(onOffCluster.SetOnOff(false), CHIP_NO_ERROR);

    EXPECT_TRUE(audioControlCluster.GetSoftMuted());
    EXPECT_EQ(hardwareDelegate.volumeAndMuteCalls, baseline + 1);
    EXPECT_TRUE(hardwareDelegate.lastSoftMuted);
    EXPECT_EQ(hardwareDelegate.lastVolume, audioControlCluster.GetVolume());
}

TEST_F(SpeakerAudioCoordinatorOnOffSyncTest, OnOffGoingTrueClearsSoftMutedAndNotifiesHardware)
{
    ASSERT_EQ(onOffCluster.SetOnOff(false), CHIP_NO_ERROR);
    const int baseline = hardwareDelegate.volumeAndMuteCalls;

    EXPECT_EQ(onOffCluster.SetOnOff(true), CHIP_NO_ERROR);

    EXPECT_FALSE(audioControlCluster.GetSoftMuted());
    EXPECT_EQ(hardwareDelegate.volumeAndMuteCalls, baseline + 1);
    EXPECT_FALSE(hardwareDelegate.lastSoftMuted);
}

// ---------------------------------------------------------------------------
// Bass/Mid/Treble pass-through (no LevelControl/OnOff interaction per Speaker.adoc)
// ---------------------------------------------------------------------------

struct SpeakerAudioCoordinatorBEQTest : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    MockHardwareDelegate hardwareDelegate;
    SpeakerAudioCoordinator coordinator{ hardwareDelegate };

    // BEQ has no OnOff/LevelControl counterpart, so AudioControlCluster is the only cluster
    // SpeakerAudioCoordinator needs here; LevelControl/OnOff are left unset (SetClusters() is
    // only required before a Volume/SoftMuted-affecting command runs).
    AudioControlCluster audioControlCluster{ kTestEndpointId, coordinator,
                                             AudioControlCluster::Config()
                                                 .WithFeatures(
                                                     BitFlags<AudioControl::Feature>(AudioControl::Feature::kBasicEqualizer))
                                                 .WithOptionalAttributes(AudioControlCluster::OptionalAttributeSet()
                                                                             .Set<AudioControl::Attributes::Bass::Id>()
                                                                             .Set<AudioControl::Attributes::Mid::Id>()
                                                                             .Set<AudioControl::Attributes::Treble::Id>()) };

    ClusterTester tester{ audioControlCluster };

    void SetUp() override { ASSERT_EQ(audioControlCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR); }
};

TEST_F(SpeakerAudioCoordinatorBEQTest, ForwardsBassMidTrebleToHardwareDelegate)
{
    EXPECT_TRUE(tester.WriteAttribute(AudioControl::Attributes::Bass::Id, static_cast<int16_t>(3)).IsSuccess());
    EXPECT_EQ(hardwareDelegate.bassChangedCalls, 1);

    EXPECT_TRUE(tester.WriteAttribute(AudioControl::Attributes::Mid::Id, static_cast<int16_t>(-2)).IsSuccess());
    EXPECT_EQ(hardwareDelegate.midChangedCalls, 1);

    EXPECT_TRUE(tester.WriteAttribute(AudioControl::Attributes::Treble::Id, static_cast<int16_t>(4)).IsSuccess());
    EXPECT_EQ(hardwareDelegate.trebleChangedCalls, 1);
}

// ---------------------------------------------------------------------------
// Fixed hardware limits (queried once at AudioControlCluster construction)
// ---------------------------------------------------------------------------

// AudioControlCluster is constructed with the coordinator itself as its delegate (it needs to
// intercept HandleVolumeAndMuteChange), so GetMinDeviceVolume() et al must be forwarded from the
// real hardware delegate -- otherwise AudioControlCluster would silently fall back to
// AudioControlDelegate's hardcoded defaults regardless of what the hardware delegate reports.
struct FixedLimitsHardwareDelegate : public MockHardwareDelegate
{
    FixedLimitsHardwareDelegate()
    {
        minDeviceVolume   = 10;
        maxDeviceVolume   = 80;
        maxDeviceVolumeDB = 1200;
        minCorrection     = -8;
        maxCorrection     = 6;
    }
};

struct SpeakerAudioCoordinatorFixedLimitsTest : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    FixedLimitsHardwareDelegate hardwareDelegate;
    SpeakerAudioCoordinator coordinator{ hardwareDelegate };

    AudioControlCluster audioControlCluster{ kTestEndpointId, coordinator,
                                             AudioControlCluster::Config()
                                                 .WithFeatures(
                                                     BitFlags<AudioControl::Feature>(AudioControl::Feature::kBasicEqualizer))
                                                 .WithOptionalAttributes(AudioControlCluster::OptionalAttributeSet()
                                                                             .Set<AudioControl::Attributes::MaxDeviceVolumeDB::Id>()
                                                                             .Set<AudioControl::Attributes::Bass::Id>())
                                                 .WithInitialVolume(10) };

    ClusterTester tester{ audioControlCluster };

    void SetUp() override { ASSERT_EQ(audioControlCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR); }
};

TEST_F(SpeakerAudioCoordinatorFixedLimitsTest, ForwardsHardwareDelegatesFixedLimitsToAudioControlCluster)
{
    EXPECT_EQ(audioControlCluster.GetMinDeviceVolume(), 10u);
    EXPECT_EQ(audioControlCluster.GetMaxDeviceVolume(), 80u);

    uint16_t maxDeviceVolumeDB{};
    EXPECT_EQ(tester.ReadAttribute(AudioControl::Attributes::MaxDeviceVolumeDB::Id, maxDeviceVolumeDB), CHIP_NO_ERROR);
    EXPECT_EQ(maxDeviceVolumeDB, 1200u);

    int16_t minCorrection{};
    EXPECT_EQ(tester.ReadAttribute(AudioControl::Attributes::MinCorrection::Id, minCorrection), CHIP_NO_ERROR);
    EXPECT_EQ(minCorrection, -8);

    int16_t maxCorrection{};
    EXPECT_EQ(tester.ReadAttribute(AudioControl::Attributes::MaxCorrection::Id, maxCorrection), CHIP_NO_ERROR);
    EXPECT_EQ(maxCorrection, 6);
}

} // namespace
