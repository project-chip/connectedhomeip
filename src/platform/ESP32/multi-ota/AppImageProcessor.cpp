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

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {

AppImageProcessor::AppImageProcessor()  = default;
AppImageProcessor::~AppImageProcessor() = default;

CHIP_ERROR AppImageProcessor::Init(const SubImageHeader & entry)
{
    mEntry         = entry;
    mTotalLength   = entry.length;
    mBytesReceived = 0;
    mPartition     = nullptr;
    mOtaHandle     = 0;
    mState         = DeviceState::kReady;
    return CHIP_NO_ERROR;
}

bool AppImageProcessor::IsInitialized()
{
    return mState != DeviceState::kUnknown;
}

CHIP_ERROR AppImageProcessor::IsReadyForOTA(DeviceState & state)
{
    state = DeviceState::kReady;
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppImageProcessor::Write(ByteSpan & block)
{
    // Lazily open the inactive OTA partition and start hashing on the first chunk.
    if (mPartition == nullptr)
    {
        mPartition = esp_ota_get_next_update_partition(nullptr);
        VerifyOrReturnError(mPartition != nullptr, CHIP_ERROR_INTERNAL, ChipLogError(SoftwareUpdate, "No OTA partition available"));

        esp_err_t err = esp_ota_begin(mPartition, OTA_WITH_SEQUENTIAL_WRITES, &mOtaHandle);
        VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_INTERNAL,
                            ChipLogError(SoftwareUpdate, "esp_ota_begin failed: %s", esp_err_to_name(err)));

        ReturnErrorOnFailure(mHasher.Begin());
    }

    VerifyOrReturnError(mBytesReceived + block.size() <= mTotalLength, CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(SoftwareUpdate, "App image overflow"));

    esp_err_t err = esp_ota_write(mOtaHandle, block.data(), block.size());
    VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_WRITE_FAILED,
                        ChipLogError(SoftwareUpdate, "esp_ota_write failed: %s", esp_err_to_name(err)));

    ReturnErrorOnFailure(mHasher.AddData(block));
    mBytesReceived += block.size();

    // Last chunk: verify the SHA-256, then close the staged image.
    if (mBytesReceived == mTotalLength)
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
    ChipLogProgress(SoftwareUpdate, "AppImageProcessor abort (reason=%u)", static_cast<unsigned>(context.reason));

    // Discard partial state. Clearing mPartition makes a later Apply() a no-op (stays on old firmware).
    if (mOtaHandle != 0)
    {
        esp_ota_abort(mOtaHandle);
        mOtaHandle = 0;
    }
    mHasher.Clear();
    mPartition     = nullptr;
    mBytesReceived = 0;
    mState         = DeviceState::kUnknown;
}

CHIP_ERROR AppImageProcessor::Apply()
{
    VerifyOrReturnError(mPartition != nullptr, CHIP_ERROR_INCORRECT_STATE);
    esp_err_t err = esp_ota_set_boot_partition(mPartition);
    VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_INTERNAL,
                        ChipLogError(SoftwareUpdate, "esp_ota_set_boot_partition failed: %s", esp_err_to_name(err)));

    ChipLogProgress(SoftwareUpdate, "App image set as boot partition; reboot to apply");
    return CHIP_NO_ERROR;
}

} // namespace chip
