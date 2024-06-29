/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <platform/CHIPDeviceLayer.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/storage/stream_flash.h>
#include <zephyr/sys/reboot.h>

static struct stream_flash_ctx stream;

#ifdef CONFIG_CHIP_OTA_REQUEST_UPGRADE_PERMANENT
#define UPDATE_TYPE BOOT_UPGRADE_PERMANENT
#else
#define UPDATE_TYPE BOOT_UPGRADE_TEST
#endif

static constexpr uint16_t deltaRebootDelayMs = 200;

namespace chip {
namespace DeviceLayer {

CHIP_ERROR OTAImageProcessorImpl::PrepareDownload()
{
    VerifyOrReturnError(mDownloader != nullptr, CHIP_ERROR_INCORRECT_STATE);

    return DeviceLayer::SystemLayer().ScheduleLambda([this] { mDownloader->OnPreparedForDownload(PrepareDownloadImpl()); });
}

CHIP_ERROR OTAImageProcessorImpl::PrepareDownloadImpl()
{
    mHeaderParser.Init();
    mParams = {};

    const struct device * flash_dev;

    flash_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_flash_controller));
    if (flash_dev == NULL)
    {
        ChipLogError(SoftwareUpdate, "Failed to get flash device");
        return System::MapErrorZephyr(-EFAULT);
    }

    int err = stream_flash_init(&stream, flash_dev, mBuffer, sizeof(mBuffer), FIXED_PARTITION_OFFSET(slot1_partition),
                                FIXED_PARTITION_SIZE(slot1_partition), NULL);

    if (err)
    {
        ChipLogError(SoftwareUpdate, "stream_flash_init failed (err %d)", err);
    }

    return System::MapErrorZephyr(err);
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
    int err = stream_flash_buffered_write(&stream, NULL, 0, true);

    if (err)
    {
        ChipLogError(SoftwareUpdate, "stream_flash_buffered_write failed (err %d)", err);
    }

    return System::MapErrorZephyr(err);
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
    ChipLogError(SoftwareUpdate, "Image upgrade aborted");

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
    // Schedule update of image
    int err = boot_request_upgrade(UPDATE_TYPE);

#ifdef CONFIG_CHIP_OTA_REQUESTOR_REBOOT_ON_APPLY
    if (!err)
    {
        PlatformMgr().HandleServerShuttingDown();
        /*
         * Restart the device in order to apply the update image.
         * This should be done with a delay so the device has enough time to send
         * the state-transition event when applying the update.
         */
        ChipLogProgress(SoftwareUpdate, "Restarting device, will reboot in %d seconds ...", mDelayBeforeRebootSec);
        return SystemLayer().StartTimer(
            System::Clock::Milliseconds32(mDelayBeforeRebootSec * 1000 + deltaRebootDelayMs),
            [](System::Layer *, void * /* context */) {
                k_msleep(CHIP_DEVICE_CONFIG_SERVER_SHUTDOWN_ACTIONS_SLEEP_MS);
                sys_reboot(SYS_REBOOT_WARM);
            },
            nullptr /* context */);
    }
    else
    {
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
        error = System::MapErrorZephyr(stream_flash_buffered_write(&stream, aBlock.data(), aBlock.size(), false));
        mParams.downloadedBytes += aBlock.size();
    }

    // Report the result back to the downloader asynchronously.
    return DeviceLayer::SystemLayer().ScheduleLambda([this, error, aBlock] {
        if (error == CHIP_NO_ERROR)
        {
            ChipLogDetail(SoftwareUpdate, "Downloaded %u/%u bytes", static_cast<unsigned>(mParams.downloadedBytes),
                          static_cast<unsigned>(mParams.totalFileBytes));
            mDownloader->FetchNextData();
        }
        else
        {
            mDownloader->EndDownload(error);
        }
    });
}

bool OTAImageProcessorImpl::IsFirstImageRun()
{
    OTARequestorInterface * requestor = GetRequestorInstance();
    ReturnErrorCodeIf(requestor == nullptr, false);

    uint32_t currentVersion;
    ReturnErrorCodeIf(ConfigurationMgr().GetSoftwareVersion(currentVersion) != CHIP_NO_ERROR, false);

    return requestor->GetCurrentUpdateState() == OTARequestorInterface::OTAUpdateStateEnum::kApplying &&
        requestor->GetTargetVersion() == currentVersion;
}

CHIP_ERROR OTAImageProcessorImpl::ConfirmCurrentImage()
{
    return System::MapErrorZephyr(boot_write_img_confirmed());
}

CHIP_ERROR OTAImageProcessorImpl::ProcessHeader(ByteSpan & aBlock)
{
    if (mHeaderParser.IsInitialized())
    {
        OTAImageHeader header;
        CHIP_ERROR error = mHeaderParser.AccumulateAndDecode(aBlock, header);

        // Needs more data to decode the header
        ReturnErrorCodeIf(error == CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
        ReturnErrorOnFailure(error);

        mParams.totalFileBytes = header.mPayloadSize;
        mHeaderParser.Clear();
    }

    return CHIP_NO_ERROR;
}

void OTAImageProcessorImpl::SetRebootDelaySec(uint16_t rebootDelay)
{
    mDelayBeforeRebootSec = rebootDelay;
}
} // namespace DeviceLayer
} // namespace chip
