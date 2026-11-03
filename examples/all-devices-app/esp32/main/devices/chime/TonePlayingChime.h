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

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <device/types/chime/Chime.h>

namespace chip::app {

/// Board-specific output for 48 kHz, 16-bit, interleaved stereo PCM. Bring-up is
/// deferred to the first chime, so a device that never chimes never powers its amplifier.
class ChimeAudioSink
{
public:
    virtual ~ChimeAudioSink() = default;

    virtual bool EnsureInitialized() = 0;

    /// Blocks until the whole buffer is queued.
    virtual bool Write(const int16_t * samples, size_t byteCount) = 0;
};

/// Synthesizes the chime sounds and plays them through a ChimeAudioSink supplied by the board.
class TonePlayingChime : public Chime
{
public:
    static constexpr uint32_t kSampleRateHz = 48000;

    TonePlayingChime(TimerDelegate & timerDelegate, ChimeAudioSink & sink);
    ~TonePlayingChime() override = default;

    Protocols::InteractionModel::Status PlayChimeSound(uint8_t chimeID) override;

private:
    static constexpr size_t kChunkSamples = 256;

    // Sine lookup table sized so that the top bits of a 32-bit phase accumulator index it
    // directly. 256 entries keep the table at 1 KB while staying well below the noise floor
    // of a chime.
    static constexpr size_t kSineTableSize    = 256;
    static constexpr uint32_t kSineTableShift = 24;

    static void PlaybackTask(void * arg);

    bool ClaimPlayback();
    void ReleasePlayback();

    void Play(uint8_t chimeID);
    void EnsureSineTable();
    float SineAt(uint32_t phase) const { return mSineTable[phase >> kSineTableShift]; }

    ChimeAudioSink & mSink;
    std::atomic<bool> mPlaybackActive{ false };
    uint8_t mPendingChimeID = 0;

    // Written only by the playback task. Members rather than locals because that task runs
    // on a small stack.
    bool mSineTableInitialized = false;
    float mSineTable[kSineTableSize];
    int16_t mChunkBuffer[kChunkSamples * 2];
};

} // namespace chip::app
