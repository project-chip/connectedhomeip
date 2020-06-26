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
 * @brief This code provides support for managing the address table.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/util/serial/command-interpreter2.h"
#include "address-table.h"

void emberAfPluginAddressTableAddCommand(void);
void emberAfPluginAddressTableRemoveCommand(void);
void emberAfPluginAddressTableLookupCommand(void);

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginAddressTableCommands[] = {
  emberCommandEntryAction("add", emberAfPluginAddressTableAddCommand, "b", "Add an entry to the address table."),
  emberCommandEntryAction("remove", emberAfPluginAddressTableRemoveCommand, "b", "Remove an entry from the address table."),
  emberCommandEntryAction("lookup", emberAfPluginAddressTableLookupCommand, "b", "Search for an entry in the address table."),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

void emberAfPluginAddressTableAddCommand(void)
{
  uint8_t index;
  EmberEUI64 entry;
  emberCopyEui64Argument(0, entry);

  index = emberAfPluginAddressTableAddEntry(entry);

  if (index == EMBER_NULL_ADDRESS_TABLE_INDEX) {
    emberAfCorePrintln("Table full, entry not added");
  } else {
    emberAfCorePrintln("Entry added at position 0x%x", index);
  }
}

void emberAfPluginAddressTableRemoveCommand(void)
{
  EmberStatus status;
  EmberEUI64 entry;
  emberCopyEui64Argument(0, entry);

  status = emberAfPluginAddressTableRemoveEntry(entry);

  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("Entry removed");
  } else {
    emberAfCorePrintln("Entry removal failed");
  }
}

void emberAfPluginAddressTableLookupCommand(void)
{
  uint8_t index;
  EmberEUI64 entry;
  emberCopyEui64Argument(0, entry);
  index = emberAfPluginAddressTableLookupByEui64(entry);

  if (index == EMBER_NULL_ADDRESS_TABLE_INDEX) {
    emberAfCorePrintln("Entry not found");
  } else {
    emberAfCorePrintln("Found entry at position 0x%x", index);
  }
}

// plugin address-table print
void emberAfPluginAddressTablePrintCommand(void)
{
  uint8_t i;
  uint8_t used = 0;
  emberAfAppPrintln("#  node   eui");
  for (i = 0; i < emberAfGetAddressTableSize(); i++) {
    EmberNodeId nodeId = emberGetAddressTableRemoteNodeId(i);
    if (nodeId != EMBER_TABLE_ENTRY_UNUSED_NODE_ID) {
      EmberEUI64 eui64;
      used++;
      emberAfAppPrint("%d: 0x%2x ", i, nodeId);
      emberGetAddressTableRemoteEui64(i, eui64);
      emberAfAppDebugExec(emberAfPrintBigEndianEui64(eui64));
      emberAfAppPrintln("");
      emberAfAppFlush();
    }
  }
  emberAfAppPrintln("%d of %d entries used.",
                    used,
                    emberAfGetAddressTableSize());
}

// plugin address-table set <index> <eui64> <node id>
void emberAfPluginAddressTableSetCommand(void)
{
  EmberEUI64 eui64;
  EmberStatus status;
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(0);
  EmberNodeId nodeId = (EmberNodeId)emberUnsignedCommandArgument(2);
  emberCopyBigEndianEui64Argument(1, eui64);
  status = emberAfSetAddressTableEntry(index, eui64, nodeId);
  UNUSED_VAR(status);
  emberAfAppPrintln("set address %d: 0x%x", index, status);
}
