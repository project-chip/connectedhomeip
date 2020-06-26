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
 * @brief Internal APIs for the Command Relay plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_COMMAND_RELAY_LOCAL_H
#define SILABS_COMMAND_RELAY_LOCAL_H

void emAfPluginCommandRelayAddBind(EmberEUI64 inEui,
                                   uint8_t inEndpoint,
                                   EmberEUI64 outEui,
                                   uint8_t outEndpoint);

void emAfPluginCommandRelayPrint(void);
void emAfPluginCommandRelayRemove(EmberAfPluginCommandRelayDeviceEndpoint* inDeviceEndpoint,
                                  EmberAfPluginCommandRelayDeviceEndpoint* outDeviceEndpoint);
void emAfPluginCommandRelayRemoveDeviceByEui64(EmberEUI64 eui64);

#endif //__COMMAND_RELAY_LOCAL_H
