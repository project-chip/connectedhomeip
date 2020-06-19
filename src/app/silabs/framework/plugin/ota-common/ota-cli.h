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
 * @brief Zigbee Over-the-air bootload cluster for upgrading firmware and
 * downloading specific file.  This is the CLI to interact with the
 * main cluster code.
 *******************************************************************************
   ******************************************************************************/

// Common CLI interface

void emAfOtaPrintAllImages(void);
EmberAfOtaImageId emAfOtaFindImageIdByIndex(uint8_t index);
void emAfOtaReloadStorageDevice(void);

#define OTA_COMMON_COMMANDS                                                      \
  emberCommandEntryAction("printImages", emAfOtaPrintAllImages, "", ""),         \
  emberCommandEntryAction("delete", (CommandAction)emAfOtaImageDelete, "u", ""), \
  emberCommandEntryAction("reload", emAfOtaReloadStorageDevice, "", ""),         \
  emberCommandEntryAction("storage-info", emAfOtaStorageInfoPrint, "", ""),      \
  emberCommandEntryTerminator(),                                                 \


// Client CLI interface

#if !defined (EMBER_AF_PLUGIN_OTA_CLIENT)
  #define OTA_CLIENT_COMMANDS
#endif

void otaFindServerCommand(void);
void otaQueryServerCommand(void);
void otaUsePageRequestCommand(void);
void otaQuerySpecificFileCommand(void);
void otaSendUpgradeCommand(void);
void emAfOtaImageDelete(void);

// Server CLI interface

#if !defined (EMBER_AF_PLUGIN_OTA_SERVER)
  #define OTA_SERVER_COMMANDS
#endif

void otaImageNotifyCommand(void);
