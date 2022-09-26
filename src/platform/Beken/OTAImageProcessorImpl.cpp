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

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <lib/support/logging/CHIPLogging.h>

#include "matter_pal.h"
#include <platform/Beken/OTAImageProcessorImpl.h>
#include <string.h>

using namespace chip::System;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {

namespace {
void HandleRestart(Layer * systemLayer, void * appState)
{
    bk_reboot();
}
} // namespace

const char ucFinishFlag[] = { 0xF0, 0x5D, 0x4A, 0x8C }; // Flag that OTA update has finished
CHIP_ERROR OTAImageProcessorImpl::PrepareDownload()
{
    ChipLogProgress(SoftwareUpdate, "Prepare download");

    DeviceLayer::PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
    ChipLogProgress(SoftwareUpdate, "Finalize");

    DeviceLayer::PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
    ChipLogProgress(SoftwareUpdate, "Apply");

    DeviceLayer::PlatformMgr().ScheduleWork(HandleApply, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
    ChipLogProgress(SoftwareUpdate, "Abort");

    DeviceLayer::PlatformMgr().ScheduleWork(HandleAbort, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
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

CHIP_ERROR OTAImageProcessorImpl::ProcessBlock(ByteSpan & block)
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

    ChipLogProgress(SoftwareUpdate, "%s [%d] OTA address space will be upgraded", __FUNCTION__, __LINE__);
    imageProcessor->mHeaderParser.Init(); // Initialize the status of OTA parse
    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    auto * imageProcessor                 = reinterpret_cast<OTAImageProcessorImpl *>(context);
    bk_logic_partition_t * partition_info = NULL;
    UINT32 dwFlagAddrOffset               = 0;
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }

#if CONFIG_FLASH_ORIGIN_API
    partition_info = bk_flash_get_info(BK_PARTITION_OTA);
#else
    partition_info = bk_flash_partition_get_info(BK_PARTITION_OTA);
#endif
    BK_CHECK_POINTER_NULL_TO_VOID(partition_info);
    dwFlagAddrOffset = partition_info->partition_length - (sizeof(ucFinishFlag) - 1);

    bk_write_ota_data_to_flash((char *) ucFinishFlag, dwFlagAddrOffset, (sizeof(ucFinishFlag) - 1));

    imageProcessor->ReleaseBlock();

    ChipLogProgress(SoftwareUpdate, "OTA image downloaded and written to flash");
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }

    // Abort OTA procedure

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

    ByteSpan block = ByteSpan(imageProcessor->mBlock.data(), imageProcessor->mBlock.size());

    CHIP_ERROR error = imageProcessor->ProcessHeader(block);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Failed to process OTA image header");
        imageProcessor->mDownloader->EndDownload(error);
        return;
    }

    if (!imageProcessor->readHeader) // First block received, process header
    {
        ota_data_struct_t * tempBuf = (ota_data_struct_t *) chip::Platform::MemoryAlloc(sizeof(ota_data_struct_t));

        if (NULL == tempBuf)
        {
            ChipLogError(SoftwareUpdate, "%s [%d] malloc failed  ", __FUNCTION__, __LINE__);
        }
        memset((char *) tempBuf, 0, sizeof(ota_data_struct_t));
        memcpy((char *) &(imageProcessor->pOtaTgtHdr), block.data(), sizeof(ota_data_struct_t));

        imageProcessor->flash_data_offset = 0;

        bk_read_ota_data_in_flash((char *) tempBuf, imageProcessor->flash_data_offset, sizeof(ota_data_struct_t));
        ChipLogProgress(SoftwareUpdate, "Download version %s,date is %ld ", tempBuf->version, tempBuf->timestamp);
        ChipLogProgress(SoftwareUpdate, "Download size_raw %ld,size_package is %ld ", tempBuf->size_raw, tempBuf->size_package);
        ChipLogProgress(SoftwareUpdate, "imageProcessor version %s,date is 0x%lx ", imageProcessor->pOtaTgtHdr.version,
                        imageProcessor->pOtaTgtHdr.timestamp);

        bk_logic_partition_t * partition_info   = NULL;
        UINT32 dwFlagAddrOffset                 = 0;
        char ucflag[(sizeof(ucFinishFlag) - 1)] = { 0 };

#if CONFIG_FLASH_ORIGIN_API
        partition_info = bk_flash_get_info(BK_PARTITION_OTA);
#else
        partition_info = bk_flash_partition_get_info(BK_PARTITION_OTA);
#endif
        BK_CHECK_POINTER_NULL_TO_VOID(partition_info);

        dwFlagAddrOffset = partition_info->partition_length - (sizeof(ucFinishFlag) - 1);
        bk_read_ota_data_in_flash((char *) ucflag, dwFlagAddrOffset, (sizeof(ucFinishFlag) - 1));
        ChipLogProgress(SoftwareUpdate, "Block size is %d ,ucFinishFlag size len is %d", block.size(), sizeof(ucFinishFlag));

        if ((0 == memcmp(ucflag, ucFinishFlag, (sizeof(ucFinishFlag) - 1))) &&
            (0 == memcmp(tempBuf->version, imageProcessor->pOtaTgtHdr.version, sizeof(imageProcessor->pOtaTgtHdr.version))))
        {
            chip::Platform::MemoryFree(tempBuf);
            tempBuf = NULL;
            ChipLogError(SoftwareUpdate, "The version is is the same as the previous version");
            return;
        }

        imageProcessor->readHeader = true;
        ChipLogProgress(SoftwareUpdate, "flash_data_offset is 0x%lx", imageProcessor->flash_data_offset);

        // Erase update partition
        ChipLogProgress(SoftwareUpdate, "Erasing target partition...");
        bk_erase_ota_data_in_flash();
        ChipLogProgress(SoftwareUpdate, "Erasing target partition...");

        if (0 != bk_write_ota_data_to_flash((char *) block.data(), imageProcessor->flash_data_offset, block.size()))
        {
            chip::Platform::MemoryFree(tempBuf);
            tempBuf = NULL;
            ChipLogError(SoftwareUpdate, "bk_write_ota_data_to_flash failed %s [%d] ", __FUNCTION__, __LINE__);
            imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
            return;
        }

        imageProcessor->flash_data_offset += block.size(); // count next write flash address

        chip::Platform::MemoryFree(tempBuf);
        tempBuf = NULL;
    }
    else // received subsequent blocks
    {
        if (0 != bk_write_ota_data_to_flash((char *) block.data(), imageProcessor->flash_data_offset, block.size()))
        {
            ChipLogError(SoftwareUpdate, "bk_write_ota_data_to_flash failed %s [%d] ", __FUNCTION__, __LINE__);
            imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
            return;
        }

        imageProcessor->flash_data_offset += block.size(); // count next write flash address

        imageProcessor->size += block.size();
    }

    imageProcessor->mParams.downloadedBytes += block.size();
    imageProcessor->mDownloader->FetchNextData();
}

void OTAImageProcessorImpl::HandleApply(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    ChipLogError(SoftwareUpdate, "Update completly,will reboot %s [%d] ", __FUNCTION__, __LINE__);

    // HandleApply is called after delayed action time seconds are elapsed, so it would be safe to schedule the restart
    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(2 * 1000), HandleRestart, nullptr);
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

CHIP_ERROR OTAImageProcessorImpl::ProcessHeader(ByteSpan & block)
{
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

} // namespace chip
