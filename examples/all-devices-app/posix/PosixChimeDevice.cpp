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

namespace chip {
namespace app {

namespace {

const double kPi = 3.14159265358979323846;
const uint16_t kAudioFormatPCM = 1;
const uint32_t kSampleRate = 44100;

// Custom data source read callback
ma_result custom_data_source_read(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead)
{
    auto* pCustomDS = reinterpret_cast<PosixChimeDevice::CustomDataSource*>(pDataSource);
    if (pCustomDS == NULL) return MA_INVALID_ARGS;

    const ma_uint64 totalSamples = static_cast<ma_uint64>(pCustomDS->duration * kSampleRate);

    ma_uint64 framesToRead = frameCount;
    if (pCustomDS->cursor + framesToRead > totalSamples)
    {
        framesToRead = totalSamples - pCustomDS->cursor;
    }

    if (framesToRead == 0)
    {
        if (pFramesRead) *pFramesRead = 0;
        return MA_AT_END;
    }

    int16_t* pOut = reinterpret_cast<int16_t*>(pFramesOut);

    for (ma_uint64 i = 0; i < framesToRead; ++i)
    {
        ma_uint64 currentSample = pCustomDS->cursor + i;
        double t = static_cast<double>(currentSample) / kSampleRate;
        double freq;
        double t_note;

        if (pCustomDS->pulse)
        {
            freq = pCustomDS->freq1;
            t_note = t;
        }
        else
        {
            if (t < pCustomDS->duration / 2.0)
            {
                freq = pCustomDS->freq1;
                t_note = t;
            }
            else
            {
                freq = pCustomDS->freq2;
                t_note = t - (pCustomDS->duration / 2.0);
            }
        }

        double volume = exp(-t_note * 4.0);

        if (pCustomDS->pulse)
        {
            bool on = (static_cast<int>(t * 20) % 2) == 0;
            if (!on) volume = 0;
        }

        double sample = 0;
        sample += 0.6 * sin(2.0 * kPi * freq * t_note);
        sample += 0.3 * sin(2.0 * kPi * freq * 2.0 * t_note);
        sample += 0.1 * sin(2.0 * kPi * freq * 3.0 * t_note);

        sample *= volume;

        pOut[i] = static_cast<int16_t>(sample * 32767.0);
    }

    pCustomDS->cursor += framesToRead;
    if (pFramesRead) *pFramesRead = framesToRead;

    return (framesToRead < frameCount) ? MA_AT_END : MA_SUCCESS;
}

// Custom data source seek callback
ma_result custom_data_source_seek(ma_data_source* pDataSource, ma_uint64 frameIndex)
{
    auto* pCustomDS = reinterpret_cast<PosixChimeDevice::CustomDataSource*>(pDataSource);
    if (pCustomDS == NULL) return MA_INVALID_ARGS;

    const ma_uint64 totalSamples = static_cast<ma_uint64>(pCustomDS->duration * kSampleRate);

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

// Custom data source get data format callback
ma_result custom_data_source_get_data_format(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap)
{
    if (pFormat) *pFormat = ma_format_s16;
    if (pChannels) *pChannels = 1;
    if (pSampleRate) *pSampleRate = kSampleRate;
    if (pChannelMap && channelMapCap > 0) *pChannelMap = MA_CHANNEL_MONO;

    return MA_SUCCESS;
}

// Custom data source get cursor callback
ma_result custom_data_source_get_cursor(ma_data_source* pDataSource, ma_uint64* pCursor)
{
    auto* pCustomDS = reinterpret_cast<PosixChimeDevice::CustomDataSource*>(pDataSource);
    if (pCustomDS == NULL) return MA_INVALID_ARGS;

    if (pCursor) *pCursor = pCustomDS->cursor;
    return MA_SUCCESS;
}

// Custom data source get length callback
ma_result custom_data_source_get_length(ma_data_source* pDataSource, ma_uint64* pLength)
{
    auto* pCustomDS = reinterpret_cast<PosixChimeDevice::CustomDataSource*>(pDataSource);
    if (pCustomDS == NULL) return MA_INVALID_ARGS;

    if (pLength) *pLength = static_cast<ma_uint64>(pCustomDS->duration * kSampleRate);
    return MA_SUCCESS;
}

// Custom data source set looping callback
ma_result custom_data_source_set_looping(ma_data_source* pDataSource, ma_bool32 isLooping)
{
    // Not supported for now
    return MA_NOT_IMPLEMENTED;
}

// Define the vtable
static ma_data_source_vtable g_custom_data_source_vtable = {
    custom_data_source_read,
    custom_data_source_seek,
    custom_data_source_get_data_format,
    custom_data_source_get_cursor,
    custom_data_source_get_length,
    custom_data_source_set_looping,
    0 // flags
};

bool InitializeSoundResource(ma_engine * engine, const ChimeDevice::Sound & sound, PosixChimeDevice::SoundResource & resource)
{
    resource.id = sound.id;
 
    // Configure the custom data source based on ID
    resource.dataSource.cursor = 0;
 
    if (sound.id == 0)
    {
        resource.dataSource.freq1 = 880;
        resource.dataSource.freq2 = 660;
        resource.dataSource.duration = 1.0;
        resource.dataSource.pulse = false;
    }
    else if (sound.id == 1)
    {
        resource.dataSource.freq1 = 1000;
        resource.dataSource.freq2 = 1000;
        resource.dataSource.duration = 1.0;
        resource.dataSource.pulse = true;
    }
    else
    {
        resource.dataSource.freq1 = 440;
        resource.dataSource.freq2 = 440;
        resource.dataSource.duration = 0.5;
        resource.dataSource.pulse = false;
    }
 
    // Initialize the base data source with our vtable
    ma_data_source_config config = ma_data_source_config_init();
    config.vtable = &g_custom_data_source_vtable;
    
    ma_result res = ma_data_source_init(&config, &resource.dataSource.base);
    if (res != MA_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to initialize base data source for sound %d: %d", sound.id, res);
        return false;
    }
 
    // Initialize sound from data source
    res = ma_sound_init_from_data_source(engine, &resource.dataSource.base, 0, NULL, &resource.sound);
    if (res != MA_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to initialize sound %d from data source: %d", sound.id, res);
        ma_data_source_uninit(&resource.dataSource.base);
        return false;
    }
 
    resource.initialized = true;
    return true;
}

} // namespace

PosixChimeDevice::PosixChimeDevice(TimerDelegate & timerDelegate, Span<const Sound> sounds) : ChimeDevice(timerDelegate, sounds)
{
    ma_result result = ma_engine_init(NULL, &mEngine);
    if (result != MA_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to initialize miniaudio engine: %d", result);
        return;
    }

    mEngineInitialized = true;
    ChipLogProgress(DeviceLayer, "Miniaudio engine initialized successfully");

    mSoundResources.resize(sounds.size());

    bool allSoundsInitialized = true;
    for (size_t i = 0; i < sounds.size(); ++i)
    {
        if (!InitializeSoundResource(&mEngine, sounds[i], mSoundResources[i]))
        {
            allSoundsInitialized = false;
        }
    }

    mSoundsInitialized = allSoundsInitialized;
    if (mSoundsInitialized)
    {
        ChipLogProgress(DeviceLayer, "In-memory sounds initialized successfully");
    }
}

PosixChimeDevice::~PosixChimeDevice()
{
    if (mEngineInitialized)
    {
        for (auto & resource : mSoundResources)
        {
            if (resource.initialized)
            {
                ma_sound_uninit(&resource.sound);
                ma_data_source_uninit(&resource.dataSource.base);
            }
        }
        ma_engine_uninit(&mEngine);
        ChipLogProgress(DeviceLayer, "Miniaudio engine uninitialized");
    }
}

Protocols::InteractionModel::Status PosixChimeDevice::PlayChimeSound(uint8_t chimeID)
{
    // Call base class to log the default message
    auto status = ChimeDevice::PlayChimeSound(chimeID);

    if (!mSoundsInitialized)
    {
        ChipLogError(DeviceLayer, "PosixChimeDevice: Sounds not initialized, cannot play sound");
        return status;
    }

    ma_sound * pSound = nullptr;
    for (auto & resource : mSoundResources)
    {
        if (resource.id == chimeID && resource.initialized)
        {
            pSound = &resource.sound;
            break;
        }
    }

    if (pSound)
    {
        ChipLogProgress(DeviceLayer, "PosixChimeDevice: Attempting to play sound %d from memory", chimeID);
        ma_sound_seek_to_pcm_frame(pSound, 0);
        ma_result result = ma_sound_start(pSound);
        if (result != MA_SUCCESS)
        {
            ChipLogError(DeviceLayer, "Failed to start sound %d: %d", chimeID, result);
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "PosixChimeDevice: Invalid or uninitialized chimeID %d", chimeID);
    }

    return status;
}

} // namespace app
} // namespace chip
