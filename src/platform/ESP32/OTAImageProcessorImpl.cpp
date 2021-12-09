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
#include "ESP32Utils.h"

#include <lib/support/CodeUtils.h>
#include <system/SystemError.h>

namespace chip {
namespace DeviceLayer {

CHIP_ERROR OTAImageProcessorImpl::PrepareDownload()
{
    VerifyOrReturnError(!mOtaUpdateInProgress, CHIP_ERROR_INCORRECT_STATE);

    mOtaUpdatePartition = esp_ota_get_next_update_partition(NULL);
    if (mOtaUpdatePartition == nullptr)
    {
        ChipLogError(DeviceLayer, "Partition not found");
        return ESP32Utils::MapError(ESP_ERR_NOT_FOUND);
    }

    ChipLogProgress(DeviceLayer, "Writing to partition subtype %d at offset 0x%x", mOtaUpdatePartition->subtype,
                    mOtaUpdatePartition->address);

    esp_err_t err = esp_ota_begin(mOtaUpdatePartition, OTA_WITH_SEQUENTIAL_WRITES, &mOtaUpdateHandle);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_ota_begin failed (%s)", esp_err_to_name(err));
        return ESP32Utils::MapError(err);
    }

    mOtaUpdateImageLen   = 0;
    mOtaUpdateInProgress = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
    VerifyOrReturnError(mOtaUpdateInProgress, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(DeviceLayer, "OTA image length %d bytes", mOtaUpdateImageLen);
    esp_err_t err = esp_ota_end(mOtaUpdateHandle);

    if (err == ESP_ERR_OTA_VALIDATE_FAILED)
    {
        ChipLogError(DeviceLayer, "Image validation failed, image is corrupted");
    }
    else if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_ota_end failed (%s)!", esp_err_to_name(err));
    }

    mOtaUpdateInProgress = false;

    return ESP32Utils::MapError(err);
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
    VerifyOrReturnError(mOtaUpdateInProgress, CHIP_ERROR_INCORRECT_STATE);

    mOtaUpdateInProgress = false;
    mOtaUpdateImageLen   = 0;

    return esp_ota_abort(mOtaUpdateHandle);
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
    VerifyOrReturnError(mOtaUpdateInProgress, CHIP_ERROR_INCORRECT_STATE);

    esp_err_t err = esp_ota_set_boot_partition(mOtaUpdatePartition);
    ReturnErrorOnFailure(ESP32Utils::MapError(err));
    ChipLogProgress(DeviceLayer, "Applying, Boot partition set offset:0x%x", mOtaUpdatePartition->address);

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessBlock(ByteSpan & block)
{
    VerifyOrReturnError(mOtaUpdateInProgress, CHIP_ERROR_INCORRECT_STATE);

    esp_err_t err = esp_ota_write(mOtaUpdateHandle, data, length);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_ota_write failed (%s)", esp_err_to_name(err));
        Abort();
        return ESP32Utils::MapError(err);
    }
    mOtaUpdateImageLen += length;
    ChipLogProgress(DeviceLayer, "Written image length %d", mOtaUpdateImageLen);
    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
