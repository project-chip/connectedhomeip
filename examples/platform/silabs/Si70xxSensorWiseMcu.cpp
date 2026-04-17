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

#if !defined(SLI_SI91X_MCU_INTERFACE) || !SLI_SI91X_MCU_INTERFACE
#error "Si70xxSensorWiseMcu.cpp must only be built when SLI_SI91X_MCU_INTERFACE is 1 (SiWx917 / WiseMCU)."
#endif

#include "Si70xxSensor.h"
#include <lib/support/CodeUtils.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

// WiFi SDK (Si91x / Si917)
#include "sl_i2c_instances.h"
#include "sl_si91x_i2c.h"
#include "sl_si91x_si70xx.h"
#include <cmsis_os2.h>
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_si91x_power_manager.h"
#endif // defined(SL_CATALOG_POWER_MANAGER_PRESENT) && SL_CATALOG_POWER_MANAGER_PRESENT

/*******************************************************************************
***************************  Defines / Macros  ********************************
******************************************************************************/
#define TX_THRESHOLD 0 // tx threshold value
#define RX_THRESHOLD 0 // rx threshold value

namespace Si70xxSensor {

sl_status_t Init();

namespace {

constexpr uint16_t kSensorTemperatureOffset = 475;

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && SL_CATALOG_POWER_MANAGER_PRESENT
/** I2C instance setup, Si70xx init, one RH/temp sample (centi-units), I2C deinit. Caller owns power-state (PS) requirements. */
void Si91xSensorInitMeasureDeinit(uint16_t & relativeHumidity, int16_t & temperature)
{
    sl_i2c_init_instances();

    sl_status_t status = Init();
    VerifyOrReturn(status == SL_STATUS_OK, ChipLogError(AppServer, "Failed to initialize the sensor : %ld", status));

    int32_t tempTemperature = 0;
    uint32_t tempHumidity   = 0;
    status = sl_si91x_si70xx_measure_rh_and_temp(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, &tempHumidity, &tempTemperature);
    VerifyOrReturn(status == SL_STATUS_OK, ChipLogError(AppServer, "Failed to measure sensor data : %ld", status));

    // Sensor precision is X. We need to multiply by 100 to change the precision to centiX to fit with the cluster attributes
    // precision.
    temperature      = static_cast<int16_t>(tempTemperature * 100) - kSensorTemperatureOffset;
    relativeHumidity = static_cast<uint16_t>(tempHumidity * 100);

    status = sl_i2c_driver_deinit(SI70XX_I2C_INSTANCE);
    VerifyOrReturn(status == SL_I2C_SUCCESS, ChipLogError(AppServer, "Failed to de-initialize I2C driver : %ld", status));
}
#endif // defined(SL_CATALOG_POWER_MANAGER_PRESENT) && SL_CATALOG_POWER_MANAGER_PRESENT

} // namespace

sl_status_t Init()
{
    sl_status_t status = chip::DeviceLayer::Silabs::GetPlatform().EnableSi70xxSensorGpio();
    VerifyOrReturnError(status == SL_STATUS_OK, status);

    sl_i2c_config_t i2c_config;
    i2c_config.mode           = SL_I2C_LEADER_MODE;
    i2c_config.transfer_type  = SL_I2C_USING_NON_DMA;
    i2c_config.operating_mode = SL_I2C_STANDARD_MODE;
    i2c_config.i2c_callback   = NULL;

    /* Wait for sensor to become ready */
    osDelay(80);

    // Initialize I2C bus
    status = sl_i2c_driver_init(SI70XX_I2C_INSTANCE, &i2c_config);
    VerifyOrReturnError(status == SL_I2C_SUCCESS, status);

    status = sl_i2c_driver_configure_fifo_threshold(SI70XX_I2C_INSTANCE, TX_THRESHOLD, RX_THRESHOLD);
    VerifyOrReturnError(status == SL_I2C_SUCCESS, status);

    // reset the sensor
    status = sl_si91x_si70xx_reset(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR);
    VerifyOrReturnError(status == SL_STATUS_OK, status);

    // Wait for sensor to recover after reset (Si70xx needs ~15ms to recover)
    osDelay(15);

    // Initializes sensor and reads electronic ID 1st byte
    status = sl_si91x_si70xx_init(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, SL_EID_FIRST_BYTE);
    VerifyOrReturnError(status == SL_STATUS_OK, status);

    // Initializes sensor and reads electronic ID 2nd byte
    status = sl_si91x_si70xx_init(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, SL_EID_SECOND_BYTE);
    VerifyOrReturnError(status == SL_STATUS_OK, status);

    return status;
}

sl_status_t GetSensorData(uint16_t & relativeHumidity, int16_t & temperature)
{
    sl_status_t status      = SL_STATUS_OK;
    int32_t tempTemperature = 0;
    uint32_t tempHumidity   = 0;

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && SL_CATALOG_POWER_MANAGER_PRESENT
    // Add PS requirement to keep the sensor awake
    sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS3);

    Si91xSensorInitMeasureDeinit(relativeHumidity, temperature);

    // Remove PS requirement to allow device to sleep (always remove, even on error)
    sl_si91x_power_manager_remove_ps_requirement(SL_SI91X_POWER_MANAGER_PS3);
#else
    status = sl_si91x_si70xx_measure_rh_and_temp(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, &tempHumidity, &tempTemperature);
    VerifyOrReturnError(status == SL_STATUS_OK, status);
    // Sensor precision is X. We need to multiply by 100 to change the precision to centiX to fit with the cluster attributes
    // precision.
    temperature      = static_cast<int16_t>(tempTemperature * 100) - kSensorTemperatureOffset;
    relativeHumidity = static_cast<uint16_t>(tempHumidity * 100);

#endif // defined(SL_CATALOG_POWER_MANAGER_PRESENT) && SL_CATALOG_POWER_MANAGER_PRESENT

    return status;
}

} // namespace Si70xxSensor
