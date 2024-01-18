/*
 *
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

/**
 *    @file
 *          Provides the implementation of the OTA Image Processor class
 *          for Open IOT SDK platform.
 */

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>

#include "OTAImageProcessorImpl.h"

namespace chip {

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

void OTAImageProcessorImpl::OnOTAStateChange(DeviceLayer::OtaState state)
{
    DeviceLayer::ChipDeviceEvent otaChange;
    otaChange.Type                     = DeviceLayer::DeviceEventType::kOtaStateChanged;
    otaChange.OtaStateChanged.newState = state;
    CHIP_ERROR error                   = DeviceLayer::PlatformMgr().PostEvent(&otaChange);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Posting OTA state change failed %" CHIP_ERROR_FORMAT, error.Format());
    }
}

CHIP_ERROR OTAImageProcessorImpl::ProcessBlock(ByteSpan & block)
{
    // Store block data for HandleProcessBlock to access
    CHIP_ERROR err = SetBlock(block);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot set block data: %" CHIP_ERROR_FORMAT, err.Format());
    }

    DeviceLayer::PlatformMgr().ScheduleWork(HandleProcessBlock, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
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

    psa_status_t rc = psa_fwu_start(imageProcessor->mImageId, NULL, 0);
    if (rc != PSA_SUCCESS)
    {
        ChipLogError(SoftwareUpdate, "Begin a firmware update operation for image %d failed [%ld]", imageProcessor->mImageId, rc);
        return;
    }

    imageProcessor->mParams.downloadedBytes = 0;
    imageProcessor->mParams.totalFileBytes  = 0;

    imageProcessor->mHeaderParser.Init();

    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
    imageProcessor->OnOTAStateChange(DeviceLayer::kOtaDownloadInProgress);
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    psa_status_t rc = psa_fwu_finish(imageProcessor->mImageId);
    if (rc != PSA_SUCCESS)
    {
        ChipLogError(SoftwareUpdate, "Mark the image %d as ready for installation failed [%ld]", imageProcessor->mImageId, rc);
        imageProcessor->UpdateShutdown();
        imageProcessor->OnOTAStateChange(DeviceLayer::kOtaDownloadFailed);
        return;
    }

    imageProcessor->ReleaseBlock();
    imageProcessor->OnOTAStateChange(DeviceLayer::kOtaDownloadComplete);
}

void OTAImageProcessorImpl::HandleApply(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    VerifyOrReturn(imageProcessor != nullptr);

    OTARequestorInterface * requestor = chip::GetRequestorInstance();
    VerifyOrReturn(requestor != nullptr);

    imageProcessor->OnOTAStateChange(DeviceLayer::kOtaApplyInProgress);

    // Only check ready for reboot state.
    // The PSA_SUCCESS_RESTART is related to restarting the external components which are not supported here.
    psa_status_t rc = psa_fwu_install();
    if (rc != PSA_SUCCESS_REBOOT)
    {
        ChipLogError(SoftwareUpdate, "Start the installation of the image %d failed [%ld]", imageProcessor->mImageId, rc);
        imageProcessor->UpdateShutdown();
        imageProcessor->OnOTAStateChange(DeviceLayer::kOtaApplyFailed);
        return;
    }

    imageProcessor->OnOTAStateChange(DeviceLayer::kOtaApplyComplete);

    DeviceLayer::PlatformMgr().ScheduleWork([](intptr_t) { DeviceLayer::PlatformMgr().HandleServerShuttingDown(); });
    DeviceLayer::PlatformMgr().ScheduleWork([](intptr_t) {
        DeviceLayer::PlatformMgr().StopEventLoopTask();
        DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Shutdown();
        psa_fwu_request_reboot();
    });
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    imageProcessor->UpdateShutdown();

    imageProcessor->ReleaseBlock();
    imageProcessor->OnOTAStateChange(DeviceLayer::kOtaDownloadAborted);
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

    ByteSpan block   = imageProcessor->mBlock;
    CHIP_ERROR error = imageProcessor->ProcessHeader(block);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Image does not contain a valid header");
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_INVALID_FILE_IDENTIFIER);
        imageProcessor->UpdateShutdown();
        imageProcessor->OnOTAStateChange(DeviceLayer::kOtaDownloadFailed);
        return;
    }

    psa_status_t rc =
        psa_fwu_write(imageProcessor->mImageId, (size_t) imageProcessor->mParams.downloadedBytes, block.data(), block.size());
    if (rc != PSA_SUCCESS)
    {
        ChipLogError(SoftwareUpdate, "Write image %d chunk failed [%ld]", imageProcessor->mImageId, rc);
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
        imageProcessor->UpdateShutdown();
        imageProcessor->OnOTAStateChange(DeviceLayer::kOtaDownloadFailed);
        return;
    }

    imageProcessor->mParams.downloadedBytes += block.size();
    imageProcessor->mDownloader->FetchNextData();
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

    return CHIP_NO_ERROR;
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

void OTAImageProcessorImpl::UpdateShutdown()
{
    psa_fwu_component_info_t info;

    psa_status_t rc = psa_fwu_query(mImageId, &info);

    if (rc == PSA_SUCCESS)
    {
        switch (info.state)
        {
        case PSA_FWU_WRITING:
        case PSA_FWU_CANDIDATE:
            psa_fwu_cancel(mImageId);
            psa_fwu_clean(mImageId);
            break;
        case PSA_FWU_FAILED:
        case PSA_FWU_UPDATED:
            psa_fwu_clean(mImageId);
            break;
        }
    }
}

} // namespace chip
