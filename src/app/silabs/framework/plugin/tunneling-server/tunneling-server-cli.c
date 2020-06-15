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
 * @brief CLI for the Tunneling Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "tunneling-server.h"

void emAfPluginTunnelingServerCliTransfer(void);

#if !defined(EMBER_AF_GENERATE_CLI)

void emAfPluginTunnelingServerCliPrint(void);

EmberCommandEntry emberAfPluginTunnelingServerCommands[] = {
  emberCommandEntryAction("transfer", emAfPluginTunnelingServerCliTransfer, "vb",
                          "Transfer data through the tunnel"),
  emberCommandEntryAction("busy", emberAfPluginTunnelingServerToggleBusyCommand, "",
                          "Toggly the busy status of the tunnel"),
  emberCommandEntryAction("print", emAfPluginTunnelingServerCliPrint, "",
                          "Print the list of tunnels"),
  emberCommandEntryTerminator(),
};

#endif // EMBER_AF_GENERATE_CLI

// plugin tunneling-server transfer <tunnel index:2> <data>
void emAfPluginTunnelingServerCliTransfer(void)
{
  uint16_t tunnelIndex = (uint16_t)emberUnsignedCommandArgument(0);
  uint8_t data[255];
  uint16_t dataLen = emberCopyStringArgument(1, data, sizeof(data), false);
  EmberAfStatus status = emberAfPluginTunnelingServerTransferData(tunnelIndex,
                                                                  data,
                                                                  dataLen);
  emberAfTunnelingClusterPrintln("%p 0x%x", "transfer", status);
}
