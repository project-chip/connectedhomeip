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
 * @brief Definitions for the Electrical Measurement Server plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_ELECTRICAL_MEASUREMENT_SERVER_H
#define SILABS_ELECTRICAL_MEASUREMENT_SERVER_H

//------------------------------------------------------------------------------
// Plugin public function declarations

/** @brief Sets the hardware read interval.
 *
 * This function sets the amount of time to wait (in seconds) between polls
 * of the power meter. This function will never set the measurement
 * interval to be greater than the plugin-specified maximum measurement
 * interval. If a value of 0 is given, the plugin-specified maximum measurement
 * interval will be used for the polling interval.
 */
void emberAfPluginElectricalMeasurementServerSetMeasurementInterval(
  uint32_t measurementRateS);

#endif //__ELECTRICAL_MEASUREMENT_SERVER_H__
