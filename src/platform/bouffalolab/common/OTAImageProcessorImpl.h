/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <lib/core/OTAImageHeader.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>

namespace chip {

#if CHIP_DEVICE_LAYER_TARGET_BL616
typedef struct _ota_header_s
{
    uint8_t header[16];

    uint8_t type[4];    // RAW XZ
    uint32_t image_len; // body len
    uint8_t pad0[8];

    uint8_t ver_hardware[16];
    uint8_t ver_software[16];

    uint8_t sha256[32];
} ota_header_s_t;

typedef struct _ota_header
{
    union
    {
        ota_header_s_t s;
        uint8_t _pad[512];
    } u;
} ota_header_t;
#endif

class OTAImageProcessorImpl : public OTAImageProcessorInterface
{
public:
    //////////// OTAImageProcessorInterface Implementation ///////////////
    CHIP_ERROR PrepareDownload() override;
    CHIP_ERROR Finalize() override;
    CHIP_ERROR Apply() override;
    CHIP_ERROR Abort() override;
    CHIP_ERROR ProcessBlock(ByteSpan & block) override;
    bool IsFirstImageRun() override;
    CHIP_ERROR ConfirmCurrentImage() override;

    void SetOTADownloader(OTADownloader * downloader) { mDownloader = downloader; }

private:
    //////////// Actual handlers for the OTAImageProcessorInterface ///////////////
    static void HandlePrepareDownload(intptr_t context);
    static void HandleFinalize(intptr_t context);
    static void HandleApply(intptr_t context);
    static void HandleAbort(intptr_t context);
    static void HandleProcessBlock(intptr_t context);

    /**
     * Called to allocate memory for mBlock if necessary and set it to block
     */
    CHIP_ERROR SetBlock(ByteSpan & block);

    /**
     * Called to release allocated memory for mBlock
     */
    CHIP_ERROR ReleaseBlock();

    MutableByteSpan mBlock;
    OTADownloader * mDownloader;
    OTAImageHeaderParser mHeaderParser;
#if CHIP_DEVICE_LAYER_TARGET_BL616
    ota_header_s_t mOtaHdr;
    uint32_t mImageTotalSize;
#endif
};

} // namespace chip
