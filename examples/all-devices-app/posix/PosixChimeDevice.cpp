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
#include <lib/support/logging/CHIPLogging.h>
#include <cmath>
#include <sstream>

namespace chip {
namespace app {

// Helper to generate a simple WAV file in memory
static void GenerateWavMemory(std::vector<uint8_t>& buffer, double freq1, double freq2, double duration, bool pulse = false)
{
    std::stringstream file(std::ios::binary | std::ios::out);

    const int sampleRate = 44100;
    const int bitsPerSample = 16;
    const int numChannels = 1;
    const int numSamples = static_cast<int>(duration * sampleRate);
    const int dataSize = numSamples * numChannels * (bitsPerSample / 8);

    // WAV Header
    file.write("RIFF", 4);
    uint32_t chunkSize = 36 + dataSize;
    file.write(reinterpret_cast<const char*>(&chunkSize), 4);
    file.write("WAVE", 4);
    file.write("fmt ", 4);
    uint32_t subChunk1Size = 16;
    file.write(reinterpret_cast<const char*>(&subChunk1Size), 4);
    uint16_t audioFormat = 1; // PCM
    file.write(reinterpret_cast<const char*>(&audioFormat), 2);
    uint16_t channels = numChannels;
    file.write(reinterpret_cast<const char*>(&channels), 2);
    uint32_t sRate = sampleRate;
    file.write(reinterpret_cast<const char*>(&sRate), 4);
    uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
    file.write(reinterpret_cast<const char*>(&byteRate), 4);
    uint16_t blockAlign = numChannels * (bitsPerSample / 8);
    file.write(reinterpret_cast<const char*>(&blockAlign), 2);
    uint16_t bps = bitsPerSample;
    file.write(reinterpret_cast<const char*>(&bps), 2);
    file.write("data", 4);
    uint32_t subChunk2Size = dataSize;
    file.write(reinterpret_cast<const char*>(&subChunk2Size), 4);

    // Generate samples
    for (int i = 0; i < numSamples; ++i)
    {
        double t = static_cast<double>(i) / sampleRate;
        double freq;
        double t_note;
        
        if (pulse)
        {
            // For pulse (Ring Ring), we don't split into two notes
            freq = freq1;
            t_note = t;
        }
        else
        {
            // For chime (Ding Dong), we split into two notes
            if (t < duration / 2.0)
            {
                freq = freq1;
                t_note = t;
            }
            else
            {
                freq = freq2;
                t_note = t - (duration / 2.0);
            }
        }
        
        // Exponential decay per note
        double volume = exp(-t_note * 4.0);
        
        if (pulse)
        {
            // Pulse every 50ms (50ms on, 50ms off)
            bool on = (static_cast<int>(t * 20) % 2) == 0;
            if (!on) volume = 0;
        }
        
        // Additive synthesis for richer sound (Fundamental + Harmonics)
        double sample = 0;
        sample += 0.6 * sin(2.0 * 3.14159265358979323846 * freq * t_note);
        sample += 0.3 * sin(2.0 * 3.14159265358979323846 * freq * 2.0 * t_note);
        sample += 0.1 * sin(2.0 * 3.14159265358979323846 * freq * 3.0 * t_note);
        
        sample *= volume;
        
        int16_t pcmSample = static_cast<int16_t>(sample * 32767.0);
        
        file.write(reinterpret_cast<const char*>(&pcmSample), 2);
    }

    std::string str = file.str();
    buffer.assign(str.begin(), str.end());
    ChipLogProgress(DeviceLayer, "Generated WAV buffer size %zu", buffer.size());
}

PosixChimeDevice::PosixChimeDevice(TimerDelegate & timerDelegate, Span<const Sound> sounds) :
    ChimeDevice(timerDelegate, sounds)
{
    ma_result result = ma_engine_init(NULL, &mEngine);
    if (result != MA_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to initialize miniaudio engine: %d", result);
    }
    else
    {
        mEngineInitialized = true;
        ChipLogProgress(DeviceLayer, "Miniaudio engine initialized successfully");
        
        // Generate dummy sound buffers
        GenerateWavMemory(mChime0Buffer, 880, 660, 1.0, false); // Ding Dong
        GenerateWavMemory(mChime1Buffer, 1000, 1000, 1.0, true); // Ring Ring

        // Initialize decoders from memory
        ma_result res0 = ma_decoder_init_memory(mChime0Buffer.data(), mChime0Buffer.size(), NULL, &mDecoder0);
        ma_result res1 = ma_decoder_init_memory(mChime1Buffer.data(), mChime1Buffer.size(), NULL, &mDecoder1);

        if (res0 == MA_SUCCESS && res1 == MA_SUCCESS)
        {
            // Initialize sounds from data sources
            res0 = ma_sound_init_from_data_source(&mEngine, &mDecoder0, 0, NULL, &mSound0);
            res1 = ma_sound_init_from_data_source(&mEngine, &mDecoder1, 0, NULL, &mSound1);

            if (res0 == MA_SUCCESS && res1 == MA_SUCCESS)
            {
                mSoundsInitialized = true;
                ChipLogProgress(DeviceLayer, "In-memory sounds initialized successfully");
            }
            else
            {
                ChipLogError(DeviceLayer, "Failed to initialize sounds from data source: %d, %d", res0, res1);
            }
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to initialize decoders from memory: %d, %d", res0, res1);
        }
    }
}

PosixChimeDevice::~PosixChimeDevice()
{
    if (mSoundsInitialized)
    {
        ma_sound_uninit(&mSound0);
        ma_sound_uninit(&mSound1);
        ma_decoder_uninit(&mDecoder0);
        ma_decoder_uninit(&mDecoder1);
    }
    if (mEngineInitialized)
    {
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

    ma_sound* pSound = nullptr;
    if (chimeID == 0) pSound = &mSound0;
    else if (chimeID == 1) pSound = &mSound1;

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
        ChipLogError(DeviceLayer, "PosixChimeDevice: Invalid chimeID %d", chimeID);
    }

    return status;
}

} // namespace app
} // namespace chip
