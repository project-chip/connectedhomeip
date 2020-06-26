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
 * @brief This file defines the interface to a Over-the-air (OTA) storage device.  It
 * can be used by either a server or client.  The server can store 0 or more
 * files that are indexed uniquely by an identifier made up of their Version
 * Number, Manufacturer ID, and Device ID.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_OTA_STORAGE_H
#define SILABS_OTA_STORAGE_H

#define OTA_MINIMUM_HEADER_LENGTH (20 + 32 + 4)
// Optional fields are: security credentials, upgrade dest, and HW versions
#define OTA_MAXIMUM_HEADER_LENGTH (OTA_MINIMUM_HEADER_LENGTH + 1 + 32 + 4)

// For EEPROM parts with 2-byte word sizes we need to make sure we read
// on word boundaries.
#define OTA_MAXIMUM_HEADER_LENGTH_2_BYTE_ALIGNED (OTA_MAXIMUM_HEADER_LENGTH + 1)
#define OTA_MAXIMUM_HEADER_LENGTH_4_BYTE_ALIGNED (OTA_MAXIMUM_HEADER_LENGTH + 3)

#define OTA_FILE_MAGIC_NUMBER        0x0BEEF11EL

#define MAGIC_NUMBER_OFFSET    0
#define HEADER_VERSION_OFFSET  4
#define HEADER_LENGTH_OFFSET   6
#define FIELD_CONTROL_OFFSET   8
#define MANUFACTURER_ID_OFFSET 10
#define IMAGE_TYPE_ID_OFFSET   12
#define VERSION_OFFSET         14
#define STACK_VERSION_OFFSET   18
#define HEADER_STRING_OFFSET   20
#define IMAGE_SIZE_OFFSET      52
#define OPTIONAL_FIELDS_OFFSET 56
// The rest are optional fields.

#define HEADER_LENGTH_FIELD_LENGTH 2

#define TAG_OVERHEAD (2 + 4)   // 2 bytes for the tag ID, 4 bytes for the length

#define isValidHeaderVersion(headerVersion) \
  ((headerVersion == OTA_HEADER_VERSION_ZIGBEE) || (headerVersion == OTA_HEADER_VERSION_THREAD))

#define SECURITY_CREDENTIAL_VERSION_FIELD_PRESENT_MASK 0x0001
#define DEVICE_SPECIFIC_FILE_PRESENT_MASK              0x0002
#define HARDWARE_VERSIONS_PRESENT_MASK                 0x0004

#define headerHasSecurityCredentials(header) \
  ((header)->fieldControl & SECURITY_CREDENTIAL_VERSION_FIELD_PRESENT_MASK)
#define headerHasUpgradeFileDest(header) \
  ((header)->fieldControl & DEVICE_SPECIFIC_FILE_PRESENT_MASK)
#define headerHasHardwareVersions(header) \
  ((header)->fieldControl & HARDWARE_VERSIONS_PRESENT_MASK)

// This size does NOT include the tag overhead.
#define SIGNATURE_TAG_DATA_SIZE       (EUI64_SIZE + EMBER_SIGNATURE_SIZE)
#define SIGNATURE_283K1_TAG_DATA_SIZE (EUI64_SIZE + EMBER_SIGNATURE_283K1_SIZE)

#define INVALID_MANUFACTURER_ID  0xFFFF
#define INVALID_DEVICE_ID        0xFFFF
#define INVALID_FIRMWARE_VERSION 0xFFFFFFFFUL
#define INVALID_EUI64 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }

#define INVALID_OTA_IMAGE_ID  \
  { INVALID_MANUFACTURER_ID,  \
    INVALID_DEVICE_ID,        \
    INVALID_FIRMWARE_VERSION, \
    INVALID_EUI64,            \
  }

#define INVALID_SLOT      (uint32_t)-1

//------------------------------------------------------------------------------

// Initialization
// (For the POSIX implementation the device will be a file or directory)
EmberAfOtaStorageStatus emAfOtaStorageSetDevice(const void* device);
void emAfOtaStorageClose(void);

const char* emAfOtaStorageGetFilepath(const EmberAfOtaImageId* id);

EmberAfOtaStorageStatus emAfOtaStorageAddImageFile(const char* filename);

// Creating (two options)
//  - Create a file based on a passed "EmberAfOtaHeader" structure, stored
//      at the filename passed to the function.  This is usually done by a
//      PC tool.
//  - Create a file based on raw data (presumably received over the air)
//      This will be stored in a single static temp file.
EmberAfOtaStorageStatus emAfOtaStorageCreateImage(EmberAfOtaHeader* header,
                                                  const char* filename);
EmberAfOtaStorageStatus emAfOtaStorageAppendImageData(const char* filename,
                                                      uint32_t length,
                                                      const uint8_t* data);

//------------------------------------------------------------------------------
// Generic routines that are independent of the actual storage mechanism.

// Will return 0 on invalid headerVersion
uint16_t emGetUpgradeFileDestinationLength(uint16_t headerVersion);

EmberAfOtaStorageStatus emAfOtaStorageGetHeaderLengthAndImageSize(const EmberAfOtaImageId* id,
                                                                  uint32_t *returnHeaderLength,
                                                                  uint32_t *returnImageSize);

EmberAfOtaStorageStatus emAfOtaStorageGetZigbeeStackVersion(const EmberAfOtaImageId* id,
                                                            uint16_t *returnZigbeeStackVersion);

EmberAfOtaImageId emAfOtaStorageGetImageIdFromHeader(const EmberAfOtaHeader* header);

// Returns the offset and size of the actual data (does not include
// tag meta-data) in the specified tag.
EmberAfOtaStorageStatus emAfOtaStorageGetTagOffsetAndSize(const EmberAfOtaImageId* id,
                                                          uint16_t tag,
                                                          uint32_t* returnTagOffset,
                                                          uint32_t* returnTagSize);

EmberAfOtaStorageStatus emAfOtaStorageGetTagOffsetsAndSizes(const EmberAfOtaImageId* id,
                                                            uint16_t tag,
                                                            uint32_t** returnTagOffset,
                                                            uint32_t** returnTagSize);

EmberAfOtaStorageStatus emAfOtaStorageGetTagDataFromImage(const EmberAfOtaImageId* id,
                                                          uint16_t tag,
                                                          uint8_t* returnData,
                                                          uint32_t* returnDataLength,
                                                          uint32_t maxReturnDataLength);

// This gets the OTA header as it is formatted in the file, including
// the magic number.
EmberAfOtaStorageStatus emAfOtaStorageGetRawHeaderData(const EmberAfOtaImageId* id,
                                                       uint8_t* returnData,
                                                       uint32_t* returnDataLength,
                                                       uint32_t maxReturnDataLength);

// This retrieves a list of all tags in the file and their lengths.
// It will read at most 'maxTags' and return that array data in tagInfo.
EmberAfOtaStorageStatus emAfOtaStorageReadAllTagInfo(const EmberAfOtaImageId* id,
                                                     EmberAfTagData* tagInfo,
                                                     uint16_t maxTags,
                                                     uint16_t* totalTags);

bool emberAfIsOtaImageIdValid(const EmberAfOtaImageId* idToCompare);

// This should be moved into the plugin callbacks file.
EmberAfOtaStorageStatus emberAfOtaStorageDeleteImageCallback(const EmberAfOtaImageId* id);

void emAfOtaStorageInfoPrint(void);
void emAfOtaStorageDriverInfoPrint(void);

uint32_t emberAfOtaStorageDriverMaxDownloadSizeCallback(void);

// This retrieves the slot being used to store OTA images if the OTA Simple
// Storage EEPROM Driver plugin has been configured to use slots
uint32_t emAfOtaStorageGetSlot(void);

//------------------------------------------------------------------------------
// Internal (for debugging malloc() and free())
int remainingAllocations(void);

#endif // SILABS_OTA_STORAGE_H
