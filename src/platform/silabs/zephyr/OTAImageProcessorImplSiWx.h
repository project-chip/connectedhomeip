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

#pragma once

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <lib/core/OTAImageHeader.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>

namespace chip {

/**
 * @brief Matter OTA image processor for SiWx917 on Zephyr.
 *
 * Strips the Matter OTA header and streams the RPS payload to the NWP via
 * sl_si91x_fwup_start / sl_si91x_fwup_load. The Security Bootloader installs
 * the image from ota_swap on reboot. This path does not use MCUboot.
 */
class OTAImageProcessorImpl : public OTAImageProcessorInterface
{
public:
    CHIP_ERROR PrepareDownload() override;
    CHIP_ERROR Finalize() override;
    CHIP_ERROR Apply() override;
    CHIP_ERROR Abort() override;
    CHIP_ERROR ProcessBlock(ByteSpan & block) override;
    bool IsFirstImageRun() override;
    CHIP_ERROR ConfirmCurrentImage() override;

    void SetOTADownloader(OTADownloader * downloader) { mDownloader = downloader; }
    CHIP_ERROR Init(OTADownloader * downloader);
    static OTAImageProcessorImpl & GetDefaultInstance();

private:
    static void HandlePrepareDownload(intptr_t context);
    static void HandleFinalize(intptr_t context);
    static void HandleApply(intptr_t context);
    static void HandleAbort(intptr_t context);
    static void HandleProcessBlock(intptr_t context);

    CHIP_ERROR ProcessHeader(ByteSpan & block);
    CHIP_ERROR SetBlock(ByteSpan & block);
    CHIP_ERROR ReleaseBlock();

    static constexpr size_t kAlignmentBytes = 64;
    static constexpr uint8_t kRpsHeader     = 1;
    static constexpr uint8_t kRpsData       = 2;

    MutableByteSpan mBlock;
    OTADownloader * mDownloader = nullptr;
    OTAImageHeaderParser mHeaderParser;

    static uint32_t mWriteOffset;
    static uint16_t writeBufOffset;
    static bool mReset;
    static uint8_t mFwChunkType;
    static uint8_t writeBuffer[kAlignmentBytes];
};

} // namespace chip
