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
 * @brief Bootload specific commands
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/ota-common/ota.h"

#include "hal/micro/bootloader-interface.h"

//------------------------------------------------------------------------------
// Forward Declarations

void printBootloaderInfoCommand(void);

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginOtaBootloadCommands[] = {
  emberCommandEntryAction("info", printBootloaderInfoCommand, "",
                          "Print information about the installed bootloader."),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

//------------------------------------------------------------------------------

void printBootloaderInfoCommand(void)
{
#if !defined(EZSP_HOST)
  UNUSED BlExtendedType blExtendedType = halBootloaderGetInstalledType();
  uint32_t getEmberVersion;
  uint32_t customVersion;
  uint8_t keyData[EMBER_ENCRYPTION_KEY_SIZE];
  halGetExtendedBootloaderVersion(&getEmberVersion, &customVersion);
  otaPrintln("Installed Type (Base):      0x%X", halBootloaderGetType());
  otaPrintln("Installed Type (Extended):  0x%2X", blExtendedType);
  otaPrintln("Bootloader Version:         0x%2X", halGetBootloaderVersion());
  otaPrintln("Bootloader Version, Ember:  0x%4X", getEmberVersion);
  otaPrintln("Bootloader Version, Custom: 0x%4X", customVersion);

#if defined(EMBER_TEST)
  MEMSET(keyData, 0xFF, EMBER_ENCRYPTION_KEY_SIZE);
#else
  halCommonGetToken(keyData, TOKEN_MFG_SECURE_BOOTLOADER_KEY);
#endif

  otaPrint("Secure Bootloader Key:      ");
  emberAfPrintZigbeeKey(keyData);
  otaPrintln("");

#else
  otaPrintln("Unsupported on EZSP Host");
#endif
}
