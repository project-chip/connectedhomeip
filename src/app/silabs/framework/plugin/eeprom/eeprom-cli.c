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
 * @brief CLI for the EEPROM plugin.
 *******************************************************************************
   ******************************************************************************/

#include PLATFORM_HEADER     // Micro and compiler specific typedefs and macros

#include "app/framework/include/af.h"
#include "app/framework/plugin/eeprom/eeprom.h"

//------------------------------------------------------------------------------
// Globals

#define DATA_BLOCK 64

#if !defined(EMBER_AF_GENERATE_CLI)

void emAfEepromDataPrintCommand(void);
void emAfEepromStatusCommand(void);
void emAfEepromInfoCommand(void);

const char * pluginEepromDataPrintCommandArguments[] = {
  "The offset within the EEPROM to print.",
  NULL
};

EmberCommandEntry emberAfPluginEepromCommands[] = {
  emberCommandEntryActionWithDetails("data-print", emAfEepromDataPrintCommand, "w", "Print raw bytes stored in the EEPROM", pluginEepromDataPrintCommandArguments),
  emberCommandEntryActionWithDetails("status", emAfEepromStatusCommand, "", "Print the state of the EEPROM driver.", NULL),
  emberCommandEntryActionWithDetails("info", emAfEepromInfoCommand, "", "Print detailed info about the EEPROM part.", NULL),
  emberCommandEntryTerminator()
};
#endif

//------------------------------------------------------------------------------

void emAfEepromDataPrintCommand(void)
{
  uint32_t offset = emberUnsignedCommandArgument(0);
  uint8_t data[DATA_BLOCK];
  uint8_t status = emberAfPluginEepromRead(offset, data, DATA_BLOCK);

  if (status) {
    emberAfCorePrintln("Error:  Failed to read from EEPROM at 0x%4X, status: %d",
                       offset,
                       status);
    return;
  }

  emberAfPrint8ByteBlocks(DATA_BLOCK >> 3,
                          data,
                          true); // CR between blocks?
}

void emAfEepromStatusCommand(void)
{
  uint8_t i;
  emberAfCorePrintln("EEPROM Initialized: %c",
                     (emAfIsEepromInitialized()
                      ? 'y'
                      : 'n'));
  // NOTE:  Calling emberAfPluginEepromBusy() will actually initialize the
  // EEPROM, so we avoid triggering a side-effect during this 'status' command.
  emberAfCorePrintln("EEPROM Busy: %c",
                     (!emAfIsEepromInitialized()
                      ? '?'
                      : (emberAfPluginEepromBusy()
                         ? 'y'
                         : 'n')));
  emberAfCorePrintln("Partial Word Storage Count: %d",
                     EMBER_AF_PLUGIN_EEPROM_PARTIAL_WORD_STORAGE_COUNT);

  for (i = 0; i < EMBER_AF_PLUGIN_EEPROM_PARTIAL_WORD_STORAGE_COUNT; i++) {
    emberAfCorePrintln("Address: 0x%4X, Partial Word: 0x%X",
                       emAfEepromSavedPartialWrites[i].address,
                       emAfEepromSavedPartialWrites[i].data);
  }
}

void emAfEepromInfoCommand(void)
{
  const HalEepromInformationType* part = emberAfPluginEepromInfo();
  emberAfCorePrintln("\nEEPROM Info");
  if (part == NULL) {
    emberAfCorePrintln("Not available (older bootloader)");
  } else {
    uint8_t wordSize = emberAfPluginEepromGetWordSize();
    bool confirmedWordSize = false;
    if (part->version >= EEPROM_INFO_MIN_VERSION_WITH_WORD_SIZE_SUPPORT) {
      confirmedWordSize = true;
    }
    emberAfCorePrintln("Part Description:          %p", part->partDescription);
    emberAfCorePrintln("Capabilities:              0x%2X", part->capabilitiesMask);
    emberAfCorePrintln("Page Erase time (ms):      %d", part->pageEraseMs);
    emberAfCorePrintln("Page size (bytes):         %l", part->pageSize);
    emberAfCorePrintln("Part size (bytes):         %l", part->partSize);
    emberAfCorePrintln("Word size (bytes):         %d (%p)",
                       wordSize,
                       (confirmedWordSize
                        ? "confirmed"
                        : "assumed"));
  }
}
