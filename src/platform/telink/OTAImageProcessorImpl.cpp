/*
 *
 *    Copyright (c) 2022-2026 Project CHIP Authors
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

#include "Reboot.h"

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/KeyValueStoreManager.h>
#include <system/SystemError.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/storage/stream_flash.h>
#include <zephyr/sys/reboot.h>

#ifdef CONFIG_CHIP_OTA_MULTI_IMAGE
#include <algorithm>
#include <cstring>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/crc.h>
#endif

static struct stream_flash_ctx stream;

using namespace ::chip::DeviceLayer::PersistedStorage;

namespace chip {
namespace {

#ifdef CONFIG_CHIP_OTA_MULTI_IMAGE
constexpr uint32_t kMultiImageMagic  = 0x494D4C54; // "TLMI"
constexpr uint32_t kContainerVersion = 1;
constexpr uint32_t kHostImage        = 0;
constexpr uint32_t kExternalImage    = 1;
constexpr uint32_t kMCUbootMagic     = 0x96F3B83D; // MCUboot image header
#endif

void PostOTAStateChangeEvent(DeviceLayer::OtaState newState)
{
    DeviceLayer::ChipDeviceEvent otaChange;
    otaChange.Type                     = DeviceLayer::DeviceEventType::kOtaStateChanged;
    otaChange.OtaStateChanged.newState = newState;
    CHIP_ERROR error                   = DeviceLayer::PlatformMgr().PostEvent(&otaChange);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Error while posting OtaChange event %" CHIP_ERROR_FORMAT, error.Format());
    }
}

} // namespace

namespace DeviceLayer {

#ifdef CONFIG_CHIP_OTA_MULTI_IMAGE
ExternalOTAImageHandler * OTAImageProcessorImpl::sExternalImageHandler = nullptr;

void OTAImageProcessorImpl::SetExternalImageHandler(ExternalOTAImageHandler * handler)
{
    sExternalImageHandler = handler;
}
#endif

CHIP_ERROR OTAImageProcessorImpl::PrepareDownload()
{
    VerifyOrReturnError(mDownloader != nullptr, CHIP_ERROR_INCORRECT_STATE);

    return DeviceLayer::SystemLayer().ScheduleLambda([this] {
        CHIP_ERROR err = mDownloader->OnPreparedForDownload(PrepareDownloadImpl());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "OTA prep cb err: %" CHIP_ERROR_FORMAT, err.Format());
        }
    });
}

const struct device * flash_dev;

CHIP_ERROR OTAImageProcessorImpl::InitFlashStream(size_t offset)
{
    flash_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_flash_controller));
    if (flash_dev == NULL)
    {
        ChipLogError(SoftwareUpdate, "Failed to get flash device");
        return System::MapErrorZephyr(-EFAULT);
    }

    int err = stream_flash_init(&stream, flash_dev, mBuffer, sizeof(mBuffer), FIXED_PARTITION_OFFSET(slot1_partition) + offset,
                                FIXED_PARTITION_SIZE(slot1_partition) - offset, NULL);
    if (err)
    {
        ChipLogError(SoftwareUpdate, "stream_flash_init failed (err %d)", err);
    }

    return System::MapErrorZephyr(err);
}

CHIP_ERROR OTAImageProcessorImpl::PrepareDownloadImpl()
{
    mHeaderParser.Init();
    mParams = {};
#ifdef CONFIG_CHIP_OTA_MULTI_IMAGE
    ResetPayload();
    mExternalImageHandler   = sExternalImageHandler;
    mPayloadFailed          = false;
    downloadedBytesRestored = 0;
    ReturnErrorOnFailure(KeyValueStoreMgr().Put(kDownloadedBytes, &mParams.downloadedBytes, sizeof(mParams.downloadedBytes)));
#endif

    CHIP_ERROR error = InitFlashStream(0);

    PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadInProgress);
    return error;
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
#ifdef CONFIG_CHIP_OTA_MULTI_IMAGE
    if (mHeaderParser.IsInitialized() || mParams.downloadedBytes != mParams.totalFileBytes ||
        (mPayloadState != PayloadState::kSingleImage && (mPayloadState != PayloadState::kImages || mImageIndex != mImageCount)))
    {
        return FailDownload(CHIP_ERROR_INVALID_ARGUMENT);
    }
#endif

    int err = stream_flash_buffered_write(&stream, NULL, 0, true);
    if (err)
    {
        ChipLogError(SoftwareUpdate, "stream_flash_buffered_write failed (err %d)", err);
#ifdef CONFIG_CHIP_OTA_MULTI_IMAGE
        return FailDownload(System::MapErrorZephyr(err));
#endif
    }

#ifdef CONFIG_CHIP_OTA_MULTI_IMAGE
    mDownloadReady = true;
    mImageTable.Free();
#endif

    PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadComplete);
    return System::MapErrorZephyr(err);
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
#ifdef CONFIG_CHIP_OTA_MULTI_IMAGE
    ResetPayload();
#endif
    ChipLogError(SoftwareUpdate, "Image upgrade aborted");
    PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadAborted);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
#ifdef CONFIG_CHIP_OTA_MULTI_IMAGE
    VerifyOrReturnError(mDownloadReady, CHIP_ERROR_INCORRECT_STATE);
    if (mExternalImagesStarted)
    {
        CHIP_ERROR error = mExternalImageHandler->Apply();
        if (error != CHIP_NO_ERROR)
        {
            ResetPayload();
            PostOTAStateChangeEvent(DeviceLayer::kOtaApplyFailed);
            return error;
        }
        mExternalImagesStarted = false;
    }
#endif

    // Schedule update of image
    int err = boot_request_upgrade(BOOT_UPGRADE_PERMANENT);

    PostOTAStateChangeEvent(DeviceLayer::kOtaApplyInProgress);
#ifdef CONFIG_CHIP_OTA_REQUESTOR_REBOOT_ON_APPLY
    if (!err)
    {
        return SystemLayer().StartTimer(
            System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_OTA_REQUESTOR_REBOOT_DELAY_MS),
            [](System::Layer *, void * /* context */) {
                PlatformMgr().HandleServerShuttingDown();
                k_msleep(CHIP_DEVICE_CONFIG_SERVER_SHUTDOWN_ACTIONS_SLEEP_MS);
                Reboot(SoftwareRebootReason::kSoftwareUpdate);
            },
            nullptr /* context */);
    }
    else
    {
        PostOTAStateChangeEvent(DeviceLayer::kOtaApplyFailed);
        return System::MapErrorZephyr(err);
    }
#else
    return System::MapErrorZephyr(err);
#endif
}

void OTAImageProcessorImpl::ResumeWatchdogHandler(System::Layer * systemLayer, void * context)
{
    auto * self = static_cast<chip::DeviceLayer::OTAImageProcessorImpl *>(context);
    ChipLogError(SoftwareUpdate, "Resume not supported by OTA provider");

    LogErrorOnFailure(KeyValueStoreMgr().Delete(kDownloadedBytes));
    LogErrorOnFailure(KeyValueStoreMgr().Delete(kImageDigest));
    self->mDownloader->EndDownload(CHIP_ERROR_NOT_IMPLEMENTED);
}

#ifdef CONFIG_CHIP_OTA_MULTI_IMAGE
void OTAImageProcessorImpl::ResetPayload()
{
    if (mExternalImagesStarted)
    {
        mExternalImageHandler->Abort();
    }
    mExternalImagesStarted = false;
    mDownloadReady         = false;
    mPayloadState          = PayloadState::kHeader;
    mHeaderBytes           = 0;
    mTableBytes            = 0;
    mImageCount            = 0;
    mImageIndex            = 0;
    mImageBytes            = 0;
    mImageCrc              = 0;
    mImageTable.Free();
}

CHIP_ERROR OTAImageProcessorImpl::FailDownload(CHIP_ERROR error)
{
    ChipLogError(SoftwareUpdate, "OTA download failed: %" CHIP_ERROR_FORMAT, error.Format());
    ResetPayload();
    mPayloadFailed = true;
    ReturnErrorOnFailure(DeviceLayer::SystemLayer().ScheduleLambda([this, error] {
        mDownloader->EndDownload(error);
        PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed);
    }));
    return error;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessImageTable()
{
    uint64_t offset = kContainerHeaderSize + static_cast<uint64_t>(mImageCount) * kImageInfoSize;
    for (uint32_t i = 0; i < mImageCount; ++i)
    {
        const uint8_t * entry = mImageTable.Get() + i * kImageInfoSize;
        const uint32_t type   = sys_get_le32(entry);
        const uint32_t size   = sys_get_le32(entry + 4);
        VerifyOrReturnError(type == (i == 0 ? kHostImage : kExternalImage) && size != 0 && sys_get_le32(entry + 8) == offset &&
                                offset + size <= mParams.totalFileBytes,
                            CHIP_ERROR_INVALID_ARGUMENT);
        if (i == 0)
        {
            VerifyOrReturnError(size <= FIXED_PARTITION_SIZE(slot1_partition), CHIP_ERROR_INVALID_ARGUMENT);
        }
        offset += size;
    }
    VerifyOrReturnError(offset == mParams.totalFileBytes, CHIP_ERROR_INVALID_ARGUMENT);
    mPayloadState = PayloadState::kImages;
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessContainerHeader(ByteSpan & block)
{
    const size_t headerSize = mHeaderBytes < sizeof(kMultiImageMagic) ? sizeof(kMultiImageMagic) : kContainerHeaderSize;
    const size_t count      = std::min(block.size(), headerSize - mHeaderBytes);
    memcpy(mContainerHeader + mHeaderBytes, block.data(), count);
    mHeaderBytes += count;
    mParams.downloadedBytes += count;
    block = block.SubSpan(count);
    VerifyOrReturnError(mHeaderBytes == headerSize, CHIP_NO_ERROR);

    if (mHeaderBytes == sizeof(kMultiImageMagic))
    {
        const uint32_t magic = sys_get_le32(mContainerHeader);
        if (magic == kMCUbootMagic)
        {
            VerifyOrReturnError(mParams.totalFileBytes <= FIXED_PARTITION_SIZE(slot1_partition), CHIP_ERROR_INVALID_ARGUMENT);
            ReturnErrorOnFailure(
                System::MapErrorZephyr(stream_flash_buffered_write(&stream, mContainerHeader, mHeaderBytes, false)));
            mPayloadState = PayloadState::kSingleImage;
        }
        else
        {
            VerifyOrReturnError(magic == kMultiImageMagic, CHIP_ERROR_INVALID_FILE_IDENTIFIER);
            if (mExternalImageHandler == nullptr)
            {
                ChipLogError(SoftwareUpdate, "External OTA image handler is not registered");
                return CHIP_ERROR_INCORRECT_STATE;
            }
        }
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(sys_get_le32(mContainerHeader + 4) == kContainerVersion, CHIP_ERROR_INVALID_ARGUMENT);
    mImageCount = sys_get_le32(mContainerHeader + 8);
    VerifyOrReturnError(mImageCount >= 2 && mParams.totalFileBytes >= kContainerHeaderSize &&
                            mParams.totalFileBytes <= UINT32_MAX &&
                            mImageCount <= (mParams.totalFileBytes - kContainerHeaderSize) / (kImageInfoSize + 1),
                        CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mImageTable.Alloc(static_cast<size_t>(mImageCount) * kImageInfoSize), CHIP_ERROR_NO_MEMORY);
    mPayloadState = PayloadState::kTable;
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessImageData(ByteSpan & block)
{
    VerifyOrReturnError(mImageIndex < mImageCount, CHIP_ERROR_INVALID_ARGUMENT);
    const uint8_t * entry = mImageTable.Get() + mImageIndex * kImageInfoSize;
    const uint32_t size   = sys_get_le32(entry + 4);
    const size_t count    = std::min(block.size(), static_cast<size_t>(size - mImageBytes));

    if (mImageIndex == 0)
    {
        ReturnErrorOnFailure(System::MapErrorZephyr(stream_flash_buffered_write(&stream, block.data(), count, false)));
    }
    else
    {
        if (mImageBytes == 0)
        {
            mExternalImagesStarted = true;
            ReturnErrorOnFailure(mExternalImageHandler->Begin(mImageIndex, size));
        }
        ReturnErrorOnFailure(mExternalImageHandler->Write(block.SubSpan(0, count)));
    }

    mImageCrc = crc32_ieee_update(mImageCrc, block.data(), count);
    mImageBytes += static_cast<uint32_t>(count);
    mParams.downloadedBytes += count;
    block = block.SubSpan(count);
    if (mImageBytes == size)
    {
        VerifyOrReturnError(mImageCrc == sys_get_le32(entry + 12), CHIP_ERROR_INVALID_ARGUMENT);
        if (mImageIndex != 0)
        {
            ReturnErrorOnFailure(mExternalImageHandler->End());
        }
        ++mImageIndex;
        mImageBytes = 0;
        mImageCrc   = 0;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessPayload(ByteSpan block)
{
    VerifyOrReturnError(mParams.downloadedBytes <= mParams.totalFileBytes &&
                            block.size() <= mParams.totalFileBytes - mParams.downloadedBytes,
                        CHIP_ERROR_INVALID_ARGUMENT);

    while (!block.empty())
    {
        switch (mPayloadState)
        {
        case PayloadState::kHeader:
            ReturnErrorOnFailure(ProcessContainerHeader(block));
            break;
        case PayloadState::kTable: {
            const size_t tableSize = static_cast<size_t>(mImageCount) * kImageInfoSize;
            const size_t count     = std::min(block.size(), tableSize - mTableBytes);
            memcpy(mImageTable.Get() + mTableBytes, block.data(), count);
            mTableBytes += count;
            mParams.downloadedBytes += count;
            block = block.SubSpan(count);
            if (mTableBytes == tableSize)
            {
                ReturnErrorOnFailure(ProcessImageTable());
            }
            break;
        }
        case PayloadState::kImages:
            ReturnErrorOnFailure(ProcessImageData(block));
            break;
        case PayloadState::kSingleImage:
            ReturnErrorOnFailure(System::MapErrorZephyr(stream_flash_buffered_write(&stream, block.data(), block.size(), false)));
            mParams.downloadedBytes += block.size();
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_NO_ERROR;
}
#endif

CHIP_ERROR OTAImageProcessorImpl::ProcessBlock(ByteSpan & aBlock)
{
    VerifyOrReturnError(mDownloader != nullptr, CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR error = ProcessHeader(aBlock);
#ifdef CONFIG_CHIP_OTA_MULTI_IMAGE
    if (error == CHIP_NO_ERROR && !mHeaderParser.IsInitialized())
    {
        error = ProcessPayload(aBlock);
    }
    if (error != CHIP_NO_ERROR)
    {
        return FailDownload(error);
    }
#else
    if (error == CHIP_NO_ERROR)
    {
        if (downloadedBytesRestored)
        {
            mParams.downloadedBytes = downloadedBytesRestored;
        }
        else
        {
            error = System::MapErrorZephyr(stream_flash_buffered_write(&stream, aBlock.data(), aBlock.size(), false));
            mParams.downloadedBytes += aBlock.size();
        }
        ReturnErrorOnFailure(KeyValueStoreMgr().Put(kDownloadedBytes, &mParams.downloadedBytes, sizeof(mParams.downloadedBytes)));
    }
#endif

    // Report the result back to the downloader asynchronously.
    return DeviceLayer::SystemLayer().ScheduleLambda([this, error, aBlock] {
#ifdef CONFIG_CHIP_OTA_MULTI_IMAGE
        if (mPayloadFailed)
        {
            return;
        }
#endif
        if (error == CHIP_NO_ERROR)
        {
            ChipLogDetail(SoftwareUpdate, "Downloaded %u/%u bytes", static_cast<unsigned>(mParams.downloadedBytes),
                          static_cast<unsigned>(mParams.totalFileBytes));
            if (downloadedBytesRestored)
            {
                CHIP_ERROR err = mDownloader->SkipData(downloadedBytesRestored - aBlock.size());
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(SoftwareUpdate, "OTA skip err: %" CHIP_ERROR_FORMAT, err.Format());
                }
                downloadedBytesRestored = 0;
                SystemLayer().StartTimer(System::Clock::Milliseconds32(kResumeWatchdogTimeoutMs), ResumeWatchdogHandler, this);
            }
            else
            {
                DeviceLayer::SystemLayer().CancelTimer(ResumeWatchdogHandler, this);
                if (mDownloader->GetState() == OTADownloader::State::kInProgress)
                {
                    CHIP_ERROR err = mDownloader->FetchNextData();
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(SoftwareUpdate, "OTA fetch err: %" CHIP_ERROR_FORMAT, err.Format());
                    }
                }
            }
        }
        else
        {
            mDownloader->EndDownload(error);
            PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed);
        }
    });
}

bool OTAImageProcessorImpl::IsFirstImageRun()
{
    OTARequestorInterface * requestor = GetRequestorInstance();
    VerifyOrReturnError(requestor != nullptr, false);

    uint32_t currentVersion;
    VerifyOrReturnError(ConfigurationMgr().GetSoftwareVersion(currentVersion) == CHIP_NO_ERROR, false);

    return requestor->GetCurrentUpdateState() == OTARequestorInterface::OTAUpdateStateEnum::kApplying &&
        requestor->GetTargetVersion() == currentVersion;
}

CHIP_ERROR OTAImageProcessorImpl::ConfirmCurrentImage()
{
    PostOTAStateChangeEvent(DeviceLayer::kOtaApplyComplete);
    return System::MapErrorZephyr(boot_write_img_confirmed());
}

CHIP_ERROR OTAImageProcessorImpl::RestoreBytes(ByteSpan & aBlock)
{
    uint8_t * ImageDigestBuffer = new uint8_t[aBlock.size()];
    MutableByteSpan mImageDigest(ImageDigestBuffer, aBlock.size());

    if (KeyValueStoreMgr().Get(kImageDigest, mImageDigest.data(), mImageDigest.size()) == CHIP_NO_ERROR &&
        KeyValueStoreMgr().Get(kDownloadedBytes, &downloadedBytesRestored, sizeof(downloadedBytesRestored)) == CHIP_NO_ERROR &&
        mImageDigest.data_equal(aBlock) && downloadedBytesRestored < mParams.totalFileBytes)
    {
        // Align to the nearest lower multiple of sector size (4 KB) for Flash erase/write
        downloadedBytesRestored = ROUND_DOWN(downloadedBytesRestored, 0x1000);
        ChipLogDetail(SoftwareUpdate, "Restored %u/%u bytes", static_cast<unsigned>(downloadedBytesRestored),
                      static_cast<unsigned>(mParams.totalFileBytes));

        // Reinit Flash Stream with offset
        ReturnErrorOnFailure(System::MapErrorZephyr(stream_flash_buffered_write(&stream, NULL, 0, true)));
        ReturnErrorOnFailure(InitFlashStream(downloadedBytesRestored));
    }
    else
    {
        downloadedBytesRestored = 0;
        ReturnErrorOnFailure(KeyValueStoreMgr().Put(kImageDigest, aBlock.data(), aBlock.size()));
    }
    delete[] ImageDigestBuffer;

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessHeader(ByteSpan & aBlock)
{
    if (mHeaderParser.IsInitialized())
    {
        OTAImageHeader header;
        CHIP_ERROR error = mHeaderParser.AccumulateAndDecode(aBlock, header);

        // Needs more data to decode the header
        VerifyOrReturnError(error != CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
        ReturnErrorOnFailure(error);

        uint16_t vendorId  = 0;
        uint16_t productId = 0;

        if (GetDeviceInstanceInfoProvider()->GetVendorId(vendorId) != CHIP_NO_ERROR)
        {
            ChipLogDetail(DeviceLayer, "Failed to retrieve local Vendor ID for OTA validation");
            return CHIP_ERROR_INCORRECT_STATE;
        }
        if (GetDeviceInstanceInfoProvider()->GetProductId(productId) != CHIP_NO_ERROR)
        {
            ChipLogDetail(DeviceLayer, "Failed to retrieve local Product ID for OTA validation");
            return CHIP_ERROR_INCORRECT_STATE;
        }
        if (header.mVendorId != vendorId || header.mProductId != productId)
        {
            ChipLogDetail(DeviceLayer, "The argument is invalid, mVendorId: 0x%x - \
                          mProductId: 0x%x \t vendorId : 0x%x - productId : 0x%x",
                          header.mVendorId, header.mProductId, vendorId, productId);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        mParams.totalFileBytes = header.mPayloadSize;

#ifndef CONFIG_CHIP_OTA_MULTI_IMAGE
        // Restore interrupted OTA process
        error = RestoreBytes(header.mImageDigest);
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "OTA restore err: %" CHIP_ERROR_FORMAT, error.Format());
        }
#endif

        mHeaderParser.Clear();
    }

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
