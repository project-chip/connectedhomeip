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

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Ameba/AmebaOTAImageProcessor.h>
#include <platform/Ameba/AmebaUtils.h>

using namespace chip::DeviceLayer::Internal;

namespace chip {

CHIP_ERROR AmebaOTAImageProcessor::PrepareDownload()
{
    ChipLogProgress(SoftwareUpdate, "Prepare download");

    DeviceLayer::PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaOTAImageProcessor::Finalize()
{
    ChipLogProgress(SoftwareUpdate, "Finalize");

    DeviceLayer::PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaOTAImageProcessor::Apply()
{
    ChipLogProgress(SoftwareUpdate, "Apply");

    DeviceLayer::PlatformMgr().ScheduleWork(HandleApply, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaOTAImageProcessor::Abort()
{
    ChipLogProgress(SoftwareUpdate, "Abort");

    DeviceLayer::PlatformMgr().ScheduleWork(HandleAbort, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaOTAImageProcessor::ProcessBlock(ByteSpan & block)
{
    ChipLogProgress(SoftwareUpdate, "Process Block");

    if ((block.data() == nullptr) || block.empty())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    CHIP_ERROR err = SetBlock(block);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot set block data: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    DeviceLayer::PlatformMgr().ScheduleWork(HandleProcessBlock, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

bool AmebaOTAImageProcessor::IsFirstImageRun()
{
    OTARequestorInterface * requestor = chip::GetRequestorInstance();
    if (requestor == nullptr)
    {
        return false;
    }

    return requestor->GetCurrentUpdateState() == OTARequestorInterface::OTAUpdateStateEnum::kApplying;
}

CHIP_ERROR AmebaOTAImageProcessor::ConfirmCurrentImage()
{
    OTARequestorInterface * requestor = chip::GetRequestorInstance();
    if (requestor == nullptr)
    {
        return CHIP_ERROR_INTERNAL;
    }

    uint32_t currentVersion;
    uint32_t targetVersion = requestor->GetTargetVersion();
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSoftwareVersion(currentVersion));
    if (currentVersion != targetVersion)
    {
        ChipLogError(SoftwareUpdate, "Current software version = %" PRIu32 ", expected software version = %" PRIu32, currentVersion,
                     targetVersion);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_NO_ERROR;
}

void AmebaOTAImageProcessor::HandlePrepareDownload(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<AmebaOTAImageProcessor *>(context);
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

    imageProcessor->mHeaderParser.Init();

    // prepare OTA update partition
    matter_ota_prepare_partition();
    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

void AmebaOTAImageProcessor::HandleFinalize(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<AmebaOTAImageProcessor *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }

    if (AmebaUtils::MapError(matter_ota_flush_last(), AmebaErrorType::kFlashError) != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Failed to finalize OTA");
        return;
    }

    imageProcessor->ReleaseBlock();

    ChipLogProgress(SoftwareUpdate, "OTA image downloaded and written to flash");
}

void AmebaOTAImageProcessor::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<AmebaOTAImageProcessor *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }

    // to make flash erase non-blocking, create background task to cleanup flash, will self-delete upon completion
    matter_ota_create_abort_task();

    imageProcessor->ReleaseBlock();
}

void AmebaOTAImageProcessor::HandleProcessBlock(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<AmebaOTAImageProcessor *>(context);
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

    ByteSpan block   = imageProcessor->mBlock;
    CHIP_ERROR error = imageProcessor->ProcessHeader(block); // process matter ota header
    uint8_t remainHeader =
        matter_ota_get_total_header_size() - matter_ota_get_current_header_size(); // size of ameba header received
    uint16_t writeLength   = block.size();             // length of actual data to write to flash, excluding header
    uint8_t * writePointer = (uint8_t *) block.data(); // pointer to the actual data to write to flash, excluding header

    // 1. Check if 32-bytes Ameba header has already been received
    if (remainHeader > 0)
    {
        if (block.size() >= remainHeader)
        {
            if (AmebaUtils::MapError(matter_ota_store_header((uint8_t *) block.data(), remainHeader),
                                     AmebaErrorType::kFlashError) != CHIP_NO_ERROR)
            {
                ChipLogError(SoftwareUpdate, "Failed to store OTA header");
                return;
            }
            writeLength -= remainHeader;
            writePointer += remainHeader;
        }
        else
        {
            if (AmebaUtils::MapError(matter_ota_store_header((uint8_t *) block.data(), block.size()),
                                     AmebaErrorType::kFlashError) != CHIP_NO_ERROR)
            {
                ChipLogError(SoftwareUpdate, "Failed to store OTA header");
                return;
            }
            writeLength = 0;
        }
    }

    // 2. Write data to flash, flash erasure logic will be handled by lower layer
    if (AmebaUtils::MapError(matter_ota_flash_burst_write(writePointer, writeLength), AmebaErrorType::kFlashError) != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "OTA write failed");
        return;
    }

    // 3. Fetch next data block
    imageProcessor->mParams.downloadedBytes += block.size();
    imageProcessor->mDownloader->FetchNextData();
}

void AmebaOTAImageProcessor::HandleApply(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<AmebaOTAImageProcessor *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        GetRequestorInstance()->CancelImageUpdate();
        return;
    }

    // Update signature
    if (AmebaUtils::MapError(matter_ota_update_signature(), AmebaErrorType::kFlashError) != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "OTA update signature failed");
        GetRequestorInstance()->CancelImageUpdate();
        return;
    }

    ChipLogProgress(SoftwareUpdate, "Rebooting in 10 seconds...");

    // Delay action time before calling HandleRestart
    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(10 * 1000), HandleRestart, nullptr);
}

void AmebaOTAImageProcessor::HandleRestart(chip::System::Layer * systemLayer, void * appState)
{
    matter_ota_platform_reset();
}

CHIP_ERROR AmebaOTAImageProcessor::ProcessHeader(ByteSpan & block)
{
    ChipLogProgress(SoftwareUpdate, "ProcessHeader");
    if (mHeaderParser.IsInitialized())
    {
        OTAImageHeader header;
        CHIP_ERROR error = mHeaderParser.AccumulateAndDecode(block, header);

        // Needs more data to decode the header
        ReturnErrorCodeIf(error == CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
        ReturnErrorOnFailure(error);

        mParams.totalFileBytes = header.mPayloadSize;
        mHeaderParser.Clear();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaOTAImageProcessor::SetBlock(ByteSpan & block)
{
    if (block.empty())
    {
        ReleaseBlock();
        return CHIP_NO_ERROR;
    }

    if (mBlock.size() < block.size())
    {
        if (!mBlock.empty())
        {
            ReleaseBlock();
        }
        uint8_t * mBlock_ptr = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(block.size()));
        if (mBlock_ptr == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        mBlock = MutableByteSpan(mBlock_ptr, block.size());
    }

    if (mBlock.size() < block.size())
    {
        if (!mBlock.empty())
        {
            ReleaseBlock();
        }

        uint8_t * mBlock_ptr = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(block.size()));

        if (mBlock_ptr == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        mBlock = MutableByteSpan(mBlock_ptr, block.size());
    }

    CHIP_ERROR err = CopySpanToMutableSpan(block, mBlock);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot copy block data: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaOTAImageProcessor::ReleaseBlock()
{
    if (mBlock.data() != nullptr)
    {
        chip::Platform::MemoryFree(mBlock.data());
    }

    mBlock = MutableByteSpan();
    return CHIP_NO_ERROR;
}
} // namespace chip
