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
 * @brief CLI for backing up or restoring TC data to unix filesystem.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/util/util.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/trust-center-backup/trust-center-backup.h"

#include "app/framework/util/af-main.h"

#include <errno.h>

#if defined(EMBER_TEST)
  #define EMBER_AF_PLUGIN_TRUST_CENTER_BACKUP_POSIX_FILE_BACKUP_SUPPORT
#endif

#if defined(EMBER_AF_PLUGIN_TRUST_CENTER_BACKUP_POSIX_FILE_BACKUP_SUPPORT)

// *****************************************************************************
// Globals

// This is passed as an argument to emberCopyStringArgument() which only
// supports 8-bit values.
#define MAX_FILEPATH_LENGTH 255

// *****************************************************************************
// Forward Declarations

static void getFilePathFromCommandLine(uint8_t* result);

// *****************************************************************************
// Functions

void emAfTcExportCommand(void)
{
  uint8_t file[MAX_FILEPATH_LENGTH];
  getFilePathFromCommandLine(file);

  emberAfTrustCenterExportBackupToFile(file);
}

void emAfTcImportCommand(void)
{
  uint8_t file[MAX_FILEPATH_LENGTH];
  getFilePathFromCommandLine(file);

  emberAfTrustCenterImportBackupFromFile(file);
}

static void getFilePathFromCommandLine(uint8_t* result)
{
  uint8_t length = emberCopyStringArgument(0,
                                           result,
                                           MAX_FILEPATH_LENGTH,
                                           false); // leftpad?
  result[length] = '\0';
}

#endif // defined(EMBER_AF_PLUGIN_POSIX_FILE_BACKUP)
