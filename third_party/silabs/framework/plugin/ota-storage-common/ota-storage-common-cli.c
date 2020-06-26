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
 * downloading specific file.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/util/common.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/ota-common/ota.h"
#include "app/framework/plugin/ota-common/ota-cli.h"

#include "callback.h"

// client and server use same storage interface
#include "app/framework/plugin/ota-storage-common/ota-storage.h"

#if defined (EMBER_AF_PLUGIN_OTA_STORAGE_COMMON)

//------------------------------------------------------------------------------
// Globals

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginOtaStorageCommonCommands[] = {
  OTA_COMMON_COMMANDS
  { NULL },
};
#endif // EMBER_AF_GENERATE_CLI

//------------------------------------------------------------------------------
// Forward Declarations

// Create a shorter name for printing to make the code more readable.
#define otaPrintln(...) emberAfOtaBootloadClusterPrintln(__VA_ARGS__)

//------------------------------------------------------------------------------
// Functions

void emAfOtaPrintAllImages(void)
{
  uint8_t i = 0;
  EmberAfOtaImageId id = emberAfOtaStorageIteratorFirstCallback();
  while (emberAfIsOtaImageIdValid(&id)) {
    EmberAfOtaHeader header;
    EmberAfTagData tagInfo[EMBER_AF_PLUGIN_OTA_STORAGE_COMMON_MAX_TAGS_IN_OTA_FILE];
    uint16_t totalTags;
    otaPrintln("Image %d", i);
    if (EMBER_AF_OTA_STORAGE_SUCCESS
        != emberAfOtaStorageGetFullHeaderCallback(&id,
                                                  &header)) {
      otaPrintln("  ERROR: Could not get full header!");
    } else {
      otaPrintln("  Header Version: 0x%2X", header.headerVersion);
      otaPrintln("  Header Length:  %d bytes", header.headerLength);
      otaPrintln("  Field Control:  0x%2X", header.fieldControl);
      emberAfOtaBootloadClusterFlush();
      otaPrintln("  Manuf ID:       0x%2X", header.manufacturerId);
      otaPrintln("  Image Type:     0x%2X", header.imageTypeId);
      otaPrintln("  Version:        0x%4X", header.firmwareVersion);
      emberAfOtaBootloadClusterFlush();
      otaPrintln("  Zigbee Version: 0x%2X", header.zigbeeStackVersion);
      otaPrintln("  Header String:  %s", header.headerString);
      otaPrintln("  Image Size:     %l bytes", header.imageSize);
      emberAfOtaBootloadClusterFlush();
      if (headerHasSecurityCredentials(&header)) {
        otaPrintln("  Security Cred:  0x%X", header.securityCredentials);
      }
      if (headerHasUpgradeFileDest(&header)) {
        emberAfOtaBootloadClusterPrint("  Upgrade Dest:   ");
        if (header.headerVersion == OTA_HEADER_VERSION_ZIGBEE) {
          emberAfOtaBootloadClusterDebugExec(emberAfPrintBigEndianEui64(header.upgradeFileDestination.EUI64));
        } else {
          emberAfOtaBootloadClusterPrintBuffer((uint8_t *)&header.upgradeFileDestination, sizeof(header.upgradeFileDestination), true);
        }
        emberAfOtaBootloadClusterFlush();
        otaPrintln("");
      }
      if (headerHasHardwareVersions(&header)) {
        otaPrintln("  Min. HW Ver:    0x%2X", header.minimumHardwareVersion);
        otaPrintln("  Max. HW Ver:    0x%2X", header.maximumHardwareVersion);
        emberAfOtaBootloadClusterFlush();
      }
      if (EMBER_AF_OTA_STORAGE_SUCCESS == emAfOtaStorageReadAllTagInfo(&id,
                                                                       tagInfo,
                                                                       EMBER_AF_PLUGIN_OTA_STORAGE_COMMON_MAX_TAGS_IN_OTA_FILE,
                                                                       &totalTags)) {
        uint16_t i;
        otaPrintln("  Total Tags: %d", totalTags);
        for (i = 0; i < EMBER_AF_PLUGIN_OTA_STORAGE_COMMON_MAX_TAGS_IN_OTA_FILE && i < totalTags; i++) {
          otaPrintln("    Tag: 0x%2X", tagInfo[i].id);
          otaPrintln("      Length: %l", tagInfo[i].length);
        }
        emberAfOtaBootloadClusterFlush();
      } else {
        otaPrintln("Error: Could not obtain tag info from image.");
      }
    }
    id = emberAfOtaStorageIteratorNextCallback();
    i++;
  }

  otaPrintln("\n%d images in OTA storage.", emberAfOtaStorageGetCountCallback());
}

EmberAfOtaImageId emAfOtaFindImageIdByIndex(uint8_t index)
{
  uint8_t i = 0;
  EmberAfOtaImageId id = emberAfOtaStorageIteratorFirstCallback();
  do {
    if (i == index) {
      return id;
    }
    i++;
    id = emberAfOtaStorageIteratorNextCallback();
  } while (emberAfIsOtaImageIdValid(&id));

  return emberAfInvalidImageId;
}

void emAfOtaImageDelete(void)
{
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(0);
  EmberAfOtaImageId id = emAfOtaFindImageIdByIndex(index);
  EmberAfOtaStorageStatus status;

  status = emberAfOtaStorageDeleteImageCallback(&id);
  if (EMBER_AF_OTA_STORAGE_SUCCESS == status
      || EMBER_AF_OTA_STORAGE_OPERATION_IN_PROGRESS == status) {
    otaPrintln("Image delete%p.",
               (EMBER_AF_OTA_STORAGE_OPERATION_IN_PROGRESS == status
                ? " in progress"
                : "d"));
    return;
  }
  otaPrintln("Error: Failed to delete image.");
}

void emAfOtaReloadStorageDevice(void)
{
#if defined (EMBER_AF_PLUGIN_OTA_STORAGE_POSIX_FILESYSTEM)
  emAfOtaStorageClose();
#endif

  emberAfOtaStorageInitCallback();
}

#define PRINT_BLOCK_LENGTH 64

void emAfOtaStorageDataPrint(void)
{
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(0);
  uint32_t offset = emberUnsignedCommandArgument(1);
  EmberAfOtaImageId id = emAfOtaFindImageIdByIndex(index);
  uint32_t actualLength;
  uint8_t data[PRINT_BLOCK_LENGTH];
  uint8_t extraLength;

  EmberAfOtaStorageStatus status = emberAfOtaStorageReadImageDataCallback(&id,
                                                                          offset,
                                                                          PRINT_BLOCK_LENGTH,
                                                                          data,
                                                                          &actualLength);
  if (status) {
    otaPrintln("Error:  Could not read block: 0x%X.\n", status);
    return;
  }

  extraLength = actualLength % 8;
  emberAfPrint8ByteBlocks(actualLength / 8,
                          data,
                          true); // CR between blocks?
  emberAfPrintBuffer(emberAfPrintActiveArea,
                     &(data[actualLength - extraLength]),
                     extraLength,
                     true);  // with whitespace?
  emberAfPrintTextLine("");
}

#endif // defined (EMBER_AF_PLUGIN_OTA_STORAGE_COMMON)
