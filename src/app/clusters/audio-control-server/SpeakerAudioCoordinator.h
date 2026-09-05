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

#pragma once

#include <app/clusters/audio-control-server/AudioControlCluster.h>
#include <app/clusters/audio-control-server/AudioControlDelegate.h>
#include <app/clusters/level-control/LevelControlCluster.h>
#include <app/clusters/level-control/LevelControlDelegate.h>
#include <app/clusters/on-off-server/OnOffDelegate.h>

namespace chip {
namespace app {
namespace Clusters {

/**
 * Keeps OnOff, LevelControl and AudioControl in sync on a Speaker endpoint, per the Speaker
 * device type's Audio Control dependencies (see Speaker.adoc):
 *   - OnOff.OnOff SHALL mirror the inverse of AudioControl.SoftMuted.
 *   - LevelControl.CurrentLevel and AudioControl.Volume SHALL represent the same audio output
 *     level, scaled proportionally between their respective ranges.
 *
 * Sits as the delegate of all three clusters and exposes AudioControlDelegate's own shape
 * (HandleVolumeAndMuteChange, HandleBassChanged/HandleMidChanged/HandleTrebleChanged, OnStartup)
 * to a single upper-level hardware delegate, so hardware backends only ever see one canonical
 * volume/mute notification regardless of which of the three clusters a change originated from.
 *
 * Lives alongside AudioControlCluster (rather than in an application's own tree) so any
 * application wiring a Speaker endpoint out of these three clusters can reuse it; it has no
 * dependency on any particular application. Opt in via the
 * "${chip_root}/src/app/clusters/audio-control-server:speaker-audio-coordinator" GN target.
 */
class SpeakerAudioCoordinator : public OnOffDelegate, public LevelControlDelegate, public AudioControlDelegate
{
public:
    explicit SpeakerAudioCoordinator(AudioControlDelegate & hardwareDelegate) : mHardwareDelegate(hardwareDelegate) {}

    // Must be called once, after all three clusters have been constructed and before any of them
    // can receive a command or write (i.e. before they are registered with the provider).
    void SetClusters(LevelControlCluster & levelControlCluster, AudioControlCluster & audioControlCluster);

    // OnOffDelegate
    void OnOffStartup(bool on) override;
    void OnOnOffChanged(bool on) override;

    // LevelControlDelegate
    void OnLevelChanged(uint8_t value) override;

    // AudioControlDelegate. This coordinator is itself the AudioControlDelegate AudioControlCluster
    // is constructed with (it needs to intercept HandleVolumeAndMuteChange below), so these fixed
    // hardware limits -- queried once at AudioControlCluster construction time -- must be forwarded
    // from the real hardware delegate, or AudioControlCluster would silently fall back to
    // AudioControlDelegate's defaults instead.
    uint16_t GetMinDeviceVolume() const override { return mHardwareDelegate.GetMinDeviceVolume(); }
    uint16_t GetMaxDeviceVolume() const override { return mHardwareDelegate.GetMaxDeviceVolume(); }
    uint16_t GetMaxDeviceVolumeDB() const override { return mHardwareDelegate.GetMaxDeviceVolumeDB(); }
    int16_t GetMinCorrection() const override { return mHardwareDelegate.GetMinCorrection(); }
    int16_t GetMaxCorrection() const override { return mHardwareDelegate.GetMaxCorrection(); }

    void OnStartup(const StartupState & state) override;
    Protocols::InteractionModel::Status HandleVolumeAndMuteChange(uint16_t newVolume, bool newSoftMuted) override;
    Protocols::InteractionModel::Status HandleBassChanged(int16_t bass) override;
    Protocols::InteractionModel::Status HandleMidChanged(int16_t mid) override;
    Protocols::InteractionModel::Status HandleTrebleChanged(int16_t treble) override;

    // Application-facing API for hardware-originated volume/mute events (e.g. a physical
    // knob/button), mirroring AudioControlCluster's own app-facing SetVolume/SetSoftMuted/
    // SetPhysicallyMuted (see that class's header comment) so a caller switching from writing the
    // cluster directly to writing through the coordinator needs no rework. Unlike
    // HandleVolumeAndMuteChange above -- which is Matter-driven and forwards to mHardwareDelegate --
    // these do NOT call mHardwareDelegate: hardware is the source of the event here, so calling
    // back into it would be circular. Each commits to AudioControlCluster first and only fans out
    // to LevelControl/OnOff once that commit succeeds, so a rejected write (e.g. Volume out of
    // range) can never leave the clusters disagreeing.
    CHIP_ERROR SetVolume(uint16_t volume);
    CHIP_ERROR SetSoftMuted(bool softMuted);
    // Pure passthrough: per Speaker.adoc (and TC-AUDIOCONTROL-2.8 step 5), PhysicallyMuted does
    // not affect OnOff/SoftMuted or LevelControl, so there is nothing to fan out.
    CHIP_ERROR SetPhysicallyMuted(bool physicallyMuted);

private:
    uint16_t ScaleLevelToVolume(uint8_t level) const;
    uint8_t ScaleVolumeToLevel(uint16_t volume) const;

    // Shared by HandleVolumeAndMuteChange and the hardware-originated setters above: pushes a
    // resolved (volume, softMuted) pair into LevelControl/OnOff via ForceOnOff/ForceCurrentLevel,
    // under mApplyingAudioControlChange so the resulting echoes are absorbed rather than re-synced.
    // oldVolume must be AudioControlCluster's Volume from immediately before this change was
    // committed to it (not re-read here) -- HandleVolumeAndMuteChange runs before that commit, but
    // the hardware-originated setters commit first, so both capture it explicitly beforehand.
    void SyncLevelControlToAudioState(uint16_t oldVolume, uint16_t newVolume, bool newSoftMuted);

    AudioControlDelegate & mHardwareDelegate;
    LevelControlCluster * mLevelControlCluster = nullptr;
    AudioControlCluster * mAudioControlCluster = nullptr;

    // Set while HandleVolumeAndMuteChange is pushing a change into OnOff/LevelControl, so the
    // resulting OnOnOffChanged/OnLevelChanged echoes are absorbed here instead of being
    // re-synced into AudioControl and re-forwarded to hardware a second time.
    bool mApplyingAudioControlChange = false;

    // Set once the first OnLevelChanged call has been seen. LevelControlCluster::Startup()
    // unconditionally fires OnLevelChanged once, synchronously, with its resolved initial
    // CurrentLevel (LevelControlCluster.cpp, guarded only by CurrentLevel being non-null -- which
    // callers of SetClusters() must therefore ensure, e.g. via Config::WithInitialCurrentLevel()) --
    // this is the only way OnLevelChanged can fire before any command has reached either cluster.
    // The cluster registry starts clusters in the *reverse* of their registration order (it's a
    // prepend-then-iterate list), so with AudioControl registered after LevelControl (as
    // Speaker::Register() does), AudioControlCluster::Startup() -- and its own resolution of
    // Volume from persisted/StartUp* state -- has already completed by the time this fires. That
    // Volume is authoritative; CurrentLevel is reconciled to match it (only if they actually
    // disagree) instead of the reverse. AudioControlCluster::OnStartup below is what notifies
    // hardware of the resolved state.
    bool mSeenFirstLevelChange = false;
};

} // namespace Clusters
} // namespace app
} // namespace chip
