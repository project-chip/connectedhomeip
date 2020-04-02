/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_HAL
#include EMBER_AF_API_EEPROM
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE

// The design of this plugin follows the same design as the znet implementation
// the OTA Simple Storage EEPROM Driver (ota-storage-simple-eeprom) plugin. Here
// are some differences in the implementations.
// - In znet, we support both read-modify-write and page-erase implementations
//   of OTA storage in EEPROM. In thread, we will only support page-erase in
//   order to cut down on code, and satisfy the primary use case. Also in znet,
//   we laying out the OTA file two different ways - one with the EBL/GBL at the
//   beginning of EEPROM for SoC devices, and one with the beginning of the OTA
//   file at the beginning of EEPROM. In thread, we will only support the former
//   of these methods, to cut down on code.
// - The file format version will be bumped to 3, as 2 is used in znet.
// - The file contains the file specification (manufacturer code + file type
//   + file version) in the metadata section.
// - I believe this was the case for the znet implementation, but the metadata
//   fields in this implementationw will be big endian.
// - In znet, the EBL/GBL offset within the OTA file was a 4-byte integer. In
//   this implementation, it is a 2-byte integer.
// - In this implementation, we statically calculate the metadata address (see
//   METADATA_START_ADDRESS) instead of dynamically waiting to know the first
//   tag data offset. This is because the Create() API needs to write some
//   metadata to the file before we know anything about the actual OTA file.
//
// EEPROM Start                     ---------------
// Beginning of EBL/GBL file        |             |
//  (upgrade image data)            |   EBL/GBL   |
//                                  |             |
// End of EBL/GBL data              |             |
//                                  ---------------
// Other OTA image data             |             |
//  (e.g. signature)                |  Signature  |
//                                  |             |
// End of OTA image                 ---------------
//                                  |             |
// ... (unused) ...                 |             |
//                                  |             |
//                                  ---------------
// Image Metadata                   |  Metadata   |
//                                  ---------------
// OTA Header (offset 0 for the     |    OTA      |
//             OTA storage module)  |   Header    |
//  ...extra space...               |             |
// EEPROM End                       ---------------
//
// The Image Metadata contains the following data:
//   Magic Number (8 bytes)
//     [different than the OTA file magic number]
//   File Version (2 bytes)
//   File Specification (8 bytes)
//     Manufacturer Code (2 bytes)
//     File Type (2 bytes)
//     File Version (4 bytes)
//   EBL/GBL start offset within OTA file (2 bytes)
//   Saved Download Offset data (??? bytes)
//     For flash drivers without read-modify-write support, we will use
//     a byte-mask indicating the last full flash page of written data.
//     The byte-mask will have negative logic (0xFF means flash page not
//     downloaded) and requires one byte per page of the EEPROM space
//     allocated for the OTA code. The first page is considered to be the
//     combination of the portion of the OTA image at the bottom of the
//     EEPROM space (the OTA header) and the first full flash page at the
//     top of the EEPROM space (the EBL and its data).
//     In other words if the client is given 200k of download space within
//     the EEPROM, and a flash page is 4k, then we need 50 bytes for the
//     byte-mask.

// -----------------------------------------------------------------------------
// Constants

#define MAX_FILE_SIZE                                            \
  ((EMBER_AF_PLUGIN_OTA_BOOTLOAD_STORAGE_EEPROM_END_ADDRESS      \
    - EMBER_AF_PLUGIN_OTA_BOOTLOAD_STORAGE_EEPROM_START_ADDRESS) \
   + 1)

#if MAX_FILE_SIZE <= 0
  #error "EMBER_AF_PLUGIN_OTA_BOOTLOAD_STORAGE_EEPROM_END_ADDRESS must be greater than EMBER_AF_PLUGIN_OTA_BOOTLOAD_STORAGE_EEPROM_START_ADDRESS."
#endif

#define OTA_HEADER_LENGTH_INDEX 6 /* 4-byte magic number, 2-byte version */

#define TAG_OVERHEAD_SIZE 6 /* 2-byte ID, 4-byte length */

// 4-byte magic number, 2-byte version, 2-byte header length, 2-byte field
// control, 2-byte manufacturer ID, 2-byte file type, 4-byte file version,
// 2-byte stack version, 32-byte header string
#define OTA_HEADER_FILE_SIZE_INDEX 52
#define OTA_HEADER_FILE_SIZE_SIZE 4

// -------------------------------------
// File format

// This magic number contains everyone's favorite phone number "867-5309" twice,
// and then is ended with my favorite hex byte.
#define MAGIC_NUMBER_OFFSET 0
#define MAGIC_NUMBER_SIZE 8
#define MAGIC_NUMBER_BYTES { 0x86, 0x75, 0x30, 0x98, 0x67, 0x53, 0x09, 0xAC }

#define FILE_VERSION_OFFSET (MAGIC_NUMBER_OFFSET + MAGIC_NUMBER_SIZE)
#define FILE_VERSION_SIZE 2
#define FILE_VERSION_BYTES { 0x00, 0x03 }

#define FILE_SPEC_OFFSET (FILE_VERSION_OFFSET + FILE_VERSION_SIZE)
#define FILE_SPEC_SIZE 8

#define FIRST_TAG_DATA_OFFSET_OFFSET (FILE_SPEC_OFFSET + FILE_SPEC_SIZE)
#define FIRST_TAG_DATA_OFFSET_SIZE 2

#define BYTEMASK_OFFSET (FIRST_TAG_DATA_OFFSET_OFFSET + FIRST_TAG_DATA_OFFSET_SIZE)
#define BYTEMASK_SIZE 1024

#define OTA_FILE_START_OFFSET (BYTEMASK_OFFSET + BYTEMASK_SIZE)

#define MAX_OTA_FILE_SIZE (MAX_FILE_SIZE - OTA_FILE_START_OFFSET)

// We need to make sure each of these fields are aligned on a word-size boundary
// to please all EEPROM parts. The majority of EEPROM parts I have seen have a
// word size of 2, so let's go with that. We make a runtime assertion of this
// below in the InitCallback().
#define METADATA_START_ADDRESS                               \
  (((EMBER_AF_PLUGIN_OTA_BOOTLOAD_STORAGE_EEPROM_END_ADDRESS \
     - TAG_OVERHEAD_SIZE                                     \
     - EMBER_ZCL_OTA_BOOTLOAD_HEADER_MAX_SIZE                \
     - OTA_FILE_START_OFFSET)                                \
    + 1)                                                     \
   & ~1)

// -----------------------------------------------------------------------------
// Types

typedef struct {
  uint32_t address;
  uint8_t *data;
  uint16_t length;
} ReadOrWriteData_t;

// -----------------------------------------------------------------------------
// Globals

EmberEventControl emZclOtaBootloadStorageEepromEraseEventControl;

static uint32_t nextEraseAddress;

static EmberZclOtaBootloadStorageDeleteCallback currentDeleteCallback = NULL;

static uint32_t currentFileSize = UINT32_MAX;
#define HAVE_CURRENT_FILE_SIZE() (currentFileSize != UINT32_MAX)
#define ERASE_CURRENT_FILE_SIZE() (currentFileSize = UINT32_MAX)

static uint32_t totalFileSize = UINT32_MAX;
#define HAVE_TOTAL_FILE_SIZE() (totalFileSize != UINT32_MAX)
#define ERASE_TOTAL_FILE_SIZE() (totalFileSize = UINT32_MAX)

static EmberZclOtaBootloadFileSpec_t currentFileSpec = {
  .version = EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION_NULL,
};
#define HAVE_CURRENT_FILE_SPEC() (currentFileSpec.version != EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION_NULL)
#define ERASE_CURRENT_FILE_SPEC() (currentFileSpec.version = EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION_NULL)

static uint16_t currentFirstTagDataOffset = UINT16_MAX;
#define HAVE_CURRENT_FIRST_TAG_DATA_OFFSET() (currentFirstTagDataOffset != UINT16_MAX)
#define ERASE_CURRENT_FIRST_TAG_DATA_OFFSET() (currentFirstTagDataOffset = UINT16_MAX)

// -----------------------------------------------------------------------------
// Private API

// -------------------------------------
// Logging

#if 0
  #ifdef EMBER_SCRIPTED_TEST
    #define debugPrint(...) fprintf(stderr, __VA_ARGS__)
  #else
    #include EMBER_AF_API_DEBUG_PRINT
    #define debugPrint(...) emberAfCorePrintln(__VA_ARGS__)
  #endif

  #define log(read, offset, address, reallyDataLength, eepromStatus) \
  debugPrint("%s: %s(%lu/0x%08X, ..., %d) -> 0x%02X\n",              \
             emberAfPluginEepromInfo()->partDescription,             \
             ((read) ? "Read" : "Write"),                            \
             (offset),                                               \
             (address),                                              \
             (reallyDataLength),                                     \
             (eepromStatus))
#else
  #define log(read, offset, address, reallyDataLength, eepromStatus)
#endif

// -------------------------------------
// Utility operations

static EmberZclOtaBootloadStorageStatus_t readOrWrite(size_t offset,
                                                      uint8_t *data,
                                                      size_t dataLength,
                                                      bool read);

static bool fileMatchesFileSpec(const EmberZclOtaBootloadFileSpec_t *fileSpec)
{
  // Cache this file spec value so that we don't have to keep wasting time going
  // back to EEPROM!
  if (HAVE_CURRENT_FILE_SPEC()) {
    return (fileSpec == NULL
            || emberZclOtaBootloadFileSpecsAreEqual(fileSpec,
                                                    &currentFileSpec));
  }

  const uint8_t magicNumber[] = MAGIC_NUMBER_BYTES;
  const uint8_t fileVersion[] = FILE_VERSION_BYTES;
  uint8_t data[MAGIC_NUMBER_SIZE + FILE_VERSION_SIZE + FILE_SPEC_SIZE];
  uint8_t *finger = data;
  EmberZclOtaBootloadStorageStatus_t storageStatus
    = readOrWrite(MAGIC_NUMBER_OFFSET,
                  data,
                  MAGIC_NUMBER_SIZE + FILE_VERSION_SIZE + FILE_SPEC_SIZE,
                  true); // read?
  // TODO: handle me more gracefully.
  assert(storageStatus == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  if (MEMCOMPARE(finger, magicNumber, MAGIC_NUMBER_SIZE) != 0) {
    return false;
  }

  finger += MAGIC_NUMBER_SIZE;
  if (MEMCOMPARE(finger, fileVersion, FILE_VERSION_SIZE) != 0) {
    return false;
  }

  finger += FILE_VERSION_SIZE;
  emberZclOtaBootloadFetchFileSpec(finger, &currentFileSpec);
  if (fileSpec != NULL
      && !emberZclOtaBootloadFileSpecsAreEqual(fileSpec, &currentFileSpec)) {
    ERASE_CURRENT_FILE_SPEC();
    return false;
  } else {
    return true;
  }
}

static bool fileExists(void)
{
  return fileMatchesFileSpec(NULL);
}

static void getFileSpec(EmberZclOtaBootloadFileSpec_t *fileSpec)
{
  // A call to fileExists will either 1) pass and cache the file spec in
  // currentFileSpec, or 2) fail, which will cause us to crash and burn.
  assert(fileExists());
  *fileSpec = currentFileSpec;
}

static uint32_t getCurrentFileSize(void)
{
  assert(fileExists());
  if (!HAVE_CURRENT_FILE_SIZE()) {
    uint8_t bytemask[BYTEMASK_SIZE];
    EmberZclOtaBootloadStorageStatus_t storageStatus
      = readOrWrite(BYTEMASK_OFFSET, bytemask, BYTEMASK_SIZE, true); // read?
    // TODO: handle me more gracefully.
    assert(storageStatus == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);

    // Find the first byte in the bytemask that is not 0xFF. The index of this
    // first byte indicates the first unwritten page of EEPROM.
    size_t i = 0;
    for (; i < BYTEMASK_SIZE; i++) {
      if (bytemask[i] == 0xFF) {
        break;
      }
    }
    currentFileSize = (i * emberAfPluginEepromInfo()->pageSize);
  }
  return currentFileSize;
}

static void setCurrentFileSize(uint32_t size)
{
  currentFileSize = size;

  // TODO: update bytemask.
}

static uint32_t getTotalFileSize(void)
{
  if (!HAVE_TOTAL_FILE_SIZE()) {
    uint8_t fileSizeData[OTA_HEADER_FILE_SIZE_SIZE];
    assert(readOrWrite((OTA_FILE_START_OFFSET + OTA_HEADER_FILE_SIZE_INDEX),
                       fileSizeData,
                       OTA_HEADER_FILE_SIZE_SIZE,
                       true) // read?
           == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
    totalFileSize = emberFetchLowHighInt32u(fileSizeData);
  }
  return totalFileSize;
}

static uint16_t getFirstTagDataOffset(void)
{
  if (!HAVE_CURRENT_FIRST_TAG_DATA_OFFSET()) {
    uint8_t firstTagDataOffset[FIRST_TAG_DATA_OFFSET_SIZE];
    EmberZclOtaBootloadStorageStatus_t storageStatus
      = readOrWrite(FIRST_TAG_DATA_OFFSET_OFFSET,
                    firstTagDataOffset,
                    FIRST_TAG_DATA_OFFSET_SIZE,
                    true); // read?
    // TODO: handle me more gracefully.
    assert(storageStatus == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
    currentFirstTagDataOffset = emberFetchHighLowInt16u(firstTagDataOffset);
  }
  return currentFirstTagDataOffset;
}

static bool setFirstTagDataOffset(uint8_t *headerData, uint16_t headerDataLength)
{
  // We must be able to get the header length, which means that we must be
  // passed at least the number of bytes in order to read the header length from
  // an OTA header.
  assert(headerDataLength >= OTA_HEADER_LENGTH_INDEX);
  uint16_t headerLength
    = emberFetchLowHighInt16u(headerData + OTA_HEADER_LENGTH_INDEX);

  uint8_t firstTagDataOffset[FIRST_TAG_DATA_OFFSET_SIZE];
  currentFirstTagDataOffset = (OTA_FILE_START_OFFSET
                               + headerLength
                               + TAG_OVERHEAD_SIZE);
  emberStoreHighLowInt16u(firstTagDataOffset, currentFirstTagDataOffset);
  return (readOrWrite(FIRST_TAG_DATA_OFFSET_OFFSET,
                      firstTagDataOffset,
                      FIRST_TAG_DATA_OFFSET_SIZE,
                      false) // read?
          == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
}

static size_t makeReadOrWriteData(size_t offset,
                                  uint8_t *data,
                                  uint16_t dataLength,
                                  ReadOrWriteData_t returnedReadOrWriteData[2])
{
  if (offset < OTA_FILE_START_OFFSET) {
    // We are only writing metadata, so this should never wrap around to the
    // beginning of the EEPROM part. Adding this if conditional block first also
    // avoids the follwing call chain cycle.
    //   readOrWrite
    //   -> makeReadOrWriteData
    //   -> getFirstTagDataOffset
    //   -> readOrWrite
    assert(offset + dataLength <= OTA_FILE_START_OFFSET);
    returnedReadOrWriteData[0].address = METADATA_START_ADDRESS + offset;
    returnedReadOrWriteData[0].data = data;
    returnedReadOrWriteData[0].length = dataLength;
    return 1;
  }

  const uint16_t firstTagDataOffset = getFirstTagDataOffset();
  if (offset < firstTagDataOffset) {
    if (offset + dataLength > firstTagDataOffset) {
      returnedReadOrWriteData[0].address = METADATA_START_ADDRESS + offset;
      returnedReadOrWriteData[0].data = data;
      returnedReadOrWriteData[0].length = firstTagDataOffset - offset;
      returnedReadOrWriteData[1].address
        = EMBER_AF_PLUGIN_OTA_BOOTLOAD_STORAGE_EEPROM_START_ADDRESS;
      returnedReadOrWriteData[1].data
        = data + returnedReadOrWriteData[0].length;
      returnedReadOrWriteData[1].length
        = dataLength - returnedReadOrWriteData[0].length;
      return 2;
    } else {
      returnedReadOrWriteData[0].address = METADATA_START_ADDRESS + offset;
      returnedReadOrWriteData[0].data = data;
      returnedReadOrWriteData[0].length = dataLength;
      return 1;
    }
  } else {
    returnedReadOrWriteData[0].address
      = (offset
         - firstTagDataOffset
         + EMBER_AF_PLUGIN_OTA_BOOTLOAD_STORAGE_EEPROM_START_ADDRESS);
    returnedReadOrWriteData[0].data = data;
    returnedReadOrWriteData[0].length = dataLength;
    return 1;
  }
}

static EmberZclOtaBootloadStorageStatus_t readOrWrite(size_t offset,
                                                      uint8_t *data,
                                                      size_t dataLength,
                                                      bool read)
{
  // The EEPROM plugin only accepts 16-bit length values.
  assert(dataLength <= UINT16_MAX);
  uint16_t reallyDataLength = (0xFFFF & dataLength);

  if (!read
      && offset == OTA_FILE_START_OFFSET
      && !setFirstTagDataOffset(data, reallyDataLength)) {
    return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_FAILED;
  }

  ReadOrWriteData_t readOrWriteData[2];
  size_t readOrWriteDataCount = makeReadOrWriteData(offset,
                                                    data,
                                                    reallyDataLength,
                                                    readOrWriteData);
  for (size_t i = 0; i < readOrWriteDataCount; i++) {
    uint8_t eepromStatus
      = (read
         ? emberAfPluginEepromRead(readOrWriteData[i].address,
                                   readOrWriteData[i].data,
                                   readOrWriteData[i].length)
         : emberAfPluginEepromWrite(readOrWriteData[i].address,
                                    readOrWriteData[i].data,
                                    readOrWriteData[i].length));
    log(read,
        offset,
        readOrWriteData[i].address,
        readOrWriteData[i].length,
        eepromStatus);

    if (eepromStatus != EEPROM_SUCCESS) {
      return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_FAILED;
    }
  }

  // If this is the last write, then we flush the EEPROM partial words. There
  // could be partial words left at the end of the EEPROM address space, or
  // somewhere in the middle where the OTA file ends.
  if (!read
      && offset >= OTA_FILE_START_OFFSET
      && (((offset - OTA_FILE_START_OFFSET) + reallyDataLength)
          == getTotalFileSize())
      && emberAfPluginEepromFlushSavedPartialWrites() != EEPROM_SUCCESS) {
    return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_FAILED;
  }

  return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS;
}

// -------------------------------------
// Erase event

void emZclOtaBootloadStorageEepromEraseEventHandler(void)
{
  assert(currentDeleteCallback != NULL);

  uint32_t pageSize = emberAfPluginEepromInfo()->pageSize;
  uint32_t pageEraseMs = emberAfPluginEepromInfo()->pageEraseMs;
  if (emberAfPluginEepromBusy()) {
    emberEventControlSetDelayMS(emZclOtaBootloadStorageEepromEraseEventControl,
                                pageEraseMs);
  } else if (nextEraseAddress
             >= EMBER_AF_PLUGIN_OTA_BOOTLOAD_STORAGE_EEPROM_END_ADDRESS) {
    // Done!
    (*currentDeleteCallback)(EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
    currentDeleteCallback = NULL;
    emberEventControlSetInactive(emZclOtaBootloadStorageEepromEraseEventControl);
  } else {
    uint8_t eepromStatus = emberAfPluginEepromErase(nextEraseAddress, pageSize);
    if (eepromStatus != EEPROM_SUCCESS) {
      (*currentDeleteCallback)(EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_FAILED);
      currentDeleteCallback = NULL;
    } else {
      nextEraseAddress += pageSize;
      emberEventControlSetDelayMS(emZclOtaBootloadStorageEepromEraseEventControl,
                                  pageEraseMs);
    }
  }
}

// -----------------------------------------------------------------------------
// Public API

void emZclOtaBootloadStorageEepromInitCallback(void)
{
  // Here is some basic validation of the external EPROM. This is difficult to
  // do at generation-time (AppBuilder) or compile-time, so we do it at runtime.
  // We make the assumption about word size since we assume that our metadata is
  // living under 2-byte alignment. Since 2-byte alignment works, then we know
  // 1-byte alignment will also work. If we work with different alignment than
  // those two options, then we need to update the metadata fields to start on
  // word boundaries and update this assumption here. For older EEPROM parts, we
  // may not have this EEPROM info, so we can't do any validation.
  const HalEepromInformationType *eepromInfo = emberAfPluginEepromInfo();
  if (eepromInfo != NULL) {
    assert(READBITS(eepromInfo->capabilitiesMask,
                    EEPROM_CAPABILITIES_ERASE_SUPPORTED));
    assert(MAX_FILE_SIZE <= eepromInfo->pageSize * BYTEMASK_SIZE);
    assert(eepromInfo->wordSizeBytes == 1
           || eepromInfo->wordSizeBytes == 2);
  }
}

void emberZclOtaBootloadStorageGetInfo(EmberZclOtaBootloadStorageInfo_t *info,
                                       EmberZclOtaBootloadFileSpec_t *returnedFiles,
                                       size_t returnedFilesMaxCount)
{
  // Make sure to leave enough room for our file metadata in the maximum file
  // size value!
  info->maximumFileSize = MAX_OTA_FILE_SIZE;
  info->fileCount = (fileExists() ? 1 : 0);
  if (returnedFiles != NULL
      && returnedFilesMaxCount > 0
      && info->fileCount > 0) {
    getFileSpec(returnedFiles);
  }
}

EmberZclOtaBootloadStorageStatus_t emberZclOtaBootloadStorageFind(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                                  EmberZclOtaBootloadStorageFileInfo_t *fileInfo)
{
  if (!fileMatchesFileSpec(fileSpec)) {
    return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
  }

  fileInfo->size = getCurrentFileSize();

  return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS;
}

EmberZclOtaBootloadStorageStatus_t emberZclOtaBootloadStorageCreate(const EmberZclOtaBootloadFileSpec_t *fileSpec)
{
  if (fileMatchesFileSpec(fileSpec)) {
    return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
  }

  if (fileExists()) {
    return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_SPACE;
  }

  const uint8_t magicNumber[] = MAGIC_NUMBER_BYTES;
  const uint8_t fileVersion[] = FILE_VERSION_BYTES;
  uint8_t data[MAGIC_NUMBER_SIZE + FILE_VERSION_SIZE + FILE_SPEC_SIZE];
  uint8_t *finger = data;
  MEMMOVE(finger, magicNumber, MAGIC_NUMBER_SIZE);
  finger += MAGIC_NUMBER_SIZE;
  MEMMOVE(finger, fileVersion, FILE_VERSION_SIZE);
  finger += FILE_VERSION_SIZE;
  emberZclOtaBootloadStoreFileSpec(fileSpec, finger);

  return readOrWrite(MAGIC_NUMBER_OFFSET,
                     data,
                     MAGIC_NUMBER_SIZE + FILE_VERSION_SIZE + FILE_SPEC_SIZE,
                     false); // read?
}

EmberZclOtaBootloadStorageStatus_t emberZclOtaBootloadStorageRead(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                                  size_t offset,
                                                                  void *data,
                                                                  size_t dataLength)
{
  if (!fileMatchesFileSpec(fileSpec)) {
    return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
  }

  if ((offset + dataLength) > getCurrentFileSize()) {
    return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_RANGE;
  }

  return readOrWrite(offset + OTA_FILE_START_OFFSET,
                     data,
                     dataLength,
                     true); // read?
}

EmberZclOtaBootloadStorageStatus_t emberZclOtaBootloadStorageWrite(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                                   size_t offset,
                                                                   const void *data,
                                                                   size_t dataLength)
{
  if (!fileMatchesFileSpec(fileSpec)) {
    return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
  }

  if (offset != getCurrentFileSize()
      || (offset + dataLength) > MAX_OTA_FILE_SIZE) {
    return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_RANGE;
  }

  EmberZclOtaBootloadStorageStatus_t storageStatus
    = readOrWrite(offset + OTA_FILE_START_OFFSET,
                  (uint8_t *)data,
                  dataLength,
                  false); // read?
  if (storageStatus == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
    setCurrentFileSize(getCurrentFileSize() + dataLength);
  }

  return storageStatus;
}

EmberZclOtaBootloadStorageStatus_t emberZclOtaBootloadStorageDelete(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                                    EmberZclOtaBootloadStorageDeleteCallback callback)
{
  if (emberZclOtaBootloadFileSpecsAreEqual(fileSpec,
                                           &emberZclOtaBootloadFileSpecNull)) {
    // If we were told to delete all of our files, and no files exist, then we
    // are done here. If not, we go on and erase the one and only image.
    if (!fileExists()) {
      (*callback)(EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
      return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS;
    }
  } else {
    if (!fileMatchesFileSpec(fileSpec)) {
      return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
    }
  }

  currentDeleteCallback = callback;
  nextEraseAddress = EMBER_AF_PLUGIN_OTA_BOOTLOAD_STORAGE_EEPROM_START_ADDRESS;
  ERASE_CURRENT_FILE_SIZE();
  ERASE_TOTAL_FILE_SIZE();
  ERASE_CURRENT_FILE_SPEC();
  ERASE_CURRENT_FIRST_TAG_DATA_OFFSET();
  emberEventControlSetActive(emZclOtaBootloadStorageEepromEraseEventControl);

  return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS;
}
