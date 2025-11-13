/*
 *
 *    Copyright (c) 2022-2025 Project CHIP Authors
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
#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/KeyValueStoreManager.h>

#include <zephyr/dfu/mcuboot.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/storage/stream_flash.h>
#include <zephyr/sys/reboot.h>

static struct stream_flash_ctx stream;

using namespace ::chip::DeviceLayer::PersistedStorage;

namespace chip {
namespace {

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

CHIP_ERROR OTAImageProcessorImpl::PrepareDownload()
{
    VerifyOrReturnError(mDownloader != nullptr, CHIP_ERROR_INCORRECT_STATE);

    return DeviceLayer::SystemLayer().ScheduleLambda([this] { mDownloader->OnPreparedForDownload(PrepareDownloadImpl()); });
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

    CHIP_ERROR error = InitFlashStream(0);

    PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadInProgress);
    return error;
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
    int err = stream_flash_buffered_write(&stream, NULL, 0, true);

    if (err)
    {
        ChipLogError(SoftwareUpdate, "stream_flash_buffered_write failed (err %d)", err);
    }

    PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadComplete);
    return System::MapErrorZephyr(err);
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
    ChipLogError(SoftwareUpdate, "Image upgrade aborted");
    PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadAborted);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
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

CHIP_ERROR OTAImageProcessorImpl::ProcessBlock(ByteSpan & aBlock)
{
    VerifyOrReturnError(mDownloader != nullptr, CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR error = ProcessHeader(aBlock);

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

    // Report the result back to the downloader asynchronously.
    return DeviceLayer::SystemLayer().ScheduleLambda([this, error, aBlock] {
        if (error == CHIP_NO_ERROR)
        {
            ChipLogDetail(SoftwareUpdate, "Downloaded %u/%u bytes", static_cast<unsigned>(mParams.downloadedBytes),
                          static_cast<unsigned>(mParams.totalFileBytes));
            if (downloadedBytesRestored)
            {
                mDownloader->SkipData(downloadedBytesRestored - aBlock.size());
                downloadedBytesRestored = 0;
            }
            else
            {
                mDownloader->FetchNextData();
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
                      static_cast<unsigned>(mParams.totalFileBytes))

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

        // Restore interrupted OTA process
        RestoreBytes(header.mImageDigest);

        mHeaderParser.Clear();
    }

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
