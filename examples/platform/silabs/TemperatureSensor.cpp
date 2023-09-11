/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "TemperatureSensor.h"

#include "sl_board_control.h"
#include "sl_i2cspm_instances.h"
#include "sl_si70xx.h"

namespace TemperatureSensor {
constexpr uint16_t kSensorTemperatureOffset = 800;
static bool initialized                     = false;

sl_status_t Init()
{
    sl_status_t status;
    sl_i2cspm_t * rht_sensor = sl_i2cspm_sensor;
    (void) sl_board_enable_sensor(SL_BOARD_SENSOR_RHT);

    status      = sl_si70xx_init(rht_sensor, SI7021_ADDR);
    initialized = (SL_STATUS_OK == status);
    return status;
}

sl_status_t GetTemp(uint32_t * relativeHumidity, int16_t * temperature)
{
    if (!initialized)
    {
        return SL_STATUS_NOT_INITIALIZED;
    }

    // Sensor resolution 0.001 C
    // DataModel resolution 0.01 C
    sl_status_t status;
    sl_i2cspm_t * rht_sensor = sl_i2cspm_sensor;
    int32_t temp             = 0;
    status                   = sl_si70xx_measure_rh_and_temp(rht_sensor, SI7021_ADDR, relativeHumidity, &temp);

    if (temperature != nullptr)
    {
        *temperature = static_cast<int16_t>(temp / 10) - kSensorTemperatureOffset;
    }

    return status;
}
}; // namespace TemperatureSensor
