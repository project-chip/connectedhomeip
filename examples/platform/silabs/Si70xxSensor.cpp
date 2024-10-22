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

#include "sl_board_control.h"
#include "sl_i2cspm_instances.h"
#include "sl_si70xx.h"
#include <Si70xxSensor.h>
#include <lib/support/CodeUtils.h>

namespace {

constexpr uint16_t kSensorTemperatureOffset = 475;
bool initialized                            = false;

} // namespace

namespace Si70xxSensor {

sl_status_t Init()
{
    sl_status_t status = SL_STATUS_OK;

    status = sl_board_enable_sensor(SL_BOARD_SENSOR_RHT);
    VerifyOrReturnError(status == SL_STATUS_OK, status);

    status = sl_si70xx_init(sl_i2cspm_sensor, SI7021_ADDR);
    VerifyOrReturnError(status == SL_STATUS_OK, status);

    initialized = true;
    return status;
}

sl_status_t GetSensorData(uint16_t & relativeHumidity, int16_t & temperature)
{
    VerifyOrReturnError(initialized, SL_STATUS_NOT_INITIALIZED);

    sl_status_t status      = SL_STATUS_OK;
    int32_t tempTemperature = 0;
    uint32_t tempHumidity   = 0;

    status = sl_si70xx_measure_rh_and_temp(sl_i2cspm_sensor, SI7021_ADDR, &tempHumidity, &tempTemperature);
    VerifyOrReturnError(status == SL_STATUS_OK, status);

    // Sensor precision is milliX. We need to reduce to change the precision to centiX to fit with the cluster attributes presicion.
    temperature      = static_cast<int16_t>(tempTemperature / 10) - kSensorTemperatureOffset;
    relativeHumidity = static_cast<uint16_t>(tempHumidity / 10);

    return status;
}

}; // namespace Si70xxSensor
