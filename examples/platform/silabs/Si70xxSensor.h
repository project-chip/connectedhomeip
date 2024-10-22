/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#pragma once

#include "sl_status.h"
#include <stdint.h>

namespace Si70xxSensor {

/**
 * @brief Initialises the Si70xx Sensor.
 *
 * @return sl_status_t SL_STATUS_OK if there were no errors occured during initialisation.
 *                     Error if an underlying platform error occured
 */
sl_status_t Init();

/**
 * @brief Reads Humidity and temperature values from the Si70xx sensor.
 *        The init function must be called before calling the GetSensorData.
 *
 * @param[out] relativeHumidity Relative humidity percentage in centi-pourcentage (1000 == 10.00%)
 * @param[out] temperature Ambiant temperature in centi-celsium (1000 == 10.00C)
 *
 * @return sl_status_t SL_STATUS_OK if there were no errors occured during initialisation.
 *                     SL_STATUS_NOT_INITIALIZED if the sensor was not initialised
 *                     Error if an underlying platform error occured
 */
sl_status_t GetSensorData(uint16_t & relativeHumidity, int16_t & temperature);

}; // namespace Si70xxSensor
