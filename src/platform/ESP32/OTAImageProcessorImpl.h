/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include "esp_ota_ops.h"
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <lib/core/OTAImageHeader.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>

#if CONFIG_ENABLE_ENCRYPTED_OTA
#include <esp_encrypted_img.h>
#endif // CONFIG_ENABLE_ENCRYPTED_OTA

namespace chip {

class OTAImageProcessorImpl : public OTAImageProcessorInterface
{
public:
    //////////// OTAImageProcessorInterface Implementation ///////////////
    CHIP_ERROR PrepareDownload() override;
    CHIP_ERROR Finalize() override;
    CHIP_ERROR Apply() override;
    CHIP_ERROR Abort() override;
    CHIP_ERROR ProcessBlock(ByteSpan & block) override;
    void SetOTADownloader(OTADownloader * downloader) { mDownloader = downloader; };
    bool IsFirstImageRun() override;
    CHIP_ERROR ConfirmCurrentImage() override;

#if CONFIG_ENABLE_ENCRYPTED_OTA
    // @brief This API initializes the handling of encrypted OTA image
    // @param key null terminated RSA-3072 key in PEM format
    // @return CHIP_NO_ERROR on success, appropriate error code otherwise
    CHIP_ERROR InitEncryptedOTA(const CharSpan & key);
#endif // CONFIG_ENABLE_ENCRYPTED_OTA

private:
    static void HandlePrepareDownload(intptr_t context);
    static void HandleFinalize(intptr_t context);
    static void HandleAbort(intptr_t context);
    static void HandleProcessBlock(intptr_t context);
    static void HandleApply(intptr_t context);

    CHIP_ERROR SetBlock(ByteSpan & block);
    CHIP_ERROR ReleaseBlock();
    CHIP_ERROR ProcessHeader(ByteSpan & block);

    OTADownloader * mDownloader = nullptr;
    MutableByteSpan mBlock;
    const esp_partition_t * mOTAUpdatePartition = nullptr;
    esp_ota_handle_t mOTAUpdateHandle;
    OTAImageHeaderParser mHeaderParser;

#if CONFIG_ENABLE_ENCRYPTED_OTA
    void EndDecryption();

    CharSpan mKey;
    bool mEncryptedOTAEnabled                 = false;
    esp_decrypt_handle_t mOTADecryptionHandle = nullptr;
#endif // CONFIG_ENABLE_ENCRYPTED_OTA
};

} // namespace chip
