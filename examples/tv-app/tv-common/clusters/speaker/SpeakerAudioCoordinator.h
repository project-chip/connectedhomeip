/*
 *
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

#pragma once

#include <cstdint>

#include <app/clusters/audio-control-server/AudioControlCluster.h>
#include <app/clusters/audio-control-server/AudioControlDelegate.h>
#include <app/clusters/level-control/LevelControlCluster.h>
#include <app/clusters/level-control/LevelControlDelegate.h>
#include <app/clusters/on-off-server/OnOffCluster.h>
#include <app/clusters/on-off-server/OnOffDelegate.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/TimerDelegate.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip::app::Clusters::Speaker {

// Coordinates On/Off, Level Control and Audio Control on an MA-speaker endpoint, per the
// Speaker device type Rev 2 (Speaker.adoc, "Cluster Requirements"):
//   * OnOff.OnOff mirrors the inverse of AudioControl.SoftMuted.
//   * LevelControl.CurrentLevel and AudioControl.Volume are the same audio level, scaled
//     proportionally between [MinLevel, MaxLevel] and [MinDeviceVolume,
//     minOf(MaxDeviceVolume, MaxUserVolume)] -- which also applies the MaxUserVolume ceiling
//     to Level Control changes.
//   * PhysicallyMuted has no On/Off representation and is left alone.
//
// Ownership: the coordinator OWNS the three code-driven clusters -- it constructs them in its
// constructor (from Config) and destroys them in its destructor, and is fully wired once the
// constructor returns (every GetXxx()/XxxRegistration() accessor stays valid for its lifetime;
// no further setup, no interior null state). The owner (tv-app: InitSpeaker() in
// SpeakerEndpoint.cpp) only drives registration: pass XxxRegistration() to the registry after
// construction and &GetXxx() to Unregister() before destruction, registering Audio Control
// last so its Startup() -> OnStartup() reconcile sees the other two started.
//
// On/Off has TWO delegates -- this coordinator and the Level Control cluster itself
// (mOnOff.AddDelegate(&mLevelControl.Cluster())), as examples/all-devices-app wires a
// lighting-style device. A raw On/Off/Toggle runs Level Control's CurrentLevel choreography,
// mirrored step by step into AudioControl.Volume via OnLevelChanged(). OnLevel is kept NULL so
// that choreography restores CurrentLevel to its pre-off value instead of leaving it at
// MinLevel, keeping level -- and volume -- across a mute. Config also gets WithOnOff() for
// Level Control's own *WithOnOff command variants.
//
// No On/Off or Level Control cluster code is changed: only their public methods are called,
// and Volume is written back via AudioControl.SetVolume()/SetSoftMuted() (which report but
// don't call the Audio Control delegate, so cannot re-enter). One flag (mSyncing) suppresses
// the reverse notification while the coordinator is driving a cluster.
//
// Propagation:
//   * HandleVolumeAndMuteChange(volume, softMuted) -- from an Audio Control command, pre-commit
//     -- drives OnOff.SetOnOff(!softMuted) and, for a real volume change,
//     LevelControl.MoveToLevel(VolumeToLevel(volume)); forwards (volume, mute) to the hardware
//     delegate, whose status gates the command's commit.
//   * OnOnOffChanged(on) -- from an On/Off command -- drives AudioControl.SetSoftMuted(!on);
//     Level Control (the other On/Off delegate) separately runs its CurrentLevel choreography.
//   * OnLevelChanged(level) -- any CurrentLevel change, command- or choreography-driven --
//     drives AudioControl.SetVolume(LevelToVolume(level)).
//   * OnStartup() -- Audio Control is authoritative: OnOff and CurrentLevel are reconciled to
//     its resolved SoftMuted/Volume.
//
// Every hardware-facing Audio Control notification (volume/mute, bass/mid/treble, startup) and
// fixed-limit query is forwarded to one hardware/logging delegate.
class SpeakerAudioCoordinator final : public OnOffDelegate, public LevelControlDelegate, public AudioControlDelegate
{
public:
    // Inputs for the three clusters. Level Control fields are passed individually because its
    // Config binds the coordinator as delegate (only the coordinator can build it); the Audio
    // Control config carries no delegate and is passed by value.
    struct Config
    {
        EndpointId endpoint;
        TimerDelegate & timerDelegate;
        AudioControlDelegate & hardwareDelegate;

        // On/Off: value reported before anything is persisted.
        bool onOffStartValue = false;

        // Level Control seeds (see LevelControlCluster::Config::WithXxx()).
        DataModel::Nullable<uint8_t> levelControlStartUpCurrentLevel;
        uint16_t levelControlOnOffTransitionTime = 0;
        DataModel::Nullable<uint16_t> levelControlOnTransitionTime;
        DataModel::Nullable<uint16_t> levelControlOffTransitionTime;
        DataModel::Nullable<uint8_t> levelControlDefaultMoveRate;
        uint8_t levelControlInitialCurrentLevel = LevelControlCluster::kLightingMinLevel;
        DataModel::Nullable<uint8_t> levelControlOnLevel;

        // Audio Control: fully-built config (features, optional attributes, initial values).
        AudioControlCluster::Config audioControl;
    };

    explicit SpeakerAudioCoordinator(const Config & config);
    ~SpeakerAudioCoordinator() override;

    SpeakerAudioCoordinator(const SpeakerAudioCoordinator &)             = delete;
    SpeakerAudioCoordinator & operator=(const SpeakerAudioCoordinator &) = delete;

    // Cluster access for the owner. Valid for the whole lifetime of the coordinator.
    OnOffCluster & GetOnOff() { return mOnOff.Cluster(); }
    LevelControlCluster & GetLevelControl() { return mLevelControl.Cluster(); }
    AudioControlCluster & GetAudioControl() { return mAudioControl.Cluster(); }

    // Registration handles for the owner to pass to the data model provider's registry, in
    // this order: On/Off, then Level Control, then Audio Control.
    ServerClusterRegistration & OnOffRegistration() { return mOnOff.Registration(); }
    ServerClusterRegistration & LevelControlRegistration() { return mLevelControl.Registration(); }
    ServerClusterRegistration & AudioControlRegistration() { return mAudioControl.Registration(); }

    // OnOffDelegate
    void OnOffStartup(bool on) override;
    void OnOnOffChanged(bool on) override;

    // LevelControlDelegate
    void OnLevelChanged(uint8_t level) override;

    // AudioControlDelegate -- hardware limits are answered by the hardware delegate.
    uint16_t GetMinDeviceVolume() const override { return mHardware.GetMinDeviceVolume(); }
    uint16_t GetMaxDeviceVolume() const override { return mHardware.GetMaxDeviceVolume(); }
    uint16_t GetMaxDeviceVolumeDB() const override { return mHardware.GetMaxDeviceVolumeDB(); }
    int16_t GetMinCorrection() const override { return mHardware.GetMinCorrection(); }
    int16_t GetMaxCorrection() const override { return mHardware.GetMaxCorrection(); }
    void OnStartup(const StartupState & state) override;
    Protocols::InteractionModel::Status HandleVolumeAndMuteChange(uint16_t newVolume, bool newSoftMuted) override;
    Protocols::InteractionModel::Status HandleBassChanged(int16_t bass) override { return mHardware.HandleBassChanged(bass); }
    Protocols::InteractionModel::Status HandleMidChanged(int16_t mid) override { return mHardware.HandleMidChanged(mid); }
    Protocols::InteractionModel::Status HandleTrebleChanged(int16_t treble) override
    {
        return mHardware.HandleTrebleChanged(treble);
    }

private:
    // Proportional maps between Volume [MinDeviceVolume, EffectiveMaxVolume()] and
    // CurrentLevel [MinLevel, MaxLevel]. Range endpoints map exactly; intermediate values are
    // rounded. EffectiveMaxVolume() is recomputed each call so a runtime MaxUserVolume change
    // takes effect immediately.
    uint16_t EffectiveMaxVolume() const;
    uint8_t VolumeToLevel(uint16_t volume) const;
    uint16_t LevelToVolume(uint8_t level) const;

    // Brings OnOff and CurrentLevel into agreement with the given Audio Control state.
    void SyncFromAudioControl(uint16_t volume, bool softMuted);

    AudioControlDelegate & mHardware;

    bool mSyncing = false;

    // Owned; constructed in the constructor (On/Off, Level Control, Audio Control) and
    // destroyed here in reverse after the destructor detaches the On/Off delegates.
    LazyRegisteredServerCluster<OnOffCluster> mOnOff;
    LazyRegisteredServerCluster<LevelControlCluster> mLevelControl;
    LazyRegisteredServerCluster<AudioControlCluster> mAudioControl;
};

} // namespace chip::app::Clusters::Speaker
