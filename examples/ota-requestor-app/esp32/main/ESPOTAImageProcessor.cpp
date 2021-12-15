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
#include <platform/ESP32/ESP32Utils.h>
#include <sys/_stdint.h>

#include "ESPOTAImageProcessor.h"
#include "core/CHIPError.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_ota_ops.h"
#include "esp_system.h"

#define TAG "OTAImageProcessor"
using namespace ::chip::DeviceLayer::Internal;

namespace chip {

CHIP_ERROR ESPOTAImageProcessor::PrepareDownload()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESPOTAImageProcessor::Finalize()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESPOTAImageProcessor::Apply()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleApply, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESPOTAImageProcessor::Abort()
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleAbort, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESPOTAImageProcessor::ProcessBlock(ByteSpan & block)
{
    if ((block.data() == nullptr) || block.empty())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    CHIP_ERROR err = SetBlock(block);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot set block data: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    DeviceLayer::PlatformMgr().ScheduleWork(HandleProcessBlock, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

void ESPOTAImageProcessor::HandlePrepareDownload(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<ESPOTAImageProcessor *>(context);
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
    esp_err_t err = esp_ota_begin(imageProcessor->mOTAUpdatePartition, OTA_WITH_SEQUENTIAL_WRITES,
                                 &(imageProcessor->mOTAUpdateHandle));
    if (err != ESP_OK)
    {
        imageProcessor->mDownloader->OnPreparedForDownload(ESP32Utils::MapError(err));
        return;
    }
    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

void ESPOTAImageProcessor::HandleFinalize(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<ESPOTAImageProcessor *>(context);
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
        return;
    }
    imageProcessor->ReleaseBlock();
    ChipLogProgress(SoftwareUpdate, "OTA image downloaded to offset 0x%x", imageProcessor->mOTAUpdatePartition->address);
}

void ESPOTAImageProcessor::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<ESPOTAImageProcessor *>(context);
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
}

void ESPOTAImageProcessor::HandleProcessBlock(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<ESPOTAImageProcessor *>(context);
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
    esp_err_t err = esp_ota_write(imageProcessor->mOTAUpdateHandle, imageProcessor->mBlock.data(),
                                  imageProcessor->mBlock.size());
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_write failed (%s)", esp_err_to_name(err));
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
    }
    imageProcessor->mParams.downloadedBytes += imageProcessor->mBlock.size();
    imageProcessor->mDownloader->FetchNextData();
}

void ESPOTAImageProcessor::HandleApply(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<ESPOTAImageProcessor *>(context);
    esp_err_t err = esp_ota_set_boot_partition(imageProcessor->mOTAUpdatePartition);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "Applying, Boot partition set offset:0x%x", imageProcessor->mOTAUpdatePartition->address);
}

CHIP_ERROR ESPOTAImageProcessor::SetBlock(ByteSpan & block)
{
    if ((block.data() == nullptr) || block.empty())
    {
        return CHIP_NO_ERROR;
    }

    if (mBlock.empty())
    {
        mBlock = MutableByteSpan(static_cast<uint8_t *>(chip::Platform::MemoryAlloc(block.size())), block.size());
        if (mBlock.data() == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
    }
    CHIP_ERROR err = CopySpanToMutableSpan(block, mBlock);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot copy block data: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESPOTAImageProcessor::ReleaseBlock()
{
    if (mBlock.data() != nullptr)
    {
        chip::Platform::MemoryFree(mBlock.data());
    }
    mBlock = MutableByteSpan();
    return CHIP_NO_ERROR;
}
}
