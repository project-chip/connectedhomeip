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

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <platform/CHIPDeviceEvent.h>
#include <platform/ESP32/ESP32Utils.h>

#include "OTAImageProcessorImpl.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "lib/core/CHIPError.h"

#define TAG "OTAImageProcessor"
using namespace chip::System;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace {

void HandleRestart(Layer * systemLayer, void * appState)
{
    esp_restart();
}

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

bool OTAImageProcessorImpl::IsFirstImageRun()
{
    OTARequestorInterface * requestor = GetRequestorInstance();
    if (requestor == nullptr)
    {
        return false;
    }

    return requestor->GetCurrentUpdateState() == OTARequestorInterface::OTAUpdateStateEnum::kApplying;
}

CHIP_ERROR OTAImageProcessorImpl::ConfirmCurrentImage()
{
    OTARequestorInterface * requestor = GetRequestorInstance();
    if (requestor == nullptr)
    {
        return CHIP_ERROR_INTERNAL;
    }

    uint32_t currentVersion;
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSoftwareVersion(currentVersion));
    if (currentVersion != requestor->GetTargetVersion())
    {
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
    CHIP_ERROR err = SetBlock(block);
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
    imageProcessor->mOTAUpdatePartition = esp_ota_get_next_update_partition(NULL);
    if (imageProcessor->mOTAUpdatePartition == NULL)
    {
        ChipLogError(SoftwareUpdate, "OTA partition not found");
        return;
    }
    esp_err_t err =
        esp_ota_begin(imageProcessor->mOTAUpdatePartition, OTA_WITH_SEQUENTIAL_WRITES, &(imageProcessor->mOTAUpdateHandle));
    if (err != ESP_OK)
    {
        imageProcessor->mDownloader->OnPreparedForDownload(ESP32Utils::MapError(err));
        return;
    }
    imageProcessor->mHeaderParser.Init();
    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
    PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadInProgress);
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }
    esp_err_t err = esp_ota_end(imageProcessor->mOTAUpdateHandle);
    if (err != ESP_OK)
    {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED)
        {
            ESP_LOGE(TAG, "Image validation failed, image is corrupted");
        }
        else
        {
            ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
        }
        PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed);
        return;
    }
    imageProcessor->ReleaseBlock();
    ChipLogProgress(SoftwareUpdate, "OTA image downloaded to offset 0x%x", imageProcessor->mOTAUpdatePartition->address);
    PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadComplete);
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }
    if (esp_ota_abort(imageProcessor->mOTAUpdateHandle) != ESP_OK)
    {
        ESP_LOGE(TAG, "ESP OTA abort failed");
    }
    imageProcessor->ReleaseBlock();
    PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadAborted);
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

    ByteSpan block = ByteSpan(imageProcessor->mBlock.data(), imageProcessor->mBlock.size());

    CHIP_ERROR error = imageProcessor->ProcessHeader(block);
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to process OTA image header");
        imageProcessor->mDownloader->EndDownload(error);
        PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed);
        return;
    }

    esp_err_t err = esp_ota_write(imageProcessor->mOTAUpdateHandle, block.data(), block.size());
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_write failed (%s)", esp_err_to_name(err));
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
        PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed);
        return;
    }
    imageProcessor->mParams.downloadedBytes += block.size();
    imageProcessor->mDownloader->FetchNextData();
}

void OTAImageProcessorImpl::HandleApply(intptr_t context)
{
    PostOTAStateChangeEvent(DeviceLayer::kOtaApplyInProgress);
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    esp_err_t err         = esp_ota_set_boot_partition(imageProcessor->mOTAUpdatePartition);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        PostOTAStateChangeEvent(DeviceLayer::kOtaApplyFailed);
        return;
    }
    ESP_LOGI(TAG, "Applying, Boot partition set offset:0x%x", imageProcessor->mOTAUpdatePartition->address);

    PostOTAStateChangeEvent(DeviceLayer::kOtaApplyComplete);

    // HandleApply is called after delayed action time seconds are elapsed, so it would be safe to schedule the restart
    DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(2 * 1000), HandleRestart, nullptr);
}

CHIP_ERROR OTAImageProcessorImpl::SetBlock(ByteSpan & block)
{
    if (!IsSpanUsable(block))
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
        uint8_t * mBlock_ptr = static_cast<uint8_t *>(Platform::MemoryAlloc(block.size()));
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
        Platform::MemoryFree(mBlock.data());
    }
    mBlock = MutableByteSpan();
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessHeader(ByteSpan & block)
{
    if (mHeaderParser.IsInitialized())
    {
        OTAImageHeader header;
        CHIP_ERROR error = mHeaderParser.AccumulateAndDecode(block, header);

        // Need more data to decode the header
        ReturnErrorCodeIf(error == CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
        ReturnErrorOnFailure(error);

        mParams.totalFileBytes = header.mPayloadSize;
        mHeaderParser.Clear();
    }

    return CHIP_NO_ERROR;
}

} // namespace chip
