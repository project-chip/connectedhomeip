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
 * @brief CLI for the Relay Control Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"

static void get(void);
static void set(void);

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginRelayControlServerCommands[] = {
  emberCommandEntryAction("get", get, "vuu", "Get the relay state"),
  emberCommandEntryAction("set", set, "vuuu", "Set the relay state"),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin relay-control-server get <nodeId:2> <srcEndpoint:1> <dstEndpoint:1>
static void get(void)
{
  EmberNodeId nodeId = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint = (uint8_t)emberUnsignedCommandArgument(2);

  emberAfFillCommandRelayControlClusterGetRelayState();
  emberAfSetCommandEndpoints(srcEndpoint,
                             dstEndpoint);
  emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
}

// plugin relay-control-server set <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <isEnabled:1>
static void set(void)
{
  EmberNodeId nodeId = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t srcEndpoint = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t dstEndpoint = (uint8_t)emberUnsignedCommandArgument(2);
  bool isEnabled = (bool)emberUnsignedCommandArgument(3);

  emberAfFillCommandRelayControlClusterSetRelayState(isEnabled,
                                                     EMBER_AF_PLUGIN_RELAY_CONTROL_SERVER_MAGIC_NUMBER);
  emberAfSetCommandEndpoints(srcEndpoint,
                             dstEndpoint);
  emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
}
