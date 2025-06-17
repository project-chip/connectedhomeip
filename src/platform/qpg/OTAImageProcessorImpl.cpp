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

// Note: enable this when skip bytes should be used (dependant on support from OTA provider on the other side)
// #define USE_SKIP_BYTES_FOR_DOWNLOAD

namespace chip {

std::unique_ptr<OTAImageProcessor> CreateProcessor(qvCHIP_OtaImageType_t imageType, uint32_t otaAreaOffset, uint32_t imageSize)
{
    switch (imageType)
    {
    case qvCHIP_OtaImageTypeBootloader:
        return std::make_unique<BLImageProcessor>(otaAreaOffset, imageSize);
    case qvCHIP_OtaImageTypeSecureElement:
        return std::make_unique<SEImageProcessor>(otaAreaOffset, imageSize);
    case qvCHIP_OtaImageTypePrimaryApplication:
        return std::make_unique<PrimaryAPPImageProcessor>(otaAreaOffset, imageSize);
    case qvCHIP_OtaImageTypeSecondaryApplication:
        return std::make_unique<SecondaryAPPImageProcessor>(otaAreaOffset, imageSize);
    default:
        ChipLogError(SoftwareUpdate, "CreateProcessor: Wrong image type - %lu", imageType);
        return nullptr;
    }
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

    DeviceLayer::PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this));
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

        if (false == qvCHIP_OtaImageDownloadInProgress())
        {
            // Load qvCHIP_Ota header structure and call application callback to validate image header
            qvCHIP_Ota_ImageHeader_t qvCHIP_OtaImgHeader;
            this->mSwVer                             = header.mSoftwareVersion; // Store software version in imageProcessor as well
            qvCHIP_OtaImgHeader.vendorId             = header.mVendorId;
            qvCHIP_OtaImgHeader.productId            = header.mProductId;
            qvCHIP_OtaImgHeader.softwareVersion      = header.mSoftwareVersion;
            qvCHIP_OtaImgHeader.minApplicableVersion = header.mMinApplicableVersion.ValueOr(0);
            qvCHIP_OtaImgHeader.maxApplicableVersion = header.mMaxApplicableVersion.ValueOr(0);

            if (true != qvCHIP_OtaValidateImage(qvCHIP_OtaImgHeader))
            {
                // Dropping image due to invalid Matter header
                return CHIP_DEVICE_ERROR_SOFTWARE_UPDATE_IGNORED;
            }

            // Extract Qorvo header version and size
            uint16_t headerVersion = *reinterpret_cast<const uint16_t *>(block.data());

            // Check header version
            VerifyOrReturnError(headerVersion == SUPPORTED_HEADER_VERSION, CHIP_ERROR_VERSION_MISMATCH);

            // Validate header signature and store header content to NVM
            // Note: this will initialize progress storage structure; do not call anything else before it
            uint16_t headerSize;
            VerifyOrReturnError(true != qvCHIP_OtaValidateandStoreHeader(block.data(), block.size(), &headerSize, nullptr, 128),
                                CHIP_ERROR_INTEGRITY_CHECK_FAILED);

            // Get and store provider location
            OTARequestorInterface * requestor = GetRequestorInstance();
            using ProviderLocation            = chip::OTARequestorInterface::ProviderLocationType;
            Optional<ProviderLocation> lastUsedProvider;
            requestor->GetProviderLocation(lastUsedProvider);
            qvCHIP_OtaSetLastProvider(lastUsedProvider.Value().providerNodeID, lastUsedProvider.Value().endpoint,
                                      lastUsedProvider.Value().fabricIndex);
            ChipLogProgress(SoftwareUpdate, "Provider location - node id: 0x" ChipLogFormatX64 ", endpoint: %u, fabric index: %u",
                            ChipLogValueX64(lastUsedProvider.Value().providerNodeID), lastUsedProvider.Value().endpoint,
                            lastUsedProvider.Value().fabricIndex);

            // On the first block, get size and use it as block size - assumption is that all blocks
            // will have the same size (except maybe the last one). We need block size to convert
            // offset of download into block count
            qvCHIP_OtaSetBlockSize(block_size);

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
    DeviceLayer::PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
    CHIP_ERROR status = CHIP_NO_ERROR;

    if (qvCHIP_OtaStatusSuccess == qvCHIP_OtaSetPendingImage())
    {
        ChipLogProgress(SoftwareUpdate, "Q: Applying - resetting device");

        // Reset into Bootloader
        qvCHIP_OtaReset();
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

    qvCHIP_OtaEraseArea();
    qvCHIP_OtaStartWrite();

    // Initialize tracking variables
    imageProcessor->mParams.downloadedBytes = 0;
    ChipLogError(SoftwareUpdate, "reset downloadedbytes to 0");

    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{

    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    ChipLogProgress(SoftwareUpdate, "Q: HandleFinalize");

    imageProcessor->ReleaseBlock();
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    ChipLogProgress(SoftwareUpdate, "Q: HandleAbort");

    imageProcessor->ReleaseBlock();
    // Start from scratch
    imageProcessor->mParams.downloadedBytes = 0;
    qvCHIP_OtaResetProgressInfo();
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

    qvCHIP_Ota_SubImageHeader_t imageHeader;
    qvCHIP_OtaImageDownloadState_t imageState;
    uint8_t currentImageIndex = QVCHIP_OTA_MAX_IMAGES + 1;

    // Check if it's the first block for this sub-image and validate header
    while (currentImageIndex != 0xFF)
    {
        currentImageIndex               = qvCHIP_OtaGetCurrentImage(&imageHeader, &imageState);
        qvCHIP_OtaImageType_t imageType = imageHeader.imageIdentifier;
        uint32_t otaAreaOffset          = imageHeader.dataOffset;
        ChipLogProgress(SoftwareUpdate, "Current image index: %d, type= %lu, size=%ld, v=%lx do=%lx", currentImageIndex, imageType,
                        imageHeader.imageSize, imageHeader.imageVersion, imageHeader.dataOffset);

        // Should not happen
        if (currentImageIndex == 0xFF)
        {
            ChipLogError(SoftwareUpdate, "Current image is invalid: 0xFF");
            imageProcessor->mDownloader->EndDownload(CHIP_ERROR_INCORRECT_STATE);
            return;
        }

        // Create processor class if processor is null
        if (imageProcessor->mProcessor == nullptr)
        {
            imageProcessor->mProcessor = CreateProcessor(imageType, otaAreaOffset, imageHeader.imageSize);
            if (imageProcessor->mProcessor == NULL)
            {
                ChipLogError(SoftwareUpdate, "Image processor is null");
                imageProcessor->mDownloader->EndDownload(CHIP_ERROR_INVALID_ARGUMENT);
                return;
            }
        }

        if (imageState == qvCHIP_OtaPendingApply)
        {
            ChipLogDetail(SoftwareUpdate, "Current image state: qvCHIP_OtaPendingApply");

            // Verify update applied
            if (true == imageProcessor->mProcessor->VerifyUpdate(imageHeader.imageVersion))
            {
                ChipLogProgress(SoftwareUpdate, "Image index %d applied successfully !", currentImageIndex);
            }
            else
            {
                // TODO: what to do when an image fails to flash ?
                ChipLogError(SoftwareUpdate, "Image index %d was not applied successfully !", currentImageIndex);
            }

            // Adjust progress for current image and advance to next one
            qvCHIP_OtaSetImageState(qvCHIP_OtaDownloadCompleted);
            currentImageIndex = qvCHIP_OtaGoNextImage();

            // Destroy this processor class and set the pointer to null
            imageProcessor->mProcessor.reset();

            continue;
        }
        else if (imageState == qvCHIP_OtaDownloadNotStarted)
        {
            ChipLogDetail(SoftwareUpdate, "Current image state: qvCHIP_OtaDownloadNotStarted");

            if (false == imageProcessor->mProcessor->ValidateHeader(imageHeader))
            {
                ChipLogError(SoftwareUpdate, "Header is not validated for image type %lu", imageType);
                // Destroy this processor class and set the pointer to null
                imageProcessor->mProcessor.reset();

                imageState = qvCHIP_OtaDownloadIgnored;
                qvCHIP_OtaSetImageState(imageState);
                currentImageIndex = qvCHIP_OtaGoNextImage();

                continue;
            }

            ChipLogProgress(SoftwareUpdate, "Starting download for image type: %lu, version: %lu, size %lu, offset in OTA: 0x%lx",
                            imageHeader.imageIdentifier, imageHeader.imageVersion, imageHeader.imageSize, otaAreaOffset);

            imageState = qvCHIP_OtaDownloadInProgress;
            qvCHIP_OtaSetImageState(imageState);

            qvCHIP_OtaEraseArea();
            qvCHIP_OtaStartWrite();

            break;
        }
        else
        {
            break;
        }
    }
    if (currentImageIndex == 0xFF)
    {
        // This means it was the last image
        qvCHIP_OtaResetProgressInfo();
        return;
    }

    if (imageState != qvCHIP_OtaDownloadInProgress)
    {
        ChipLogError(SoftwareUpdate, "Invalid download state: %d (expected: %d)", imageState, qvCHIP_OtaDownloadInProgress);
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    if ((imageProcessor->mParams.downloadedBytes + imageProcessor->mBlock.size()) < imageHeader.dataOffset)
    {
        // Request corresponding block for the next image
        imageProcessor->mParams.downloadedBytes += imageProcessor->mBlock.size();
        ChipLogDetail(SoftwareUpdate, "Downloaded bytes: %" PRIu32, static_cast<uint32_t>(imageProcessor->mParams.downloadedBytes));
#ifdef USE_SKIP_BYTES_FOR_DOWNLOAD
        uint32_t skip_bytes = imageHeader.dataOffset - imageProcessor->mParams.downloadedBytes;
        imageProcessor->mParams.downloadedBytes += skip_bytes;
#endif
        ChipLogDetail(SoftwareUpdate, "Skipping this data, fetching block: %" PRIu32,
                      static_cast<uint32_t>((imageProcessor->mParams.downloadedBytes + qvCHIP_OtaGetBlockSize() - 1) /
                                            qvCHIP_OtaGetBlockSize()));
#ifdef USE_SKIP_BYTES_FOR_DOWNLOAD
        imageProcessor->mDownloader->SkipData(skip_bytes);
#else
        imageProcessor->mDownloader->FetchNextData();
#endif
        return;
    }

    // Check here if we need to trim the buffer in order to skip data from previous image
    uint32_t trimBytes = 0;
    if (imageProcessor->mParams.downloadedBytes < imageHeader.dataOffset)
    {
        trimBytes = imageHeader.dataOffset - imageProcessor->mParams.downloadedBytes;
        ChipLogDetail(SoftwareUpdate, "Trimming current block by: %" PRIu32, trimBytes);
    }
    MutableByteSpan localData = imageProcessor->mBlock.SubSpan(trimBytes);

    if (false == imageProcessor->mProcessor->ProcessBlock(localData))
    {
        ChipLogError(SoftwareUpdate, "Processor ProcessBlock op failed");
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
    }
    else
    {
        imageProcessor->mParams.downloadedBytes += imageProcessor->mBlock.size();
        ChipLogDetail(SoftwareUpdate, "Downloaded bytes: %" PRIu32, static_cast<uint32_t>(imageProcessor->mParams.downloadedBytes));
        imageProcessor->mDownloader->FetchNextData();
    }
}

CHIP_ERROR OTAImageProcessorImpl::SetBlock(ByteSpan & block)
{
    if (block.empty())
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
