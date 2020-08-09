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
 * @brief CLI for the Groups Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"

void emAfGroupsServerCliPrint(void);
void emAfGroupsServerCliClear(void);

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginGroupsServerCommands[] = {
  emberCommandEntryAction("print", emAfGroupsServerCliPrint, "", "Print the state of the groups table."),
  emberCommandEntryAction("clear", emAfGroupsServerCliClear, "", "Clear the groups table on every endpoint."),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin groups-server print
void emAfGroupsServerCliPrint(void)
{
  EmberStatus status;
  uint8_t i;

  for (i = 0; i < EMBER_BINDING_TABLE_SIZE; i++) {
    EmberBindingTableEntry entry;
    status = emberGetBinding(i, &entry);
    if ((status == EMBER_SUCCESS) && (entry.type == EMBER_MULTICAST_BINDING)) {
      emberAfCorePrintln("ep[%x] id[%2x]", entry.local,
                         HIGH_LOW_TO_INT(entry.identifier[1], entry.identifier[0]));
    }
  }
}

// plugin groups-server clear
void emAfGroupsServerCliClear(void)
{
  emberAfCorePrintln("Clearing all groups.");
  emberAfGroupsClusterClearGroupTableCallback(EMBER_BROADCAST_ENDPOINT);
}
