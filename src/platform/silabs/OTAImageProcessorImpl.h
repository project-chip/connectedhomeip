/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <lib/core/OTAImageHeader.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>

#include <fstream>

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
    bool IsFirstImageRun() override;
    CHIP_ERROR ConfirmCurrentImage() override;

    void SetOTADownloader(OTADownloader * downloader) { mDownloader = downloader; }
    void SetOTAImageFile(const char * imageFile) { mImageFile = imageFile; }

private:
    //////////// Actual handlers for the OTAImageProcessorInterface ///////////////
    static void HandlePrepareDownload(intptr_t context);
    static void HandleFinalize(intptr_t context);
    static void HandleApply(intptr_t context);
    static void HandleAbort(intptr_t context);
    static void HandleProcessBlock(intptr_t context);
    CHIP_ERROR ProcessHeader(ByteSpan & block);

    /**
     * Called to allocate memory for mBlock if necessary and set it to block
     */
    CHIP_ERROR SetBlock(ByteSpan & block);

    /**
     * Called to release allocated memory for mBlock
     */
    CHIP_ERROR ReleaseBlock();

    // EFR32 platform creates a single instance of OTAImageProcessorImpl class.
    // If that changes then the use of static members and functions must be revisited
    static uint32_t mWriteOffset; // End of last written block
    static uint8_t mSlotId;       // Bootloader storage slot
    MutableByteSpan mBlock;
    OTADownloader * mDownloader;
    OTAImageHeaderParser mHeaderParser;
    const char * mImageFile                 = nullptr;
    static constexpr size_t kAlignmentBytes = 64;
    // Intermediate, word-aligned buffer for writing to the bootloader storage.
    // Bootloader storage API requires the buffer size to be a multiple of 4.
    static uint8_t writeBuffer[kAlignmentBytes] __attribute__((aligned(4)));
    // Offset indicates how far the write buffer has been filled
    static uint16_t writeBufOffset;
};

} // namespace chip
