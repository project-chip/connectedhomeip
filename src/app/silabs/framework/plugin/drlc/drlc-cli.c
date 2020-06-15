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
 * @brief CLI for the DRLC plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/drlc/demand-response-load-control.h"
#include "app/framework/plugin/drlc/load-control-event-table.h"

#if !defined(EMBER_AF_GENERATE_CLI)

void emberAfPluginDrlcOptCommand(void);
void emberAfPluginDrlcPrintCommand(void);
void emberAfPluginDrlcClearCommand(void);

EmberCommandEntry emberAfPluginDrlcOptCommands[] = {
  emberCommandEntryAction("in", emberAfPluginDrlcOptCommand, "uw", ""),
  emberCommandEntryAction("out", emberAfPluginDrlcOptCommand, "uw", ""),
  emberCommandEntryTerminator(),
};

EmberCommandEntry emberAfPluginDrlcCommands[] = {
  emberCommandEntryAction("opt", NULL, (const char *)emberAfPluginDrlcOptCommands, ""),
  emberCommandEntryAction("print", emberAfPluginDrlcPrintCommand, "u", ""),
  emberCommandEntryAction("clear", emberAfPluginDrlcClearCommand, "u", ""),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin drlc opt <in | out> <endpoint:1> <id:4>
void emberAfPluginDrlcOptCommand(void)
{
  emAfLoadControlEventOptInOrOut((uint8_t)emberUnsignedCommandArgument(0),
                                 emberUnsignedCommandArgument(1),
                                 emberCurrentCommand->name[0] == 'i');
}

// plugin drlc print <endpoint:1>
void emberAfPluginDrlcPrintCommand(void)
{
  emAfLoadControlEventTablePrint((uint8_t)emberUnsignedCommandArgument(0));
}

// plugin drlc clear <endpoint:1>
void emberAfPluginDrlcClearCommand(void)
{
  emAfLoadControlEventTableClear((uint8_t)emberUnsignedCommandArgument(0));

  emberAfCorePrintln("Events cleared");
}
