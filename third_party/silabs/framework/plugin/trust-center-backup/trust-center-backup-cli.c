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
 * @brief User interface for backing up and restoring a trust center.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/util/util.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/trust-center-backup/trust-center-backup.h"

#include "app/framework/util/af-main.h"

#if defined(EMBER_TEST)
  #include "app/framework/test/test-framework-security.h"
#endif

// *****************************************************************************
// Forward Declarations

void printExportDataCommand(void);
void printImportDataCommand(void);
void setExtendedPanIdCommand(void);
void restoreFromBackupCommand(void);
void importKeyCommand(void);
void importClearCommand(void);

// *****************************************************************************
// Globals

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginTrustCenterBackupCommands[] = {
#if defined(EMBER_AF_PRINT_SECURITY)
  emberCommandEntryAction("print-export", printExportDataCommand, "",
                          "Print the set of export data that a TC must backup"),
  emberCommandEntryAction("import-key", importKeyCommand, "ubb",
                          "Set a key in the import data set"),
  emberCommandEntryAction("set-ext-pan", setExtendedPanIdCommand, "b",
                          "Set the extended PAN ID in the import data set."),
  emberCommandEntryAction("print-import", printImportDataCommand, "",
                          "Print the import data set"),
  emberCommandEntryAction("clear-import", importClearCommand, "",
                          "Clear the import data set"),
  emberCommandEntryAction("restore", restoreFromBackupCommand, "",
                          "Use the import data set in a restore operation to bring the TC back online"),
#if defined(EMBER_AF_PLUGIN_TRUST_CENTER_BACKUP_POSIX_FILE_BACKUP_SUPPORT)
  emberCommandEntryAction("file-export", emAfTcExportCommand, "b",
                          "Write the TC backup data to a file"),
  emberCommandEntryAction("file-import", emAfTcImportCommand, "b",
                          "Read the TC Backup data from a file and bring the TC back online"),
#endif

#endif
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

#if defined(EMBER_SCRIPTED_TEST)
  #define EMBER_LINK_KEY_TABLE_SIZE                               TEST_FRAMEWORK_KEY_TABLE_SIZE
  #define EMBER_AF_PLUGIN_TRUST_CENTER_BACKUP_MAX_CLI_BACKUP_SIZE TEST_FRAMEWORK_KEY_TABLE_SIZE
#endif

#if EMBER_LINK_KEY_TABLE_SIZE > EMBER_AF_PLUGIN_TRUST_CENTER_BACKUP_MAX_CLI_BACKUP_SIZE
  #error EMBER_LINK_KEY_TABLE_SIZE > EMBER_AF_PLUGIN_TRUST_CENTER_BACKUP_MAX_CLI_BACKUP_SIZE
#endif

// A shorter more manageable name.
#define MAX_CLI_SIZE EMBER_AF_PLUGIN_TRUST_CENTER_BACKUP_MAX_CLI_BACKUP_SIZE

static EmberAfLinkKeyBackupData importKeyList[MAX_CLI_SIZE];
static EmberAfTrustCenterBackupData importData = {
  { 0, 0, 0, 0, 0, 0, 0, 0 }, // extended PAN ID (filled in later)
  0,                          // key list length (will be filled in later)
  MAX_CLI_SIZE,
  importKeyList
};

// *****************************************************************************
// Functions

void printBackup(const EmberAfTrustCenterBackupData* backup)
{
  uint8_t i = 0;

  emberAfSecurityPrint("Extended PAN ID: ");
  emberAfPrintIeeeLine(backup->extendedPanId);

  emberAfSecurityPrintln("Index  EUI64                Hashed Key");
  //123456 (>)0123456789ABCDEF

  for (i = 0; i < backup->keyListLength; i++) {
    emberAfSecurityFlush();
    emberAfSecurityPrint("%d      ", i);
    emberAfPrintBigEndianEui64(backup->keyList[i].deviceId);
    emberAfSecurityPrint("  ");
    emberAfPrintZigbeeKey(emberKeyContents(&(backup->keyList[i].key)));
  }
  emberAfSecurityFlush();
  emberAfSecurityPrintln("\n%d keys in backup", i);
}

void printExportDataCommand(void)
{
  EmberAfLinkKeyBackupData exportKeyList[MAX_CLI_SIZE];
  EmberAfTrustCenterBackupData export;
  EmberStatus status;

  export.maxKeyListLength = MAX_CLI_SIZE;
  export.keyList = exportKeyList;
  status = emberTrustCenterExportBackupData(&export);

  if (status != EMBER_SUCCESS) {
    emberAfSecurityPrintln("%p: Failed to get TC backup data.", "Error");
    return;
  }

  printBackup(&export);
}

void printImportDataCommand(void)
{
  printBackup(&importData);
}

void importClearCommand(void)
{
  MEMSET(&importData, 0, sizeof(EmberAfTrustCenterBackupData));
  MEMSET(importKeyList,
         0,
         sizeof(EmberAfLinkKeyBackupData) * MAX_CLI_SIZE);
  importData.keyList = importKeyList;
  importData.maxKeyListLength = MAX_CLI_SIZE;
}

void importKeyCommand(void)
{
  EmberEUI64 partnerEUI64;
  EmberKeyData newKey;
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(0);
  emberCopyBigEndianEui64Argument(1, partnerEUI64);
  emberCopyKeyArgument(2, &newKey);

  if (0 == MEMCOMPARE(emberAfNullEui64, partnerEUI64, EUI64_SIZE)) {
    emberAfSecurityPrintln("%p: EUI64 cannot be all zeroes.",
                           "Error");
    return;
  }

  if (index > MAX_CLI_SIZE) {
    emberAfSecurityPrintln("%p: index %d is greater than max import limit of %d.",
                           "Error",
                           index,
                           MAX_CLI_SIZE);
    return;
  }

  MEMMOVE(importKeyList[index].deviceId, partnerEUI64, EUI64_SIZE);
  MEMMOVE(emberKeyContents(&(importKeyList[index].key)),
          emberKeyContents(&newKey),
          EMBER_ENCRYPTION_KEY_SIZE);
  if (importData.keyListLength <= index) {
    importData.keyListLength = index + 1;
  }
}

void setExtendedPanIdCommand(void)
{
  emberCopyBigEndianEui64Argument(0, importData.extendedPanId);
}

void restoreFromBackupCommand(void)
{
  EmberStatus status;

  if (0 == MEMCOMPARE(importData.extendedPanId,
                      emberAfNullEui64,
                      EUI64_SIZE)) {
    emberAfSecurityPrintln("%p: Import contains NULL extended PAN ID",
                           "Error");
    return;
  }

  status = emberTrustCenterImportBackupAndStartNetwork(&importData);
  if (status != EMBER_SUCCESS) {
    emberAfSecurityPrintln("%p: Failed to import backup data and form network.",
                           "Error");
    return;
  }
  emberAfSecurityPrintln("Import successful.");
}
