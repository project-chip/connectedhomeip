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
 * @brief CLI for the Messaging Client plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/messaging-client/messaging-client.h"

void emAfMessagingClientCliConfirm(void);
void emAfMessagingClientCliPrint(void);

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginMessagingClientCommands[] = {
  emberCommandEntryAction("confirm", emAfMessagingClientCliConfirm, "u", ""),
  emberCommandEntryAction("print", emAfMessagingClientCliPrint, "u", ""),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin messaging-client confirm <endpoint:1>
void emAfMessagingClientCliConfirm(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  EmberAfStatus status = emberAfPluginMessagingClientConfirmMessage(endpoint);
  emberAfMessagingClusterPrintln("%p 0x%x", "confirm", status);
}

// plugin messaging-client print <endpoint:1>
void emAfMessagingClientCliPrint(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  emAfPluginMessagingClientPrintInfo(endpoint);
}

// plugin messaging-client clear <endpoint:1>
void emAfMessagingClientCliClear(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  emAfPluginMessagingClientClearMessage(endpoint);
}
