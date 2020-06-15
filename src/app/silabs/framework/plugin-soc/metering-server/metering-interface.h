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
 * @brief Interface code for the Metering Server plugin.
 *******************************************************************************
   ******************************************************************************/

enum {
  EMBER_AF_PLUGIN_METERING_SERVER_DISABLE_PROFILING   = 0,
  EMBER_AF_PLUGIN_METERING_SERVER_ENABLE_PROFILING    = 1,
  EMBER_AF_PLUGIN_METERING_SERVER_CLEAR_PROFILES      = 2,
};

typedef uint8_t EmberAfPluginMeteringServerProfileState;

/** @brief Tick function for metering server tick counter.
 *
 * Called every second for the metering server
 * to calculate the energy consumed by the
 * metering device.
 *
 * @param endpoint Affected endpoint.
 */
void emberAfPluginMeteringServerInterfaceTick(uint8_t endpoint);

/** @brief Initialization function of the metering interface.
 *
 * Initializes all parameters and attributes
 * needed for metering server.
 *
 * @param endpoint Affected endpoint.
 */
void emberAfPluginMeteringServerInterfaceInit(uint8_t endpoint);

/** @brief Enables profiling of metering server.
 *
 * Enables and disables profiling of metering server.
 *
 * @param profileState
 *        EMBER_AF_PLUGIN_METERING_SERVER_DISABLE_PROFILING: Disable profiling,
 *        EMBER_AF_PLUGIN_METERING_SERVER_ENABLE_PROFILING: enable profiling, and
 *        EMBER_AF_PLUGIN_METERING_SERVER_CLEAR_PROFILES: clear all profiles.
 *
 * @return false: if number of profiles is 0, or
 *         true: if number of profiles is larger than 0 and it is set
 *               by the parameter profileState.
 */
bool emberAfPluginMeteringServerInterfaceSetProfiles(
  EmberAfPluginMeteringServerProfileState profileState);

/** @brief Initializes function of metering server attributes.
 *
 * Initializes all attributes of simple metering server.
 *
 * @param endpoint Affected endpoint.
 */
void emberAfPluginMeteringServerAttributeInit(uint8_t endpoint);

/** @brief Gets profiles.
 *
 * Called upon receiving of the GetProfile command for the
 * simple metering client and will get the profiles and response to the client.
 *
 * @param intervalChannel Interval channel to get.
 *
 * @param endTime Requested end time.
 *
 * @param numberOfPeriods Number of periods requested.
 */
bool emberAfPluginMeteringServerInterfaceGetProfiles(
  uint8_t intervalChannel,
  uint32_t endTime,
  uint8_t numberOfPeriods);
