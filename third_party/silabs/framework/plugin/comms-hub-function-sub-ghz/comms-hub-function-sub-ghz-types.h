/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @brief Typedefs for the Comms Hub Function Sub Ghz plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef COMMS_HUB_FUNCTION_SUB_GHZ_TYPES_H_INCLUDED
#define COMMS_HUB_FUNCTION_SUB_GHZ_TYPES_H_INCLUDED

/** @brief Channel change request outcome.
 *
 * Reported by emberAfPluginCommsHubFunctionSubGhzChannelChangeCallback().
 */
typedef enum {
  // Energy scan enabled (GSME has read the notification flag)
  EMBER_AF_CHF_SUB_GHZ_ENERGY_SCAN_ENABLED_NORMAL,
  // Energy scan enabled due to a timeout (GSME has not read the flag)
  EMBER_AF_CHF_SUB_GHZ_ENERGY_SCAN_ENABLED_TIMEOUT,
  // Channel changed following a normal sequence (GSME has read the attributes)
  EMBER_AF_CHF_SUB_GHZ_CHANNEL_CHANGE_SUCCESS_NORMAL,
  // Channel changed due to a timeout (GSME has not read the attributes)
  EMBER_AF_CHF_SUB_GHZ_CHANNEL_CHANGE_SUCCESS_TIMEOUT,
  // Channel change was attempted after a normal sequence but it failed
  EMBER_AF_CHF_SUB_GHZ_CHANNEL_CHANGE_FAIL_NORMAL,
  // Channel change was attempted due to a timeout sequence but it failed
  EMBER_AF_CHF_SUB_GHZ_CHANNEL_CHANGE_FAIL_TIMEOUT,
} EmberAfPluginCommsHubFunctionSubGhzChannelChangeOutcome;

#endif // COMMS_HUB_FUNCTION_SUB_GHZ_TYPES_H_INCLUDED
