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
 * @brief Definitions for the Update TC Link Key plugin.
 *******************************************************************************
   ******************************************************************************/

// -----------------------------------------------------------------------------
// Constants

#define EMBER_AF_PLUGIN_UPDATE_TC_LINK_KEY_PLUGIN_NAME "Update TC Link Key"

// -----------------------------------------------------------------------------
// API

/* @brief Starts a link key update process.
 *
 * Kicks off a link key update process.
 *
 * @return An ::EmberStatus value. If the current node is not on a network,
 * this will return ::EMBER_NOT_JOINED. If the current node is on a
 * distributed security network, this will return
 * ::EMBER_SECURITY_CONFIGURATION_INVALID. If the current node is the
 * trust center, this will return ::EMBER_INVALID_CALL.
 */
EmberStatus emberAfPluginUpdateTcLinkKeyStart(void);

/* @brief Stops a link key update process.
 *
 * Stops a link key update process.
 *
 * @return Whether or not a TCLK update was in progress.
 */
bool emberAfPluginUpdateTcLinkKeyStop(void);

/** @brief change the time between two subsequent update tc link key calls*/
void emberAfPluginSetTCLinkKeyUpdateTimerMilliSeconds(uint32_t timeInMilliseconds);

/* @brief Sets the delay until the next request is made to update the trust
 * center link key.
 */
void emberAfPluginUpdateTcLinkKeySetDelay(uint32_t delayMs);

/* @brief Stops the periodic tc link key update process.
 */
void emberAfPluginUpdateTcLinkKeySetInactive(void);
