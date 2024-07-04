/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**********************************************************
 * Includes
 *********************************************************/

#include "SensorManager.h"
#include "AppTask.h"
#include "hdc2010.h"
#include "wiced_hal_gpio.h"
#include <cycfg_pins.h>
#include <lib/support/logging/CHIPLogging.h>

#define HDC2010_ADDR 0x40

/* Calibration */
#define TEMPERATURE_CALIBRATION 0xEA
#define HUMIDITY_CALIBRATION 0x0B

/**********************************************************
 * Defines and Constants
 *********************************************************/

using namespace chip;
using namespace ::chip::DeviceLayer;

/**********************************************************
 * Variable declarations
 *********************************************************/

SensorManager SensorManager::sSensorManager;
AsyncWorkFunct SensorManager::sAppInterruptHandler = nullptr;

void SensorManager::Init(AsyncWorkFunct interruptHandler)
{
    wiced_hdc2010_address(HDC2010_ADDR);

    // Begin with a device reset
    wiced_hdc2010_reset();

    // Configure interrupt pin
    wiced_hdc2010_enable_interrupt();                                // Enable the Interrupt/DRDY pin
    wiced_hdc2010_set_interrupt_polarity(WICED_HDC2010_ACTIVE_HIGH); // Set Interrupt pin to Active High
    wiced_hdc2010_set_interrupt_mode(WICED_HDC2010_LEVEL_MODE);      // Set Interrupt to return to inactive state when in bounds
    wiced_hdc2010_enable_DRDY_interrupt();

    // Configure Measurements
    wiced_hdc2010_set_measurement_mode(WICED_HDC2010_TEMP_AND_HUMID); // Set measurements to temperature and humidity
    wiced_hdc2010_set_rate(WICED_HDC2010_TEN_SECONDS);
    wiced_hdc2010_set_temp_res(WICED_HDC2010_FOURTEEN_BIT);
    wiced_hdc2010_set_humid_res(WICED_HDC2010_FOURTEEN_BIT);
    wiced_hdc2010_temp_adjust(TEMPERATURE_CALIBRATION);
    wiced_hdc2010_humidity_adjust(HUMIDITY_CALIBRATION);

    wiced_hal_gpio_register_pin_for_interrupt(PLATFORM_SENSOR_INT, InterruptHandler, NULL);
    // begin measuring
    wiced_hdc2010_trigger_measurement();

    sAppInterruptHandler = interruptHandler;
}

void SensorManager::InterruptHandler(void * data, uint8_t port_pin)
{
    if (sAppInterruptHandler != nullptr)
    {
        PlatformMgr().ScheduleWork(sAppInterruptHandler);
    }
}
