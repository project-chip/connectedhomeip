/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "Si70xxSensor.h"
#include <lib/support/CodeUtils.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#include <algorithm>
#include <cstdint>
#include <limits>

// WiFi SDK (Si91x / Si917)
#include "sl_i2c_instances.h"
#include "sl_si91x_i2c.h"
#include "sl_si91x_si70xx.h"
#include <cmsis_os2.h>
#if defined(SL_ICD_ENABLED) && SL_ICD_ENABLED
#include "sl_si91x_power_manager.h"
#endif // defined(SL_ICD_ENABLED) && SL_ICD_ENABLED

namespace Si70xxSensor {

sl_status_t Init();

constexpr uint16_t kSensorTemperatureOffset = 475;

constexpr uint32_t kSi70xxPowerUpDelay = 80;
constexpr uint32_t kSi70xxResetDelay   = 15;

uint32_t sFifoTxThreshold = 0;
uint32_t sFifoRxThreshold = 0;

/** Full I2C + Si70xx setup (used at boot and from on-demand reads when power manager is present). */
sl_status_t InitI2cAndSensor()
{
    chip::DeviceLayer::Silabs::SilabsPlatformAbstractionBase & platform = chip::DeviceLayer::Silabs::GetPlatform();
    sl_status_t status                                                  = platform.EnableSi70xxSensorGpio();
    VerifyOrReturnError(status == SL_STATUS_OK, status);

    sl_i2c_config_t i2c_config;
    i2c_config.mode           = SL_I2C_LEADER_MODE;
    i2c_config.transfer_type  = SL_I2C_USING_NON_DMA;
    i2c_config.operating_mode = SL_I2C_STANDARD_MODE;
    i2c_config.i2c_callback   = NULL;

    // Si70xx power-up and I2C bus settle after supply enable. Takes ~80 ms.
    osDelay(kSi70xxPowerUpDelay);

    // Initialize I2C bus
    status = sl_i2c_driver_init(SI70XX_I2C_INSTANCE, &i2c_config);
    VerifyOrReturnError(status == SL_I2C_SUCCESS, status);

    status = sl_i2c_driver_configure_fifo_threshold(SI70XX_I2C_INSTANCE, sFifoTxThreshold, sFifoRxThreshold);
    VerifyOrReturnError(status == SL_I2C_SUCCESS, status);

    // reset the sensor
    status = sl_si91x_si70xx_reset(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR);
    VerifyOrReturnError(status == SL_STATUS_OK, status);

    // Wait for sensor to recover after reset (Si70xx needs ~15ms to recover)
    osDelay(kSi70xxResetDelay);

    // Initializes sensor and reads electronic ID 1st byte
    status = sl_si91x_si70xx_init(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, SL_EID_FIRST_BYTE);
    VerifyOrReturnError(status == SL_STATUS_OK, status);

    // Initializes sensor and reads electronic ID 2nd byte
    status = sl_si91x_si70xx_init(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, SL_EID_SECOND_BYTE);
    VerifyOrReturnError(status == SL_STATUS_OK, status);

    return status;
}

#if defined(SL_ICD_ENABLED) && SL_ICD_ENABLED
/** I2C instance setup, Si70xx init, one RH/temp sample (centi-units), I2C deinit. Caller owns power-state (PS) requirements. */
void Si91xSensorInitMeasureDeinit(uint16_t & relativeHumidity, int16_t & temperature)
{
    sl_i2c_init_instances();

    sl_status_t status = InitI2cAndSensor();
    VerifyOrReturn(status == SL_STATUS_OK, ChipLogError(AppServer, "Failed to initialize the sensor : %ld", status));

    int32_t tempTemperature = 0;
    uint32_t tempHumidity   = 0;
    status = sl_si91x_si70xx_measure_rh_and_temp(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, &tempHumidity, &tempTemperature);
    VerifyOrReturn(status == SL_STATUS_OK, ChipLogError(AppServer, "Failed to measure sensor data : %ld", status));

    // Sensor precision is X. We need to multiply by 100 to change the precision to centiX to fit with the cluster attributes
    // precision.
    tempTemperature = (tempTemperature * 100) - static_cast<int32_t>(kSensorTemperatureOffset);
    tempHumidity    = tempHumidity * 100;

    temperature      = static_cast<int16_t>(std::clamp(tempTemperature, INT32_MIN, INT32_MAX));
    relativeHumidity = static_cast<uint16_t>(std::clamp<uint32_t>(tempHumidity, 0, UINT16_MAX));

    status = sl_i2c_driver_deinit(SI70XX_I2C_INSTANCE);
    VerifyOrReturn(status == SL_I2C_SUCCESS, ChipLogError(AppServer, "Failed to de-initialize I2C driver : %ld", status));
}
#endif // defined(SL_ICD_ENABLED) && SL_ICD_ENABLED

CHIP_ERROR SetI2cFifoThresholds(uint32_t txThreshold, uint32_t rxThreshold)
{
    sFifoTxThreshold = txThreshold;
    sFifoRxThreshold = rxThreshold;
    return CHIP_NO_ERROR;
}

sl_status_t Init()
{
#if defined(SL_ICD_ENABLED) && SL_ICD_ENABLED
    // Defer I2C / sensor bring-up to GetSensorData() so we do not hold the sensor active at boot while sleeping (ICD).
    return SL_STATUS_OK;
#endif // defined(SL_ICD_ENABLED) && SL_ICD_ENABLED
    return InitI2cAndSensor();
}

sl_status_t GetSensorData(uint16_t & relativeHumidity, int16_t & temperature)
{
    sl_status_t status = SL_STATUS_OK;

#if defined(SL_ICD_ENABLED) && SL_ICD_ENABLED
    // Add PS requirement to keep the sensor awake
    sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS3);

    Si91xSensorInitMeasureDeinit(relativeHumidity, temperature);

    // Remove PS requirement to allow device to sleep (always remove, even on error)
    sl_si91x_power_manager_remove_ps_requirement(SL_SI91X_POWER_MANAGER_PS3);
#else
    int32_t tempTemperature = 0;
    uint32_t tempHumidity   = 0;
    status = sl_si91x_si70xx_measure_rh_and_temp(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, &tempHumidity, &tempTemperature);
    VerifyOrReturnError(status == SL_STATUS_OK, status);

    // Sensor precision is X. We need to multiply by 100 to change the precision to centiX to fit with the cluster attributes
    // precision.
    tempTemperature = (tempTemperature * 100) - static_cast<int32_t>(kSensorTemperatureOffset);
    tempHumidity    = tempHumidity * 100;

    temperature      = static_cast<int16_t>(std::clamp(tempTemperature, INT32_MIN, INT32_MAX));
    relativeHumidity = static_cast<uint16_t>(std::clamp<uint32_t>(tempHumidity, 0, UINT16_MAX));

#endif // defined(SL_ICD_ENABLED) && SL_ICD_ENABLED

    return status;
}

} // namespace Si70xxSensor
