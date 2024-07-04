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
#include "app_common.h"
#if (OTA_SUPPORT == 1)
#include <app/clusters/ota-requestor/OTADownloader.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>

#include "OTAImageProcessorImpl.h"
#include "ota.h"
#include "sfu_fwimg_regions.h"
#include "stm_ext_flash.h"
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
#include "mapping_fwimg.h"
#include "mapping_sbsfu.h"
#elif defined(__ICCARM__) || defined(__GNUC__)
#include "mapping_export.h"
#endif /* __CC_ARM || __ARMCC_VERSION */
#include "sfu_standalone_loader.h"

#define STM_HEADER_SIZE 8
static uint32_t mCPU1Size;
static uint32_t mCPU2Size;
static uint32_t mDownloadedBytesCPU2;
static uint8_t mSwitchDwlSlot;

namespace chip {

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
    ChipLogProgress(DeviceLayer, "OTA Confirm current image");
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

CHIP_ERROR OTAImageProcessorImpl::PrepareDownload()
{

    // Get OTA status - under what circumstances does prepared break?
    // what happens if a prepare is pending and another one is invoked
    // Should we store the state here and wait until we receive notification

    mHeaderParser.Init();

    DeviceLayer::PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessHeader(ByteSpan & block)
{
    ChipLogProgress(DeviceLayer, "OTA Process Header");
    if (mHeaderParser.IsInitialized())
    {
        OTAImageHeader header;
        CHIP_ERROR error = mHeaderParser.AccumulateAndDecode(block, header);

        // Needs more data to decode the header
        ReturnErrorCodeIf(error == CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
        ReturnErrorOnFailure(error);

        mParams.totalFileBytes = header.mPayloadSize;
        mHeaderParser.Clear();

        // Load Ota_ImageHeader_t header structure and call application callback to validate image header
        Ota_ImageHeader_t OtaImgHeader;
        this->mSwVer                      = header.mSoftwareVersion; // Store software version in imageProcessor as well
        OtaImgHeader.vendorId             = header.mVendorId;
        OtaImgHeader.productId            = header.mProductId;
        OtaImgHeader.softwareVersion      = header.mSoftwareVersion;
        OtaImgHeader.minApplicableVersion = header.mMinApplicableVersion.ValueOr(0);
        OtaImgHeader.maxApplicableVersion = header.mMaxApplicableVersion.ValueOr(0);

        if (true != OtaHeaderValidation(OtaImgHeader))
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
    ChipLogProgress(SoftwareUpdate, "Applying - resetting device");

    if (mCPU2Size == 0)
    {
        STANDALONE_LOADER_STATE = STANDALONE_LOADER_INSTALL_REQ; // install only CPU1
    }
    else if (mCPU1Size == 0)
    {
        STANDALONE_LOADER_STATE = STANDALONE_LOADER_BYPASS_REQ; // install only CPU2
    }
    else
    {
        STANDALONE_LOADER_STATE = STANDALONE_LOADER_BYPASS_REQ_AND_INSTALL_REQ; // install CPU1 and CPU2
    }
    NVIC_SystemReset();

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

    CHIP_ERROR err = ProcessHeader(block);
    ChipLogProgress(DeviceLayer, "OTA Process Block");
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Matter image header parser error %s", chip::ErrorStr(err));
        this->mDownloader->EndDownload(CHIP_ERROR_INVALID_FILE_IDENTIFIER);
        return err;
    }

    // Store block data for HandleProcessBlock to access
    err = SetBlock(block);
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
    ChipLogProgress(DeviceLayer, "OTA Prepare DL");
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

    // running this in a thread so won't block main event loop
    ChipLogProgress(SoftwareUpdate, "HandlePrepareDownload");

    mSwitchDwlSlot = false; // start with dwl_slot1 for CPU1

    STM_EXT_FLASH_Delete_Image(EXTERNAL_FLASH_ADDRESS + SLOT_DWL_1_START, SLOT_SIZE(SLOT_DWL_1));
    STM_EXT_FLASH_Delete_Image(EXTERNAL_FLASH_ADDRESS + SLOT_DWL_4_START, SLOT_SIZE(SLOT_DWL_4));

    // Initialize tracking variables
    imageProcessor->mParams.downloadedBytes = 0;
    mDownloadedBytesCPU2                    = 0;
    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    ChipLogProgress(SoftwareUpdate, "HandleFinalize");

    imageProcessor->ReleaseBlock();
    // Start from scratch
    imageProcessor->mParams.downloadedBytes = 0;
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    ChipLogProgress(SoftwareUpdate, "HandleAbort");

    STM_EXT_FLASH_Delete_Image(EXTERNAL_FLASH_ADDRESS + SLOT_DWL_1_START, SLOT_SIZE(SLOT_DWL_1));
    STM_EXT_FLASH_Delete_Image(EXTERNAL_FLASH_ADDRESS + SLOT_DWL_4_START, SLOT_SIZE(SLOT_DWL_4));
    imageProcessor->ReleaseBlock();
    // Start from scratch
    imageProcessor->mParams.downloadedBytes = 0;
    mDownloadedBytesCPU2                    = 0;
    mSwitchDwlSlot                          = false; // start with dwl_slot1 for CPU1
}

void OTAImageProcessorImpl::HandleProcessBlock(intptr_t context)
{
    STM_OTA_StatusTypeDef status;
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
    if (mSwitchDwlSlot == false)
    { // CPU1 write in DWL_SLOT 1
        if (imageProcessor->mParams.downloadedBytes == 0)
        { // get STM_Header
            uint8_t STMHeader[STM_HEADER_SIZE];

            memcpy(STMHeader, reinterpret_cast<std::uint8_t *>(imageProcessor->mBlock.data()), sizeof(STMHeader));

            // retrieve the cpu1/2 size with STM header
            mCPU1Size = STMHeader[0] + ((STMHeader[1]) << 8) + ((STMHeader[2]) << 16) + ((STMHeader[3]) << 24);
            mCPU2Size = STMHeader[4] + ((STMHeader[5]) << 8) + ((STMHeader[6]) << 16) + ((STMHeader[7]) << 24);

            // check the header
            if ((mCPU1Size > SLOT_SIZE(SLOT_DWL_1)) || (mCPU2Size > SLOT_SIZE(SLOT_DWL_4)) || (mCPU2Size + mCPU1Size == 0))
            {
                ChipLogError(SoftwareUpdate, "Flash decode failed");
                imageProcessor->mDownloader->EndDownload(CHIP_ERROR_DECODE_FAILED);
                return;
            }

            if (mCPU1Size == 0)
            { // update only CPU2
                // write in DWL_SLOT1 data without STM header
                status = STM_EXT_FLASH_WriteChunk(imageProcessor->mParams.downloadedBytes + SLOT_DWL_4_START,
                                                  reinterpret_cast<std::uint8_t *>(imageProcessor->mBlock.data()) + STM_HEADER_SIZE,
                                                  static_cast<std::uint32_t>(imageProcessor->mBlock.size()) - STM_HEADER_SIZE);
                if (status != STM_EXT_FLASH_OK)
                {
                    ChipLogError(SoftwareUpdate, "Flash write failed");
                    imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                    return;
                }

                imageProcessor->mParams.downloadedBytes += imageProcessor->mBlock.size() - STM_HEADER_SIZE;
                mSwitchDwlSlot       = true; // switch to DWL_SLOT 4 for all the next data
                mDownloadedBytesCPU2 = imageProcessor->mBlock.size() - STM_HEADER_SIZE; // update the bytes write in Dwl_slot4
            }
            else
            {

                // write in DWL_SLOT1 data without STM header
                status = STM_EXT_FLASH_WriteChunk(imageProcessor->mParams.downloadedBytes + SLOT_DWL_1_START,
                                                  reinterpret_cast<std::uint8_t *>(imageProcessor->mBlock.data()) + STM_HEADER_SIZE,
                                                  static_cast<std::uint32_t>(imageProcessor->mBlock.size()) - STM_HEADER_SIZE);
                if (status != STM_EXT_FLASH_OK)
                {
                    ChipLogError(SoftwareUpdate, "Flash write failed");
                    imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                    return;
                }

                imageProcessor->mParams.downloadedBytes += imageProcessor->mBlock.size() - STM_HEADER_SIZE;
            }
        }
        else if ((imageProcessor->mParams.downloadedBytes + static_cast<std::uint32_t>(imageProcessor->mBlock.size())) >= mCPU1Size)
        {

            // split the block in 2 for Dwl_slot1 and Dwl_slot4
            uint32_t CPU2BlockStart =
                imageProcessor->mParams.downloadedBytes + static_cast<std::uint32_t>(imageProcessor->mBlock.size()) - mCPU1Size;
            uint32_t CPU1BlockEnd = static_cast<std::uint32_t>(imageProcessor->mBlock.size()) - CPU2BlockStart;

            if (CPU2BlockStart + CPU1BlockEnd == static_cast<std::uint32_t>(imageProcessor->mBlock.size()))
            {
                mDownloadedBytesCPU2 = 0;

                // CPU1 write in DWL_SLOT 1
                status = STM_EXT_FLASH_WriteChunk(imageProcessor->mParams.downloadedBytes + SLOT_DWL_1_START,
                                                  reinterpret_cast<std::uint8_t *>(imageProcessor->mBlock.data()), CPU1BlockEnd);
                if (status != STM_EXT_FLASH_OK)
                {
                    ChipLogError(SoftwareUpdate, "Flash write failed");
                    imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                    return;
                }

                if (mCPU2Size != 0)
                {
                    // CPU2 write in DWL_SLOT 4
                    status = STM_EXT_FLASH_WriteChunk(
                        mDownloadedBytesCPU2 + SLOT_DWL_4_START,
                        reinterpret_cast<std::uint8_t *>(imageProcessor->mBlock.data()) + CPU1BlockEnd, CPU2BlockStart);
                    if (status != STM_EXT_FLASH_OK)
                    {
                        ChipLogError(SoftwareUpdate, "Flash write failed");
                        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                        return;
                    }

                    mSwitchDwlSlot = true; // switch to DWL_SLOT 4 for all the next data
                    imageProcessor->mParams.downloadedBytes += imageProcessor->mBlock.size(); // keep track of all bytes dwl
                    mDownloadedBytesCPU2 = CPU2BlockStart;                                    // update the bytes write in Dwl_slot4
                }
            }
            else
            {
                ChipLogError(SoftwareUpdate, "Flash decode failed");
                imageProcessor->mDownloader->EndDownload(CHIP_ERROR_DECODE_FAILED);
                return;
            }
        }
        else
        { // CPU1 write in DWL_SLOT 1
            status = STM_EXT_FLASH_WriteChunk(imageProcessor->mParams.downloadedBytes + SLOT_DWL_1_START,
                                              reinterpret_cast<std::uint8_t *>(imageProcessor->mBlock.data()),
                                              static_cast<std::uint32_t>(imageProcessor->mBlock.size()));
            if (status != STM_EXT_FLASH_OK)
            {
                ChipLogError(SoftwareUpdate, "Flash write failed");
                imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                return;
            }
            imageProcessor->mParams.downloadedBytes += imageProcessor->mBlock.size();
        }
    }
    else
    { // CPU2 write in DWL_SLOT 4
        status = STM_EXT_FLASH_WriteChunk(mDownloadedBytesCPU2 + SLOT_DWL_4_START,
                                          reinterpret_cast<std::uint8_t *>(imageProcessor->mBlock.data()),
                                          static_cast<std::uint32_t>(imageProcessor->mBlock.size()));

        if (status != STM_EXT_FLASH_OK)
        {
            ChipLogError(SoftwareUpdate, "Flash write failed");
            imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
            return;
        }

        imageProcessor->mParams.downloadedBytes += imageProcessor->mBlock.size(); // keep track of all bytes dwl
        mDownloadedBytesCPU2 += imageProcessor->mBlock.size();                    // update the bytes write in Dwl_slot4
    }

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
#endif
