/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file
 * @brief Test definitions for the Security Sensor plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_SECURITY_SENSOR_TEST_H
#define SILABS_SECURITY_SENSOR_TEST_H

#define EMBER_AF_API_LED_BLINK "util/plugin/plugin-common/led-blink/led-blink.h"
#define EMBER_AF_API_GPIO_SENSOR "util/plugin/plugin-common/gpio-sensor/gpio-sensor.h"

// Macros to allow plugin options to be defined in unit tests
#define EMBER_AF_PLUGIN_SECURITY_SENSOR_TIME_BETWEEN_PRESSES_MS 500
#define EMBER_AF_PLUGIN_SECURITY_SENSOR_BUTTON_DEBOUNCE_TIME_MS 100
#define EMBER_AF_PLUGIN_SECURITY_SENSOR_NETWORK_LEAVE_TIME_MS   1000
#define EMBER_AF_PLUGIN_SECURITY_SENSOR_IAS_ZONE_ENDPOINT       1

#endif
