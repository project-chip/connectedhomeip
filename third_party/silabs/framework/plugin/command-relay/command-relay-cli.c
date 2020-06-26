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
 * @brief CLI for the Command Relay plugin.
 *******************************************************************************
   ******************************************************************************/

#include "af.h"
#include "command-relay.h"
#include "command-relay-local.h"

static void parseDeviceEndpointsFromArguments(EmberAfPluginCommandRelayDeviceEndpoint *inDeviceEndpoint,
                                              EmberAfPluginCommandRelayDeviceEndpoint *outDeviceEndpoint);

void emAfPluginCommandRelayAddCommand(void)
{
  EmberAfPluginCommandRelayDeviceEndpoint inDeviceEndpoint;
  EmberAfPluginCommandRelayDeviceEndpoint outDeviceEndpoint;
  parseDeviceEndpointsFromArguments(&inDeviceEndpoint, &outDeviceEndpoint);

  emberAfPluginCommandRelayAdd(&inDeviceEndpoint, &outDeviceEndpoint);
}

void emAfPluginCommandRelayRemoveCommand(void)
{
  EmberAfPluginCommandRelayDeviceEndpoint inDeviceEndpoint;
  EmberAfPluginCommandRelayDeviceEndpoint outDeviceEndpoint;
  parseDeviceEndpointsFromArguments(&inDeviceEndpoint, &outDeviceEndpoint);

  emberAfPluginCommandRelayRemove(&inDeviceEndpoint, &outDeviceEndpoint);
}

void emAfPluginCommandRelayClearCommand(void)
{
  emberAfPluginCommandRelayClear();
}

void emAfPluginCommandRelaySaveCommand(void)
{
  emberAfPluginCommandRelaySave();
}

void emAfPluginCommandRelayLoadCommand(void)
{
  emberAfPluginCommandRelayLoad();
}

void emAfPluginCommandRelayPrintCommand(void)
{
  emAfPluginCommandRelayPrint();
}

static void parseDeviceEndpointsFromArguments(EmberAfPluginCommandRelayDeviceEndpoint *inDeviceEndpoint,
                                              EmberAfPluginCommandRelayDeviceEndpoint *outDeviceEndpoint)
{
  emberCopyBigEndianEui64Argument(0, inDeviceEndpoint->eui64);
  inDeviceEndpoint->endpoint = (uint8_t)emberUnsignedCommandArgument(1);
  inDeviceEndpoint->clusterId = (uint16_t)emberUnsignedCommandArgument(2);

  emberCopyBigEndianEui64Argument(3, outDeviceEndpoint->eui64);
  outDeviceEndpoint->endpoint = (uint8_t)emberUnsignedCommandArgument(4);
  outDeviceEndpoint->clusterId = (uint16_t)emberUnsignedCommandArgument(5);
}
