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
#include <PosixChimeDevice.h>
#include <cmath>
#include <lib/support/logging/CHIPLogging.h>
#include <sstream>

// This file implements the audio playback for the Chime cluster on POSIX systems using the
// miniaudio library. To avoid shipping large audio files or consuming significant memory with
// pre-rendered PCM buffers, this implementation uses an "incremental synthesis" approach.
//
// A custom miniaudio data source (`CustomDataSource`) is defined. When miniaudio needs more
// audio data, it calls `custom_data_source_read`, which generates the audio samples on-the-fly.
// The sounds are synthesized using additive synthesis (combining a fundamental frequency with
// harmonics) and an exponential decay to simulate a natural bell or chime fade-out.
//
// The `SoundResource` class manages the lifecycle of these miniaudio structures using RAII,
// ensuring proper cleanup when the device is destroyed.

namespace chip {
namespace app {

namespace {

constexpr double kPi             = 3.14159265358979323846;
constexpr uint32_t kSampleRateHz = 44100;

// Custom data source read callback. This is called by miniaudio to fetch more audio frames.
// It generates the audio on-the-fly instead of reading from a buffer.
ma_result custom_data_source_read(ma_data_source * pDataSource, void * pFramesOut, ma_uint64 frameCount, ma_uint64 * pFramesRead)
{
    auto * pCustomDS = reinterpret_cast<PosixChimeDevice::CustomDataSource *>(pDataSource);
    if (pCustomDS == NULL)
        return MA_INVALID_ARGS;

    // Calculate total samples for the full duration of the sound
    const ma_uint64 totalSamples = static_cast<ma_uint64>(pCustomDS->duration * kSampleRateHz);

    ma_uint64 framesToRead = frameCount;
    // Ensure we don't read past the end of the sound
    if (pCustomDS->cursor + framesToRead > totalSamples)
    {
        framesToRead = totalSamples - pCustomDS->cursor;
    }

    if (framesToRead == 0)
    {
        if (pFramesRead)
            *pFramesRead = 0;
        return MA_AT_END;
    }

    int16_t * pOut = reinterpret_cast<int16_t *>(pFramesOut);

    // Generate samples on the fly
    for (ma_uint64 i = 0; i < framesToRead; ++i)
    {
        ma_uint64 currentSample = pCustomDS->cursor + i;
        // Calculate current time in seconds
        double t = static_cast<double>(currentSample) / kSampleRateHz;
        double freq;
        double t_note;

        if (pCustomDS->pulse)
        {
            // Pulsing sound: keep same frequency
            freq   = pCustomDS->freq1;
            t_note = t;
        }
        else
        {
            // Two-tone sound (Ding-Dong): switch frequency halfway
            if (t < pCustomDS->duration / 2.0)
            {
                freq   = pCustomDS->freq1;
                t_note = t;
            }
            else
            {
                freq   = pCustomDS->freq2;
                t_note = t - (pCustomDS->duration / 2.0); // Reset relative time for second tone
            }
        }

        // Apply exponential decay for a natural chime fade-out effect
        double volume = exp(-t_note * 4.0);

        if (pCustomDS->pulse)
        {
            // Apply a square wave modulation for the pulse effect
            bool on = (static_cast<int>(t * 20) % 2) == 0;
            if (!on)
                volume = 0;
        }

        // Additive synthesis: combine fundamental frequency and harmonics
        double sample = 0;
        sample += 0.6 * sin(2.0 * kPi * freq * t_note);       // Fundamental
        sample += 0.3 * sin(2.0 * kPi * freq * 2.0 * t_note); // 2nd harmonic
        sample += 0.1 * sin(2.0 * kPi * freq * 3.0 * t_note); // 3rd harmonic

        sample *= volume;

        // Scale to 16-bit signed PCM and store
        pOut[i] = static_cast<int16_t>(sample * 32767.0);
    }

    pCustomDS->cursor += framesToRead;
    if (pFramesRead)
        *pFramesRead = framesToRead;

    return (framesToRead < frameCount) ? MA_AT_END : MA_SUCCESS;
}

// Custom data source seek callback. Allows miniaudio to jump to a specific frame.
ma_result custom_data_source_seek(ma_data_source * pDataSource, ma_uint64 frameIndex)
{
    auto * pCustomDS = reinterpret_cast<PosixChimeDevice::CustomDataSource *>(pDataSource);
    if (pCustomDS == NULL)
        return MA_INVALID_ARGS;

    const ma_uint64 totalSamples = static_cast<ma_uint64>(pCustomDS->duration * kSampleRateHz);

    // Cap the cursor at the end of the sound
    if (frameIndex > totalSamples)
    {
        pCustomDS->cursor = totalSamples;
    }
    else
    {
        pCustomDS->cursor = frameIndex;
    }

    return MA_SUCCESS;
}

// Custom data source get data format callback. Tells miniaudio what format we are generating.
ma_result custom_data_source_get_data_format(ma_data_source * pDataSource, ma_format * pFormat, ma_uint32 * pChannels,
                                             ma_uint32 * pSampleRate, ma_channel * pChannelMap, size_t channelMapCap)
{
    if (pFormat)
        *pFormat = ma_format_s16; // 16-bit signed integer PCM
    if (pChannels)
        *pChannels = 1; // Mono
    if (pSampleRate)
        *pSampleRate = kSampleRateHz;
    if (pChannelMap && channelMapCap > 0)
        *pChannelMap = MA_CHANNEL_MONO;

    return MA_SUCCESS;
}

// Custom data source get cursor callback. Returns current playback position.
ma_result custom_data_source_get_cursor(ma_data_source * pDataSource, ma_uint64 * pCursor)
{
    auto * pCustomDS = reinterpret_cast<PosixChimeDevice::CustomDataSource *>(pDataSource);
    if (pCustomDS == NULL)
        return MA_INVALID_ARGS;

    if (pCursor)
        *pCursor = pCustomDS->cursor;
    return MA_SUCCESS;
}

// Custom data source get length callback. Returns total duration in frames.
ma_result custom_data_source_get_length(ma_data_source * pDataSource, ma_uint64 * pLength)
{
    auto * pCustomDS = reinterpret_cast<PosixChimeDevice::CustomDataSource *>(pDataSource);
    if (pCustomDS == NULL)
        return MA_INVALID_ARGS;

    if (pLength)
        *pLength = static_cast<ma_uint64>(pCustomDS->duration * kSampleRateHz);
    return MA_SUCCESS;
}

// Custom data source set looping callback.
ma_result custom_data_source_set_looping(ma_data_source * pDataSource, ma_bool32 isLooping)
{
    // Not supported for now as chime sounds are usually one-shot.
    return MA_NOT_IMPLEMENTED;
}

// Define the vtable for the custom data source, mapping callbacks to miniaudio operations.
static ma_data_source_vtable g_custom_data_source_vtable = {
    custom_data_source_read,
    custom_data_source_seek,
    custom_data_source_get_data_format,
    custom_data_source_get_cursor,
    custom_data_source_get_length,
    custom_data_source_set_looping,
    0 // flags
};

} // namespace

// SoundResource Constructor: Initializes a specific sound based on its ID.
PosixChimeDevice::SoundResource::SoundResource(ma_engine * engine, const ChimeDevice::Sound & soundInfo)
{
    id = soundInfo.id;

    // Configure the custom data source parameters based on ID.
    // These hardcoded values define the characteristics of each chime.
    dataSource.cursor = 0;

    if (soundInfo.id == 0)
    {
        // Chime 0: Two-tone "Ding-Dong" (880Hz then 660Hz)
        dataSource.freq1    = 880;
        dataSource.freq2    = 660;
        dataSource.duration = 1.0;
        dataSource.pulse    = false;
    }
    else if (soundInfo.id == 1)
    {
        // Chime 1: Pulsing warning tone (1000Hz pulsing)
        dataSource.freq1    = 1000;
        dataSource.freq2    = 1000;
        dataSource.duration = 1.0;
        dataSource.pulse    = true;
    }
    else
    {
        // Chime 2 (Default): Single short tone (440Hz)
        dataSource.freq1    = 440;
        dataSource.freq2    = 440;
        dataSource.duration = 0.5;
        dataSource.pulse    = false;
    }

    // Initialize the base data source with our vtable mapping to the callbacks above
    ma_data_source_config config = ma_data_source_config_init();
    config.vtable                = &g_custom_data_source_vtable;

    ma_result res = ma_data_source_init(&config, &dataSource.base);
    if (res != MA_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to initialize base data source for sound %d: %d", soundInfo.id, res);
        return;
    }

    // Initialize the sound object from the data source. Miniaudio will pull data from it during playback.
    res = ma_sound_init_from_data_source(engine, &dataSource.base, 0, NULL, &sound);
    if (res != MA_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to initialize sound %d from data source: %d", soundInfo.id, res);
        ma_data_source_uninit(&dataSource.base);
        return;
    }

    mInitialized = true;
}

// SoundResource Destructor: Ensures safe cleanup of miniaudio structures.
PosixChimeDevice::SoundResource::~SoundResource()
{
    if (mInitialized)
    {
        ma_sound_uninit(&sound);
        ma_data_source_uninit(&dataSource.base);
    }
}

// PosixChimeDevice Constructor: Initializes the audio engine and pre-loads sound resources.
PosixChimeDevice::PosixChimeDevice(TimerDelegate & timerDelegate, Span<const Sound> sounds) : ChimeDevice(timerDelegate, sounds)
{
    // Initialize the miniaudio engine with default configuration
    ma_result result = ma_engine_init(NULL, &mEngine);
    if (result != MA_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to initialize miniaudio engine: %d", result);
        return;
    }

    mEngineInitialized = true;
    ChipLogProgress(DeviceLayer, "Miniaudio engine initialized successfully");

    // Pre-initialize all requested sound resources
    bool allSoundsInitialized = true;
    for (size_t i = 0; i < sounds.size(); ++i)
    {
        auto resource = std::make_unique<SoundResource>(&mEngine, sounds[i]);
        if (!resource->mInitialized)
        {
            allSoundsInitialized = false;
        }
        mSoundResources.push_back(std::move(resource));
    }

    mSoundsInitialized = allSoundsInitialized;
    if (mSoundsInitialized)
    {
        ChipLogProgress(DeviceLayer, "In-memory sounds initialized successfully");
    }
}

// PosixChimeDevice Destructor: Cleans up resources and shuts down the engine.
PosixChimeDevice::~PosixChimeDevice()
{
    if (mEngineInitialized)
    {
        // Clear the vector first to trigger SoundResource destructors before engine shutdown
        mSoundResources.clear();
        ma_engine_uninit(&mEngine);
        ChipLogProgress(DeviceLayer, "Miniaudio engine uninitialized");
    }
}

// PlayChimeSound: Triggers the playback of a sound by its ID.
Protocols::InteractionModel::Status PosixChimeDevice::PlayChimeSound(uint8_t chimeID)
{
    // Call base class to log the default message
    auto status = ChimeDevice::PlayChimeSound(chimeID);

    if (!mSoundsInitialized)
    {
        ChipLogError(DeviceLayer, "PosixChimeDevice: Sounds not initialized, cannot play sound");
        return status;
    }

    // Find the requested sound resource by ID
    ma_sound * pSound = nullptr;
    for (auto & resource : mSoundResources)
    {
        if (resource && resource->id == chimeID && resource->mInitialized)
        {
            pSound = &resource->sound;
            break;
        }
    }

    if (pSound)
    {
        ChipLogProgress(DeviceLayer, "PosixChimeDevice: Attempting to play sound %d from memory", chimeID);

        // Rewind sound to the beginning before playing
        ma_sound_seek_to_pcm_frame(pSound, 0);

        // Start playback. This will trigger callbacks to custom_data_source_read.
        ma_result result = ma_sound_start(pSound);
        if (result != MA_SUCCESS)
        {
            ChipLogError(DeviceLayer, "Failed to start sound %d: %d", chimeID, result);
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "PosixChimeDevice: Sound %d not found in memory", chimeID);
    }

    return status;
}

} // namespace app
} // namespace chip
