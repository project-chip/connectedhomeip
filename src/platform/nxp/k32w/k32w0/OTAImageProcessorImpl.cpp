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

#include <src/app/clusters/ota-requestor/OTADownloader.h>

#include "OTAImageProcessorImpl.h"
#include "OtaSupport.h"
#include "OtaUtils.h"

extern "C" void ResetMCU(void);

namespace chip {

CHIP_ERROR OTAImageProcessorImpl::PrepareDownload()
{
    if (mParams.imageFile.empty())
    {
        ChipLogError(SoftwareUpdate, "Invalid output image file supplied");
        return CHIP_ERROR_INTERNAL;
    }

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
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
    if (mParams.imageFile.empty())
    {
        ChipLogError(SoftwareUpdate, "Invalid output image file supplied");
        return CHIP_ERROR_INTERNAL;
    }

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

    if (gOtaSuccess_c == OTA_ClientInit())
    {
        if (gOtaSuccess_c == OTA_StartImage(imageProcessor->mParams.imageFile.size()))
        {
            imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
        }
    }
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    remove(imageProcessor->mParams.imageFile.data());
    imageProcessor->ReleaseBlock();
}

void OTAImageProcessorImpl::HandleProcessBlock(intptr_t context)
{
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

    /* Will start an erase of 4K if necessary */
    if (gOtaSuccess_c == OTA_MakeHeadRoomForNextBlock(imageProcessor->mBlock.size(), NULL, 0))
    {
        if (gOtaSuccess_c ==
            OTA_PushImageChunk(imageProcessor->mBlock.data(), (uint16_t) imageProcessor->mBlock.size(), NULL, NULL))
        {
            imageProcessor->mParams.downloadedBytes += imageProcessor->mBlock.size();
            return;
        }
    }
}

CHIP_ERROR OTAImageProcessorImpl::SetBlock(ByteSpan & block)
{
    if (!IsSpanUsable(block))
    {
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
    CHIP_ERROR err = CopySpanToMutableSpan(block, mBlock);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot copy block data: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    return CHIP_NO_ERROR;
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    OTA_CommitImage(NULL);
    if (OTA_ImageAuthenticate() == gOtaImageAuthPass_c)
    {
        /* Set the necessary information to inform the SSBL that a new image is available */
        OTA_SetNewImageFlag();
        ResetMCU();
    }

    imageProcessor->ReleaseBlock();
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
