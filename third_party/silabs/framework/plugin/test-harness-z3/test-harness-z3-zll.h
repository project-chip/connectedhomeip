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
 * @brief ZLL test harness functions for the Test Harness Z3 plugin.
 *******************************************************************************
   ******************************************************************************/

//
// test-harness-z3-zll.c
//
// Wednesday, December 9, 2015
//
// ZigBee 3.0 touchlink test harness functionality
//

// -----------------------------------------------------------------------------
// Internal constants

#define EM_AF_PLUGIN_TEST_HARNESS_Z3_ZLL_SERVER_TO_CLIENT_FRAME_CONTROL \
  (ZCL_CLUSTER_SPECIFIC_COMMAND                                         \
   | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT                                 \
   | ZCL_DISABLE_DEFAULT_RESPONSE_MASK)
#define EM_AF_PLUGIN_TEST_HARNESS_Z3_ZLL_CLIENT_TO_SERVER_FRAME_CONTROL \
  (ZCL_CLUSTER_SPECIFIC_COMMAND                                         \
   | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER                                 \
   | ZCL_DISABLE_DEFAULT_RESPONSE_MASK)

// -----------------------------------------------------------------------------
// Framework-internal callbacks

void emAfPluginTestHarnessZ3ZllStackStatusCallback(EmberStatus status);

void emAfPluginTestHarnessZ3ZllNetworkFoundCallback(const EmberZllNetwork *networkInfo);
void emAfPluginTestHarnessZ3ZllScanCompleteCallback(EmberStatus status);
EmberPacketAction emAfPluginTestHarnessZ3ZllCommandCallback(uint8_t *command,
                                                            EmberEUI64 sourceEui64);
EmberPacketAction emAfPluginTestHarnessZ3ZllModifyInterpanCommand(uint8_t *commandData,
                                                                  uint8_t macHeaderLength);
