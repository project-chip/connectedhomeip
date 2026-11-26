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

#include "TonePlayingChime.h"

#include <cmath>
#include <cstring>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <iterator>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {
namespace {

constexpr float kPi = 3.14159265358979323846f;

// Synthesis parameters for one chime. A chime with both frequencies equal is a single
// note; otherwise the second note starts halfway through.
struct ChimeTone
{
    float firstFrequencyHz;
    float secondFrequencyHz;
    float durationSec;
    bool pulse;
};

// Chime ID is the index into both tables; keep them in sync.
const Chime::Sound kSounds[] = {
    { 0, "Ding Dong"_span },
    { 1, "Ring Ring"_span },
    { 2, "Alert Beep"_span },
};

constexpr ChimeTone kTones[] = {
    { 880.0f, 660.0f, 1.0f, false },  // Ding Dong: two notes
    { 1000.0f, 1000.0f, 1.0f, true }, // Ring Ring: pulsed single note
    { 440.0f, 440.0f, 0.5f, false },  // Alert Beep: short single note
};
static_assert(std::size(kTones) == std::size(kSounds));

// Phase is a 32-bit fraction of a full cycle, so one sample advances by
// frequency / sampleRate of 2^32.
uint32_t FrequencyToPhaseIncrement(float frequency)
{
    constexpr float kPhaseRange = 4294967296.0f;
    return static_cast<uint32_t>((frequency * kPhaseRange) / static_cast<float>(TonePlayingChime::kSampleRateHz));
}

} // namespace

TonePlayingChime::TonePlayingChime(TimerDelegate & timerDelegate, ChimeAudioSink & sink) :
    Chime(timerDelegate, Span<const Sound>(kSounds)), mSink(sink)
{}

Protocols::InteractionModel::Status TonePlayingChime::PlayChimeSound(uint8_t chimeID)
{
    auto status = Chime::PlayChimeSound(chimeID);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        return status;
    }

    if (!mSink.EnsureInitialized())
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    // Refuse an overlapping chime rather than queueing a task that would sit idle for the
    // length of the current sound.
    //
    // This deviates from TC-CHIME-2.4 step 7, which sends three PlayChimeSound commands back to
    // back and expects Success for each, with overlapping audio simply dropped. Restarting or
    // queueing playback is deliberately not implemented: the overlap window is short and a single
    // output channel cannot mix two tones.
    if (!ClaimPlayback())
    {
        return Protocols::InteractionModel::Status::Busy;
    }
    mPendingChimeID = chimeID;

    // Priority 2 sits above the CHIP event loop (priority 1) so playback is not delayed by
    // Matter processing, but below the display task so it cannot stall the UI.
    if (xTaskCreate(PlaybackTask, "chime_play", 4096, this, 2, nullptr) != pdPASS)
    {
        ChipLogError(DeviceLayer, "Chime: Failed to start the playback task");
        ReleasePlayback();
        return Protocols::InteractionModel::Status::Failure;
    }

    return Protocols::InteractionModel::Status::Success;
}

void TonePlayingChime::PlaybackTask(void * arg)
{
    auto * chime = static_cast<TonePlayingChime *>(arg);
    chime->Play(chime->mPendingChimeID);
    chime->ReleasePlayback();
    vTaskDelete(nullptr);
}

bool TonePlayingChime::ClaimPlayback()
{
    bool expected = false;
    return mPlaybackActive.compare_exchange_strong(expected, true, std::memory_order_acq_rel);
}

void TonePlayingChime::ReleasePlayback()
{
    mPlaybackActive.store(false, std::memory_order_release);
}

void TonePlayingChime::Play(uint8_t chimeID)
{
    if (chimeID >= std::size(kTones))
    {
        return;
    }
    const ChimeTone & tone = kTones[chimeID];
    EnsureSineTable();

    const uint32_t totalSamples = static_cast<uint32_t>(tone.durationSec * kSampleRateHz);
    // A two-tone chime switches to the second note halfway through; a single-tone chime
    // never reaches the boundary.
    const uint32_t noteBoundary       = (tone.firstFrequencyHz != tone.secondFrequencyHz) ? (totalSamples / 2) : totalSamples;
    const uint32_t pulsePeriodSamples = kSampleRateHz / 20;
    // exp(-4t) sampled at the output rate is a constant ratio between consecutive
    // samples, so the envelope costs one multiply per sample rather than a call to expf.
    const float envelopeDecay = std::exp(-4.0f / static_cast<float>(kSampleRateHz));

    uint32_t phase          = 0;
    uint32_t phaseIncrement = FrequencyToPhaseIncrement(tone.firstFrequencyHz);
    float envelope          = 1.0f;
    uint32_t produced       = 0;

    while (produced < totalSamples)
    {
        uint32_t count = totalSamples - produced;
        if (count > kChunkSamples)
        {
            count = kChunkSamples;
        }
        for (uint32_t i = 0; i < count; ++i)
        {
            const uint32_t index = produced + i;
            if (index == noteBoundary)
            {
                phaseIncrement = FrequencyToPhaseIncrement(tone.secondFrequencyHz);
                phase          = 0;
                envelope       = 1.0f;
            }
            // Unsigned overflow wraps the phase, so multiplying it yields the harmonics.
            float sample = 0.6f * SineAt(phase) + 0.3f * SineAt(phase * 2) + 0.1f * SineAt(phase * 3);
            sample *= envelope;
            if (tone.pulse && (((index / pulsePeriodSamples) & 1) != 0))
            {
                sample = 0.0f;
            }
            const int16_t value     = static_cast<int16_t>(sample * 28000.0f);
            mChunkBuffer[2 * i]     = value;
            mChunkBuffer[2 * i + 1] = value;
            phase += phaseIncrement;
            envelope *= envelopeDecay;
        }

        if (!mSink.Write(mChunkBuffer, count * 2 * sizeof(int16_t)))
        {
            ChipLogError(DeviceLayer, "Chime: Audio write failed, aborting tone");
            return;
        }
        produced += count;
    }

    // Flush a silent tail so the amplifier does not hold the last sample.
    std::memset(mChunkBuffer, 0, sizeof(mChunkBuffer));
    if (!mSink.Write(mChunkBuffer, sizeof(mChunkBuffer)))
    {
        ChipLogError(DeviceLayer, "Chime: Silence write failed");
    }
}

void TonePlayingChime::EnsureSineTable()
{
    if (mSineTableInitialized)
    {
        return;
    }
    for (size_t i = 0; i < kSineTableSize; ++i)
    {
        mSineTable[i] = std::sin((2.0f * kPi * static_cast<float>(i)) / static_cast<float>(kSineTableSize));
    }
    mSineTableInitialized = true;
}

} // namespace chip::app
