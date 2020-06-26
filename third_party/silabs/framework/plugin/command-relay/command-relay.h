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
 * @brief APIs for the Command Relay plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_COMMAND_RELAY_H
#define SILABS_COMMAND_RELAY_H

typedef struct {
  EmberEUI64 eui64;
  uint8_t endpoint;
  uint16_t clusterId;
} EmberAfPluginCommandRelayDeviceEndpoint;

typedef struct {
  EmberAfPluginCommandRelayDeviceEndpoint inDeviceEndpoint;
  EmberAfPluginCommandRelayDeviceEndpoint outDeviceEndpoint;
} EmberAfPluginCommandRelayEntry;

/** @brief Adds a command relay item.
 *
 * Adds a command relay rule that will forward the command that matches EUI,
 * endpoint, and cluster in the inDeviceEndpoint structure to where the
 * outDeviceEndpoint defines.
 *
 * @param inDeviceEndpoint EUI64, endpoint, and cluster that will be matched
 * with incoming commands.
 *
 * @param outDeviceEndpoint EUI64, endpoint, and cluster that the command will
 * be forwarded.
 */
void emberAfPluginCommandRelayAdd(
  EmberAfPluginCommandRelayDeviceEndpoint* inDeviceEndpoint,
  EmberAfPluginCommandRelayDeviceEndpoint* outDeviceEndpoint);

/** @brief Removes a command relay item.
 *
 * Removes the rule item that matches inDeviceEndpoint and outDeviceEndpoint.
 *
 * @param inDeviceEndpoint in endpoint structure.
 *
 * @param outDeviceEndpoint out endpoint structure.
 */
void emberAfPluginCommandRelayRemove(
  EmberAfPluginCommandRelayDeviceEndpoint* inDeviceEndpoint,
  EmberAfPluginCommandRelayDeviceEndpoint* outDeviceEndpoint);

/** @brief Saves command relay rules in a backup file.
 *
 * Command relay rules will be saved in a backup file in the host once called.
 *
 */
void emberAfPluginCommandRelaySave(void);

/** @brief Loads command relay rules from a backup file.
 *
 * Command relay rules will be loaded from a backup file in the host once
 * called.
 *
 */
void emberAfPluginCommandRelayLoad(void);

/** @brief Clears command relay rules.
 *
 * All command relay rules will be clear, including the backup file.
 *
 */
void emberAfPluginCommandRelayClear(void);

/** @brief A pointer to the relay table structure.
 *
 * For some MQTT messages, it is more convenient to use the relay table data
 * structure directly. Use this API to get a pointer to the relay table
 * structure.
 *
 * @out  A pointer to the address table structure.
 */
EmberAfPluginCommandRelayEntry* emberAfPluginCommandRelayTablePointer(void);

#endif //__COMMAND_RELAY_H
