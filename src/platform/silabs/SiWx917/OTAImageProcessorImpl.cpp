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
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>
#include <platform/silabs/wifi/WifiInterface.h>

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <platform/silabs/wifi/icd/WifiSleepManager.h>
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

#ifdef __cplusplus
extern "C" {
#endif
#include "sl_si91x_driver.h"
#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_hal_soc_soft_reset.h"
#endif
#ifdef __cplusplus
}
#endif

#define RPS_HEADER 1
#define RPS_DATA 2

uint8_t flag = RPS_HEADER;
static chip::OTAImageProcessorImpl gImageProcessor;

using namespace chip::DeviceLayer::Silabs;

namespace chip {

// Define static memebers
bool OTAImageProcessorImpl::mReset                                                      = false;
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
    int32_t status        = SL_STATUS_OK;
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

    ChipLogProgress(SoftwareUpdate, "HandlePrepareDownload");

    mReset                                  = false;
    mSlotId                                 = 0; // Single slot until we support multiple images
    writeBufOffset                          = 0;
    mWriteOffset                            = 0;
    imageProcessor->mParams.downloadedBytes = 0;

    imageProcessor->mHeaderParser.Init();

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    // Setting the device in high performance - no-sleep mode during OTA tranfer
    DeviceLayer::Silabs::WifiSleepManager::GetInstance().RequestHighPerformance();
#endif /* CHIP_CONFIG_ENABLE_ICD_SERVER*/

    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    int32_t status        = SL_STATUS_OK;
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }
    if (writeBufOffset != 0)
    {
        // Account for last bytes of the image not yet written to storage
        imageProcessor->mParams.downloadedBytes += writeBufOffset;
        status = sl_si91x_fwup_load(writeBuffer, writeBufOffset);
        ChipLogProgress(SoftwareUpdate, "status: 0x%lX", status);

        if (status != SL_STATUS_OK)
        {
            if (status == SL_STATUS_SI91X_FW_UPDATE_DONE)
            {
                mReset = true;
            }
            else
            {
                ChipLogError(SoftwareUpdate, "ERROR: In HandleFinalize for last chunk sl_si91x_fwup_load() error 0x%lx", status);
                imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                return;
            }
        }
    }
    imageProcessor->ReleaseBlock();

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    // Setting the device back to power save mode when transfer is completed successfully
    DeviceLayer::Silabs::WifiSleepManager::GetInstance().RemoveHighPerformanceRequest();
#endif /* CHIP_CONFIG_ENABLE_ICD_SERVER*/

    ChipLogProgress(SoftwareUpdate, "OTA image downloaded successfully");
}

void OTAImageProcessorImpl::HandleApply(intptr_t context)
{
    int32_t status = SL_STATUS_OK;

    ChipLogProgress(SoftwareUpdate, "OTAImageProcessorImpl::HandleApply()");

    // Force KVS to store pending keys such as data from StoreCurrentUpdateInfo()
    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().ForceKeyMapSave();

    ChipLogProgress(SoftwareUpdate, "OTA image downloaded successfully in HandleApply");

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    // Setting the device is in high performace - no-sleepy mode before soft reset as soft reset is not happening in sleep mode
    DeviceLayer::Silabs::WifiSleepManager::GetInstance().RequestHighPerformance();
#endif /* CHIP_CONFIG_ENABLE_ICD_SERVER*/

    if (mReset)
    {
        ChipLogProgress(SoftwareUpdate, "M4 Firmware update complete");
        // send system reset request to reset the MCU and upgrade the m4 image
        ChipLogProgress(SoftwareUpdate, "SoC Soft Reset initiated!");
        // Reboots the device
        GetPlatform().SoftwareReset();
    }
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    // Setting the device back to power save mode when transfer is aborted in the middle
    DeviceLayer::Silabs::WifiSleepManager::GetInstance().RemoveHighPerformanceRequest();
#endif /* CHIP_CONFIG_ENABLE_ICD_SERVER*/

    // Not clearing the image storage area as it is done during each write
    imageProcessor->ReleaseBlock();
}

void OTAImageProcessorImpl::HandleProcessBlock(intptr_t context)
{
    int32_t status        = SL_STATUS_OK;
    int32_t content_block = 0;
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

    ByteSpan block        = imageProcessor->mBlock;
    CHIP_ERROR chip_error = imageProcessor->ProcessHeader(block);

    if (chip_error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Matter image header parser error %s", chip::ErrorStr(chip_error));
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
            if (flag == RPS_HEADER)
            {
                // Send RPS header which is received as first chunk
                status = sl_si91x_fwup_start(writeBuffer);
                status = sl_si91x_fwup_load(writeBuffer, kAlignmentBytes);
                flag   = RPS_DATA;
            }
            else if (flag == RPS_DATA)
            {
                // Send RPS content
                status = sl_si91x_fwup_load(writeBuffer, kAlignmentBytes);
                if (status != SL_STATUS_OK)
                {
                    // If the last chunk of last block-writeBufOffset length is exactly kAlignmentBytes(64) bytes then mReset value
                    // should be set to true in HandleProcessBlock
                    if (status == SL_STATUS_SI91X_FW_UPDATE_DONE)
                    {
                        mReset = true;
                    }
                    else
                    {
                        ChipLogError(SoftwareUpdate, "ERROR: In HandleProcessBlock sl_si91x_fwup_load() error 0x%lx", status);
                        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                        return;
                    }
                }
            }
            //     ChipLogProgress(SoftwareUpdate, "HandleProcessBlock status: 0x%lX", status);
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
