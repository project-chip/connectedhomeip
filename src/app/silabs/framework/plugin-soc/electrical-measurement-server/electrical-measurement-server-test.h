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
 * @brief Test definitions for the Electrical Measurement Server plugin.
 *******************************************************************************
   ******************************************************************************/

#define EMBER_AF_API_POWER_METER "../../util/plugin/plugin-afv2/power-meter-cs5463/power-meter.h"
#define EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S 30
#define EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_MAX_VOLTAGE_RATING_V 260
#define EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_MAX_CURRENT_RATING_A  16
#define EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_MAX_POWER_RATING_W  4000
#define EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_DEFAULT_REPORTABLE_RMS_VOLTAGE_CHANGE_M_C 500
#define EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_DEFAULT_REPORTABLE_RMS_CURRENT_CHANGE_M_C 500
#define EMBER_AF_PLUGIN_ELECTRICAL_MEASUREMENT_SERVER_DEFAULT_REPORTABLE_ACTIVE_POWER_CHANGE_M_C
