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

#ifdef CONFIG_ENABLE_DELTA_OTA
#include <crypto/CHIPCryptoPAL.h>
#include <esp_partition.h>
#include <string.h>
#endif

namespace chip {

namespace {

void HandleRestart(chip::System::Layer * systemLayer, void * appState)
{
    ChipLogProgress(SoftwareUpdate, "Rebooting...");
    esp_restart();
}

} // namespace

CHIP_ERROR AppImageProcessor::Init(const SubImageHeader &)
{
    mPartition   = nullptr;
    mOtaHandle   = 0;
    mInitialized = true;
#ifdef CONFIG_ENABLE_DELTA_OTA
    mPatchHeaderVerified = false;
    mPatchHeaderRead     = 0;
    mImgHeaderVerified   = false;
    mImgHeaderRead       = 0;
#endif
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

    // On the first chunk, open the partition
    if (mPartition == nullptr)
    {
        mPartition = esp_ota_get_next_update_partition(nullptr);
        VerifyOrReturnError(mPartition != nullptr, CHIP_ERROR_INTERNAL, ChipLogError(SoftwareUpdate, "No OTA partition available"));
#ifdef CONFIG_ENABLE_DELTA_OTA
        // The reconstructed image size is unknown for a delta patch.
        esp_err_t err = esp_ota_begin(mPartition, OTA_SIZE_UNKNOWN, &mOtaHandle);
#else
        esp_err_t err = esp_ota_begin(mPartition, OTA_WITH_SEQUENTIAL_WRITES, &mOtaHandle);
#endif
        VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_INTERNAL,
                            ChipLogError(SoftwareUpdate, "esp_ota_begin failed: %s", esp_err_to_name(err)));

#ifdef CONFIG_ENABLE_ENCRYPTED_OTA
        if (IsEncryptedOTAEnabled())
        {
            ReturnErrorOnFailure(DecryptStart());
        }
#endif

#ifdef CONFIG_ENABLE_DELTA_OTA
        esp_delta_ota_cfg_t cfg     = {};
        cfg.user_data               = this;
        cfg.read_cb_with_user_data  = &AppImageProcessor::DeltaReadCallback;
        cfg.write_cb_with_user_data = &AppImageProcessor::DeltaWriteCallback;
        mDeltaHandle                = esp_delta_ota_init(&cfg);
        VerifyOrReturnError(mDeltaHandle != nullptr, CHIP_ERROR_INTERNAL,
                            ChipLogError(SoftwareUpdate, "esp_delta_ota_init failed"));
#endif
    }

    // Decrypt chunk if needed
    ByteSpan bytes = block;
#ifdef CONFIG_ENABLE_ENCRYPTED_OTA
    if (IsDecrypting())
    {
        ReturnErrorOnFailure(Decrypt(block, bytes));
        VerifyOrReturnError(!bytes.empty(), CHIP_NO_ERROR);
    }
#endif

    // Apply as a delta patch, or write straight to the partition.
#ifdef CONFIG_ENABLE_DELTA_OTA
    return FeedPatch(bytes);
#else
    esp_err_t err = esp_ota_write(mOtaHandle, bytes.data(), bytes.size());
    VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_WRITE_FAILED,
                        ChipLogError(SoftwareUpdate, "esp_ota_write failed: %s", esp_err_to_name(err)));
    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR AppImageProcessor::Finish()
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

#ifdef CONFIG_ENABLE_ENCRYPTED_OTA
    if (IsDecrypting())
    {
        ReturnErrorOnFailure(DecryptEnd());
    }
#endif
#ifdef CONFIG_ENABLE_DELTA_OTA
    if (mDeltaHandle != nullptr)
    {
        esp_err_t derr = esp_delta_ota_finalize(mDeltaHandle);
        VerifyOrReturnError(derr == ESP_OK, CHIP_ERROR_INTERNAL,
                            ChipLogError(SoftwareUpdate, "esp_delta_ota_finalize failed: %s", esp_err_to_name(derr)));
        derr         = esp_delta_ota_deinit(mDeltaHandle);
        mDeltaHandle = nullptr;
        VerifyOrReturnError(derr == ESP_OK, CHIP_ERROR_INTERNAL,
                            ChipLogError(SoftwareUpdate, "esp_delta_ota_deinit failed: %s", esp_err_to_name(derr)));
    }
#endif

    esp_err_t err = esp_ota_end(mOtaHandle);
    VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_INTERNAL,
                        ChipLogError(SoftwareUpdate, "esp_ota_end failed: %s", esp_err_to_name(err)));
    mOtaHandle = 0;
    ChipLogProgress(SoftwareUpdate, "App image written to OTA partition");
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
#ifdef CONFIG_ENABLE_ENCRYPTED_OTA
    DecryptAbort();
#endif
#ifdef CONFIG_ENABLE_DELTA_OTA
    if (mDeltaHandle != nullptr)
    {
        esp_delta_ota_deinit(mDeltaHandle);
        mDeltaHandle = nullptr;
    }
    mPatchHeaderVerified = false;
    mPatchHeaderRead     = 0;
    mImgHeaderVerified   = false;
    mImgHeaderRead       = 0;
#endif
    mPartition   = nullptr;
    mInitialized = false;
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

#ifdef CONFIG_ENABLE_DELTA_OTA

esp_err_t AppImageProcessor::DeltaReadCallback(uint8_t * buf, size_t size, int srcOffset, void * arg)
{
    VerifyOrReturnValue(buf != nullptr && size > 0, ESP_ERR_INVALID_ARG);
    const esp_partition_t * running = esp_ota_get_running_partition();
    VerifyOrReturnValue(running != nullptr, ESP_FAIL);
    return esp_partition_read(running, srcOffset, buf, size);
}

esp_err_t AppImageProcessor::DeltaWriteCallback(const uint8_t * buf, size_t size, void * arg)
{
    auto * self = static_cast<AppImageProcessor *>(arg);
    VerifyOrReturnValue(self != nullptr && buf != nullptr && size > 0, ESP_ERR_INVALID_ARG);
    return self->WritePatchedOutput(buf, size);
}

esp_err_t AppImageProcessor::WritePatchedOutput(const uint8_t * buf, size_t size)
{
    size_t index = 0;

    // Accumulate and verify the reconstructed image header (chip id) before writing it out.
    if (!mImgHeaderVerified)
    {
        const size_t need = sizeof(esp_image_header_t) - mImgHeaderRead;
        if (size < need)
        {
            memcpy(mImgHeader + mImgHeaderRead, buf, size);
            mImgHeaderRead += size;
            return ESP_OK;
        }
        memcpy(mImgHeader + mImgHeaderRead, buf, need);
        index          = need;
        mImgHeaderRead = 0;

        const esp_image_header_t * header = reinterpret_cast<const esp_image_header_t *>(mImgHeader);
        if (header->chip_id != CONFIG_IDF_FIRMWARE_CHIP_ID)
        {
            ChipLogError(SoftwareUpdate, "Delta: chip id mismatch (got %d, expected %d)", header->chip_id,
                         CONFIG_IDF_FIRMWARE_CHIP_ID);
            return ESP_ERR_INVALID_VERSION;
        }
        mImgHeaderVerified = true;

        esp_err_t err = esp_ota_write(mOtaHandle, mImgHeader, sizeof(esp_image_header_t));
        VerifyOrReturnValue(err == ESP_OK, err);
    }

    // The image header may have consumed the whole chunk; nothing left to write.
    if (size == index)
    {
        return ESP_OK;
    }
    return esp_ota_write(mOtaHandle, buf + index, size - index);
}

CHIP_ERROR AppImageProcessor::VerifyAndStripPatchHeader(const uint8_t *& data, size_t & size)
{
    const size_t need = kPatchHeaderSize - mPatchHeaderRead;
    if (size < need)
    {
        memcpy(mPatchHeader + mPatchHeaderRead, data, size);
        mPatchHeaderRead += size;
        data += size;
        size = 0; // whole chunk consumed by the header; more is needed
        return CHIP_NO_ERROR;
    }

    memcpy(mPatchHeader + mPatchHeaderRead, data, need);
    mPatchHeaderRead = 0;
    data += need;
    size -= need;

    const uint32_t kEspDeltaOtaMagic = 0xfccdde10;
    uint32_t magic;
    memcpy(&magic, mPatchHeader, sizeof(magic));
    VerifyOrReturnError(magic == kEspDeltaOtaMagic, CHIP_ERROR_INVALID_FILE_IDENTIFIER,
                        ChipLogError(SoftwareUpdate, "Delta: invalid patch magic"));

    // The patch header's base digest must match the running image (the base it is applied against).
    uint8_t baseDigest[Crypto::kSHA256_Hash_Length] = { 0 };
    VerifyOrReturnError(esp_partition_get_sha256(esp_ota_get_running_partition(), baseDigest) == ESP_OK, CHIP_ERROR_INTERNAL,
                        ChipLogError(SoftwareUpdate, "esp_partition_get_sha256 failed"));
    VerifyOrReturnError(memcmp(mPatchHeader + sizeof(magic), baseDigest, sizeof(baseDigest)) == 0,
                        CHIP_ERROR_INVALID_FILE_IDENTIFIER,
                        ChipLogError(SoftwareUpdate, "Delta: patch base does not match running image"));

    mPatchHeaderVerified = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppImageProcessor::FeedPatch(ByteSpan patch)
{
    const uint8_t * data = patch.data();
    size_t size          = patch.size();

    if (!mPatchHeaderVerified)
    {
        ReturnErrorOnFailure(VerifyAndStripPatchHeader(data, size));
        VerifyOrReturnError(size > 0, CHIP_NO_ERROR); // whole chunk was the patch header
    }

    esp_err_t err = esp_delta_ota_feed_patch(mDeltaHandle, data, size);
    VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_WRITE_FAILED,
                        ChipLogError(SoftwareUpdate, "esp_delta_ota_feed_patch failed: %s", esp_err_to_name(err)));
    return CHIP_NO_ERROR;
}

#endif // CONFIG_ENABLE_DELTA_OTA

} // namespace chip
