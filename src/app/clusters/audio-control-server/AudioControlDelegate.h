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

#pragma once

#include <protocols/interaction_model/StatusCode.h>

namespace chip::app::Clusters {

/** @brief
 *  Defines methods for implementing application-specific logic for the Audio Control Cluster.
 */
class AudioControlDelegate
{
public:
    AudioControlDelegate()          = default;
    virtual ~AudioControlDelegate() = default;

    /// Snapshot of all hardware-facing cluster state at startup.
    /// Passed to OnStartup() after all KVS restores, StartUp* overrides, and clamping are complete.
    struct StartupState
    {
        uint16_t volume;
        bool softMuted;
        /// BEQ equalizer bands, reflecting raw internal state regardless of whether the
        /// BasicEqualizer feature or the corresponding band attribute is enabled in this Config.
        /// The delegate is constructed alongside the same Config, so it already knows which bands
        /// are active and should ignore these fields for bands it did not enable.
        int16_t bass;
        int16_t mid;
        int16_t treble;
    };

    /**
     * Called once during Startup() after all persisted state and StartUp* overrides have been
     * resolved. Lets the application sync hardware to the effective Matter-reported startup values.
     *
     * Unlike HandleVolumeAndMuteChange, this is informational: the return value is ignored and
     * the cluster state is already committed. Default implementation is a no-op.
     */
    virtual void OnStartup(const StartupState & state) {}

    /**
     * Called when the volume or soft-muted state changes due to a Matter command (Mute, Unmute,
     * ToggleMuted, SetVolume, IncreaseVolume, DecreaseVolume). Both new values are provided even
     * if only one changed, so the implementation can apply them atomically to hardware.
     *
     * The cluster updates its internal state only if this returns Status::Success.
     *
     * @param newVolume     The resulting volume level.
     * @param newSoftMuted  The resulting soft-muted state.
     *                      Note: the device may still be physically muted even when this is false.
     */
    virtual Protocols::InteractionModel::Status HandleVolumeAndMuteChange(uint16_t newVolume, bool newSoftMuted) = 0;

    /**
     * Called when the Bass EQ attribute changes via a Matter write (BEQ feature only).
     * Default implementation is a no-op that returns Success.
     */
    virtual Protocols::InteractionModel::Status HandleBassChanged(int16_t bass) { return Protocols::InteractionModel::Status::Success; }

    /**
     * Called when the Mid EQ attribute changes via a Matter write (BEQ feature only).
     * Default implementation is a no-op that returns Success.
     */
    virtual Protocols::InteractionModel::Status HandleMidChanged(int16_t mid) { return Protocols::InteractionModel::Status::Success; }

    /**
     * Called when the Treble EQ attribute changes via a Matter write (BEQ feature only).
     * Default implementation is a no-op that returns Success.
     */
    virtual Protocols::InteractionModel::Status HandleTrebleChanged(int16_t treble)
    {
        return Protocols::InteractionModel::Status::Success;
    }
};

} // namespace chip::app::Clusters
