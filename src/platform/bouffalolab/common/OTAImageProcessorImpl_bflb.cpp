/*
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

extern "C" {
#include <bflb_ota.h>
#include <bl_sys.h>
}

using namespace chip::System;

namespace chip {

#define OTA_IMAGE_TYPE_XZ "XZ"
#define OTA_IMAGE_TYPE_RAW "RAW"

static bool check_ota_header(ota_header_s_t * ota_header_s)
{
    char str[sizeof(ota_header_s->header) + 1];

    memcpy(str, ota_header_s->header, sizeof(ota_header_s->header));
    str[sizeof(ota_header_s->header)] = '\0';
    ChipLogProgress(SoftwareUpdate, "Bouffalo Lab OTA header: %s", str);

    if (0 == memcmp(OTA_IMAGE_TYPE_XZ, ota_header_s->type, strlen(OTA_IMAGE_TYPE_XZ)))
    {
        ChipLogProgress(SoftwareUpdate, "Bouffalo Lab OTA image type: %s", OTA_IMAGE_TYPE_XZ);
    }
    else if (0 == memcmp(OTA_IMAGE_TYPE_RAW, ota_header_s->type, strlen(OTA_IMAGE_TYPE_RAW)))
    {
        ChipLogProgress(SoftwareUpdate, "Bouffalo Lab OTA image type: %s", OTA_IMAGE_TYPE_RAW);
    }
    else
    {
        return false;
    }

    ChipLogProgress(SoftwareUpdate, "Bouffalo Lab OTA image file size: %ld", ota_header_s->image_len);

    memcpy(str, ota_header_s->ver_hardware, sizeof(ota_header_s->ver_hardware));
    str[sizeof(ota_header_s->ver_hardware)] = '\0';
    ChipLogProgress(SoftwareUpdate, "OTA image hardware version: %s", str);

    memcpy(str, ota_header_s->ver_software, sizeof(ota_header_s->ver_software));
    str[sizeof(ota_header_s->ver_software)] = '\0';
    ChipLogProgress(SoftwareUpdate, "OTA image software version: %s", str);

    return true;
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

    imageProcessor->mParams.downloadedBytes = 0;
    imageProcessor->mParams.totalFileBytes  = 0;
    imageProcessor->mHeaderParser.Init();

    memset(&(imageProcessor->mOtaHdr), 0, sizeof(ota_header_s_t));
    imageProcessor->mImageTotalSize = 0;

    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);

    if (imageProcessor == nullptr)
    {
        return;
    }

    ByteSpan block = imageProcessor->mBlock;

    if (bflb_ota_update(imageProcessor->mImageTotalSize, imageProcessor->mParams.downloadedBytes - sizeof(ota_header_t),
                        imageProcessor->mOtaHdr.sha256, sizeof(imageProcessor->mOtaHdr.sha256)) < 0)
    {
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
        return;
    }

    if (bflb_ota_check() < 0)
    {
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
        ChipLogProgress(SoftwareUpdate, "OTA image verification error");
    }
    else
    {
        ChipLogProgress(SoftwareUpdate, "OTA image downloaded");
    }

    imageProcessor->ReleaseBlock();
}

void OTAImageProcessorImpl::HandleApply(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);

    if (imageProcessor == nullptr)
    {
        return;
    }

    bflb_ota_apply();

    DeviceLayer::SystemLayer().StartTimer(
        System::Clock::Seconds32(OTA_AUTO_REBOOT_DELAY),
        [](Layer *, void *) {
            ChipLogProgress(SoftwareUpdate, "Rebooting...");
            bl_sys_reset_por();
        },
        nullptr);
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    bflb_ota_abort();

    imageProcessor->ReleaseBlock();
}

void OTAImageProcessorImpl::HandleProcessBlock(intptr_t context)
{
    OTAImageHeader header;
    CHIP_ERROR error;
    uint32_t iOffset, iSize;

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

    ByteSpan block = imageProcessor->mBlock;
    if (imageProcessor->mHeaderParser.IsInitialized())
    {
        error = imageProcessor->mHeaderParser.AccumulateAndDecode(block, header);
        if (CHIP_ERROR_BUFFER_TOO_SMALL == error)
        {
            return;
        }
        else if (CHIP_NO_ERROR != error)
        {
            ChipLogError(SoftwareUpdate, "Matter image header parser error %s", chip::ErrorStr(error));
            imageProcessor->mDownloader->EndDownload(CHIP_ERROR_INVALID_FILE_IDENTIFIER);
            imageProcessor->mHeaderParser.Clear();
            return;
        }

        ChipLogProgress(SoftwareUpdate, "Image Header software version: %ld payload size: %lu", header.mSoftwareVersion,
                        (long unsigned int) header.mPayloadSize);
        imageProcessor->mParams.totalFileBytes = header.mPayloadSize;
        imageProcessor->mHeaderParser.Clear();
    }

    if (imageProcessor->mParams.totalFileBytes)
    {
        if (0 == imageProcessor->mImageTotalSize)
        {

            iSize = sizeof(ota_header_s_t) - imageProcessor->mParams.downloadedBytes;
            if (block.size() < iSize)
            {
                iSize = block.size();
            }

            memcpy(reinterpret_cast<uint8_t *>(&imageProcessor->mOtaHdr) + imageProcessor->mParams.downloadedBytes, block.data(),
                   iSize);

            if (imageProcessor->mParams.downloadedBytes + iSize >= sizeof(ota_header_s_t))
            {
                if (!check_ota_header(&imageProcessor->mOtaHdr))
                {
                    imageProcessor->mDownloader->EndDownload(CHIP_ERROR_DECODE_FAILED);
                    return;
                }

                if (bflb_ota_start(imageProcessor->mOtaHdr.image_len + sizeof(imageProcessor->mOtaHdr.sha256)) < 0)
                {
                    imageProcessor->mDownloader->EndDownload(CHIP_ERROR_OPEN_FAILED);
                    return;
                }

                imageProcessor->mImageTotalSize = imageProcessor->mOtaHdr.image_len + sizeof(imageProcessor->mOtaHdr.sha256);
            }
        }

        if (imageProcessor->mImageTotalSize && imageProcessor->mParams.downloadedBytes + block.size() > sizeof(ota_header_t))
        {

            if (imageProcessor->mParams.downloadedBytes >= sizeof(ota_header_t))
            {
                iOffset = imageProcessor->mParams.downloadedBytes - sizeof(ota_header_t);
                iSize   = 0;
            }
            else
            {
                iOffset = 0;
                iSize   = sizeof(ota_header_t) - imageProcessor->mParams.downloadedBytes;
            }

            if (bflb_ota_update(imageProcessor->mImageTotalSize, iOffset, const_cast<uint8_t *>(block.data() + iSize),
                                block.size() - iSize) < 0)
            {
                imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                return;
            }
        }

        imageProcessor->mParams.downloadedBytes += block.size();
    }

    imageProcessor->mDownloader->FetchNextData();
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

} // namespace chip
