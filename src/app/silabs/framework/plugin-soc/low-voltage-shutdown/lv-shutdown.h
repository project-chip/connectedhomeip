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
 * @brief Definitions for the LV Shutdown plugin.
 *******************************************************************************
   ******************************************************************************/

// Obtain a single reading of the VDD voltage (used to check for shutdown)
// Result is in millivolts.
uint16_t emberAfPluginLowVoltageShutdownGetVoltage(void);

// Enable or disable shutdown logic at runtime for testing purposes
void emberAfPluginLowVoltageShutdownEnable(bool enable);

// Check whether the shutdown logic has is enabled at runtime (as it can be
// artificially disabled)
bool emberAfPluginLowVoltageShutdownEnabled(void);

// Force a shutdown event (regardless of VDD) for testing
void emberAfPluginLowVoltageShutdownForceShutdown(void);
