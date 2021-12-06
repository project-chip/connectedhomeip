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
#pragma once

#include "esp_err.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"

static const char * TAG = "OTAUpdater";

class OTAUpdater
{
public:
    static OTAUpdater & GetInstance(void)
    {
        static OTAUpdater instance;
        return instance;
    }
    bool IsInProgress(void) { return otaUpdateInProgress; }
    esp_err_t Begin(void);
    esp_err_t End(void);
    esp_err_t Write(const void * data, size_t length);
    esp_err_t Abort(void);
    esp_err_t Apply();

private:
    bool otaUpdateInProgress                   = false;
    const esp_partition_t * otaUpdatePartition = nullptr;
    esp_ota_handle_t otaUpdateHandle;
    uint32_t otaUpdateImageLen = 0;

    OTAUpdater(void) {}
    ~OTAUpdater() {}
};

inline esp_err_t OTAUpdater::Begin(void)
{
    if (otaUpdateInProgress)
    {
        ESP_LOGW(TAG, "Already in progress");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Begin OTA");
    otaUpdatePartition = esp_ota_get_next_update_partition(NULL);
    if (otaUpdatePartition == NULL)
    {
        ESP_LOGE(TAG, "Partition not found");
        return ESP_ERR_NOT_FOUND;
    }
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x", otaUpdatePartition->subtype, otaUpdatePartition->address);

    esp_err_t err = esp_ota_begin(otaUpdatePartition, OTA_WITH_SEQUENTIAL_WRITES, &otaUpdateHandle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
        return err;
    }
    otaUpdateImageLen   = 0;
    otaUpdateInProgress = true;
    return ESP_OK;
}

inline esp_err_t OTAUpdater::Write(const void * data, size_t length)
{
    if (otaUpdateInProgress == false)
    {
        return ESP_ERR_INVALID_STATE;
    }
    esp_err_t err = esp_ota_write(otaUpdateHandle, data, length);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_write failed (%s)", esp_err_to_name(err));
        Abort();
        return err;
    }
    otaUpdateImageLen += length;
    ESP_LOGI(TAG, "Written image length %d", otaUpdateImageLen);
    return ESP_OK;
}

inline esp_err_t OTAUpdater::Abort(void)
{
    if (otaUpdateInProgress == false)
    {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "OTA Abort");
    otaUpdateInProgress = false;
    otaUpdateImageLen   = 0;
    return esp_ota_abort(otaUpdateHandle);
}

inline esp_err_t OTAUpdater::End(void)
{
    if (otaUpdateInProgress == false)
    {
        return ESP_ERR_INVALID_STATE;
    }
    ESP_LOGI(TAG, "OTA image length %d bytes", otaUpdateImageLen);
    esp_err_t err = esp_ota_end(otaUpdateHandle);
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
    }
    otaUpdateInProgress = false;
    return err;
}

inline esp_err_t OTAUpdater::Apply()
{
    if (otaUpdateInProgress == true)
    {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = esp_ota_set_boot_partition(otaUpdatePartition);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        return err;
    }
    ESP_LOGI(TAG, "Applying, Boot partition set offset:0x%x", otaUpdatePartition->address);
    return ESP_OK;
}
