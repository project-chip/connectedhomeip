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
 * @brief CLI for the Poll Control Client plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/poll-control-client/poll-control-client.h"

void mode(void);
void timeout(void);
void respond(void);
void print(void);

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginPollControlClientCommands[] = {
  emberCommandEntryAction("mode", mode, "u", ""),
  emberCommandEntryAction("timeout", timeout, "v", ""),
  emberCommandEntryAction("respond", respond, "u", ""),
  emberCommandEntryAction("print", print, "", ""),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin poll-control-client mode <mode:1>
void mode(void)
{
  uint8_t mode = (uint8_t)emberUnsignedCommandArgument(0);
  emAfSetFastPollingMode(mode);
  emberAfPollControlClusterPrintln("%p 0x%x", "mode", mode);
}

// plugin poll-control-client timeout <timeout:2>
void timeout(void)
{
  uint16_t timeout = (uint16_t)emberUnsignedCommandArgument(0);
  emAfSetFastPollingTimeout(timeout);
  emberAfPollControlClusterPrintln("%p 0x%2x", "timeout", timeout);
}

// plugin poll-control-client respond <mode:1>
void respond(void)
{
  uint8_t mode = (uint8_t)emberUnsignedCommandArgument(0);
  emAfSetResponseMode(mode);
  emberAfPollControlClusterPrintln("%p 0x%x", "respond", mode);
}

void print(void)
{
  emAfPollControlClientPrint();
}
