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

#include "LinuxOTAImageProcessor.h"

namespace chip {

CHIP_ERROR LinuxOTAImageProcessor::PrepareDownload()
{
    if (mParams.imageFile.empty())
    {
        ChipLogError(SoftwareUpdate, "Invalid output image file supplied");
        return CHIP_ERROR_INTERNAL;
    }

    DeviceLayer::PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LinuxOTAImageProcessor::Finalize()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LinuxOTAImageProcessor::Abort()
{
    if (mParams.imageFile.empty())
    {
        ChipLogError(SoftwareUpdate, "Invalid output image file supplied");
        return CHIP_ERROR_INTERNAL;
    }

    DeviceLayer::PlatformMgr().ScheduleWork(HandleAbort, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LinuxOTAImageProcessor::ProcessBlock(ByteSpan & block)
{
    if (!mOfs.is_open() || !mOfs.good())
    {
        return CHIP_ERROR_INTERNAL;
    }

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

void LinuxOTAImageProcessor::HandlePrepareDownload(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<LinuxOTAImageProcessor *>(context);
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

    imageProcessor->mOfs.open(imageProcessor->mParams.imageFile.data(),
                              std::ofstream::out | std::ofstream::ate | std::ofstream::app);
    if (!imageProcessor->mOfs.good())
    {
        imageProcessor->mDownloader->OnPreparedForDownload(CHIP_ERROR_OPEN_FAILED);
        return;
    }

    // TODO: if file already exists and is not empty, erase previous contents

    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

void LinuxOTAImageProcessor::HandleFinalize(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<LinuxOTAImageProcessor *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    imageProcessor->mOfs.close();
    imageProcessor->ReleaseBlock();

    ChipLogProgress(SoftwareUpdate, "OTA image downloaded to %s", imageProcessor->mParams.imageFile.data());
}

void LinuxOTAImageProcessor::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<LinuxOTAImageProcessor *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    imageProcessor->mOfs.close();
    remove(imageProcessor->mParams.imageFile.data());
    imageProcessor->ReleaseBlock();
}

void LinuxOTAImageProcessor::HandleProcessBlock(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<LinuxOTAImageProcessor *>(context);
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

    // TODO: Process block header if any

    if (!imageProcessor->mOfs.write(reinterpret_cast<const char *>(imageProcessor->mBlock.data()),
                                    static_cast<std::streamsize>(imageProcessor->mBlock.size())))
    {
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
        return;
    }

    imageProcessor->mParams.downloadedBytes += imageProcessor->mBlock.size();
    imageProcessor->mDownloader->FetchNextData();
}

CHIP_ERROR LinuxOTAImageProcessor::SetBlock(ByteSpan & block)
{
    if ((block.data() == nullptr) || block.empty())
    {
        return CHIP_NO_ERROR;
    }

    // Allocate memory for block data if it has not been done yet
    if (mBlock.empty())
    {
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

CHIP_ERROR LinuxOTAImageProcessor::ReleaseBlock()
{
    if (mBlock.data() != nullptr)
    {
        chip::Platform::MemoryFree(mBlock.data());
    }

    mBlock = MutableByteSpan();
    return CHIP_NO_ERROR;
}

} // namespace chip
