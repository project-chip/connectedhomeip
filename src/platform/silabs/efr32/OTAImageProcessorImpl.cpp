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
#include <platform/silabs/OTAImageProcessorImpl.h>
#include <platform/silabs/SilabsConfig.h>

#if SL_WIFI
#include <platform/silabs/wifi/ncp/spi_multiplex.h>
#endif // SL_WIFI

extern "C" {
#include "btl_interface.h"
#include "sl_core.h"
}

#include <platform/silabs/SilabsConfig.h>

#ifdef _SILICON_LABS_32B_SERIES_2
// Series 2 bootloader_ api calls must be called from a critical section context for thread safeness
#define WRAP_BL_DFU_CALL(code)                                                                                                     \
    {                                                                                                                              \
        CORE_CRITICAL_SECTION(code;)                                                                                               \
    }
#else
// series 3 bootloader_ calls uses rtos mutex for thread safety. Cannot be called within a critical section
#define WRAP_BL_DFU_CALL(code)                                                                                                     \
    {                                                                                                                              \
        code;                                                                                                                      \
    }
#endif

/// No error, operation OK
#define SL_BOOTLOADER_OK 0L

static chip::OTAImageProcessorImpl gImageProcessor;

namespace chip {

// Define static memebers
uint8_t OTAImageProcessorImpl::mSlotId                                                  = 0;
uint32_t OTAImageProcessorImpl::mWriteOffset                                            = 0;
uint16_t OTAImageProcessorImpl::writeBufOffset                                          = 0;
uint8_t OTAImageProcessorImpl::writeBuffer[kAlignmentBytes] __attribute__((aligned(4))) = { 0 };

CHIP_ERROR OTAImageProcessorImpl::Init(OTADownloader * downloader)
{
    VerifyOrReturnError(downloader != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    gImageProcessor.SetOTADownloader(downloader);

    return CHIP_NO_ERROR;
}

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
        return err;
    }

    DeviceLayer::PlatformMgr().ScheduleWork(HandleProcessBlock, reinterpret_cast<intptr_t>(this));
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

void OTAImageProcessorImpl::HandlePrepareDownload(intptr_t context)
{
    int32_t err           = SL_BOOTLOADER_OK;
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);

    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "imageProcessor context is null");
        return;
    }
    else if (imageProcessor->mDownloader == nullptr)
    {
        ChipLogError(SoftwareUpdate, "mDownloader is null");
        return;
    }

    ChipLogProgress(SoftwareUpdate, "HandlePrepareDownload: started");

    WRAP_BL_DFU_CALL(err = bootloader_init())
    if (err != SL_BOOTLOADER_OK)
    {
        ChipLogProgress(SoftwareUpdate, "bootloader_init Failed error: %ld", err);
    }

    mSlotId                                 = 0; // Single slot until we support multiple images
    writeBufOffset                          = 0;
    mWriteOffset                            = 0;
    imageProcessor->mParams.downloadedBytes = 0;

    imageProcessor->mHeaderParser.Init();

    // Not calling bootloader_eraseStorageSlot(mSlotId) here because we erase during each write

    imageProcessor->mDownloader->OnPreparedForDownload(err == SL_BOOTLOADER_OK ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL);
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    uint32_t err          = SL_BOOTLOADER_OK;
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "imageProcessor context is null");
        return;
    }

    // Pad the remainder of the write buffer with zeros and write it to bootloader storage
    if (writeBufOffset != 0)
    {
        // Account for last bytes of the image not yet written to storage
        imageProcessor->mParams.downloadedBytes += writeBufOffset;

        while (writeBufOffset != kAlignmentBytes)
        {
            writeBuffer[writeBufOffset] = 0;
            writeBufOffset++;
        }
#if SL_BTLCTRL_MUX
        err = sl_wfx_host_pre_bootloader_spi_transfer();
        if (err != SL_STATUS_OK)
        {
            ChipLogError(SoftwareUpdate, "sl_wfx_host_pre_bootloader_spi_transfer() error: %ld", err);
            return;
        }
#endif // SL_BTLCTRL_MUX
        WRAP_BL_DFU_CALL(err = bootloader_eraseWriteStorage(mSlotId, mWriteOffset, writeBuffer, kAlignmentBytes))

#if SL_BTLCTRL_MUX
        err = sl_wfx_host_post_bootloader_spi_transfer();
        if (err != SL_STATUS_OK)
        {
            ChipLogError(SoftwareUpdate, "sl_wfx_host_post_bootloader_spi_transfer() error: %ld", err);
            return;
        }
#endif // SL_BTLCTRL_MUX
        if (err)
        {
            ChipLogError(SoftwareUpdate, "bootloader_eraseWriteStorage() error: %ld", err);
            imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
            return;
        }
    }

    imageProcessor->ReleaseBlock();

    ChipLogProgress(SoftwareUpdate, "OTA image downloaded successfully");
}

// TODO: SE access is not thread safe. It asserts if other tasks accesses it during bootloader_verifyImage or
// bootloader_setImageToBootload steps - MATTER-4155 - PLATFORM_HYD-3235
void OTAImageProcessorImpl::LockRadioProcessing()
{
#if !SL_WIFI && defined(_SILICON_LABS_32B_SERIES_3)
    DeviceLayer::ThreadStackMgr().LockThreadStack();
#endif // SL_WIFI
}

void OTAImageProcessorImpl::UnlockRadioProcessing()
{
#if !SL_WIFI && defined(_SILICON_LABS_32B_SERIES_3)
    DeviceLayer::ThreadStackMgr().UnlockThreadStack();
#endif // SL_WIFI
}

void OTAImageProcessorImpl::HandleApply(intptr_t context)
{
    uint32_t err = SL_BOOTLOADER_OK;

    ChipLogProgress(SoftwareUpdate, "HandleApply: verifying image");

    // Force KVS to store pending keys such as data from StoreCurrentUpdateInfo()
    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().ForceKeyMapSave();
#if SL_BTLCTRL_MUX
    err = sl_wfx_host_pre_bootloader_spi_transfer();
    if (err != SL_STATUS_OK)
    {
        ChipLogError(SoftwareUpdate, "sl_wfx_host_pre_bootloader_spi_transfer() error: %ld", err);
        return;
    }
#endif // SL_BTLCTRL_MUX

#if defined(_SILICON_LABS_32B_SERIES_3) && CHIP_PROGRESS_LOGGING
    osDelay(100); // sl-temp: delay for uart print before verifyImage
#endif            // _SILICON_LABS_32B_SERIES_3 && CHIP_PROGRESS_LOGGING
    LockRadioProcessing();
#if defined(SL_TRUSTZONE_NONSECURE)
    WRAP_BL_DFU_CALL(err = bootloader_verifyImage(mSlotId))
#else
    WRAP_BL_DFU_CALL(err = bootloader_verifyImage(mSlotId, NULL))
#endif
    UnlockRadioProcessing();

    if (err != SL_BOOTLOADER_OK)
    {
        ChipLogError(SoftwareUpdate, "bootloader_verifyImage() error: %ld", err);
        // Call the OTARequestor API to reset the state
        GetRequestorInstance()->CancelImageUpdate();
#if SL_BTLCTRL_MUX
        err = sl_wfx_host_post_bootloader_spi_transfer();
        if (err != SL_STATUS_OK)
        {
            ChipLogError(SoftwareUpdate, "sl_wfx_host_post_bootloader_spi_transfer() error: %ld", err);
        }
#endif // SL_BTLCTRL_MUX
        return;
    }
    ChipLogProgress(SoftwareUpdate, "Image verified, Set image to bootload");
    LockRadioProcessing();
    WRAP_BL_DFU_CALL(err = bootloader_setImageToBootload(mSlotId))
    UnlockRadioProcessing();
    if (err != SL_BOOTLOADER_OK)
    {
        ChipLogError(SoftwareUpdate, "bootloader_setImageToBootload() error: %ld", err);
        // Call the OTARequestor API to reset the state
        GetRequestorInstance()->CancelImageUpdate();
#if SL_BTLCTRL_MUX
        err = sl_wfx_host_post_bootloader_spi_transfer();
        if (err != SL_STATUS_OK)
        {
            ChipLogError(SoftwareUpdate, "sl_wfx_host_post_bootloader_spi_transfer() error: %ld", err);
        }
#endif // SL_BTLCTRL_MUX
        return;
    }

#if SL_BTLCTRL_MUX
    err = sl_wfx_host_post_bootloader_spi_transfer();
    if (err != SL_STATUS_OK)
    {
        ChipLogError(SoftwareUpdate, "sl_wfx_host_post_bootloader_spi_transfer() error: %ld", err);
        return;
    }
#endif // SL_BTLCTRL_MUX

    ChipLogProgress(SoftwareUpdate, "Reboot and install new image...");
#if defined(_SILICON_LABS_32B_SERIES_3) && CHIP_PROGRESS_LOGGING
    osDelay(100); // sl-temp: delay for uart print before reboot
#endif            // _SILICON_LABS_32B_SERIES_3 && CHIP_PROGRESS_LOGGING
    LockRadioProcessing();
    // This reboots the device
    WRAP_BL_DFU_CALL(bootloader_rebootAndInstall())
    UnlockRadioProcessing(); // Unneccessay but for good measure
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "imageProcessor context is null");
        return;
    }
    // Not clearing the image storage area as it is done during each write
    imageProcessor->ReleaseBlock();
}

void OTAImageProcessorImpl::HandleProcessBlock(intptr_t context)
{
    uint32_t err          = SL_BOOTLOADER_OK;
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "imageProcessor context is null");
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
        ChipLogError(SoftwareUpdate, "Matter image header parser error: %s", chip::ErrorStr(chip_error));
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_INVALID_FILE_IDENTIFIER);
        return;
    }

    // Copy data into the word-aligned writeBuffer, once it fills write its contents to the bootloader storage
    // Final data block is handled in HandleFinalize().
    uint32_t blockReadOffset = 0;
    while (blockReadOffset < block.size())
    {
        writeBuffer[writeBufOffset] = *((block.data()) + blockReadOffset);
        writeBufOffset++;
        blockReadOffset++;
        if (writeBufOffset == kAlignmentBytes)
        {
            writeBufOffset = 0;
#if SL_BTLCTRL_MUX
            err = sl_wfx_host_pre_bootloader_spi_transfer();
            if (err != SL_STATUS_OK)
            {
                ChipLogError(SoftwareUpdate, "sl_wfx_host_pre_bootloader_spi_transfer() error: %ld", err);
                return;
            }
#endif // SL_BTLCTRL_MUX
            WRAP_BL_DFU_CALL(err = bootloader_eraseWriteStorage(mSlotId, mWriteOffset, writeBuffer, kAlignmentBytes))

#if SL_BTLCTRL_MUX
            err = sl_wfx_host_post_bootloader_spi_transfer();
            if (err != SL_STATUS_OK)
            {
                ChipLogError(SoftwareUpdate, "sl_wfx_host_post_bootloader_spi_transfer() error: %ld", err);
                return;
            }
#endif // SL_BTLCTRL_MUX
            if (err)
            {
                ChipLogError(SoftwareUpdate, "bootloader_eraseWriteStorage() error: %ld", err);
                imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                return;
            }
            mWriteOffset += kAlignmentBytes;
            imageProcessor->mParams.downloadedBytes += kAlignmentBytes;
        }
    }

    imageProcessor->mDownloader->FetchNextData();
}

CHIP_ERROR OTAImageProcessorImpl::ProcessHeader(ByteSpan & block)
{
    if (mHeaderParser.IsInitialized())
    {
        OTAImageHeader header;
        CHIP_ERROR error = mHeaderParser.AccumulateAndDecode(block, header);

        // Needs more data to decode the header
        VerifyOrReturnError(error != CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
        ReturnErrorOnFailure(error);

        // SL TODO -- store version somewhere
        ChipLogProgress(SoftwareUpdate, "Image Header software version: %ld payload size: %lu", header.mSoftwareVersion,
                        (long unsigned int) header.mPayloadSize);
        mParams.totalFileBytes = header.mPayloadSize;
        mHeaderParser.Clear();
    }
    return CHIP_NO_ERROR;
}

// Store block data for HandleProcessBlock to access
CHIP_ERROR OTAImageProcessorImpl::SetBlock(ByteSpan & block)
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

CHIP_ERROR OTAImageProcessorImpl::ReleaseBlock()
{
    if (mBlock.data() != nullptr)
    {
        chip::Platform::MemoryFree(mBlock.data());
    }

    mBlock = MutableByteSpan();
    return CHIP_NO_ERROR;
}

OTAImageProcessorImpl & OTAImageProcessorImpl::GetDefaultInstance()
{
    return gImageProcessor;
}

} // namespace chip
