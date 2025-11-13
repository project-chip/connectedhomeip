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

#include "Reboot.h"

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemError.h>

#ifdef CONFIG_CHIP_CERTIFICATION_DECLARATION_STORAGE
#include <credentials/CertificationDeclaration.h>
#include <platform/Zephyr/ZephyrConfig.h>
#include <zephyr/settings/settings.h>
#endif

#include "DFUSync.h"

#include <dfu/dfu_multi_image.h>
#include <dfu/dfu_target.h>
#include <dfu/dfu_target_mcuboot.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>

namespace chip {
namespace {
#ifdef CONFIG_CHIP_CERTIFICATION_DECLARATION_STORAGE
// Cd globals are needed to be accessed from dfu image writer lambdas
uint8_t sCdBuf[chip::Credentials::kMaxCMSSignedCDMessage] = { 0 };
size_t sCdSavedBytes                                      = 0;
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

CHIP_ERROR OTAImageProcessorImpl::PrepareDownload()
{
    VerifyOrReturnError(mDownloader != nullptr, CHIP_ERROR_INCORRECT_STATE);

    if (DFUSync::GetInstance().Take(mDfuSyncMutexId) != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot start Matter OTA, another DFU in progress.");
        return CHIP_ERROR_BUSY;
    }

    TriggerFlashAction(ExternalFlashManager::Action::WAKE_UP);

    return DeviceLayer::SystemLayer().ScheduleLambda([this] {
        CHIP_ERROR err = PrepareDownloadImpl();
        if (err != CHIP_NO_ERROR)
        {
            DFUSync::GetInstance().Free(mDfuSyncMutexId);
        }
        mDownloader->OnPreparedForDownload(err);
    });
}

CHIP_ERROR OTAImageProcessorImpl::PrepareDownloadImpl()
{
    mHeaderParser.Init();
    mParams = {};
    ReturnErrorOnFailure(System::MapErrorZephyr(dfu_target_mcuboot_set_buf(mBuffer, sizeof(mBuffer))));
    ReturnErrorOnFailure(System::MapErrorZephyr(dfu_multi_image_init(mBuffer, sizeof(mBuffer))));

    for (int image_id = 0; image_id < CONFIG_UPDATEABLE_IMAGE_NUMBER; ++image_id)
    {
        dfu_image_writer writer;
        writer.image_id = image_id;
        writer.open     = [](int id, size_t size) { return dfu_target_init(DFU_TARGET_IMAGE_TYPE_MCUBOOT, id, size, nullptr); };
        writer.write    = [](const uint8_t * chunk, size_t chunk_size) { return dfu_target_write(chunk, chunk_size); };
        writer.close    = [](bool success) { return success ? dfu_target_done(success) : dfu_target_reset(); };

        ReturnErrorOnFailure(System::MapErrorZephyr(dfu_multi_image_register_writer(&writer)));
    };

#ifdef CONFIG_CHIP_CERTIFICATION_DECLARATION_STORAGE
    dfu_image_writer cdWriter;
    cdWriter.image_id = CONFIG_CHIP_CERTIFiCATION_DECLARATION_OTA_IMAGE_ID;
    cdWriter.open     = [](int id, size_t size) { return size <= sizeof(sCdBuf) ? 0 : -EFBIG; };
    cdWriter.write    = [](const uint8_t * chunk, size_t chunk_size) {
        memcpy(&sCdBuf[sCdSavedBytes], chunk, chunk_size);
        sCdSavedBytes += chunk_size;
        return 0;
    };
    cdWriter.close = [](bool success) {
        return settings_save_one(Internal::ZephyrConfig::kConfigKey_CertificationDeclaration, sCdBuf, sCdSavedBytes);
    };

    ReturnErrorOnFailure(System::MapErrorZephyr(dfu_multi_image_register_writer(&cdWriter)));
#endif

    PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadInProgress);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
    PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadComplete);
    DFUSync::GetInstance().Free(mDfuSyncMutexId);
    return System::MapErrorZephyr(dfu_multi_image_done(true));
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
    CHIP_ERROR error = System::MapErrorZephyr(dfu_multi_image_done(false));

    DFUSync::GetInstance().Free(mDfuSyncMutexId);
    TriggerFlashAction(ExternalFlashManager::Action::SLEEP);
    PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadAborted);

    return error;
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
    PostOTAStateChangeEvent(DeviceLayer::kOtaApplyInProgress);
    // Schedule update of all images
    int err = dfu_target_schedule_update(-1);

    TriggerFlashAction(ExternalFlashManager::Action::SLEEP);

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
        // DFU target library buffers data internally, so do not clone the block data.
        if (mParams.downloadedBytes > std::numeric_limits<size_t>::max())
        {
            error = CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        else
        {
            error = System::MapErrorZephyr(
                dfu_multi_image_write(static_cast<size_t>(mParams.downloadedBytes), aBlock.data(), aBlock.size()));
            mParams.downloadedBytes += aBlock.size();
        }
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
    return mImageConfirmed ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE;
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

        mParams.totalFileBytes = header.mPayloadSize;
        mHeaderParser.Clear();
    }

    return CHIP_NO_ERROR;
}

void OTAImageProcessorImpl::TriggerFlashAction(ExternalFlashManager::Action action)
{
    if (mFlashHandler)
    {
        mFlashHandler->DoAction(action);
    }
}

} // namespace DeviceLayer
} // namespace chip
