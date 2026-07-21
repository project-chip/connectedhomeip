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
#include <PosixAudioManager.h>

#include <miniaudio.h>

#include <cmath>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <limits>
#include <vector>

namespace chip {
namespace app {

// Define the Pimpl struct containing all miniaudio-specific structures
struct PosixAudioManager::Impl
{
    struct CustomDataSource
    {
        ma_data_source_base base; // Base structure required by miniaudio
        double freq1_hz;          // Primary frequency or first tone in Hz
        double freq2_hz;          // Secondary frequency or second tone in Hz
        double duration_sec;      // Total duration of the sound in seconds
        bool pulse;               // Whether to apply a pulse modulation effect
        ma_uint64 cursor;         // Current playback position in samples
    };

    struct SoundResource
    {
        uint8_t id;
        CustomDataSource dataSource;
        ma_sound sound;
        bool initialized = false;

        ~SoundResource()
        {
            if (initialized)
            {
                ma_sound_uninit(&sound);
                ma_data_source_uninit(&dataSource.base);
            }
        }
    };

    ma_engine engine;
    bool engineInitialized = false;

    std::vector<std::unique_ptr<SoundResource>> soundResources;
    bool soundsInitialized = false;
};

namespace {

constexpr double kPi             = 3.14159265358979323846;
constexpr uint32_t kSampleRateHz = 44100;

// Custom data source read callback. Synthesizes audio samples on-the-fly.
ma_result custom_data_source_read(ma_data_source * pDataSource, void * pFramesOut, ma_uint64 frameCount, ma_uint64 * pFramesRead)
{
    auto * pCustomDS = reinterpret_cast<PosixAudioManager::Impl::CustomDataSource *>(pDataSource);
    VerifyOrReturnError(pCustomDS != nullptr, MA_INVALID_ARGS);

    auto * pOut = reinterpret_cast<int16_t *>(pFramesOut);

    const ma_uint64 totalSamples = static_cast<ma_uint64>(pCustomDS->duration_sec * kSampleRateHz);
    ma_uint64 framesToRead       = frameCount;

    // Limit read count to the remaining samples
    if (pCustomDS->cursor + framesToRead > totalSamples)
    {
        framesToRead = (totalSamples > pCustomDS->cursor) ? (totalSamples - pCustomDS->cursor) : 0;
    }

    for (ma_uint64 i = 0; i < framesToRead; ++i)
    {
        double t      = static_cast<double>(pCustomDS->cursor + i) / kSampleRateHz;
        double t_note = t;
        double freq   = pCustomDS->freq1_hz;

        // For two-tone sound ("Ding-Dong"), switch to the second frequency halfway
        if (pCustomDS->freq1_hz != pCustomDS->freq2_hz)
        {
            if (t >= (pCustomDS->duration_sec / 2.0))
            {
                freq   = pCustomDS->freq2_hz;
                t_note = t - (pCustomDS->duration_sec / 2.0); // Reset relative time for second tone
            }
        }

        // Apply exponential decay for a natural chime fade-out effect
        double volume = exp(-t_note * 4.0);

        if (pCustomDS->pulse)
        {
            // Apply a square wave modulation for the pulse effect
            bool on = (static_cast<int>(t * 20) % 2) == 0;
            if (!on)
            {
                volume = 0;
            }
        }

        // Additive synthesis: combine fundamental frequency and harmonics
        double sample = 0;
        sample += 0.6 * sin(2.0 * kPi * freq * t_note);       // Fundamental
        sample += 0.3 * sin(2.0 * kPi * freq * 2.0 * t_note); // 2nd harmonic
        sample += 0.1 * sin(2.0 * kPi * freq * 3.0 * t_note); // 3rd harmonic

        sample *= volume;

        // Scale to 16-bit signed PCM and store
        pOut[i] = static_cast<int16_t>(sample * static_cast<double>(std::numeric_limits<int16_t>::max()));
    }

    pCustomDS->cursor += framesToRead;
    if (pFramesRead)
    {
        *pFramesRead = framesToRead;
    }

    return (framesToRead < frameCount) ? MA_AT_END : MA_SUCCESS;
}

// Custom data source seek callback. Allows miniaudio to jump to a specific frame.
ma_result custom_data_source_seek(ma_data_source * pDataSource, ma_uint64 frameIndex)
{
    auto * pCustomDS = reinterpret_cast<PosixAudioManager::Impl::CustomDataSource *>(pDataSource);
    VerifyOrReturnError(pCustomDS != nullptr, MA_INVALID_ARGS);

    const ma_uint64 totalSamples = static_cast<ma_uint64>(pCustomDS->duration_sec * kSampleRateHz);

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
    {
        *pFormat = ma_format_s16; // 16-bit signed integer PCM
    }
    if (pChannels)
    {
        *pChannels = 1; // Mono
    }
    if (pSampleRate)
    {
        *pSampleRate = kSampleRateHz;
    }

    if (pChannelMap && channelMapCap > 0)
    {
        *pChannelMap = MA_CHANNEL_MONO;
    }

    return MA_SUCCESS;
}

// Custom data source get cursor callback. Returns current playback position.
ma_result custom_data_source_get_cursor(ma_data_source * pDataSource, ma_uint64 * pCursor)
{
    auto * pCustomDS = reinterpret_cast<PosixAudioManager::Impl::CustomDataSource *>(pDataSource);
    VerifyOrReturnError(pCustomDS != nullptr, MA_INVALID_ARGS);

    if (pCursor)
    {
        *pCursor = pCustomDS->cursor;
    }
    return MA_SUCCESS;
}

// Custom data source get length callback. Returns total duration in frames.
ma_result custom_data_source_get_length(ma_data_source * pDataSource, ma_uint64 * pLength)
{
    auto * pCustomDS = reinterpret_cast<PosixAudioManager::Impl::CustomDataSource *>(pDataSource);
    VerifyOrReturnError(pCustomDS != nullptr, MA_INVALID_ARGS);

    if (pLength)
    {
        *pLength = static_cast<ma_uint64>(pCustomDS->duration_sec * kSampleRateHz);
    }
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

// SoundResource Factory: Initializes a specific sound based on its ID.
std::unique_ptr<PosixAudioManager::Impl::SoundResource> CreateSoundResource(ma_engine * engine, uint8_t soundId)
{
    auto resource = std::make_unique<PosixAudioManager::Impl::SoundResource>();
    resource->id  = soundId;

    // Configure the custom data source parameters based on ID.
    // These hardcoded values define the characteristics of each chime.
    resource->dataSource.cursor = 0;

    if (soundId == 0)
    {
        // Chime 0: Two-tone "Ding-Dong" (880Hz then 660Hz)
        resource->dataSource.freq1_hz     = 880;
        resource->dataSource.freq2_hz     = 660;
        resource->dataSource.duration_sec = 1.0;
        resource->dataSource.pulse        = false;
    }
    else if (soundId == 1)
    {
        // Chime 1: Pulsing warning tone (1000Hz pulsing)
        resource->dataSource.freq1_hz     = 1000;
        resource->dataSource.freq2_hz     = 1000;
        resource->dataSource.duration_sec = 1.0;
        resource->dataSource.pulse        = true;
    }
    else
    {
        // Chime 2 (Default): Single short tone (440Hz)
        resource->dataSource.freq1_hz     = 440;
        resource->dataSource.freq2_hz     = 440;
        resource->dataSource.duration_sec = 0.5;
        resource->dataSource.pulse        = false;
    }

    // Initialize the base data source with our vtable mapping to the callbacks above
    ma_data_source_config config = ma_data_source_config_init();
    config.vtable                = &g_custom_data_source_vtable;

    ma_result res = ma_data_source_init(&config, &resource->dataSource.base);
    if (res != MA_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to initialize base data source for sound %d: %d", soundId, res);
        return nullptr;
    }

    // Initialize the sound object from the data source. Miniaudio will pull data from it during playback.
    res = ma_sound_init_from_data_source(engine, &resource->dataSource.base, 0, NULL, &resource->sound);
    if (res != MA_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to initialize sound object for sound %d: %d", soundId, res);
        ma_data_source_uninit(&resource->dataSource.base);
        return nullptr;
    }

    resource->initialized = true;
    return resource;
}

} // namespace

static const PosixAudioManager::Sound kSupportedSounds[] = {
    { 0, "Ding Dong" },
    { 1, "Ring Ring" },
};

// Defined in the source file because PosixAudioManager::Impl is an incomplete type in the header.
// Defaulting the constructor/destructor in the header would fail to compile due to std::unique_ptr requirements.
PosixAudioManager::PosixAudioManager()  = default;
PosixAudioManager::~PosixAudioManager() = default;

CHIP_ERROR PosixAudioManager::Init()
{
    if (mImpl)
    {
        return CHIP_NO_ERROR;
    }

    mImpl = std::make_unique<Impl>();

    ma_result result = ma_engine_init(NULL, &mImpl->engine);
    if (result != MA_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to initialize shared miniaudio engine: %d", result);
        mImpl.reset();
        return CHIP_ERROR_INTERNAL;
    }

    mImpl->engineInitialized = true;
    ChipLogProgress(DeviceLayer, "Shared miniaudio engine initialized successfully");

    // Pre-initialize all requested sound resources
    bool allSoundsInitialized = true;
    for (const auto & sound : kSupportedSounds)
    {
        auto resource = CreateSoundResource(&mImpl->engine, sound.id);
        if (resource)
        {
            mImpl->soundResources.push_back(std::move(resource));
        }
        else
        {
            allSoundsInitialized = false;
        }
    }

    mImpl->soundsInitialized = allSoundsInitialized;
    if (mImpl->soundsInitialized)
    {
        ChipLogProgress(DeviceLayer, "Shared miniaudio sound resources initialized successfully");
    }
    else
    {
        ChipLogError(DeviceLayer, "Some shared miniaudio sound resources failed to initialize!");
    }

    return CHIP_NO_ERROR;
}

Span<const PosixAudioManager::Sound> PosixAudioManager::GetSupportedSounds() const
{
    return Span<const Sound>(kSupportedSounds);
}

void PosixAudioManager::Shutdown()
{
    VerifyOrReturn(mImpl != nullptr);

    if (mImpl->engineInitialized)
    {
        ma_engine_stop(&mImpl->engine);
    }
    mImpl->soundResources.clear();
    if (mImpl->engineInitialized)
    {
        ma_engine_uninit(&mImpl->engine);
        ChipLogProgress(DeviceLayer, "Shared miniaudio engine uninitialized");
    }
    mImpl.reset();
}

void PosixAudioManager::SetVolume(uint8_t volume)
{
    if (mImpl && mImpl->engineInitialized)
    {
        float volumeFloat = static_cast<float>(volume) / 254.0f;
        ma_engine_set_volume(&mImpl->engine, volumeFloat);
    }
}

CHIP_ERROR PosixAudioManager::PlaySound(uint8_t soundId)
{
    if (!mImpl || !mImpl->soundsInitialized)
    {
        ChipLogError(DeviceLayer, "PosixAudioManager: Sound resources not initialized, cannot play sound");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ma_sound * pSound = nullptr;
    for (auto & resource : mImpl->soundResources)
    {
        if (resource && resource->id == soundId && resource->initialized)
        {
            pSound = &resource->sound;
            break;
        }
    }

    if (pSound)
    {
        // Rewind sound to the beginning before playing
        ma_sound_seek_to_pcm_frame(pSound, 0);

        // Start playback.
        ma_result result = ma_sound_start(pSound);
        if (result != MA_SUCCESS)
        {
            ChipLogError(DeviceLayer, "Failed to start sound %d: %d", soundId, result);
            return CHIP_ERROR_INTERNAL;
        }

        return CHIP_NO_ERROR;
    }

    ChipLogError(DeviceLayer, "PosixAudioManager: Sound ID %d not found or not initialized", soundId);
    return CHIP_ERROR_KEY_NOT_FOUND;
}

} // namespace app
} // namespace chip
