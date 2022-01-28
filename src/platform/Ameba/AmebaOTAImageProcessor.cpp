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
#include <lib/support/logging/CHIPLogging.h>

#include <platform/Ameba/AmebaOTAImageProcessor.h>

#if defined(CONFIG_PLATFORM_8710C)
static flash_t flash_ota;
#endif

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

    // Get OTA update partition
#if defined(CONFIG_PLATFORM_8721D)
    if (ota_get_cur_index() == OTA_INDEX_1)
        imageProcessor->ota_target_index = OTA_INDEX_2;
    else if (ota_get_cur_index() == OTA_INDEX_2)
        imageProcessor->ota_target_index = OTA_INDEX_1;
    ChipLogProgress(SoftwareUpdate, "OTA%d address space will be upgraded", imageProcessor->ota_target_index + 1);
#elif defined(CONFIG_PLATFORM_8710C)
    imageProcessor->flash_addr = sys_update_ota_prepare_addr();
    ChipLogProgress(SoftwareUpdate, "New Flash Address: 0x%X", imageProcessor->flash_addr);
#endif

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

    // Verify checksum
#if defined(CONFIG_PLATFORM_8721D)
    if (verify_ota_checksum(imageProcessor->pOtaTgtHdr) != 1)
    {
        ChipLogError(SoftwareUpdate, "OTA  checksum verification failed");
        return;
    }
#endif

    // Update signature
#if defined(CONFIG_PLATFORM_8721D)
    if (change_ota_signature(imageProcessor->pOtaTgtHdr, imageProcessor->ota_target_index) != 1)
    {
        ChipLogError(SoftwareUpdate, "OTA update signature failed");
        return;
    }
#elif defined(CONFIG_PLATFORM_8710C)
    if (update_ota_signature(imageProcessor->signature, imageProcessor->flash_addr) < 0)
    {
        ChipLogError(SoftwareUpdate, "OTA update signature failed");
        return;
    }
#endif

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

    // Abort OTA procedure

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

#if defined(CONFIG_PLATFORM_8721D)
    if (!imageProcessor->readHeader) // First block received, process header
    {
        uint8_t * tempBuf          = (uint8_t *) ota_update_malloc(32);
        imageProcessor->pOtaTgtHdr = (update_ota_target_hdr *) ota_update_malloc(sizeof(update_ota_target_hdr));

        memcpy(tempBuf, imageProcessor->mBlock.data(), 32);
        memcpy(imageProcessor->pOtaTgtHdr, tempBuf, 8); // Store FwVer, HdrNum
        memcpy(&(imageProcessor->pOtaTgtHdr->FileImgHdr[0].ImgHdrLen), tempBuf + 12,
               16);                                                                 // Store ImgHdrLen, Checksum, ImgLen, Offset
        memcpy(&(imageProcessor->pOtaTgtHdr->FileImgHdr[0].ImgId), tempBuf + 8, 4); // Store OTA id

        if (imageProcessor->ota_target_index == OTA_INDEX_1)
            imageProcessor->pOtaTgtHdr->FileImgHdr[0].FlashAddr = LS_IMG2_OTA1_ADDR;
        else if (imageProcessor->ota_target_index == OTA_INDEX_2)
            imageProcessor->pOtaTgtHdr->FileImgHdr[0].FlashAddr = LS_IMG2_OTA2_ADDR;

        imageProcessor->pOtaTgtHdr->ValidImgCnt = 1;

        if (strncmp("OTA", (const char *) &(imageProcessor->pOtaTgtHdr->FileImgHdr[0].ImgId), 3) != 0)
        {
            ChipLogError(SoftwareUpdate, "Wrong Image ID for OTA");
            return;
        }

        imageProcessor->readHeader = true;
        ChipLogProgress(SoftwareUpdate, "Correct OTA Image ID, get firmware header success");
        ChipLogProgress(SoftwareUpdate, "FwVer: 0x%X", imageProcessor->pOtaTgtHdr->FileHdr.FwVer);
        ChipLogProgress(SoftwareUpdate, "HdrNum: 0x%X", imageProcessor->pOtaTgtHdr->FileHdr.HdrNum);
        ChipLogProgress(SoftwareUpdate, "ImgHdrLen: 0x%X", imageProcessor->pOtaTgtHdr->FileImgHdr[0].ImgHdrLen);
        ChipLogProgress(SoftwareUpdate, "Checksum: 0x%X", imageProcessor->pOtaTgtHdr->FileImgHdr[0].Checksum);
        ChipLogProgress(SoftwareUpdate, "ImgLen: 0x%X", imageProcessor->pOtaTgtHdr->FileImgHdr[0].ImgLen);
        ChipLogProgress(SoftwareUpdate, "Offset: 0x%X", imageProcessor->pOtaTgtHdr->FileImgHdr[0].Offset);
        ChipLogProgress(SoftwareUpdate, "FlashAddr: 0x%X", imageProcessor->pOtaTgtHdr->FileImgHdr[0].FlashAddr);

        // Erase update partition
        ChipLogProgress(SoftwareUpdate, "Erasing target partition...");
        erase_ota_target_flash(imageProcessor->pOtaTgtHdr->FileImgHdr[0].FlashAddr,
                               imageProcessor->pOtaTgtHdr->FileImgHdr[0].ImgLen);
        ChipLogProgress(SoftwareUpdate, "Erased partition OTA%d", imageProcessor->ota_target_index + 1);

        // Set RemainBytes to image length, excluding 8bytes of signature
        imageProcessor->RemainBytes = imageProcessor->pOtaTgtHdr->FileImgHdr[0].ImgLen - 8;

        // Set flash address, incremented by 8bytes to account for signature
        imageProcessor->flash_addr = imageProcessor->pOtaTgtHdr->FileImgHdr[0].FlashAddr - SPI_FLASH_BASE + 8;

        // Set signature to point to pOtaTgtHdr->Sign
        imageProcessor->signature = &(imageProcessor->pOtaTgtHdr->Sign[0][0]);

        // Store the signature temporarily
        uint8_t * tempbufptr = imageProcessor->mBlock.data() + imageProcessor->pOtaTgtHdr->FileImgHdr[0].Offset;
        memcpy(imageProcessor->signature, tempbufptr, 8);
        tempbufptr += 8;

        // Write remaining downloaded bytes to flash_addr
        uint32_t tempsize = imageProcessor->mBlock.size() - imageProcessor->pOtaTgtHdr->FileImgHdr[0].Offset - 8;
        device_mutex_lock(RT_DEV_LOCK_FLASH);
        if (ota_writestream_user(imageProcessor->flash_addr + imageProcessor->size, tempsize, tempbufptr) < 0)
        {
            ChipLogError(SoftwareUpdate, "Write to flash failed");
            device_mutex_unlock(RT_DEV_LOCK_FLASH);
            imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
            return;
        }
        device_mutex_unlock(RT_DEV_LOCK_FLASH);

        imageProcessor->size += tempsize;
        imageProcessor->RemainBytes -= tempsize;

        ota_update_free(tempBuf);
    }
    else // received subsequent blocks
    {
        device_mutex_lock(RT_DEV_LOCK_FLASH);
        if (ota_writestream_user(imageProcessor->flash_addr + imageProcessor->size, imageProcessor->mBlock.size(),
                                 imageProcessor->mBlock.data()) < 0)
        {
            ChipLogError(SoftwareUpdate, "Write to flash failed");
            device_mutex_unlock(RT_DEV_LOCK_FLASH);
            imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
            return;
        }
        device_mutex_unlock(RT_DEV_LOCK_FLASH);

        imageProcessor->size += imageProcessor->mBlock.size();
        imageProcessor->RemainBytes -= imageProcessor->mBlock.size();
    }
#elif defined(CONFIG_PLATFORM_8710C)
    if (!imageProcessor->readHeader) // First block received, process 32bytes signature
    {
        // Store signature temporarily
        memcpy(imageProcessor->signature, imageProcessor->mBlock.data(), 32);

        imageProcessor->block_len = imageProcessor->mBlock.size() - 32; // minus 32 to account for signature

        // Erase target partition
        ChipLogProgress(SoftwareUpdate, "Erasing partition");
        imageProcessor->NewFWBlkSize = ((0x1F8000 - 1) / 4096) + 1; // Use a fixed image length of 0xF8000, change in the future
        ChipLogProgress(SoftwareUpdate, "Erasing %d sectors", imageProcessor->NewFWBlkSize);
        device_mutex_lock(RT_DEV_LOCK_FLASH);
        for (int i = 0; i < imageProcessor->NewFWBlkSize; i++)
            flash_erase_sector(&flash_ota, imageProcessor->flash_addr + i * 4096);
        device_mutex_unlock(RT_DEV_LOCK_FLASH);

        // Write first block to target flash
        if (imageProcessor->block_len > 0)
        {
            device_mutex_lock(RT_DEV_LOCK_FLASH);
            if (flash_burst_write(&flash_ota, imageProcessor->flash_addr + 32, imageProcessor->block_len,
                                  imageProcessor->mBlock.data() + 32) < 0)
            {
                device_mutex_unlock(RT_DEV_LOCK_FLASH);
                ChipLogError(SoftwareUpdate, "Write to flash failed");
                return;
            }
            else
            {
                imageProcessor->size += imageProcessor->block_len;
                device_mutex_unlock(RT_DEV_LOCK_FLASH);
            }
        }
        else
        {
            ChipLogError(SoftwareUpdate, "Invalid size");
            return;
        }

        imageProcessor->readHeader = true;
    }
    else // received subsequent blocks
    {
        imageProcessor->block_len = imageProcessor->mBlock.size();

        // Write first block to target flash
        if (imageProcessor->block_len > 0)
        {
            device_mutex_lock(RT_DEV_LOCK_FLASH);
            if (flash_burst_write(&flash_ota, imageProcessor->flash_addr + 32 + imageProcessor->size, imageProcessor->block_len,
                                  imageProcessor->mBlock.data()) < 0)
            {
                device_mutex_unlock(RT_DEV_LOCK_FLASH);
                ChipLogError(SoftwareUpdate, "Write to flash failed");
                return;
            }
            else
            {
                imageProcessor->size += imageProcessor->block_len;
                device_mutex_unlock(RT_DEV_LOCK_FLASH);
            }
        }
        else
        {
            ChipLogError(SoftwareUpdate, "Invalid size");
            return;
        }
    }
#endif
    imageProcessor->mParams.downloadedBytes += imageProcessor->mBlock.size();
    imageProcessor->mDownloader->FetchNextData();
}

void AmebaOTAImageProcessor::HandleApply(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<AmebaOTAImageProcessor *>(context);

    // Reboot
    ota_platform_reset();
}

CHIP_ERROR AmebaOTAImageProcessor::SetBlock(ByteSpan & block)
{
    if ((block.data() == nullptr) || block.empty())
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
