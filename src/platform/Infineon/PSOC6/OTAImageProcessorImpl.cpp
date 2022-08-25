/*
 *
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

#include "OTAImageProcessorImpl.h"
#include <app/clusters/ota-requestor/OTADownloader.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <lib/support/CodeUtils.h>
#include <ota_serial_flash.h>
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

#ifdef P6_OTA
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

CHIP_ERROR OTAImageProcessorImpl::ProcessHeader(ByteSpan & block)
{
    // Only modify the ByteSpan if the OTAImageHeaderParser is currently initialized.
    if (mHeaderParser.IsInitialized())
    {
        OTAImageHeader header;

        // AccumulateAndDecode will cause the OTAImageHeader bytes to be stored
        // in header. We don't do anything with header, however, the other
        // consequence of this call is to advance the data pointer in block. In
        // this or subsequent calls to this API, block will end up pointing at
        // the first byte after OTAImageHeader.
        CHIP_ERROR error = mHeaderParser.AccumulateAndDecode(block, header);

        // If we have not received all the bytes of the OTAImageHeader yet, that is OK.
        // Return CHIP_NO_ERROR and expect that future blocks will contain the rest.
        ReturnErrorCodeIf(error == CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);

        // If there is some error other than "too small", return that so future
        // processing will be aborted.
        ReturnErrorOnFailure(error);

        mParams.totalFileBytes = header.mPayloadSize;

        // If we are here, then we have received all the OTAImageHeader bytes.
        // Calling Clear() here results in the parser state being set to
        // uninitialized. This means future calls to ProcessHeader will not
        // modify block and those future bytes will be written to the device.
        mHeaderParser.Clear();
    }

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

bool OTAImageProcessorImpl::IsFirstImageRun()
{
    OTARequestorInterface * requestor = GetRequestorInstance();
    ReturnErrorCodeIf(requestor == nullptr, false);

    uint32_t currentVersion;
    ReturnErrorCodeIf(ConfigurationMgr().GetSoftwareVersion(currentVersion) != CHIP_NO_ERROR, false);

    ChipLogProgress(SoftwareUpdate, "%ld", currentVersion);
    ChipLogProgress(SoftwareUpdate, "%ld", requestor->GetTargetVersion());

    return ((requestor->GetCurrentUpdateState() == OTARequestorInterface::OTAUpdateStateEnum::kApplying) &&
            (requestor->GetTargetVersion() == currentVersion));
}

CHIP_ERROR OTAImageProcessorImpl::ConfirmCurrentImage()
{
    OTARequestorInterface * requestor = chip::GetRequestorInstance();
    if (requestor == nullptr)
    {
        return CHIP_ERROR_INTERNAL;
    }

    uint32_t currentVersion;
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSoftwareVersion(currentVersion));
    if (currentVersion != requestor->GetTargetVersion())
    {
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

    /* Initialize SMIF subsystem for OTA Image download */
    ota_smif_initialize();

    // Open and erase secondary flash area to prepare
    if (flash_area_open(FLASH_AREA_IMAGE_SECONDARY(0), &(imageProcessor->mFlashArea)) != 0)
    {
        imageProcessor->mDownloader->OnPreparedForDownload(CHIP_ERROR_OPEN_FAILED);
        return;
    }
    // Note: This only erases up to the secondary slot size (which fa_size is
    // set to). It will not erase the persistent storage as that only uses
    // a small region towards the end of flash.
    if (flash_area_erase(imageProcessor->mFlashArea, 0, imageProcessor->mFlashArea->fa_size) != 0)
    {
        imageProcessor->mDownloader->OnPreparedForDownload(CHIP_ERROR_OPEN_FAILED);
        return;
    }

    // init the OTAImageHeaderParser instance to indicate that we haven't yet
    // parsed the header out of the incoming image.
    imageProcessor->mHeaderParser.Init();

    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    flash_area_close(imageProcessor->mFlashArea);
    ChipLogProgress(SoftwareUpdate, "Setting boot pending");
    int ret = boot_set_pending(0, 1);

    if (ret != 0)
    {
        ChipLogError(SoftwareUpdate, "Failed to set boot pending");
        return;
    }

    imageProcessor->ReleaseBlock();
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    flash_area_erase(imageProcessor->mFlashArea, 0, imageProcessor->mFlashArea->fa_size);

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

    // The call to ProcessHeader will result in the modification of the block ByteSpan data
    // pointer if the OTAImageHeader is present in the image. The result is that only
    // the new application bytes will be written to the device in the flash_area_write calls,
    // as all bytes for the header are skipped.
    ByteSpan block = ByteSpan(imageProcessor->mBlock.data(), imageProcessor->mBlock.size());

    CHIP_ERROR error = imageProcessor->ProcessHeader(block);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Failed to process OTA image header");
        imageProcessor->mDownloader->EndDownload(error);
        return;
    }

    // send down only the post-processed bytes from block to this call, rather than sending down
    // the original bytes from imageProcessor. The bytes in imageProcessor may include date
    // from the OTAImageHeader, which we don't want.
    int rc = flash_area_write(imageProcessor->mFlashArea, imageProcessor->mParams.downloadedBytes, block.data(), block.size());
    if (rc != 0)
    {
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
        return;
    }

    // increment the total downloaded bytes by the potentially modified block ByteSpan size
    imageProcessor->mParams.downloadedBytes += block.size();
    imageProcessor->mDownloader->FetchNextData();
}

void OTAImageProcessorImpl::HandleApply(intptr_t context)
{
    ChipLogProgress(SoftwareUpdate, "Rebooting after 2 seconds...");

    cy_rtos_delay_milliseconds(2000);

    NVIC_SystemReset();

    return;
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

CHIP_ERROR OTAImageProcessorImpl::ReleaseBlock()
{
    if (mBlock.data() != nullptr)
    {
        chip::Platform::MemoryFree(mBlock.data());
    }

    mBlock = MutableByteSpan();
    return CHIP_NO_ERROR;
}
#endif // P6_OTA

} // namespace DeviceLayer
} // namespace chip
