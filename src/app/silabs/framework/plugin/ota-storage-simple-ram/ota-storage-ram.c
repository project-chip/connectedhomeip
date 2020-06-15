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
 *
 * THIS IS A TEST IMPLEMENTATION.  It defines a single static, NULL, upgrade
 * file that contains the upgrade information for a single manufacturer
 * and device ID.  The payload is a real OTA file but dummy data.
 *
 * This can serve as both the storage for the OTA client and the server.
 * The data is stored in RAM and thus is limited by the size of available
 * memory.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "callback.h"
#include "app/framework/plugin/ota-common/ota.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"

#include "app/framework/util/util.h"
#include "app/framework/util/common.h"

#include "ota-static-file-data.h"

//------------------------------------------------------------------------------
// Globals

// This is used to store both the static SERVER image, and to hold
// the temporary CLIENT image being downloaded.  It can't do both at the same
// time so a client download will wipe out the server image.
static uint8_t storage[] = STATIC_IMAGE_DATA;

static uint32_t lastDownloadOffset = STATIC_IMAGE_DATA_SIZE;

//------------------------------------------------------------------------------

bool emberAfOtaStorageDriverInitCallback(void)
{
  return true;
}

bool emberAfOtaStorageDriverReadCallback(uint32_t offset,
                                         uint32_t length,
                                         uint8_t* returnData)
{
  if ((offset + length) > STATIC_IMAGE_DATA_SIZE) {
    return false;
  }

  MEMMOVE(returnData, storage + offset, length);
  return true;
}

bool emberAfOtaStorageDriverWriteCallback(const uint8_t* dataToWrite,
                                          uint32_t offset,
                                          uint32_t length)
{
  if ((offset + length) > STATIC_IMAGE_DATA_SIZE) {
    return false;
  }

  MEMMOVE(storage + offset, dataToWrite, length);
  return true;
}

uint32_t emberAfOtaStorageDriverRetrieveLastStoredOffsetCallback(void)
{
  return lastDownloadOffset;
}

void emberAfOtaStorageDriverDownloadFinishCallback(uint32_t finalOffset)
{
  lastDownloadOffset = finalOffset;
}

void emAfOtaStorageDriverCorruptImage(uint16_t index)
{
  if (index < STATIC_IMAGE_DATA_SIZE) {
    storage[index]++;
  }
}

uint16_t emAfOtaStorageDriveGetImageSize(void)
{
  return STATIC_IMAGE_DATA_SIZE;
}

EmberAfOtaStorageStatus emberAfOtaStorageDriverInvalidateImageCallback(void)
{
  uint8_t zeroMagicNumber[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  // Wipe out the magic number in the file and the Header length field.
  // EEPROM driver requires a write of at least 8 bytes in length.
  if (!emberAfOtaStorageDriverWriteCallback(zeroMagicNumber,
                                            0,      // offset
                                            sizeof(zeroMagicNumber))) {   // length
    return EMBER_AF_OTA_STORAGE_ERROR;
  }

  // Set the offset to 0 to indicate on reboot that there is no previous image
  // to resume downloading.
  emberAfOtaStorageDriverDownloadFinishCallback(0);

  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

uint32_t emberAfOtaStorageDriverMaxDownloadSizeCallback(void)
{
  return STATIC_IMAGE_DATA_SIZE;
}

void emAfOtaStorageDriverInfoPrint(void)
{
  otaPrintln("Storage Driver:       OTA Simple Storage RAM");
  otaPrintln("Data Size (bytes):    %d", STATIC_IMAGE_DATA_SIZE);
}

EmberAfOtaStorageStatus emberAfOtaStorageDriverPrepareToResumeDownloadCallback(void)
{
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

uint32_t emAfOtaStorageGetSlot(void)
{
  return INVALID_SLOT;
}
