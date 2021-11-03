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
#include <esp_err.h>

#pragma once

/**
 * @class OTAUpdater
 *
 * @brief This is a helper class to perform OTA on ESP devices.
 *        It supports preparing the flash region for new OTA image,
 *        writing OTA data to the flash region, successfully ending an OTA,
 *        aborting the OTA process, and booting from the new OTA image.
 */
class OTAUpdater
{
public:
    static OTAUpdater & GetInstance(void)
    {
        static OTAUpdater instance;
        return instance;
    }

    /**
     * @brief
     *   Test if OTA is in progress or not.
     *
     * @return \c true if OTA is in progress;
     *  return \c false if OTA is not in progress;
     */
    bool IsInProgress(void);

    /**
     * @brief
     *   Begins an OTA data writing.
     *   This function finds the available flash region to write the OTA data and erase the region.
     *
     * @return \c ESP_OK on success;
     *  return \c ESP_ERR_INVALID_STATE if OTA is in progress;
     *  return appropriate error code otherwise;
     */
    esp_err_t Begin(void);

    /**
     * @brief
     *   Finish OTA update and validate newly written OTA image.
     *
     * @return \c ESP_OK on success;
     *  return \c ESP_ERR_INVALID_STATE if OTA is not in progress;
     *  return \c ESP_ERR_OTA_VALIDATE_FAILED if OTA image is invalid;
     *  return appropriate error code otherwise;
     */
    esp_err_t End(void);

    /**
     * @brief
     *   Write OTA data. This function can be called multiple times as data is received during the OTA operation.
     *   Data is written sequentially to the partition.
     *
     * @param[in] data   OTA data
     * @param[in] length Length of OTA data
     *
     * @return \c ESP_OK on success;
     *  return \c ESP_ERR_OTA_VALIDATE_FAILED First byte of image contains invalid app image magic byte;
     *  return appropriate error code otherwise;
     */
    esp_err_t Write(const void * data, size_t length);

    /**
     * @brief
     *   Abort the OTA.
     *
     * @return \c ESP_OK on success;
     * return appropriate error code otherwise;
     */
    esp_err_t Abort(void);

    /**
     * @brief
     *   Apply an OTA update.
     *   Configures the boot partition to newly written OTA partition and restart device to boot from new app.
     *
     * @return \c ESP_OK on success;
     * return appropriate error code otherwise;
     */
    esp_err_t Apply(uint32_t delayedActionTime);

private:
    OTAUpdater(void) {}
    ~OTAUpdater() {}
};
