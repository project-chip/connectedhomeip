/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    Copyright (c) 2024 Google LLC.
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

#include "AirQualitySensor.h"
#include "sl_board_control.h"
#include <platform/CHIPDeviceLayer.h>

#ifdef USE_SPARKFUN_AIR_QUALITY_SENSOR
#include "sl_i2cspm_instances.h"
#include <sparkfun_sgp40.h>
#endif // USE_SPARKFUN_AIR_QUALITY_SENSOR

namespace {
bool initialized = false;
}

namespace AirQualitySensor {

/**
 * @brief Initializes the air quality sensor.
 *
 * This function initializes the air quality sensor, specifically supporting the SparkFun SGP40 air quality sensor.
 * It uses the I2C protocol for communication and sets up the VOC (Volatile Organic Compounds) algorithm for air quality
 * measurement.
 *
 * @note If the macro USE_SPARKFUN_AIR_QUALITY_SENSOR is defined, the SparkFun SGP40 sensor is initialized.
 *       If initialization is successful, the function returns SL_STATUS_OK, indicating the sensor is ready for use.
 *       If the initialization fails, it returns SL_STATUS_FAIL. If USE_SPARKFUN_AIR_QUALITY_SENSOR is not defined,
 *       it is expected that the user provides an implementation for their specific air quality sensor.
 *
 * @return sl_status_t Returns SL_STATUS_OK if the initialization is successful, otherwise returns SL_STATUS_FAIL.
 */

sl_status_t Init()
{
    sl_status_t status = SL_STATUS_FAIL;

#ifdef USE_SPARKFUN_AIR_QUALITY_SENSOR
    status = sparkfun_sgp40_init(sl_i2cspm_qwiic);
    VerifyOrReturnError(status == SL_STATUS_OK, SL_STATUS_FAIL);
    initialized = true;

    sparkfun_sgp40_voc_algorithm_init();

#else
// User implementation of Init
#endif // USE_SPARKFUN_AIR_QUALITY_SENSOR

    return status;
}

/**
 * @brief Retrieves the air quality measurement.
 *
 * This function fetches the current air quality measurement from the air quality sensor. It is designed to work
 * with the SparkFun SGP40 air quality sensor when the USE_SPARKFUN_AIR_QUALITY_SENSOR macro is defined. The function
 * calculates the VOC (Volatile Organic Compounds) index, which is used as a measure of air quality.
 *
 * @note The function requires the sensor to be initialized before calling. If the sensor is not initialized,
 *       SL_STATUS_NOT_INITIALIZED is returned. For the SparkFun SGP40 sensor, the function uses hardcoded
 *       values for humidity and temperature (50% and 25°C, respectively) as part of the VOC index calculation.
 *       It is recommended to replace these magic numbers with actual sensor readings.
 *
 * @param air_quality A pointer to an integer where the air quality measurement will be stored.
 *
 * @return sl_status_t Returns SL_STATUS_OK if the air quality measurement is successfully retrieved,
 *         SL_STATUS_NOT_INITIALIZED if the sensor has not been initialized, or other error codes as defined.
 */

sl_status_t GetAirQuality(int32_t & air_quality)
{
    sl_status_t status = SL_STATUS_FAIL;
    VerifyOrReturnError(initialized, SL_STATUS_NOT_INITIALIZED);

#ifdef USE_SPARKFUN_AIR_QUALITY_SENSOR
    constexpr float relativeHumidity = 50; // 50%
    constexpr float temperature      = 25; // 25°C

    status = sparkfun_sgp40_get_voc_index(&air_quality, relativeHumidity, temperature);
    VerifyOrReturnError(status == SL_STATUS_OK, status);

#else
    // User implementation of GetAirQuality
#endif // USE_SPARKFUN_AIR_QUALITY_SENSOR

    return status;
}
}; // namespace AirQualitySensor
