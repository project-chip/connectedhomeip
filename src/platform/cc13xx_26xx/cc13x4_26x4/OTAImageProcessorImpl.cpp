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
#include <lib/support/DefaultStorageKeyAllocator.h>

#include "OTAImageProcessorImpl.h"

#include <ti_drivers_config.h>

#include "inttypes.h"

// clang-format off
/* driverlib header for resetting the SoC */
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
// clang-format on

using namespace chip::DeviceLayer;
using namespace chip::DeviceLayer::PersistedStorage;

uint64_t totalBytesWrittenNvs = 0;

#define MATTER_OTA_HEADER_MAGIC_NUMBER_LENGTH 4
#define MATTER_OTA_HEADER_IMG_LENGTH_BYTES 4
#define MATTER_OTA_HEADER_PADDING 4
#define MATTER_OTA_HEADER_LENGTH_BYTES 4

namespace chip {

CHIP_ERROR OTAImageProcessorImpl::PrepareDownload()
{
    PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
    PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
    PlatformMgr().ScheduleWork(HandleApply, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
    PlatformMgr().ScheduleWork(HandleAbort, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessBlock(ByteSpan & block)
{
    if (nullptr == mNvsHandle)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if ((nullptr == block.data()) || block.empty())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Store block data for HandleProcessBlock to access
    CHIP_ERROR err = SetBlock(block);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot set block data: %" CHIP_ERROR_FORMAT, err.Format());
    }

    PlatformMgr().ScheduleWork(HandleProcessBlock, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

bool OTAImageProcessorImpl::IsFirstImageRun()
{
    OTARequestorInterface * requestor;
    uint32_t runningSwVer;

    if (CHIP_NO_ERROR != ConfigurationMgr().GetSoftwareVersion(runningSwVer))
    {
        return false;
    }

    requestor = GetRequestorInstance();

    return (requestor->GetCurrentUpdateState() == chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kApplying);
}

/* makes room for the new block if needed */
static bool writeExtFlashImgPages(NVS_Handle handle, ssize_t offset, MutableByteSpan block)
{
    int_fast16_t status;
    NVS_Attrs regionAttrs;
    unsigned int erasedSectors;
    unsigned int neededSectors;
    size_t sectorSize;
    size_t imageOffset;
    uint8_t * data;
    size_t dataSize;

    if (offset < 0)
    {
        size_t blockOffset = -offset;
        if (blockOffset >= block.size())
        {
            /* We have not downloaded past the Matter OTA header */
            return true;
        }

        imageOffset = 0;
        data        = block.data() + blockOffset;
        dataSize    = block.size() - blockOffset;
    }
    else
    {
        imageOffset = offset;
        data        = block.data();
        dataSize    = block.size();
    }

    NVS_getAttrs(handle, &regionAttrs);
    sectorSize    = regionAttrs.sectorSize;
    erasedSectors = (imageOffset + (sectorSize - 1)) / sectorSize;
    neededSectors = ((imageOffset + dataSize) + (sectorSize - 1)) / sectorSize;
    if (neededSectors != erasedSectors)
    {
        status = NVS_erase(handle, (erasedSectors * sectorSize), (neededSectors - erasedSectors) * sectorSize);
        if (status != NVS_STATUS_SUCCESS)
        {
            ChipLogError(SoftwareUpdate, "NVS_erase failed status: %d", status);
            return false;
        }
    }
    status = NVS_write(handle, imageOffset, data, dataSize, NVS_WRITE_POST_VERIFY);
    if (status != NVS_STATUS_SUCCESS)
    {
        ChipLogError(SoftwareUpdate, "NVS_write failed status: %d", status);
        return false;
    }
    else
    {
        totalBytesWrittenNvs += dataSize;
        ChipLogProgress(SoftwareUpdate, "Total written bytes: %d", (size_t) totalBytesWrittenNvs);
    }
    return true;
}

/* Erase the MCUBoot slot */
#define BOOT_SLOT_SIZE (0x000F2000) /* must match flash_map_backend */
static bool eraseExtSlot(NVS_Handle handle)
{
    int_fast16_t status;
    NVS_Attrs regionAttrs;
    unsigned int sectors;

    NVS_getAttrs(handle, &regionAttrs);
    /* calculate the number of sectors to erase */
    sectors = (BOOT_SLOT_SIZE + (regionAttrs.sectorSize - 1)) / regionAttrs.sectorSize;
    status  = NVS_erase(handle, 0U, sectors * regionAttrs.sectorSize);

    return (status == NVS_STATUS_SUCCESS);
}

/* Erase the MCUBoot header to ensure the image isn't applied */
#define BOOT_HEADER_SIZE (0x80)
static bool eraseExtHeader(NVS_Handle handle)
{
    int_fast16_t status;
    NVS_Attrs regionAttrs;
    unsigned int sectors;

    NVS_getAttrs(handle, &regionAttrs);
    /* calculate the number of sectors to erase */
    sectors = (BOOT_HEADER_SIZE + (regionAttrs.sectorSize - 1)) / regionAttrs.sectorSize;
    status  = NVS_erase(handle, 0U, sectors * regionAttrs.sectorSize);

    return (status == NVS_STATUS_SUCCESS);
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

    if (NULL == imageProcessor->mNvsHandle)
    {
        NVS_Params nvsParams;
        NVS_Params_init(&nvsParams);
        imageProcessor->mNvsHandle = NVS_open(CONFIG_NVSEXTERNAL, &nvsParams);

        if (NULL == imageProcessor->mNvsHandle)
        {
            imageProcessor->mDownloader->OnPreparedForDownload(CHIP_ERROR_OPEN_FAILED);
            return;
        }
    }

    if (!eraseExtSlot(imageProcessor->mNvsHandle))
    {
        imageProcessor->mDownloader->OnPreparedForDownload(CHIP_ERROR_WRITE_FAILED);
    }

    imageProcessor->mFixedOtaHeader = { 0 };
    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);

    if (imageProcessor == nullptr)
    {
        return;
    }

    /* possible improvement, add MCUBoot magic at the end of the slot. This
     * could be done if the ota file generation truncates the image instead of
     * sending the full MCUBoot slot.
     */

    imageProcessor->ReleaseBlock();

    ChipLogProgress(SoftwareUpdate, "OTA image downloaded");
    ChipLogProgress(SoftwareUpdate, "Total downloaded bytes: %d", (size_t) imageProcessor->mParams.downloadedBytes);
}

void OTAImageProcessorImpl::HandleApply(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    /* reset SoC to kick MCUBoot */
    ChipLogProgress(SoftwareUpdate, "Resetting device to kick off MCUBoot");
    SysCtrlSystemReset();
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    if (!eraseExtHeader(imageProcessor->mNvsHandle))
    {
        imageProcessor->mDownloader->OnPreparedForDownload(CHIP_ERROR_WRITE_FAILED);
    }

    NVS_close(imageProcessor->mNvsHandle);
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

    /* Save the fixed size header */
    if (imageProcessor->mParams.downloadedBytes < sizeof(imageProcessor->mFixedOtaHeader))
    {
        uint8_t * header = reinterpret_cast<uint8_t *>(&(imageProcessor->mFixedOtaHeader));
        if (imageProcessor->mBlock.size() + imageProcessor->mParams.downloadedBytes < sizeof(imageProcessor->mFixedOtaHeader))
        {
            // the first block is smaller than the header, somehow
            memcpy(header + imageProcessor->mParams.downloadedBytes, imageProcessor->mBlock.data(), imageProcessor->mBlock.size());
        }
        else
        {
            // we have received the whole header, fill it up
            memcpy(header + imageProcessor->mParams.downloadedBytes, imageProcessor->mBlock.data(),
                   sizeof(imageProcessor->mFixedOtaHeader) - imageProcessor->mParams.downloadedBytes);

            // update the total size for download tracking
            imageProcessor->mParams.totalFileBytes = imageProcessor->mFixedOtaHeader.totalSize;
            ChipLogDetail(SoftwareUpdate, "Total file size: %d", (size_t) imageProcessor->mParams.totalFileBytes);
        }
    }

    if (imageProcessor->mParams.downloadedBytes + imageProcessor->mBlock.size() > imageProcessor->mFixedOtaHeader.headerSize)
        /* chip::OTAImageHeaderParser can be used for processing the variable size header */

        /* Do not write Matter OTA image header to the external flash, MCUBoot
         * needs to have it's header at address 0
         */
        if (imageProcessor->mFixedOtaHeader.headerSize > 0)
        {
            ssize_t offset = imageProcessor->mParams.downloadedBytes -
                (imageProcessor->mFixedOtaHeader.headerSize + MATTER_OTA_HEADER_MAGIC_NUMBER_LENGTH +
                 MATTER_OTA_HEADER_IMG_LENGTH_BYTES + MATTER_OTA_HEADER_PADDING + MATTER_OTA_HEADER_LENGTH_BYTES);
            ChipLogDetail(SoftwareUpdate, "Write block %d, %d", (size_t) imageProcessor->mParams.downloadedBytes,
                          imageProcessor->mBlock.size());
            if (!writeExtFlashImgPages(imageProcessor->mNvsHandle, offset, imageProcessor->mBlock))
            {
                imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                return;
            }
        }

    imageProcessor->mParams.downloadedBytes += imageProcessor->mBlock.size();
    ChipLogDetail(SoftwareUpdate, "Total downloaded bytes: %d", (size_t) imageProcessor->mParams.downloadedBytes);
    imageProcessor->mDownloader->FetchNextData();
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

} // namespace chip
