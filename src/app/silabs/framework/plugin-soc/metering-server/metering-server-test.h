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
 * @brief Test definitions for the Metering Server plugin.
 *******************************************************************************
   ******************************************************************************/

#define EMBER_AF_API_POWER_METER "../../util/plugin/plugin-afv2/power-meter-cs5463/power-meter.h"
#define TEST_METERING_SERVER_METER_STATUS_ERROR_NONE                    0
#define TEST_METERING_SUMMATION_FORMAT                               0x33
#define TEST_METERING_MULTIPLIER                                        1
#define TEST_METERING_DIVISOR                                     3600000
#define EMBER_AF_PLUGIN_METERING_SERVER_MAX_SAMPLES_PER_SESSION         5
#define EMBER_AF_SIMPLE_METERING_CLUSTER_SERVER_ENDPOINT_COUNT          1
#define EMBER_AF_PLUGIN_METERING_SERVER_MAX_SAMPLING_SESSIONS           5
#define EMBER_AF_PLUGIN_METERING_SERVER_METER_PROFILES                  5
