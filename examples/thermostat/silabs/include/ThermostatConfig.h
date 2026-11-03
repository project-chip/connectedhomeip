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

#ifndef SL_MATTER_THERMOSTAT_CONFIG_H
#define SL_MATTER_THERMOSTAT_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// ThermostatConfig.h
// Preprocessor knobs for the silabs thermostat example. Edit these #define
// statements to customize the cluster endpoint, the sensor sampling cadence,
// and the minimum temperature change that flags `LocalTemperature` dirty for
// reporting.

// <o THERMOSTAT_ENDPOINT> Thermostat cluster endpoint
// <i> Default: 1
#define THERMOSTAT_ENDPOINT 1

// <o SENSOR_TIMER_PERIOD_MS> Sensor sampling period in milliseconds
// <i> Default: 30000 (30 seconds)
#define SENSOR_TIMER_PERIOD_MS 30000

// <o MIN_TEMPERATURE_DELTA> Minimum LocalTemperature change before marking the attribute dirty,
// <i> in units of 0.01 deg C.
// <i> Default: 50 (0.5 deg C)
#define MIN_TEMPERATURE_DELTA 50

// <<< end of configuration section >>>

#endif // SL_MATTER_THERMOSTAT_CONFIG_H
