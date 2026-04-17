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

// Helper to generate raw PCM data in memory
void GeneratePcmMemory(std::vector<uint8_t> & buffer, double freq1, double freq2, double duration, bool pulse = false)
{
    const int sampleRate    = 44100;
    const int bitsPerSample = 16;
    const int numChannels   = 1;
    const int numSamples    = static_cast<int>(duration * sampleRate);
    const int dataSize      = numSamples * numChannels * (bitsPerSample / 8);

    buffer.resize(dataSize);
    uint8_t * p = buffer.data();

    // Helper to write Little-Endian 16-bit values
    auto writeVal16 = [&](uint16_t val) {
        *p++ = static_cast<uint8_t>(val & 0xFF);
        *p++ = static_cast<uint8_t>((val >> 8) & 0xFF);
    };

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
        sample += 0.6 * sin(2.0 * kPi * freq * t_note);
        // First overtone / 2nd harmonic (30% weight)
        sample += 0.3 * sin(2.0 * kPi * freq * 2.0 * t_note);
        // Second overtone / 3rd harmonic (10% weight)
        sample += 0.1 * sin(2.0 * kPi * freq * 3.0 * t_note);

        sample *= volume;

        // Scale to 16-bit signed integer PCM
        int16_t pcmSample = static_cast<int16_t>(sample * 32767.0);
        writeVal16(static_cast<uint16_t>(pcmSample));
    }

    ChipLogProgress(DeviceLayer, "Generated PCM buffer size %zu", buffer.size());
}

bool InitializeSoundResource(ma_engine * engine, const ChimeDevice::Sound & sound, PosixChimeDevice::SoundResource & resource)
{
    resource.id = sound.id;
 
    // Generate sound based on ID
    if (sound.id == 0)
    {
        GeneratePcmMemory(resource.buffer, 880, 660, 1.0, false); // Ding Dong
    }
    else if (sound.id == 1)
    {
        GeneratePcmMemory(resource.buffer, 1000, 1000, 1.0, true); // Ring Ring
    }
    else
    {
        GeneratePcmMemory(resource.buffer, 440, 440, 0.5, false); // Default beep
    }
 
    // Initialize audio buffer from raw PCM memory
    // We use 44100 Hz, 16-bit signed PCM, mono.
    // Frames count is buffer size / 2 (since each sample is 2 bytes).
    ma_audio_buffer_config config = ma_audio_buffer_config_init(ma_format_s16, 1, resource.buffer.size() / 2, resource.buffer.data(), NULL);
    config.sampleRate = 44100; // Explicitly set sample rate as it's not in the init helper arguments for audio_buffer
    
    ma_result res = ma_audio_buffer_init(&config, &resource.audioBuffer);
    if (res != MA_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to initialize audio buffer for sound %d: %d", sound.id, res);
        return false;
    }
 
    // Initialize sound from data source
    res = ma_sound_init_from_data_source(engine, &resource.audioBuffer, 0, NULL, &resource.sound);
    if (res != MA_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to initialize sound %d from data source: %d", sound.id, res);
        ma_audio_buffer_uninit(&resource.audioBuffer);
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
                ma_audio_buffer_uninit(&resource.audioBuffer);
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
