/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemError.h>

#include <dfu/dfu_target.h>
#include <dfu/dfu_target_mcuboot.h>
#include <dfu/mcuboot.h>
#include <logging/log.h>
#include <pm/device.h>
#include <sys/reboot.h>

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
    mContentHeaderParser.Init();
    ReturnErrorOnFailure(System::MapErrorZephyr(dfu_target_mcuboot_set_buf(mBuffer, sizeof(mBuffer))));
    ReturnErrorOnFailure(System::MapErrorZephyr(dfu_target_reset()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
    return System::MapErrorZephyr(dfu_target_reset());
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
    int err = dfu_target_done(true);
    if (err == 0)
    {
        // schedule update of all possible targets by caling this function with argument -1
        err = dfu_target_schedule_update(-1);
    }

#ifdef CONFIG_CHIP_OTA_REQUESTOR_REBOOT_ON_APPLY
    if (err == 0)
    {
        return SystemLayer().StartTimer(
            System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_OTA_REQUESTOR_REBOOT_DELAY_MS),
            [](System::Layer *, void * /* context */) {
                PlatformMgr().HandleServerShuttingDown();
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

CHIP_ERROR OTAImageProcessorImpl::ProcessBlock(ByteSpan & block)
{
    VerifyOrReturnError(mDownloader != nullptr, CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR error = ProcessHeader(block);
    if (error == CHIP_NO_ERROR)
    {
        mCurrentImage.mCurrentOffset += block.size();
        if (mCurrentImage.mCurrentOffset >= mCurrentImage.mFileInfo->mFileSize)
        {
            // calculate how many data should be moved to the next image
            uint64_t remainingDataSize = mCurrentImage.mCurrentOffset - static_cast<uint64_t>(mCurrentImage.mFileInfo->mFileSize);
            // write last data of previous image
            error = System::MapErrorZephyr(dfu_target_write(block.data(), block.size() - remainingDataSize));
            // switch to net image
            mCurrentImage.mIndex++;
            mCurrentImage.mFileInfo = &mContentHeader.mFiles[mCurrentImage.mIndex];

            if (OTAImageContentHeader::FileId::kNetMcuboot == mCurrentImage.mFileInfo->mFileId &&
                mCurrentImage.mFileInfo->mFileSize > 0 && CHIP_NO_ERROR == error)
            {
                // finish previous image and reset target
                dfu_target_done(true);
                dfu_target_reset();
                // initialize next dfu target to store net-core image.
                dfu_target_init(DFU_TARGET_IMAGE_TYPE_MCUBOOT, mCurrentImage.mIndex, /* size */ 0, nullptr);
                // write remaining data to new image
                error =
                    System::MapErrorZephyr(dfu_target_write(block.data() + (block.size() - remainingDataSize), remainingDataSize));
                mCurrentImage.mCurrentOffset = remainingDataSize;
            }
            else
            {
                // Finish process with error to ensure that only two images are available.
                error = CHIP_ERROR_INVALID_DATA_LIST;
            }
        }
        else
        {
            // DFU target library buffers data internally, so do not clone the block data.
            error = System::MapErrorZephyr(dfu_target_write(block.data(), block.size()));
        }
        ChipLogDetail(SoftwareUpdate, "Processed %llu/%u Bytes of image no. %u", mCurrentImage.mCurrentOffset,
                      mCurrentImage.mFileInfo->mFileSize, mCurrentImage.mIndex);
    }

    // Report the result back to the downloader asynchronously.
    return DeviceLayer::SystemLayer().ScheduleLambda([this, error, block] {
        if (error == CHIP_NO_ERROR)
        {
            mParams.downloadedBytes += block.size();
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
    return mcuboot_swap_type() == BOOT_SWAP_TYPE_REVERT;
}

CHIP_ERROR OTAImageProcessorImpl::ConfirmCurrentImage()
{
    return System::MapErrorZephyr(boot_write_img_confirmed());
}

CHIP_ERROR OTAImageProcessorImpl::ProcessHeader(ByteSpan & block)
{
    if (mHeaderParser.IsInitialized())
    {
        OTAImageHeader header;
        CHIP_ERROR error = mHeaderParser.AccumulateAndDecode(block, header);

        // Needs more data to decode the header
        ReturnErrorCodeIf(error == CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
        ReturnErrorOnFailure(error);

        mParams.totalFileBytes = header.mPayloadSize;
        mHeaderParser.Clear();
    }

    if (mContentHeaderParser.IsInitialized() && !block.empty())
    {
        CHIP_ERROR error = mContentHeaderParser.AccumulateAndDecode(block, mContentHeader);

        // Needs more data to decode the header
        ReturnErrorCodeIf(error == CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
        ReturnErrorOnFailure(error);

        if (OTAImageContentHeader::FileId::kAppMcuboot == mContentHeader.mFiles[0].mFileId)
        {
            mCurrentImage.mIndex    = 0;
            mCurrentImage.mFileInfo = &mContentHeader.mFiles[mCurrentImage.mIndex];
            // Initialize dfu target to receive first image
            error =
                System::MapErrorZephyr(dfu_target_init(DFU_TARGET_IMAGE_TYPE_MCUBOOT, mCurrentImage.mIndex, /* size */ 0, nullptr));
            ReturnErrorOnFailure(error);
        }

        mContentHeaderParser.Clear();
    }

    return CHIP_NO_ERROR;
}

// external flash power consumption optimization
void ExtFlashHandler::DoAction(Action action)
{
#if CONFIG_PM_DEVICE && CONFIG_NORDIC_QSPI_NOR && !CONFIG_SOC_NRF52840 // nRF52 is optimized per default
    // utilize the QSPI driver sleep power mode
    const auto * qspi_dev = device_get_binding(DT_LABEL(DT_INST(0, nordic_qspi_nor)));
    if (qspi_dev)
    {
        const auto requestedAction = Action::WAKE_UP == action ? PM_DEVICE_ACTION_RESUME : PM_DEVICE_ACTION_SUSPEND;
        (void) pm_device_action_run(qspi_dev, requestedAction); // not much can be done in case of a failure
    }
#endif
}

OTAImageProcessorImplPMDevice::OTAImageProcessorImplPMDevice(ExtFlashHandler & aHandler) : mHandler(aHandler)
{
    mHandler.DoAction(ExtFlashHandler::Action::SLEEP);
}

CHIP_ERROR OTAImageProcessorImplPMDevice::PrepareDownload()
{
    mHandler.DoAction(ExtFlashHandler::Action::WAKE_UP);
    return OTAImageProcessorImpl::PrepareDownload();
}

CHIP_ERROR OTAImageProcessorImplPMDevice::Abort()
{
    auto status = OTAImageProcessorImpl::Abort();
    mHandler.DoAction(ExtFlashHandler::Action::SLEEP);
    return status;
}

CHIP_ERROR OTAImageProcessorImplPMDevice::Apply()
{
    auto status = OTAImageProcessorImpl::Apply();
    mHandler.DoAction(ExtFlashHandler::Action::SLEEP);
    return status;
}

} // namespace DeviceLayer
} // namespace chip
