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

// Helper to generate a simple WAV file in memory
static void GenerateWavMemory(std::vector<uint8_t> & buffer, double freq1, double freq2, double duration, bool pulse = false)
{
    const int sampleRate    = 44100;
    const int bitsPerSample = 16;
    const int numChannels   = 1;
    const int numSamples    = static_cast<int>(duration * sampleRate);
    const int dataSize      = numSamples * numChannels * (bitsPerSample / 8);
    const int totalSize     = 44 + dataSize;

    buffer.resize(totalSize);
    uint8_t * p = buffer.data();

    // Helper to write strings
    auto writeStr = [&](const char * str, size_t len) {
        memcpy(p, str, len);
        p += len;
    };

    // Helper to write Little-Endian 16-bit values
    auto writeVal16 = [&](uint16_t val) {
        *p++ = static_cast<uint8_t>(val & 0xFF);
        *p++ = static_cast<uint8_t>((val >> 8) & 0xFF);
    };

    // Helper to write Little-Endian 32-bit values
    auto writeVal32 = [&](uint32_t val) {
        *p++ = static_cast<uint8_t>(val & 0xFF);
        *p++ = static_cast<uint8_t>((val >> 8) & 0xFF);
        *p++ = static_cast<uint8_t>((val >> 16) & 0xFF);
        *p++ = static_cast<uint8_t>((val >> 24) & 0xFF);
    };

    // WAV Header
    writeStr("RIFF", sizeof("RIFF") - 1);
    uint32_t chunkSize = 36 + dataSize;
    writeVal32(chunkSize);
    writeStr("WAVE", sizeof("WAVE") - 1);
    writeStr("fmt ", sizeof("fmt ") - 1);
    uint32_t subChunk1Size = 16;
    writeVal32(subChunk1Size);
    uint16_t audioFormat = 1; // PCM
    writeVal16(audioFormat);
    uint16_t channels = numChannels;
    writeVal16(channels);
    uint32_t sRate = sampleRate;
    writeVal32(sRate);
    uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
    writeVal32(byteRate);
    uint16_t blockAlign = numChannels * (bitsPerSample / 8);
    writeVal16(blockAlign);
    uint16_t bps = bitsPerSample;
    writeVal16(bps);
    writeStr("data", sizeof("data") - 1);
    uint32_t subChunk2Size = dataSize;
    writeVal32(subChunk2Size);

    // Generate samples
    for (int i = 0; i < numSamples; ++i)
    {
        // Current time in seconds
        double t = static_cast<double>(i) / sampleRate;
        double freq;
        double t_note; // Time relative to the start of the current note

        if (pulse)
        {
            freq   = freq1;
            t_note = t;
        }
        else
        {
            // Two-tone sound: first half plays freq1, second half plays freq2
            if (t < duration / 2.0)
            {
                freq   = freq1;
                t_note = t;
            }
            else
            {
                freq   = freq2;
                t_note = t - (duration / 2.0);
            }
        }

        // Exponential decay for a natural "bell-like" volume drop
        double volume = exp(-t_note * 4.0);

        // If pulsing, toggle sound on and off at 20Hz rate
        if (pulse)
        {
            bool on = (static_cast<int>(t * 20) % 2) == 0;
            if (!on)
                volume = 0;
        }

        // Additive synthesis: combine fundamental frequency with overtones
        // to make it sound richer than a simple pure beep.
        double sample = 0;
        // Fundamental frequency (60% weight)
        sample += 0.6 * sin(2.0 * 3.14159265358979323846 * freq * t_note);
        // First overtone / 2nd harmonic (30% weight)
        sample += 0.3 * sin(2.0 * 3.14159265358979323846 * freq * 2.0 * t_note);
        // Second overtone / 3rd harmonic (10% weight)
        sample += 0.1 * sin(2.0 * 3.14159265358979323846 * freq * 3.0 * t_note);

        sample *= volume;

        // Scale to 16-bit signed integer PCM
        int16_t pcmSample = static_cast<int16_t>(sample * 32767.0);
        writeVal16(static_cast<uint16_t>(pcmSample));
    }

    ChipLogProgress(DeviceLayer, "Generated WAV buffer size %zu", buffer.size());
}

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
        const auto & sound = sounds[i];
        auto & resource    = mSoundResources[i];
        resource.id        = sound.id;

        // Generate sound based on ID
        if (sound.id == 0)
        {
            GenerateWavMemory(resource.buffer, 880, 660, 1.0, false); // Ding Dong
        }
        else if (sound.id == 1)
        {
            GenerateWavMemory(resource.buffer, 1000, 1000, 1.0, true); // Ring Ring
        }
        else
        {
            GenerateWavMemory(resource.buffer, 440, 440, 0.5, false); // Default beep
        }

        // Initialize decoder from memory
        ma_result res = ma_decoder_init_memory(resource.buffer.data(), resource.buffer.size(), NULL, &resource.decoder);
        if (res != MA_SUCCESS)
        {
            ChipLogError(DeviceLayer, "Failed to initialize decoder for sound %d: %d", sound.id, res);
            allSoundsInitialized = false;
            continue;
        }

        // Initialize sound from data source
        res = ma_sound_init_from_data_source(&mEngine, &resource.decoder, 0, NULL, &resource.sound);
        if (res != MA_SUCCESS)
        {
            ChipLogError(DeviceLayer, "Failed to initialize sound %d from data source: %d", sound.id, res);
            ma_decoder_uninit(&resource.decoder);
            allSoundsInitialized = false;
            continue;
        }

        resource.initialized = true;
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
                ma_decoder_uninit(&resource.decoder);
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
