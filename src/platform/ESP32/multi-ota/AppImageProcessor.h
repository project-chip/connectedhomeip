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
 *
 */

#pragma once
#include "SubImageProcessor.h"
#include <esp_ota_ops.h>
#include <lib/support/Span.h>

#ifdef CONFIG_ENABLE_ENCRYPTED_OTA
#include "EncryptedOTAHelper.h"
#endif

#ifdef CONFIG_ENABLE_DELTA_OTA
#include <esp_app_format.h>
#include <esp_delta_ota.h>
#endif

namespace chip {

/**
 * @brief Application-firmware sub-processor: writes the app image to the inactive OTA partition and
 *        activates it on apply.
 *
 * On-wire integrity is verified by the dispatcher. Encryption is an inherited capability of
 * EncryptedOTAHelper. Delta is built in (the patch base is the running partition, which only the
 * application can read with random access) rather than exposed as a shared helper.
 */
class AppImageProcessor : public SubImageProcessor
#ifdef CONFIG_ENABLE_ENCRYPTED_OTA
    ,
                          public EncryptedOTAHelper
#endif
{
public:
    CHIP_ERROR Init(const SubImageHeader & entry) override;
    bool IsInitialized() override;
    CHIP_ERROR IsReadyForOTA(DeviceState & state) override;
    CHIP_ERROR Write(ByteSpan & block) override;
    CHIP_ERROR Finish() override;
    void Abort(AbortContext & context) override;
    CHIP_ERROR Apply() override;

private:
    const esp_partition_t * mPartition = nullptr;
    esp_ota_handle_t mOtaHandle        = 0;
    bool mInitialized                  = false;

#ifdef CONFIG_ENABLE_DELTA_OTA
    static constexpr size_t kPatchHeaderSize = 64; // magic(4) + base SHA-256(32) + reserved

    // esp_delta_ota callbacks (cfg.user_data == this): base is the running partition, reconstructed
    // bytes go to esp_ota_write.
    static esp_err_t DeltaReadCallback(uint8_t * buf, size_t size, int srcOffset, void * arg);
    static esp_err_t DeltaWriteCallback(const uint8_t * buf, size_t size, void * arg);
    CHIP_ERROR FeedPatch(ByteSpan patch);
    // Validate/consume the CHIP patch header on the leading bytes; advances @p data / @p size.
    CHIP_ERROR VerifyAndStripPatchHeader(const uint8_t *& data, size_t & size);
    // Accumulate/verify the reconstructed image header (chip id), then write the patched output.
    esp_err_t WritePatchedOutput(const uint8_t * buf, size_t size);

    esp_delta_ota_handle_t mDeltaHandle = nullptr;
    bool mPatchHeaderVerified           = false;
    size_t mPatchHeaderRead             = 0;
    uint8_t mPatchHeader[kPatchHeaderSize];
    bool mImgHeaderVerified = false;
    size_t mImgHeaderRead   = 0;
    uint8_t mImgHeader[sizeof(esp_image_header_t)];
#endif
};

} // namespace chip
