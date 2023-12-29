/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    Copyright 2023 NXP
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

#include "OTAImageProcessorImpl.h"

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

    /* Store block data for HandleProcessBlock to access */
    CHIP_ERROR err = SetBlock(block);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot set block data: %" CHIP_ERROR_FORMAT, err.Format());
    }

    DeviceLayer::PlatformMgr().ScheduleWork(HandleProcessBlock, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

void OTAImageProcessorImpl::TriggerNewRequestForData(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    imageProcessor->mDownloader->FetchNextData();
}

bool OTAImageProcessorImpl::IsFirstImageRun()
{
    OTARequestorInterface * requestor = chip::GetRequestorInstance();
    if (requestor == nullptr)
    {
        return false;
    }

    return requestor->GetCurrentUpdateState() == OTARequestorInterface::OTAUpdateStateEnum::kApplying;
}

CHIP_ERROR OTAImageProcessorImpl::ConfirmCurrentImage()
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

    /* Initialize OTA External Storage Memory */
    if (OTA_SelectExternalStoragePartition() != gOtaSuccess_c)
    {
        ChipLogError(SoftwareUpdate, "Failed to select valid External Flash partition");
    }

    /* Resume flash write/erase transactions only in the idle task */
    ota_config_t OTAconfig;
    OTA_GetDefaultConfig(&OTAconfig);
    OTAconfig.PostedOpInIdleTask = true;
    OTA_SetConfig(&OTAconfig);

    /* Initialize OTA service for posted operations */
    if (gOtaSuccess_c == OTA_ServiceInit(&imageProcessor->mPostedOperationsStorage[0], NB_PENDING_TRANSACTIONS * TRANSACTION_SZ))
    {
        imageProcessor->mHeaderParser.Init();
        imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
    }
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
    CHIP_ERROR error = CHIP_NO_ERROR;

    /* Process Header of the received OTA block if mHeaderParser is in Initialized state */
    if (imageProcessor->mHeaderParser.IsInitialized())
    {
        ByteSpan block = ByteSpan(imageProcessor->mBlock.data(), imageProcessor->mBlock.size());

        error = imageProcessor->ProcessHeader(block);

        if (error == CHIP_NO_ERROR)
        {
            /* Start the OTA Image writing session */
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
    /* Stop the downloading process if header isn't valid */
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Failed to process OTA image header: cancel image update.");
        GetRequestorInstance()->CancelImageUpdate();
        return;
    }

    if (imageProcessor->mBlock.data() != nullptr)
    {
        ChipLogProgress(SoftwareUpdate, "OTA Block received, preparing to write in flash...");
    }

    /*
     * Prior to writing in flash the block received, we must erase enough space to store it.
     * OTA_MakeHeadRoomForNextBlock and OTA_PushImageChunk post erase / write transactions to the queue,
     * these operations are later processed in the context of the idle task.
     * After the flash erase transaction is processed, the HandleBlockEraseComplete callback
     * requests the next OTA block to the provider.
     */
    if (gOtaSuccess_c ==
        OTA_MakeHeadRoomForNextBlock(imageProcessor->mBlock.size(), HandleBlockEraseComplete, (uint32_t) imageProcessor))
    {
        /* Send block to be written in external flash */
        if (gOtaSuccess_c ==
            OTA_PushImageChunk(imageProcessor->mBlock.data(), (uint16_t) imageProcessor->mBlock.size(), NULL, NULL))
        {
            imageProcessor->mParams.downloadedBytes += imageProcessor->mBlock.size();
            return;
        }
    }
}

CHIP_ERROR OTAImageProcessorImpl::ProcessHeader(ByteSpan & block)
{
    OTAImageHeader header;
    CHIP_ERROR error = mHeaderParser.AccumulateAndDecode(block, header);

    /* Needs more data to decode the header */
    ReturnErrorCodeIf(error == CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
    ReturnErrorOnFailure(error);

    mParams.totalFileBytes = header.mPayloadSize;
    mSoftwareVersion       = header.mSoftwareVersion;
    mHeaderParser.Clear();

    return CHIP_NO_ERROR;
}

void OTAImageProcessorImpl::HandleBlockEraseComplete(uint32_t param)
{
    intptr_t context = (intptr_t) param;
    DeviceLayer::PlatformMgr().ScheduleWork(TriggerNewRequestForData, reinterpret_cast<intptr_t>(context));
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }
    /* Finalize writing the OTA update image in flash */
    OTA_CommitImage(NULL);

    imageProcessor->ReleaseBlock();

    ChipLogProgress(SoftwareUpdate, "OTA Image download complete");
}

void OTAImageProcessorImpl::HandleApply(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    /*
     * Set the new image state as ready for test, this would allow the bootloader to perform
     * the upgrade at next reboot.
     */
    OTA_SetNewImageFlag();

    ChipLogProgress(SoftwareUpdate, "Update ready for test");

    /*
     * Restart the device in order to apply the update image.
     * This should be done with a delay so the device has enough time to send
     * the state-transition event when applying the update.
     */
    ChipLogProgress(SoftwareUpdate, "Restarting device in 5 seconds ...");
    DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(5 * 1000), HandleRestart, nullptr);

    /*
     * At next boot time, the bootloader will test + validate new image.
     * If validated, the image is marked "ok" at run time and the update state is switched to permanent.
     * If the image is not valid, the bootloader will revert back to the primary application.
     */
}

void OTAImageProcessorImpl::HandleRestart(System::Layer * aLayer, void * context)
{
    DeviceLayer::PlatformMgrImpl().ScheduleResetInIdle();
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    OTA_CancelImage();

    imageProcessor->ReleaseBlock();
}

CHIP_ERROR OTAImageProcessorImpl::SetBlock(ByteSpan & block)
{
    if (!IsSpanUsable(block))
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
        /* Allocate memory for block data */
        uint8_t * mBlock_ptr = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(block.size()));
        if (mBlock_ptr == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        mBlock = MutableByteSpan(mBlock_ptr, block.size());
    }
    /* Store the actual block data */
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
