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
#include "Si70xxSensor.h"
#include <lib/support/CodeUtils.h>

namespace {

constexpr uint16_t kSensorTemperatureOffset = 475;

} // namespace

#ifdef SLI_SI91X_MCU_INTERFACE
// WiFi SDK (Si91x) includes
#include "sl_i2c_instances.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_si91x_gpio.h"
#include "sl_si91x_i2c.h"
#include "sl_si91x_si70xx.h"
#if defined(SL_ICD_ENABLED) && SL_ICD_ENABLED
#include "sl_si91x_power_manager.h"
#endif // defined(SL_ICD_ENABLED) && SL_ICD_ENABLED

/*******************************************************************************
***************************  Defines / Macros  ********************************
******************************************************************************/
#define TX_THRESHOLD 0 // tx threshold value
#define RX_THRESHOLD 0 // rx threshold value
#define OUTPUT_VALUE 1 // GPIO output value

/*******************************************************************************
 * Function to provide 1 ms Delay
 *******************************************************************************/
static void delay(uint32_t idelay)
{
    for (uint32_t x = 0; x < 4600 * idelay; x++) // 1.002ms delay
    {
        __NOP();
    }
}

namespace Si70xxSensor {

sl_status_t Init()
{
    sl_status_t status = SL_STATUS_OK;
    sl_i2c_config_t i2c_config;
    i2c_config.mode           = SL_I2C_LEADER_MODE;
    i2c_config.transfer_type  = SL_I2C_USING_NON_DMA;
    i2c_config.operating_mode = SL_I2C_STANDARD_MODE;
    i2c_config.i2c_callback   = NULL;

#if defined(SENSOR_ENABLE_GPIO_MAPPED_TO_UULP)
    if (sl_si91x_gpio_driver_get_uulp_npss_pin(SENSOR_ENABLE_GPIO_PIN) != 1)
    {
        // Enable GPIO ULP_CLK
        status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t) ULPCLK_GPIO);
        VerifyOrReturnError(status == SL_STATUS_OK, status);
        // Set NPSS GPIO pin MUX
        status = sl_si91x_gpio_driver_set_uulp_npss_pin_mux(SENSOR_ENABLE_GPIO_PIN, NPSS_GPIO_PIN_MUX_MODE0);
        VerifyOrReturnError(status == SL_STATUS_OK, status);
        // Set NPSS GPIO pin direction
        status = sl_si91x_gpio_driver_set_uulp_npss_direction(SENSOR_ENABLE_GPIO_PIN, (sl_si91x_gpio_direction_t) GPIO_OUTPUT);
        VerifyOrReturnError(status == SL_STATUS_OK, status);
        // Set UULP GPIO pin
        status = sl_si91x_gpio_driver_set_uulp_npss_pin_value(SENSOR_ENABLE_GPIO_PIN, GPIO_PIN_SET);
        VerifyOrReturnError(status == SL_STATUS_OK, status);
    }
#else
    sl_gpio_t sensor_enable_port_pin = { (sl_gpio_port_t) SENSOR_ENABLE_GPIO_PORT, SENSOR_ENABLE_GPIO_PIN };
    uint8_t pin_value;

    status = sl_gpio_driver_get_pin(&sensor_enable_port_pin, &pin_value);
    VerifyOrReturnError(status == SL_STATUS_OK, status);
    if (pin_value != 1)
    {
        // Enable GPIO CLK
#ifdef SENSOR_ENABLE_GPIO_MAPPED_TO_ULP
        status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t) ULPCLK_GPIO);
#else
        status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t) M4CLK_GPIO);
#endif
        VerifyOrReturnError(status == SL_STATUS_OK, status);

        // Set the pin mode for GPIO pins.
        status = sl_gpio_driver_set_pin_mode(&sensor_enable_port_pin, (sl_gpio_mode_t) 0, OUTPUT_VALUE);
        VerifyOrReturnError(status == SL_STATUS_OK, status);
        // Select the direction of GPIO pin whether Input/ Output
        status = sl_si91x_gpio_driver_set_pin_direction(SENSOR_ENABLE_GPIO_PORT, SENSOR_ENABLE_GPIO_PIN,
                                                        (sl_si91x_gpio_direction_t) GPIO_OUTPUT);
        VerifyOrReturnError(status == SL_STATUS_OK, status);
        // Set GPIO pin
        status = sl_gpio_driver_set_pin(&sensor_enable_port_pin); // Set ULP GPIO pin
        VerifyOrReturnError(status == SL_STATUS_OK, status);
    }
#endif

    /* Wait for sensor to become ready */
    delay(80);

    // Initialize I2C bus
    status = sl_i2c_driver_init(SI70XX_I2C_INSTANCE, &i2c_config);
    VerifyOrReturnError(status == SL_I2C_SUCCESS, status);

    status = sl_i2c_driver_configure_fifo_threshold(SI70XX_I2C_INSTANCE, TX_THRESHOLD, RX_THRESHOLD);
    VerifyOrReturnError(status == SL_I2C_SUCCESS, status);

    // reset the sensor
    status = sl_si91x_si70xx_reset(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR);
    VerifyOrReturnError(status == SL_STATUS_OK, status);

    // Wait for sensor to recover after reset (Si70xx needs ~15ms to recover)
    delay(15);

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
#if (defined(SL_ICD_ENABLED) && SL_ICD_ENABLED && defined(SLI_SI91X_MCU_INTERFACE))
    // Add PS requirement to keep the sensor awake
    sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS3);

    // Initialize I2C instances
    sl_i2c_init_instances();

    // Initialize sensor
    status = Init();
    VerifyOrExit(status == SL_STATUS_OK, ChipLogError(AppServer, "Failed to initialize the sensor : %ld", status));
#endif // (defined(SL_ICD_ENABLED) && SL_ICD_ENABLED && defined(SLI_SI91X_MCU_INTERFACE))

    status = sl_si91x_si70xx_measure_rh_and_temp(SI70XX_I2C_INSTANCE, SI70XX_SLAVE_ADDR, &tempHumidity, &tempTemperature);
    VerifyOrExit(status == SL_STATUS_OK, ChipLogError(AppServer, "Failed to measure sensor data : %ld", status));

    // Sensor precision is X. We need to multiply by 100 to change the precision to centiX to fit with the cluster attributes
    // precision.
    temperature      = static_cast<int16_t>(tempTemperature * 100) - kSensorTemperatureOffset;
    relativeHumidity = static_cast<uint16_t>(tempHumidity * 100);

#if (defined(SL_ICD_ENABLED) && SL_ICD_ENABLED && defined(SLI_SI91X_MCU_INTERFACE))
    // De-initialize I2C driver
    status = sl_i2c_driver_deinit(SI70XX_I2C_INSTANCE);
    VerifyOrExit(status == SL_I2C_SUCCESS, ChipLogError(AppServer, "Failed to de-initialize I2C driver : %ld", status));
#endif // (defined(SL_ICD_ENABLED) && SL_ICD_ENABLED && defined(SLI_SI91X_MCU_INTERFACE))

exit:
#if (defined(SL_ICD_ENABLED) && SL_ICD_ENABLED && defined(SLI_SI91X_MCU_INTERFACE))
    // Remove PS requirement to allow device to sleep (always remove, even on error)
    sl_si91x_power_manager_remove_ps_requirement(SL_SI91X_POWER_MANAGER_PS3);
#endif // (defined(SL_ICD_ENABLED) && SL_ICD_ENABLED && defined(SLI_SI91X_MCU_INTERFACE))
    return status;
}

} // namespace Si70xxSensor

#else
// EFR/Simplicity SDK includes
#include "sl_board_control.h"
#include "sl_i2cspm_instances.h"
#include "sl_si70xx.h"

namespace Si70xxSensor {

namespace {
bool initialized = false;
} // namespace

sl_status_t Init()
{
    sl_status_t status = SL_STATUS_OK;
    status             = sl_board_enable_sensor(SL_BOARD_SENSOR_RHT);
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

} // namespace Si70xxSensor
#endif // SLI_SI91X_MCU_INTERFACE
