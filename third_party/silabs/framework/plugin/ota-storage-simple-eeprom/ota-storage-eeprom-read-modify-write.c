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
 * @brief This code is intended for EEPROM devices that support read-modify-write
 * of arbitrary page sizes, and across page boundaries.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/plugin/ota-common/ota.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"
#include EMBER_AF_API_EEPROM

#define OTA_STORAGE_EEPROM_INTERNAL_HEADER
#include "ota-storage-eeprom.h"
#undef OTA_STORAGE_EEPROM_INTERNAL_HEADER

#if (EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_READ_MODIFY_WRITE_SUPPORT == TRUE)

//------------------------------------------------------------------------------
// Globals

//------------------------------------------------------------------------------

uint32_t emberAfOtaStorageDriverRetrieveLastStoredOffsetCallback(void)
{
  uint32_t offset;

  if (!emAfOtaStorageCheckDownloadMetaData()) {
    return 0;
  }

  offset = emAfOtaStorageReadInt32uFromEeprom(
    otaStorageEepromGetImageInfoStartAddress()
    + SAVED_DOWNLOAD_OFFSET_INDEX);
  if (offset == 0xFFFFFFFFL) {
    return 0;
  }
  return offset;
}

void emAfStorageEepromUpdateDownloadOffset(uint32_t offset, bool finalOffset)
{
  uint32_t oldDownloadOffset =
    emberAfOtaStorageDriverRetrieveLastStoredOffsetCallback();

  if (finalOffset
      || offset == 0
      || (offset > SAVE_RATE
          && (oldDownloadOffset + SAVE_RATE) <= offset)) {
    // The actual offset we are writing TO is the second parameter.
    // The data we are writing (first param) also happens to be an offset but
    // is not a location for the write operation in this context.
    debugFlush();
    debugPrint("Recording download offset: 0x%4X", offset);
    debugFlush();

    emAfOtaStorageWriteInt32uToEeprom(offset,
                                      otaStorageEepromGetImageInfoStartAddress()
                                      + SAVED_DOWNLOAD_OFFSET_INDEX);
    //printImageInfoStartData();
  }
}

EmberAfOtaStorageStatus emberAfOtaStorageDriverInvalidateImageCallback(void)
{
  uint8_t zeroMagicNumber[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  if (!emAfOtaStorageCheckDownloadMetaData()) {
    emAfOtaStorageWriteDownloadMetaData();
  }

  // Wipe out the magic number in the OTA file and the Header length field.
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

void emberAfPluginOtaStorageSimpleEepromPageEraseEventHandler(void)
{
  // This event should never fire.
}

void emAfOtaStorageEepromInit(void)
{
  // Older drivers do not have an EEPROM info structure that we can reference
  // so we must just assume they are okay.
  if (emberAfPluginEepromInfo() != NULL) {
    // OTA code must match the capabilities of the part.  This code
    // assumes that a page erase prior to writing data is NOT required.
    assert((emberAfPluginEepromInfo()->capabilitiesMask
            & EEPROM_CAPABILITIES_PAGE_ERASE_REQD)
           == 0);
  }
}

EmberAfOtaStorageStatus emberAfOtaStorageDriverPrepareToResumeDownloadCallback(void)
{
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

#endif // #if (EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_READ_MODIFY_WRITE_SUPPORT == TRUE)
