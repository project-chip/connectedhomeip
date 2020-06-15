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
 * @brief Routines for the Relay Control Client plugin.
 *******************************************************************************
   ******************************************************************************/

// this file contains all the common includes for clusters in the util
#include "app/framework/include/af.h"
#include "app/framework/plugin/relay-control-client/relay-control-client.h"

bool emberAfRelayControlClusterGetRelayStateResponseCallback(bool isEnabled)
{
  emberAfRelayControlClusterPrintln("Relay: %p", (isEnabled ? "on" : "off"));

  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

void emberAfPluginRelayControlClientSendSetRelayState(EmberNodeId nodeId,
                                                      uint8_t srcEndpoint,
                                                      uint8_t dstEndpoint,
                                                      bool isEnabled,
                                                      uint32_t magicNumber)
{
  EmberStatus status;
  emberAfFillCommandRelayControlClusterSetRelayState(isEnabled,
                                                     magicNumber);
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
  if (status != EMBER_SUCCESS) {
    emberAfRelayControlClusterPrintln("Error in send set relay state %x", status);
  }
}
