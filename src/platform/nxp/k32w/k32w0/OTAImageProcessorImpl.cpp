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

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <src/app/clusters/ota-requestor/OTADownloader.h>
#include <src/app/clusters/ota-requestor/OTARequestorInterface.h>

#include "OTAImageProcessorImpl.h"
#include "OtaSupport.h"
#include "OtaUtils.h"

extern "C" void ResetMCU(void);

using namespace chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {

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
    if (mImageFile == nullptr)
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

void OTAImageProcessorImpl::TriggerNewRequestForData()
{
    if (mDownloader)
    {
        this->mDownloader->FetchNextData();
    }
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
        imageProcessor->mHeaderParser.Init();
        imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
    }
}

CHIP_ERROR OTAImageProcessorImpl::ProcessHeader(ByteSpan & block)
{
    OTAImageHeader header;
    CHIP_ERROR error = mHeaderParser.AccumulateAndDecode(block, header);

    // Needs more data to decode the header
    ReturnErrorCodeIf(error == CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
    ReturnErrorOnFailure(error);
    mParams.totalFileBytes = header.mPayloadSize;
    mSoftwareVersion       = header.mSoftwareVersion;
    mHeaderParser.Clear();

    return CHIP_NO_ERROR;
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    remove(imageProcessor->mImageFile);
    imageProcessor->ReleaseBlock();
}

void OTAImageProcessorImpl::HandleProcessBlock(intptr_t context)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

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

    /* process OTA header if not already did */
    if (imageProcessor->mHeaderParser.IsInitialized())
    {
        ByteSpan block = ByteSpan(imageProcessor->mBlock.data(), imageProcessor->mBlock.size());

        error = imageProcessor->ProcessHeader(block);
        if (error == CHIP_NO_ERROR)
        {
            if (gOtaSuccess_c == OTA_StartImage(imageProcessor->mParams.totalFileBytes))
            {
                uint8_t * ptr = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(block.size()));

                if (ptr != nullptr)
                {
                    MutableByteSpan mutableBlock = MutableByteSpan(ptr, block.size());
                    error                        = CopySpanToMutableSpan(block, mutableBlock);

                    if (error == CHIP_NO_ERROR)
                    {
                        imageProcessor->ReleaseBlock();
                        imageProcessor->mBlock = MutableByteSpan(mutableBlock.data(), mutableBlock.size());
                    }
                }
                else
                {
                    error = CHIP_ERROR_NO_MEMORY;
                }
            }
            else
            {
                error = CHIP_ERROR_INTERNAL;
            }
        }
    }

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Failed to process OTA image header");
        imageProcessor->mDownloader->EndDownload(error);
        return;
    }

    /* Will start an erase of 4K if necessary */
    if (gOtaSuccess_c == OTA_MakeHeadRoomForNextBlock(imageProcessor->mBlock.size(), HandleBlockEraseComplete, 0))
    {
        if (gOtaSuccess_c ==
            OTA_PushImageChunk(imageProcessor->mBlock.data(), (uint16_t) imageProcessor->mBlock.size(), NULL, NULL))
        {
            imageProcessor->mParams.downloadedBytes += imageProcessor->mBlock.size();
            return;
        }
    }
}

bool OTAImageProcessorImpl::IsFirstImageRun()
{
    bool firstRun = false;

    if (CHIP_NO_ERROR == (K32WConfig::ReadConfigValue(K32WConfig::kConfigKey_FirstRunOfOTAImage, firstRun)))
    {
        return firstRun;
    }

    return false;
}

CHIP_ERROR OTAImageProcessorImpl::ConfirmCurrentImage()
{
    bool firstRun = false;

    return K32WConfig::WriteConfigValue(K32WConfig::kConfigKey_FirstRunOfOTAImage, firstRun);
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

    imageProcessor->ReleaseBlock();
}

void OTAImageProcessorImpl::HandleApply(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    bool firstRun         = true;

    if (imageProcessor == nullptr)
    {
        return;
    }

    OTA_CommitImage(NULL);
    if (OTA_ImageAuthenticate() == gOtaImageAuthPass_c)
    {

        /* TODO internal: MATTER-126 */
        /*if (CHIP_NO_ERROR == K32WConfig::WriteConfigValue(K32WConfig::kConfigKey_FirstRunOfOTAImage, firstRun)) */
        {
            /* Set the necessary information to inform the SSBL that a new image is available */
            DeviceLayer::ConfigurationMgr().StoreSoftwareVersion(imageProcessor->mSoftwareVersion);
            OTA_SetNewImageFlag();
            ChipLogProgress(SoftwareUpdate, "OTA image authentication success. Device will reboot with the new image!");
            ResetMCU();
        }
    }
    else
    {
        ChipLogError(SoftwareUpdate, "Image authentication error");
    }
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

void OTAImageProcessorImpl::HandleBlockEraseComplete(uint32_t)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    ChipDeviceEvent otaChange;
    otaChange.Type                     = DeviceEventType::kOtaStateChanged;
    otaChange.OtaStateChanged.newState = kOtaSpaceAvailable;
    error                              = PlatformMgr().PostEvent(&otaChange);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Error while posting OtaChange event");
    }
}

} // namespace chip
