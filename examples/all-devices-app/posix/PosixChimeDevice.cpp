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
#include <fstream>

namespace chip {
namespace app {

// Helper to generate a simple WAV file
static void GenerateWavFile(const char* filename, double freq1, double freq2, double duration, bool pulse = false)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        ChipLogError(DeviceLayer, "Failed to open file %s for writing", filename);
        return;
    }

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
        double freq = (t < duration / 2.0) ? freq1 : freq2;
        
        double volume = 1.0 - (t / duration);
        
        if (pulse)
        {
            // Pulse every 50ms (10Hz)
            bool on = (static_cast<int>(t * 20) % 2) == 0;
            if (!on) volume = 0;
        }
        
        double sample = volume * sin(2.0 * 3.14159265358979323846 * freq * t);
        int16_t pcmSample = static_cast<int16_t>(sample * 32767.0);
        
        file.write(reinterpret_cast<const char*>(&pcmSample), 2);
    }

    file.close();
    ChipLogProgress(DeviceLayer, "Generated WAV file %s", filename);
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
        
        // Generate dummy sound files
        GenerateWavFile("chime_0.wav", 880, 660, 1.0, false); // Ding Dong
        GenerateWavFile("chime_1.wav", 1000, 1000, 1.0, true); // Ring Ring
    }
}

PosixChimeDevice::~PosixChimeDevice()
{
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

    if (!mEngineInitialized)
    {
        ChipLogError(DeviceLayer, "PosixChimeDevice: Engine not initialized, cannot play sound");
        return status;
    }

    char filePath[32];
    snprintf(filePath, sizeof(filePath), "chime_%d.wav", chimeID);

    ChipLogProgress(DeviceLayer, "PosixChimeDevice: Attempting to play sound %s", filePath);
    ma_result result = ma_engine_play_sound(&mEngine, filePath, NULL);
    if (result != MA_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to play sound %s: %d", filePath, result);
    }

    return status;
}

} // namespace app
} // namespace chip
