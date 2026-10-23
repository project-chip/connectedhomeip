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

#include "Core2Chime.h"

#include "sdkconfig.h"
#include <bsp/esp-bsp.h>
#include <esp_codec_dev.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {
namespace {

// The Core2's speaker is a fixed-gain amplifier fed directly by I2S: the BSP opens it
// through esp_codec_dev with no actual codec chip attached (codec_if == NULL), so
// esp_codec_dev_set_out_vol() scales the PCM samples in software rather than writing a
// hardware volume register. Its default curve still maps 0..100 onto -50..0 dB; 0 is a
// hard mute.
int AttenuationToVolumePercent(int attenuationDb)
{
    constexpr int kMaxVolumePercent  = 100;
    constexpr int kMinAudiblePercent = 1;
    constexpr int kPercentPerDb      = 2;
    const int volume                 = kMaxVolumePercent - attenuationDb * kPercentPerDb;
    return volume < kMinAudiblePercent ? kMinAudiblePercent : volume;
}

class Core2AudioSink : public ChimeAudioSink
{
public:
    static Core2AudioSink & Instance()
    {
        static Core2AudioSink sink;
        return sink;
    }

    bool EnsureInitialized() override
    {
        if (mSpeaker != nullptr)
        {
            return true;
        }

        esp_codec_dev_handle_t speaker = bsp_audio_codec_speaker_init();
        if (speaker == nullptr)
        {
            ChipLogError(DeviceLayer, "Core2Chime: Failed to initialize the speaker codec");
            return false;
        }

        esp_codec_dev_sample_info_t format = {
            .bits_per_sample = 16,
            .channel         = 2,
            .channel_mask    = 0,
            .sample_rate     = TonePlayingChime::kSampleRateHz,
            .mclk_multiple   = 0,
        };
        int err = esp_codec_dev_open(speaker, &format);
        if (err != ESP_CODEC_DEV_OK)
        {
            ChipLogError(DeviceLayer, "Core2Chime: Failed to open the speaker codec: %d", err);
            return false;
        }

        err = esp_codec_dev_set_out_vol(speaker, AttenuationToVolumePercent(CONFIG_CHIME_ATTENUATION_DB));
        if (err != ESP_CODEC_DEV_OK)
        {
            ChipLogError(DeviceLayer, "Core2Chime: Failed to set the speaker volume: %d", err);
            esp_codec_dev_close(speaker);
            return false;
        }

        mSpeaker = speaker;
        return true;
    }

    bool Write(const int16_t * samples, size_t byteCount) override
    {
        int err = esp_codec_dev_write(mSpeaker, const_cast<int16_t *>(samples), static_cast<int>(byteCount));
        if (err != ESP_CODEC_DEV_OK)
        {
            ChipLogError(DeviceLayer, "Core2Chime: Codec write failed: %d", err);
            return false;
        }
        return true;
    }

private:
    Core2AudioSink() = default;

    esp_codec_dev_handle_t mSpeaker = nullptr;
};

} // namespace

Core2Chime::Core2Chime(TimerDelegate & timerDelegate) : TonePlayingChime(timerDelegate, Core2AudioSink::Instance()) {}

} // namespace chip::app
