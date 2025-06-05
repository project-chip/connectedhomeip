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

#pragma once

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <lib/core/OTAImageHeader.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>

namespace chip {

constexpr size_t SUPPORTED_HEADER_VERSION = 1;
constexpr size_t HASH_SIZE                = 32;
constexpr size_t SIGNATURE_SIZE           = 256;

class OTAImageProcessor
{
public:
    virtual ~OTAImageProcessor() = default;

    OTAImageProcessor(uint32_t otaAreaOffset, uint32_t imageSize) :
        mOTAAreaOffset(otaAreaOffset), mImageSize(imageSize), mWrittenBytes(0)
    {}

    virtual bool ValidateHeader(const qvCHIP_Ota_SubImageHeader_t imageHeader) = 0;
    virtual void Apply()                                                       = 0;
    virtual bool VerifyUpdate(uint32_t newVersion)                             = 0;

    bool ProcessBlock(MutableByteSpan & block)
    {
        // The block might contain data for the next image, we need to just flash until
        // the declared image size
        uint16_t bytesToWrite = static_cast<std::uint16_t>(block.size());
        if (mWrittenBytes + bytesToWrite > mImageSize)
        {
            bytesToWrite = static_cast<std::uint16_t>(mImageSize - mWrittenBytes);
        }

        ChipLogProgress(SoftwareUpdate, "Q: HandleProcessBlock written: %" PRIu32 ", to write: %u", mWrittenBytes, bytesToWrite);

        qvCHIP_OtaStatus_t status =
            qvCHIP_OtaWriteChunk(mWrittenBytes, bytesToWrite, reinterpret_cast<std::uint8_t *>(block.data()));
        if (status != qvCHIP_OtaStatusSuccess)
        {
            ChipLogError(SoftwareUpdate, "Flash write failed");
            return false;
        }
        // TODO: hash content as we write it

        mWrittenBytes += bytesToWrite;

        if (mWrittenBytes >= mImageSize)
        {
            // Check hash on content
            if (true == qvCHIP_OtaVerifyContentHash())
            {
                qvCHIP_OtaSetImageState(qvCHIP_OtaPendingApply);
                Apply();
            }
            else
            {
                // Invalid content downloaded
                ChipLogError(SoftwareUpdate, "Failed content hash verification");
                return false;
            }
        }

        return true;
    }

protected:
    uint32_t mOTAAreaOffset;
    uint32_t mImageSize;
    uint32_t mWrittenBytes;
    std::array<uint8_t, HASH_SIZE> mContentHash;
};

class BLImageProcessor : public OTAImageProcessor
{
public:
    BLImageProcessor(uint32_t otaAreaOffset, uint32_t imageSize) : OTAImageProcessor(otaAreaOffset, imageSize) {}

    bool ValidateHeader(const qvCHIP_Ota_SubImageHeader_t imageHeader) override
    {
        uint32_t actualVersion;
        uint32_t changelistSpecifier;

        // Compare image version with the current one - should be greater, otherwise we skip it
        if (true == qvCHIP_GetBootloaderVersion(&actualVersion, &changelistSpecifier, nullptr))
        {
            ChipLogProgress(SoftwareUpdate, "BL hdr validation: actual version: 0x%lx / image version: 0x%lx", actualVersion,
                            imageHeader.imageVersion);
            return (imageHeader.imageVersion > actualVersion);
        }
        else
        {
            ChipLogError(SoftwareUpdate, "BL version retrieval failed");
            return false;
        }
    }

    void Apply() override
    {
        ChipLogProgress(SoftwareUpdate, "BL Apply");
        if (qvCHIP_OtaStatusSuccess == qvCHIP_OtaSetPendingBootloader())
        {
            qvCHIP_OtaReset();
        }
        else
        {
            ChipLogError(SoftwareUpdate, "failed to set BL pending!");
        }
    }

    bool VerifyUpdate(uint32_t newVersion) override
    {
        uint32_t actualVersion;
        uint32_t changelistSpecifier;

        if (true == qvCHIP_GetBootloaderVersion(&actualVersion, &changelistSpecifier, nullptr))
        {
            ChipLogProgress(SoftwareUpdate, "BL hdr validation: actual version: 0x%lx / image version: 0x%lx", actualVersion,
                            newVersion);
            return (newVersion == actualVersion);
        }
        else
        {
            ChipLogError(SoftwareUpdate, "BL version retrieval failed");
            return false;
        }
    }
};

class SEImageProcessor : public OTAImageProcessor
{
public:
    SEImageProcessor(uint32_t otaAreaOffset, uint32_t imageSize) : OTAImageProcessor(otaAreaOffset, imageSize) {}

    bool ValidateHeader(const qvCHIP_Ota_SubImageHeader_t imageHeader) override
    {
        uint32_t actualVersion;

        // Compare image version with the current one - should be greater, otherwise we skip it
        if (true == qvCHIP_GetSecureElementVersion(&actualVersion))
        {
            ChipLogProgress(SoftwareUpdate, "SE hdr validation: actual version: 0x%lx / image version: 0x%lx", actualVersion,
                            imageHeader.imageVersion);
            return (imageHeader.imageVersion > actualVersion);
        }
        else
        {
            ChipLogError(SoftwareUpdate, "failed to retrieve running SE version!");
            return false;
        }
    }

    void Apply() override
    {
        ChipLogProgress(SoftwareUpdate, "SE Apply");
        if (qvCHIP_OtaStatusSuccess == qvCHIP_OtaSetPendingSecureElement())
        {
            qvCHIP_OtaReset();
        }
        else
        {
            ChipLogError(SoftwareUpdate, "failed to set SE pending!");
        }
    }

    bool VerifyUpdate(uint32_t newVersion) override
    {
        uint32_t actualVersion;

        if (true == qvCHIP_GetSecureElementVersion(&actualVersion))
        {
            ChipLogProgress(SoftwareUpdate, "SE hdr validation: actual version: 0x%lx / image version: 0x%lx", actualVersion,
                            newVersion);
            return (newVersion == actualVersion);
        }
        else
        {
            ChipLogError(SoftwareUpdate, "failed to retrieve running SE version");
            return false;
        }
    }
};

class PrimaryAPPImageProcessor : public OTAImageProcessor
{
public:
    PrimaryAPPImageProcessor(uint32_t otaAreaOffset, uint32_t imageSize) : OTAImageProcessor(otaAreaOffset, imageSize) {}

    bool ValidateHeader(const qvCHIP_Ota_SubImageHeader_t imageHeader) override
    {
        uint32_t actualVersion;

        // Compare image version with the current one - should be greater, otherwise we skip it
        if (true == qvCHIP_GetApplicationVersionFromHeader(qvCHIP_OtaImageTypePrimaryApplication, &actualVersion))
        {
            ChipLogProgress(SoftwareUpdate, "APP hdr validation: actual version: 0x%lx / image version: 0x%lx", actualVersion,
                            imageHeader.imageVersion);
            return (imageHeader.imageVersion > actualVersion);
        }
        else
        {
            ChipLogError(SoftwareUpdate, "failed to get running application version");
            return false;
        }
    }

    void Apply() override
    {
        ChipLogDetail(SoftwareUpdate, "APP Apply");
        // Assume application is the last to be flashed and we don't need the progress struct
        qvCHIP_OtaResetProgressInfo();
    }

    bool VerifyUpdate(uint32_t newVersion) override
    {
        uint32_t actualVersion;

        if (true == qvCHIP_GetApplicationVersionFromHeader(qvCHIP_OtaImageTypePrimaryApplication, &actualVersion))
        {
            ChipLogProgress(SoftwareUpdate, "APP hdr validation: actual version: 0x%lx / image version: 0x%lx", actualVersion,
                            newVersion);
            return (newVersion == actualVersion);
        }
        else
        {
            ChipLogProgress(SoftwareUpdate, "failed to retrieve running application version");
            return false;
        }
    }
};

class SecondaryAPPImageProcessor : public OTAImageProcessor
{
public:
    SecondaryAPPImageProcessor(uint32_t otaAreaOffset, uint32_t imageSize) : OTAImageProcessor(otaAreaOffset, imageSize) {}

    bool ValidateHeader(const qvCHIP_Ota_SubImageHeader_t imageHeader) override
    {
        uint32_t actualVersion;

        // Compare image version with the current one - should be greater, otherwise we skip it
        if (true == qvCHIP_GetApplicationVersionFromHeader(qvCHIP_OtaImageTypeSecondaryApplication, &actualVersion))
        {
            ChipLogProgress(SoftwareUpdate, "Secondary APP hdr validation: actual version: 0x%lx / image version: 0x%lx",
                            actualVersion, imageHeader.imageVersion);
            return (imageHeader.imageVersion > actualVersion);
        }
        else
        {
            ChipLogError(SoftwareUpdate, "Failed to get secondary application version");
            return false;
        }
    }

    void Apply() override
    {
        ChipLogProgress(SoftwareUpdate, "Secondary APP Apply");
        if (qvCHIP_OtaStatusSuccess == qvCHIP_OtaSetPendingImage())
        {
            qvCHIP_OtaReset();
        }
        else
        {
            ChipLogError(SoftwareUpdate, "Failed to set pending flag for Secondary application !");
        }
    }

    bool VerifyUpdate(uint32_t newVersion) override
    {
        uint32_t actualVersion;

        if (true == qvCHIP_GetApplicationVersionFromHeader(qvCHIP_OtaImageTypeSecondaryApplication, &actualVersion))
        {
            ChipLogProgress(SoftwareUpdate, "Secondary APP hdr validation: actual version: 0x%lx / image version: 0x%lx",
                            actualVersion, newVersion);
            return (newVersion == actualVersion);
        }
        else
        {
            ChipLogProgress(SoftwareUpdate, "Failed to get secondary application version");
            return false;
        }
    }
};

class OTAImageProcessorImpl : public OTAImageProcessorInterface
{
public:
    //////////// OTAImageProcessorInterface Implementation ///////////////
    CHIP_ERROR PrepareDownload() override;
    CHIP_ERROR ProcessHeader(ByteSpan & block);
    CHIP_ERROR Finalize() override;
    CHIP_ERROR Apply() override;
    CHIP_ERROR Abort() override;
    CHIP_ERROR ProcessBlock(ByteSpan & block) override;
    bool IsFirstImageRun() override;
    CHIP_ERROR ConfirmCurrentImage() override;

    void SetOTADownloader(OTADownloader * downloader) { mDownloader = downloader; }

private:
    //////////// Actual handlers for the OTAImageProcessorInterface ///////////////
    static void HandlePrepareDownload(intptr_t context);
    static void HandleFinalize(intptr_t context);
    static void HandleAbort(intptr_t context);
    static void HandleProcessBlock(intptr_t context);

    /**
     * Called to allocate memory for mBlock if necessary and set it to block
     */
    CHIP_ERROR SetBlock(ByteSpan & block);

    /**
     * Called to release allocated memory for mBlock
     */
    CHIP_ERROR ReleaseBlock();

    std::uint32_t mSwVer;
    std::uint32_t mHwVer;

    MutableByteSpan mBlock;
    OTADownloader * mDownloader = nullptr;
    OTAImageHeaderParser mHeaderParser;
    std::unique_ptr<OTAImageProcessor> mProcessor;
};

} // namespace chip
