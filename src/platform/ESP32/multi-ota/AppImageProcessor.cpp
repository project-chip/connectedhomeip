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

#include "AppImageProcessor.h"

#include <esp_system.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {

namespace {

void HandleRestart(chip::System::Layer * systemLayer, void * appState)
{
    ChipLogProgress(SoftwareUpdate, "Rebooting...");
    esp_restart();
}

} // namespace

AppImageProcessor::~AppImageProcessor() = default;

CHIP_ERROR AppImageProcessor::Init(const SubImageHeader & entry)
{
    mEntry         = entry;
    mBytesReceived = 0;
    mPartition     = nullptr;
    mOtaHandle     = 0;
    mInitialized   = true;
    return CHIP_NO_ERROR;
}

bool AppImageProcessor::IsInitialized()
{
    return mInitialized;
}

CHIP_ERROR AppImageProcessor::IsReadyForOTA(DeviceState & state)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    state = DeviceState::kReady;
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppImageProcessor::Write(ByteSpan & block)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    if (mPartition == nullptr)
    {
        mPartition = esp_ota_get_next_update_partition(nullptr);
        VerifyOrReturnError(mPartition != nullptr, CHIP_ERROR_INTERNAL, ChipLogError(SoftwareUpdate, "No OTA partition available"));

        esp_err_t err = esp_ota_begin(mPartition, OTA_WITH_SEQUENTIAL_WRITES, &mOtaHandle);
        VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_INTERNAL,
                            ChipLogError(SoftwareUpdate, "esp_ota_begin failed: %s", esp_err_to_name(err)));

        ReturnErrorOnFailure(mHasher.Begin());
    }

    VerifyOrReturnError(mBytesReceived + block.size() <= mEntry.length, CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(SoftwareUpdate, "App image overflow"));

    esp_err_t err = esp_ota_write(mOtaHandle, block.data(), block.size());
    VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_WRITE_FAILED,
                        ChipLogError(SoftwareUpdate, "esp_ota_write failed: %s", esp_err_to_name(err)));

    ReturnErrorOnFailure(mHasher.AddData(block));
    mBytesReceived += block.size();

    // Last chunk: verify the SHA-256, then close the staged image.
    if (mBytesReceived == mEntry.length)
    {
        uint8_t digest[Crypto::kSHA256_Hash_Length];
        MutableByteSpan digestSpan(digest);
        ReturnErrorOnFailure(mHasher.Finish(digestSpan));

        VerifyOrReturnError(digestSpan.data_equal(ByteSpan(mEntry.sha256)), CHIP_ERROR_INTEGRITY_CHECK_FAILED,
                            ChipLogError(SoftwareUpdate, "App image SHA-256 mismatch"));

        err = esp_ota_end(mOtaHandle);
        VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_INTERNAL,
                            ChipLogError(SoftwareUpdate, "esp_ota_end failed: %s", esp_err_to_name(err)));
        mOtaHandle = 0;
        ChipLogProgress(SoftwareUpdate, "App image downloaded and verified (%llu bytes)",
                        static_cast<unsigned long long>(mBytesReceived));
    }

    return CHIP_NO_ERROR;
}

void AppImageProcessor::Abort(AbortContext & context)
{
    VerifyOrReturn(mInitialized);

    ChipLogProgress(SoftwareUpdate, "AppImageProcessor abort (reason=%u)", static_cast<unsigned>(context.reason));

    if (mOtaHandle != 0)
    {
        esp_ota_abort(mOtaHandle);
        mOtaHandle = 0;
    }
    mHasher.Clear();
    mPartition     = nullptr;
    mBytesReceived = 0;
    mInitialized   = false;
}

CHIP_ERROR AppImageProcessor::Apply()
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mPartition != nullptr, CHIP_ERROR_INCORRECT_STATE);
    esp_err_t err = esp_ota_set_boot_partition(mPartition);
    VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_INTERNAL,
                        ChipLogError(SoftwareUpdate, "esp_ota_set_boot_partition failed: %s", esp_err_to_name(err)));

#ifdef CONFIG_OTA_AUTO_REBOOT_ON_APPLY
    // HandleApply is called after the delayed action time has elapsed, so it is safe to schedule the restart now.
    ReturnErrorOnFailure(DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(CONFIG_OTA_AUTO_REBOOT_DELAY_MS),
                                                               HandleRestart, nullptr));
#else
    ChipLogProgress(SoftwareUpdate, "Please reboot the device manually to apply the new image");
#endif
    return CHIP_NO_ERROR;
}

} // namespace chip
