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

#include "ASROTAImageProcessor.h"
#include <app/clusters/ota-requestor/OTADownloader.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>

namespace chip {

CHIP_ERROR ASROTAImageProcessor::PrepareDownload()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ASROTAImageProcessor::Finalize()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}
CHIP_ERROR ASROTAImageProcessor::Apply()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleApply, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ASROTAImageProcessor::Abort()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleAbort, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ASROTAImageProcessor::ProcessBlock(ByteSpan & block)
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

bool ASROTAImageProcessor::IsFirstImageRun()
{
    OTARequestorInterface * requestor = chip::GetRequestorInstance();
    if (requestor == nullptr)
    {
        return false;
    }

    return requestor->GetCurrentUpdateState() == OTARequestorInterface::OTAUpdateStateEnum::kApplying;
}

CHIP_ERROR ASROTAImageProcessor::ConfirmCurrentImage()
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

void ASROTAImageProcessor::HandlePrepareDownload(intptr_t context)
{
    int32_t err           = LEGA_OTA_OK;
    auto * imageProcessor = reinterpret_cast<ASROTAImageProcessor *>(context);

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

    imageProcessor->ota_boot_para.off_bp = 0;
    err                                  = lega_ota_init(&imageProcessor->ota_boot_para);

    imageProcessor->mWriteOffset = nullptr;

    imageProcessor->mHeaderParser.Init();

    imageProcessor->mDownloader->OnPreparedForDownload(
        ((err == LEGA_OTA_OK) || (err == LEGA_OTA_INIT_ALREADY)) ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL);
}

void ASROTAImageProcessor::HandleFinalize(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<ASROTAImageProcessor *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    imageProcessor->ReleaseBlock();

    ChipLogProgress(SoftwareUpdate, "OTA image downloaded successfully");
}

void ASROTAImageProcessor::HandleApply(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<ASROTAImageProcessor *>(context);

    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }

    ChipLogProgress(SoftwareUpdate, "ASROTAImageProcessor::HandleApply()");

    imageProcessor->ota_boot_para.res_type = LEGA_OTA_FINISH;
    // Set boot mode and then reboot
    lega_ota_set_boot(&imageProcessor->ota_boot_para);
}

void ASROTAImageProcessor::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<ASROTAImageProcessor *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    // Not clearing the image storage area as it is done during each write
    imageProcessor->ReleaseBlock();
}

void ASROTAImageProcessor::HandleProcessBlock(intptr_t context)
{
    uint32_t err          = LEGA_OTA_OK;
    auto * imageProcessor = reinterpret_cast<ASROTAImageProcessor *>(context);
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

    err = lega_ota_write(imageProcessor->mWriteOffset, (char *) (block.data()), block.size());

    if (err)
    {
        ChipLogError(SoftwareUpdate, "HandleProcessBlock ERROR %ld", err);

        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
        return;
    }

    imageProcessor->mParams.downloadedBytes += block.size();
    imageProcessor->mDownloader->FetchNextData();
}

CHIP_ERROR ASROTAImageProcessor::ProcessHeader(ByteSpan & block)
{
    if (mHeaderParser.IsInitialized())
    {
        OTAImageHeader header;
        CHIP_ERROR error = mHeaderParser.AccumulateAndDecode(block, header);

        // Needs more data to decode the header
        ReturnErrorCodeIf(error == CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
        ReturnErrorOnFailure(error);

        ChipLogProgress(SoftwareUpdate, "Image Header software version: %ld payload size: %lu", header.mSoftwareVersion,
                        (long unsigned int) header.mPayloadSize);
        mParams.totalFileBytes = header.mPayloadSize;
        mHeaderParser.Clear();
    }
    return CHIP_NO_ERROR;
}

// Store block data for HandleProcessBlock to access
CHIP_ERROR ASROTAImageProcessor::SetBlock(ByteSpan & block)
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

CHIP_ERROR ASROTAImageProcessor::ReleaseBlock()
{
    if (mBlock.data() != nullptr)
    {
        chip::Platform::MemoryFree(mBlock.data());
    }

    mBlock = MutableByteSpan();
    return CHIP_NO_ERROR;
}

} // namespace chip
