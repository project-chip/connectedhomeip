/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "OTAImageProcessorImpl.h"
#include <app/clusters/ota-requestor/OTADownloader.h>

extern "C" {
#include "platform/bootloader/api/btl_interface.h"
}

/// No error, operation OK
#define SL_BOOTLOADER_OK 0L

#define ALIGNMENT_BYTES 64
uint8_t writeBuffer[ALIGNMENT_BYTES] = {0};
uint16_t writeBufOffset = 0;

namespace chip {

// Define static memebers
uint8_t OTAImageProcessorImpl::mSlotId;
uint32_t OTAImageProcessorImpl::mWriteOffset;

CHIP_ERROR OTAImageProcessorImpl::PrepareDownload()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}
CHIP_ERROR OTAImageProcessorImpl::Apply()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleApply, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleAbort, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessBlock(ByteSpan & block)
{
    if ((block.data() == nullptr) || block.empty())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Store block data for HandleProcessBlock to access
    CHIP_ERROR err = SetBlock(block);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot set block data: %" CHIP_ERROR_FORMAT, err.Format());
    }

    DeviceLayer::PlatformMgr().ScheduleWork(HandleProcessBlock, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

void OTAImageProcessorImpl::HandlePrepareDownload(intptr_t context)
{
    int32_t err           = SL_BOOTLOADER_OK;
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);

    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }
    else if (imageProcessor->mDownloader == nullptr)
    {
        ChipLogError(SoftwareUpdate, "mDownloader is null");
        return;
    }

    ChipLogProgress(SoftwareUpdate, "HandlePrepareDownload");

    bootloader_init();
    mSlotId      = 0; // Single slot until we support multiple images
    writeBufOffset = 0;
    mWriteOffset = 0;
    imageProcessor->mParams.downloadedBytes = 0;

    imageProcessor->mHeaderParser.Init();

    // Not calling bootloader_eraseStorageSlot(mSlotId) here because we erase during each write

    imageProcessor->mDownloader->OnPreparedForDownload(err == SL_BOOTLOADER_OK ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL);
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    uint32_t err          = SL_BOOTLOADER_OK;
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    // Pad the rest of the write buffer with zeros and write it to bootloader storage
    if(writeBufOffset != 0)
        {
            // Account for last bytes of the image not yet written to storage
            imageProcessor->mParams.downloadedBytes += writeBufOffset;
            ChipLogProgress(SoftwareUpdate, "HandleFinalize: mWriteOffset %lu writeBufOffset %u writeBuffer %p", mWriteOffset, writeBufOffset, writeBuffer);

            while(writeBufOffset != ALIGNMENT_BYTES)
                {
                    writeBuffer[writeBufOffset] = 0;
                    writeBufOffset++;
                }

            err = bootloader_eraseWriteStorage(mSlotId, mWriteOffset, writeBuffer, ALIGNMENT_BYTES);
            if (err)
                {
                    ChipLogError(SoftwareUpdate, "ERROR: In HandleFinalize bootloader_eraseWriteStorage() error %ld", err);
                    imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                    return;
                }
        }

    imageProcessor->ReleaseBlock();

    ChipLogProgress(SoftwareUpdate, "OTA image downloaded successfully");
}

#include "sl_simple_button_instances.h"

void OTAImageProcessorImpl::HandleApply(intptr_t context)
{
    uint32_t err = SL_BOOTLOADER_OK;

    ChipLogProgress(SoftwareUpdate, "OTAImageProcessorImpl::HandleApply()");

    CORE_CRITICAL_SECTION(err = bootloader_verifyImage(mSlotId, NULL);)

    if (err != SL_BOOTLOADER_OK)
    {
        ChipLogError(SoftwareUpdate, "ERROR: bootloader_verifyImage() error %ld", err);
        //   ChipLogError(SoftwareUpdate, "ERROR: bootloader_verifyImage() error %ld", err);
        //  ChipLogError(SoftwareUpdate, "ERROR: bootloader_verifyImage() error %ld", err);
        //   ChipLogError(SoftwareUpdate, "ERROR: bootloader_verifyImage() error %ld", err);
        return;
    }
    else
        {
            ChipLogProgress(SoftwareUpdate, "bootloader_verifyImage SUCCESS");
            //    ChipLogProgress(SoftwareUpdate, "bootloader_verifyImage SUCCESS");  
            //    ChipLogProgress(SoftwareUpdate, "bootloader_verifyImage SUCCESS");
            // ChipLogProgress(SoftwareUpdate, "bootloader_verifyImage SUCCESS");
        }

    CORE_CRITICAL_SECTION(err = bootloader_setImageToBootload(mSlotId);)
    if (err != SL_BOOTLOADER_OK)
    {
        ChipLogError(SoftwareUpdate, "ERROR: bootloader_setImageToBootload() error %ld", err);
        //  ChipLogError(SoftwareUpdate, "ERROR: bootloader_setImageToBootload() error %ld", err);
        //  ChipLogError(SoftwareUpdate, "ERROR: bootloader_setImageToBootload() error %ld", err);
        //   ChipLogError(SoftwareUpdate, "ERROR: bootloader_setImageToBootload() error %ld", err);
        return;
    }
    else
        {
            ChipLogProgress(SoftwareUpdate, "bootloader_setImageToBootload SUCCESS");
            //    ChipLogProgress(SoftwareUpdate, "bootloader_setImageToBootload SUCCESS");
            //   ChipLogProgress(SoftwareUpdate, "bootloader_setImageToBootload SUCCESS");
            // ChipLogProgress(SoftwareUpdate, "bootloader_setImageToBootload SUCCESS");
        }

    // This reboots the device
    CORE_CRITICAL_SECTION(bootloader_rebootAndInstall();)
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    // Not clearing the image storage area as it is done during each write
    imageProcessor->ReleaseBlock();
}

void OTAImageProcessorImpl::HandleProcessBlock(intptr_t context)
{
    uint32_t err          = SL_BOOTLOADER_OK;
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }
    else if (imageProcessor->mDownloader == nullptr)
    {
        ChipLogError(SoftwareUpdate, "mDownloader is null");
        return;
    }

    ByteSpan block        = imageProcessor->mBlock;
    CHIP_ERROR chip_error = imageProcessor->ProcessHeader(block);

    if (chip_error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Matter image header parser error %s", chip::ErrorStr(chip_error));
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_INVALID_FILE_IDENTIFIER);
        return;
    }

    uint32_t blockReadOffset = 0;

    // LISS debug, do not commit !!!!
    ChipLogProgress(SoftwareUpdate, "HandleProcessBlock: mWriteOffset %lu writeBufOffset %u block.size() %u", mWriteOffset, writeBufOffset, block.size());

    while (blockReadOffset < block.size())
        {
            writeBuffer[writeBufOffset] = *((block.data()) + blockReadOffset);
            writeBufOffset++;
            blockReadOffset++;
            if(writeBufOffset == ALIGNMENT_BYTES)
                {
                    writeBufOffset = 0;

                    err = bootloader_eraseWriteStorage(mSlotId, mWriteOffset, writeBuffer, ALIGNMENT_BYTES);
                    if (err)
                        {
                            ChipLogError(SoftwareUpdate, "ERROR: In HandleProcessBlock bootloader_eraseWriteStorage() error %ld", err);
                            ChipLogProgress(SoftwareUpdate, "HandleProcessBlock: mWriteOffset %lu writeBuffer %p", mWriteOffset, writeBuffer);
                            imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                            return;
                        }
                    mWriteOffset += ALIGNMENT_BYTES;
                    imageProcessor->mParams.downloadedBytes += ALIGNMENT_BYTES;
                }
        }

    // LISS debug, do not commit !!!!
    ChipLogProgress(SoftwareUpdate, ": mWriteOffset %lu writeBufOffset %u blockReadOffset %lu", mWriteOffset, writeBufOffset, blockReadOffset);
    

    imageProcessor->mDownloader->FetchNextData();
}

CHIP_ERROR OTAImageProcessorImpl::ProcessHeader(ByteSpan & block)
{
    if (mHeaderParser.IsInitialized())
    {
        OTAImageHeader header;
        CHIP_ERROR error = mHeaderParser.AccumulateAndDecode(block, header);

        // Needs more data to decode the header
        ReturnErrorCodeIf(error == CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
        ReturnErrorOnFailure(error);

        // SL TODO -- store version somewhere
        ChipLogProgress(SoftwareUpdate, "Image Header software version: %ld payload size: %lu", header.mSoftwareVersion,
                        (long unsigned int) header.mPayloadSize);
        mParams.totalFileBytes = header.mPayloadSize;
        mHeaderParser.Clear();
    }
    return CHIP_NO_ERROR;
}

// Store block data for HandleProcessBlock to access
CHIP_ERROR OTAImageProcessorImpl::SetBlock(ByteSpan & block)
{
    if ((block.data() == nullptr) || block.empty())
    {
        return CHIP_NO_ERROR;
    }

    // Allocate memory for block data if we don't have enough already
    if (mBlock.size() < block.size())
    {
        ReleaseBlock();

        mBlock = MutableByteSpan(static_cast<uint8_t *>(chip::Platform::MemoryAlloc(block.size())), block.size());
        if (mBlock.data() == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    // Store the actual block data
    CHIP_ERROR err = CopySpanToMutableSpan(block, mBlock);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot copy block data: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ReleaseBlock()
{
    if (mBlock.data() != nullptr)
    {
        chip::Platform::MemoryFree(mBlock.data());
    }

    mBlock = MutableByteSpan();
    return CHIP_NO_ERROR;
}

} // namespace chip
