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

#include "OTAImageProcessorImpl.h"

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
    OTARequestorInterface * requestor = chip::GetRequestorInstance();
    if (requestor == nullptr)
    {
        return CHIP_ERROR_INTERNAL;
    }

    uint32_t currentSwVersion;
    uint32_t targetVersion = requestor->GetTargetVersion();
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSoftwareVersion(currentSwVersion));

    if (currentSwVersion != targetVersion)
    {
        ChipLogError(SoftwareUpdate, "Current software version = %" PRIu32 ", expected software version = %" PRIu32,
                     currentSwVersion, targetVersion);
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

    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessHeader(ByteSpan & block)
{
    if (mHeaderParser.IsInitialized())
    {
        OTAImageHeader header;
        size_t block_size = block.size();
        CHIP_ERROR error  = mHeaderParser.AccumulateAndDecode(block, header);
        // Note: block now points to the remaining data after the header

        // Needs more data to decode the header
        VerifyOrReturnError(error != CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
        ReturnErrorOnFailure(error);

        mParams.totalFileBytes = header.mPayloadSize;
        mHeaderParser.Clear();

        if (false == qvOta_ImageDownloadInProgress())
        {
            // Load qvOta_ header structure and call application callback to validate image header
            qvOta_ImageHeader_t qvOta_ImgHeader;
            this->mSwVer                         = header.mSoftwareVersion; // Store software version in imageProcessor as well
            qvOta_ImgHeader.vendorId             = header.mVendorId;
            qvOta_ImgHeader.productId            = header.mProductId;
            qvOta_ImgHeader.softwareVersion      = header.mSoftwareVersion;
            qvOta_ImgHeader.minApplicableVersion = header.mMinApplicableVersion.ValueOr(0);
            qvOta_ImgHeader.maxApplicableVersion = header.mMaxApplicableVersion.ValueOr(0);

            // Call Matter OTA processor to handle the header
            uint16_t headerSize = 0;
            VerifyOrReturnError(
                qvOta_StatusSuccess ==
                    this->mProcessor->ProcessHeader(&qvOta_ImgHeader, &headerSize, block_size, block.data(), block.size()),
                CHIP_DEVICE_ERROR_SOFTWARE_UPDATE_IGNORED);

            // Get and store provider location
            OTARequestorInterface * requestor = GetRequestorInstance();
            using ProviderLocation            = chip::OTARequestorInterface::ProviderLocationType;
            Optional<ProviderLocation> lastUsedProvider;
            requestor->GetProviderLocation(lastUsedProvider);
            qvOta_SetLastProvider(lastUsedProvider.Value().providerNodeID, lastUsedProvider.Value().endpoint,
                                  lastUsedProvider.Value().fabricIndex);
            ChipLogProgress(SoftwareUpdate, "Provider location - node id: 0x" ChipLogFormatX64 ", endpoint: %u, fabric index: %u",
                            ChipLogValueX64(lastUsedProvider.Value().providerNodeID), lastUsedProvider.Value().endpoint,
                            lastUsedProvider.Value().fabricIndex);

            block = block.SubSpan(headerSize);

            // Adjust downloaded bytes to include Qorvo header data - it is not written in OTA, but
            // included in the package offset address for each sub-image
            this->mParams.downloadedBytes += (headerSize);
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
    CHIP_ERROR status = CHIP_NO_ERROR;

    if (qvOta_StatusSuccess == qvOta_SetPendingImage())
    {
        ChipLogProgress(SoftwareUpdate, "Q: Applying - resetting device");

        // Reset into Bootloader
        qvOta_Reset();
    }
    else
    {
        ChipLogError(SoftwareUpdate, "failed to set app pending!");
        status = CHIP_ERROR_INTERNAL;
    }

    return status;
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(HandleAbort, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessBlock(ByteSpan & block)
{
    if ((block.data() == nullptr) || block.empty())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CHIP_ERROR err = ProcessHeader(block);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Matter image header parser error: %" CHIP_ERROR_FORMAT, err.Format());
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

    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(HandleProcessBlock, reinterpret_cast<intptr_t>(this));
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

    // running this in a thread so won't block main event loop
    ChipLogProgress(SoftwareUpdate, "Q: HandlePrepareDownload");

    qvOta_Status_t result = imageProcessor->mProcessor->PrepareDownload();
    if (result != qvOta_StatusSuccess)
    {
        ChipLogError(SoftwareUpdate, "Failed to prepare download");
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    // Initialize tracking variables
    imageProcessor->mParams.downloadedBytes = 0;
    ChipLogProgress(SoftwareUpdate, "reset downloadedbytes to 0");

    TEMPORARY_RETURN_IGNORED imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{

    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    ChipLogProgress(SoftwareUpdate, "Q: HandleFinalize");

    TEMPORARY_RETURN_IGNORED imageProcessor->ReleaseBlock();

    // Call processor finalize method
    qvOta_Status_t result = imageProcessor->mProcessor->Finalize();
    if (result != qvOta_StatusSuccess)
    {
        ChipLogError(SoftwareUpdate, "Failed to finalize download");
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_INTERNAL);
        return;
    }
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    ChipLogProgress(SoftwareUpdate, "Q: HandleAbort");

    TEMPORARY_RETURN_IGNORED imageProcessor->ReleaseBlock();
    // Start from scratch
    imageProcessor->mParams.downloadedBytes = 0;

    // Call processor abort method
    qvOta_Status_t result = imageProcessor->mProcessor->Abort();
    if (result != qvOta_StatusSuccess)
    {
        ChipLogError(SoftwareUpdate, "Failed to abort download");
        return;
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

    // Call Matter OTA processor method to process the block
    uint32_t skip_bytes   = 0;
    qvOta_Status_t result = imageProcessor->mProcessor->ProcessBlock(&imageProcessor->mParams.downloadedBytes, &skip_bytes,
                                                                     imageProcessor->mBlock.data(), imageProcessor->mBlock.size());
    if (result != qvOta_StatusSuccess)
    {
        ChipLogError(SoftwareUpdate, "Failed to process block");
        if (result == qvOta_StatusIncorrectState)
        {
            ChipLogError(SoftwareUpdate, "Incorrect state during block processing");
            imageProcessor->mDownloader->EndDownload(CHIP_ERROR_INCORRECT_STATE);
        }
        else if (result == qvOta_StatusWriteError)
        {
            ChipLogError(SoftwareUpdate, "Failed to write block data");
            imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
        }
        else if (result == qvOta_StatusInvalidParam)
        {
            ChipLogError(SoftwareUpdate, "Invalid argument during block processing");
            imageProcessor->mDownloader->EndDownload(CHIP_ERROR_INVALID_ARGUMENT);
        }
        else
        {
            ChipLogError(SoftwareUpdate, "Unknown error during block processing: %d", result);
            imageProcessor->mDownloader->EndDownload(CHIP_ERROR_INTERNAL);
        }
        return;
    }
#if USE_SKIP_BYTES_FOR_DOWNLOAD
    if (skip_bytes > 0)
    {
        // Skip data from previous image if needed
        // Note: this is only used if the OTA provider supports skipping bytes
        imageProcessor->mDownloader->SkipData(skip_bytes);
    }
    else
#endif
    {
        TEMPORARY_RETURN_IGNORED imageProcessor->mDownloader->FetchNextData();
    }
}

CHIP_ERROR OTAImageProcessorImpl::SetBlock(ByteSpan & block)
{
    if (block.empty())
    {
        TEMPORARY_RETURN_IGNORED ReleaseBlock();
        return CHIP_NO_ERROR;
    }
    if (mBlock.size() < block.size())
    {
        if (!mBlock.empty())
        {
            TEMPORARY_RETURN_IGNORED ReleaseBlock();
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
