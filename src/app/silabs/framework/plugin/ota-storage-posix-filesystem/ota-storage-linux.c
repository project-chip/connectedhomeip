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
 * @brief Over The Air [Cluster] Upgrade Image Storage
 *******************************************************************************
   ******************************************************************************/

// this file contains all the common includes for clusters in the util
#include "app/framework/include/af.h"
#include "app/framework/util/common.h"

#include "app/framework/plugin/ota-common/ota.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"

#include "ota-storage-linux.h"

#if defined (IMAGE_BUILDER)
// For our PC tool, we use a simpler #define to turn on this code.
  #define GATEWAY_APP
#endif

#if defined(GATEWAY_APP)
// These includes are wrapped inside the #ifdef because they are platform
// specific.

#define _GNU_SOURCE     // strnlen
#include <string.h>     // ""

#include <stdio.h>      // fopen, fread, fwrite, fclose, fseek, fprintf
#include <sys/types.h>  // stat
#include <sys/stat.h>   // ""
#include <unistd.h>     // ""
#include <stdarg.h>     // va_start, va_end

#include <stdlib.h>     // malloc, free
#include <errno.h>      // errno, strerror

#include <dirent.h>     // opendir, readdir

#ifdef __APPLE__
#define strnlen(string, n) strlen((string))
#endif

//------------------------------------------------------------------------------
// Globals

// This will always end with a '/'.  The code will append one if not present.
static char* storageDevice = NULL;
static bool storageDeviceIsDirectory = false;
static char* tempStorageFilepath  = NULL;

static const char* tempStorageFile = "temporary-storage.ota";

typedef struct {
  EmberAfOtaHeader* header;
  char* filepath;
  const char* filenameStart;  // ptr to data in 'filepath'
  struct OtaImage* next;
  struct OtaImage* prev;
  off_t fileSize;
} OtaImage;

static OtaImage* imageListFirst = NULL;
static OtaImage* imageListLast = NULL;
static uint8_t imageCount = 0;

#define OTA_MAX_FILENAME_LENGTH 1000

static const uint8_t otaFileMagicNumberBytes[] = {
  (uint8_t)(OTA_FILE_MAGIC_NUMBER),
  (uint8_t)(OTA_FILE_MAGIC_NUMBER >> 8),
  (uint8_t)(OTA_FILE_MAGIC_NUMBER >> 16),
  (uint8_t)(OTA_FILE_MAGIC_NUMBER >> 24),
};

#define ALWAYS_PRESENT_MASK 0xFFFF

enum FieldType{
  INVALID_FIELD    = 0,
  INTEGER_FIELD    = 1,
  BYTE_ARRAY_FIELD = 2,
  STRING_FIELD     = 3,
};

typedef struct {
  const char* name;
  const enum FieldType type;
  const uint16_t length;
  const uint16_t maskForOptionalField;
} EmberAfOtaHeaderFieldDefinition;

// This global is used to define what is in the OTA header and help map that
// to the 'EmberAfOtaHeader' data structure. Must use
// emGetOtaHeaderFieldDefinition() to get the field definitions. Do not use this
// array directly.
const static EmberAfOtaHeaderFieldDefinition otaHeaderFieldDefinitions[] = {
  { "Magic Number", INTEGER_FIELD, 4, ALWAYS_PRESENT_MASK },
  { "Header Version", INTEGER_FIELD, 2, ALWAYS_PRESENT_MASK },
  // The fields returned above SHALL not depend on otaHeaderVersion, because
  // when reading in the header, we must be able to determine the location of
  // the header version
  { "Header Length", INTEGER_FIELD, 2, ALWAYS_PRESENT_MASK },
  { "Field Control", INTEGER_FIELD, 2, ALWAYS_PRESENT_MASK },
  { "Manufacturer ID", INTEGER_FIELD, 2, ALWAYS_PRESENT_MASK },
  { "Image Type", INTEGER_FIELD, 2, ALWAYS_PRESENT_MASK },
  { "Firmware Version", INTEGER_FIELD, 4, ALWAYS_PRESENT_MASK },
  { "Zigbee Stack Version", INTEGER_FIELD, 2, ALWAYS_PRESENT_MASK },
  { "Header String", STRING_FIELD, 32, ALWAYS_PRESENT_MASK },
  { "Total Image Size", INTEGER_FIELD, 4, ALWAYS_PRESENT_MASK },
  { "Security Credentials", INTEGER_FIELD, 1, SECURITY_CREDENTIAL_VERSION_FIELD_PRESENT_MASK },
  { "Upgrade File Destination", BYTE_ARRAY_FIELD, EUI64_SIZE, DEVICE_SPECIFIC_FILE_PRESENT_MASK },
  { "Minimum Hardware Version", INTEGER_FIELD, 2, HARDWARE_VERSIONS_PRESENT_MASK },
  { "Maximum Hardware Version", INTEGER_FIELD, 2, HARDWARE_VERSIONS_PRESENT_MASK },

  // Tag Data information omitted

  { NULL, INVALID_FIELD, 0, 0 },
};
typedef enum {
  // Don't define all index numbers so the compiler
  // automatically numbers them.  We just define the first.
  MAGIC_NUMBER_INDEX = 0,
  HEADER_VERSION_INDEX,
  HEADER_LENGTH_INDEX,
  FIELD_CONTROL_INDEX,
  MANUFACTURER_ID_INDEX,
  IMAGE_TYPE_INDEX,
  FIRMWARE_VERSION_INDEX,
  ZIGBEE_STACK_VERSION_INDEX,
  HEADER_STRING_INDEX,
  TOTAL_IMAGE_SIZE_INDEX,
  SECURITY_CREDENTIALS_INDEX,
  UPGRADE_FILE_DESTINATION_INDEX,
  MINIMUM_HARDWARE_VERSION_INDEX,
  MAXIMUM_HARDWARE_VERSION_INDEX,

  // This defines the maximum number of entries and refers to the invalid entry
  INVALID_FIELD_INDEX,
} EmberAfOtaBootloadFileHeaderFieldIndex_t;
#define FIELD_INDEX_MAX INVALID_FIELD_INDEX

typedef struct {
  void* location;
  bool found;
} EmberAfOtaHeaderFieldLocation;

static EmberAfOtaHeaderFieldLocation otaHeaderFieldLocations[FIELD_INDEX_MAX];

#define MAX_FILEPATH_LENGTH 1024

static OtaImage* iterator = NULL;

#if defined(WIN32)
  #define portableMkdir(x) _mkdir(x)
#else
  #define portableMkdir(x) \
  mkdir((x), S_IRUSR | S_IWUSR | S_IXUSR)   /* permissions (o=rwx) */
#endif

static EmAfOtaStorageLinuxConfig config = {
  false,  // memoryDebug
  false,  // fileDebug
  false,  // fieldDebug
  true,   // ignoreFilesWithUnderscorePrefix
  true,   // printFileDiscoveryOrRemoval
  NULL,   // fileAddedHandler
};

const char* messagePrefix = NULL;  // prefix for all printed messages

// Debug
static int allocations = 0;

#define APPEND_OFFSET (0xFFFFFFFFL)

//------------------------------------------------------------------------------
// Forward Declarations

static EmberAfOtaStorageStatus createDefaultStorageDirectory(void);
static EmberAfOtaStorageStatus initImageDirectory(void);
static OtaImage* addImageFileToList(const char* filename,
                                    bool printImageInfo);
static OtaImage* findImageById(const EmberAfOtaImageId* id);
static void freeOtaImage(OtaImage* image);
static void mapHeaderFieldDefinitionToDataStruct(EmberAfOtaHeader* header);
static void unmapHeaderFieldDefinitions(void);
static EmberAfOtaStorageStatus readHeaderDataFromBuffer(EmberAfOtaBootloadFileHeaderFieldIndex_t fieldIndex,
                                                        uint16_t headerVersion,
                                                        uint8_t* bufferPtr,
                                                        int32_t headerLengthRemaining,
                                                        int32_t actualBufferDataRemaining);
static uint16_t calculateOtaFileHeaderLength(EmberAfOtaHeader* header);
static uint8_t* writeHeaderDataToBuffer(EmberAfOtaBootloadFileHeaderFieldIndex_t fieldIndex,
                                        uint16_t headerVersion,
                                        uint8_t* bufferPtr);
const EmberAfOtaHeaderFieldDefinition *emGetOtaHeaderFieldDefinition(uint16_t headerVersion,
                                                                     EmberAfOtaBootloadFileHeaderFieldIndex_t headerIndex);
static EmberAfOtaHeader* readImageHeader(const char* filename);
static OtaImage* imageSearchInternal(const EmberAfOtaImageId* id);
static EmberAfOtaImageId getIteratorImageId(void);
static EmberAfOtaStorageStatus writeRawData(uint32_t offset,
                                            const char* filepath,
                                            uint32_t length,
                                            const uint8_t* data);
static OtaImage* findImageByFilename(const char* tempFilepath);
static void removeImage(OtaImage* image);

static void* myMalloc(size_t size, const char* allocName);
static void myFree(void* ptr);
static void error(const char* formatString, ...);
static void note(const char* formatString, ...);
static void debug(bool debugOn, const char* formatString, ...);

static const char defaultStorageDirectory[] = "ota-files";
static bool initDone = false;
static bool createStorageDirectory = true;

//==============================================================================
// Public API
//==============================================================================

// Our storage device examines all files in the passed directory, or simply
// loads a single file into its header cache.
EmberAfOtaStorageStatus emAfOtaSetStorageDevice(const void* device)
{
  if (initDone) {
    return EMBER_AF_OTA_STORAGE_ERROR;
  }

  if (device == NULL) {
    // We interpet this to mean, don't use a storage directory.
    // This option is useful when creating files via the PC
    // tool.
    createStorageDirectory = false;
    return EMBER_AF_OTA_STORAGE_SUCCESS;
  }

  const char* directoryOrFile = (const char*)device;
  int length = strnlen(directoryOrFile, MAX_FILEPATH_LENGTH + 1);
  if (MAX_FILEPATH_LENGTH < length) {
    error("Storage directory path too long (max = %d)!\n",
          MAX_FILEPATH_LENGTH);
    return EMBER_AF_OTA_STORAGE_ERROR;
  }

  // Add 1 for '\0' and 1 for '/'.  This may or may not be necessary
  // because the path already has it or it is only a file.
  storageDevice = myMalloc(length + 2,
                           "emAfSetStorageDevice(): storageDevice");
  if (storageDevice == NULL) {
    error("Could not allocate %d bytes!\n", length);
    return EMBER_AF_OTA_STORAGE_ERROR;
  }
  if (directoryOrFile[length - 1] == '/') {
    // We don't want to copy the '/' yet, since stat() will
    // complain if we pass it in.
    length--;
  }
  MEMSET(storageDevice, 0, length + 2);
  strncpy(storageDevice, directoryOrFile, length);

  struct stat statInfo;
  int returnValue = stat(storageDevice, &statInfo);
  debug(config.fileDebug,
        "Checking for existence of '%s'\n",
        storageDevice);
  if (returnValue != 0) {
    error("Could not read storage device '%s'. %s\n",
          directoryOrFile,
          strerror(errno));
    myFree(storageDevice);
    storageDevice = NULL;
    return EMBER_AF_OTA_STORAGE_ERROR;
  }

  if (S_ISDIR(statInfo.st_mode)) {
    storageDeviceIsDirectory = true;
    storageDevice[length] = '/';
  }
  debug(config.fileDebug, "Storage device set to '%s'.\n", storageDevice);
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

void emAfOtaStorageGetConfig(EmAfOtaStorageLinuxConfig* currentConfig)
{
  memcpy(currentConfig, &config, sizeof(EmAfOtaStorageLinuxConfig));
}

void emAfOtaStorageSetConfig(const EmAfOtaStorageLinuxConfig* newConfig)
{
  memcpy(&config, newConfig, sizeof(EmAfOtaStorageLinuxConfig));
}

EmberAfOtaStorageStatus emberAfOtaStorageInitCallback(void)
{
  if (initDone) {
    return EMBER_AF_OTA_STORAGE_SUCCESS;
  }

  if (config.fileDebug) {
    char cwd[MAX_FILEPATH_LENGTH];
    debug(config.fileDebug, "Current Working Directory: %s\n",
          (NULL == getcwd(cwd, MAX_FILEPATH_LENGTH)
           ? "UNKNOWN!"
           : cwd));
  }

  EmberAfOtaStorageStatus status;

  if (storageDevice == NULL) {
    if (createStorageDirectory) {
      status = createDefaultStorageDirectory();
      if (status != EMBER_AF_OTA_STORAGE_SUCCESS) {
        return status;
      }
    } else {
      return EMBER_AF_OTA_STORAGE_SUCCESS;
    }
  }

  if (storageDeviceIsDirectory) {
    status = initImageDirectory();
  } else {
    OtaImage* newImage = addImageFileToList(storageDevice, true);
    if (config.fileAddedHandler != NULL
        && newImage != NULL) {
      (config.fileAddedHandler)(newImage->header);
    }
    status = (NULL == newImage
              ? EMBER_AF_OTA_STORAGE_ERROR
              : EMBER_AF_OTA_STORAGE_SUCCESS);
  }

  iterator = NULL; // Must be initialized via
                   // otaStorageIteratorReset()

  if (status == EMBER_AF_OTA_STORAGE_SUCCESS) {
    initDone = true;
  }

  return status;
}

void emAfOtaStorageClose(void)
{
  OtaImage* ptr = imageListLast;
  while (ptr != NULL) {
    OtaImage* current = ptr;
    ptr = (OtaImage*)ptr->prev;
    freeOtaImage(current);
  }
  imageListLast = NULL;
  imageListFirst = NULL;

  if (storageDevice != NULL) {
    myFree(storageDevice);
    storageDevice = NULL;
  }
  if (tempStorageFilepath != NULL) {
    myFree(tempStorageFilepath);
    tempStorageFilepath = NULL;
  }
  initDone = false;
  imageCount = 0;
}

uint8_t emberAfOtaStorageGetCountCallback(void)
{
  return imageCount;
}

EmberAfOtaImageId emberAfOtaStorageSearchCallback(uint16_t manufacturerId,
                                                  uint16_t manufacturerDeviceId,
                                                  const uint16_t* hardwareVersion)
{
  EmberAfOtaImageId id = {
    manufacturerId,
    manufacturerDeviceId,
    INVALID_FIRMWARE_VERSION,
    INVALID_EUI64,
  };
  OtaImage* image = imageSearchInternal(&id);
  if (image == NULL) {
    return emberAfInvalidImageId;
  }
  // We assume that if there is no hardware version information in the header
  // then it can be used on ANY hardware version.  The behavior of the server
  // in this case is not spelled out by the specification.
  if (hardwareVersion != NULL
      && headerHasHardwareVersions(image->header)
      && !(*hardwareVersion >= image->header->minimumHardwareVersion
           && *hardwareVersion <= image->header->maximumHardwareVersion)) {
    return emberAfInvalidImageId;
  }
  id.firmwareVersion = image->header->firmwareVersion;
  return id;
}

const char* emAfOtaStorageGetFilepath(const EmberAfOtaImageId* id)
{
  OtaImage* image = findImageById(id);
  if (image == NULL) {
    return NULL;
  }
  return image->filepath;
}

EmberAfOtaImageId emberAfOtaStorageIteratorFirstCallback(void)
{
  iterator = imageListFirst;
  return getIteratorImageId();
}

EmberAfOtaImageId emberAfOtaStorageIteratorNextCallback(void)
{
  if (iterator != NULL) {
    iterator = (OtaImage*)iterator->next;
  }
  return getIteratorImageId();
}

EmberAfOtaStorageStatus emberAfOtaStorageReadImageDataCallback(const EmberAfOtaImageId* id,
                                                               uint32_t offset,
                                                               uint32_t length,
                                                               uint8_t* returnData,
                                                               uint32_t* returnedLength)
{
  OtaImage* image = imageSearchInternal(id);
  if (image == NULL) {
    error("No such Image (Mfg ID: 0x%04X, Image ID: 0x%04X, Version: 0x%08X\n",
          id->manufacturerId,
          id->imageTypeId,
          id->firmwareVersion);
    return EMBER_AF_OTA_STORAGE_ERROR;
  }

  // Windows requires the 'b' (binary) as part of the mode so that line endings
  // are not truncated.  POSIX ignores this.
  FILE* fileHandle = fopen(image->filepath, "rb");
  if (fileHandle == NULL) {
    error("Failed to open file '%s' for reading: %s\n",
          image->filenameStart,
          strerror(errno));
    return EMBER_AF_OTA_STORAGE_ERROR;
  }
  if (0 != fseek(fileHandle, offset, SEEK_SET)) {
    error("Failed to seek in file '%s' to offset %d\n",
          image->filenameStart,
          offset);
    fclose(fileHandle);
    return EMBER_AF_OTA_STORAGE_ERROR;
  }
  *returnedLength = fread(returnData, 1, length, fileHandle);
  fclose(fileHandle);
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

EmberAfOtaStorageStatus emAfOtaStorageCreateImage(EmberAfOtaHeader* header,
                                                  const char* filename)
{
  // Windows requires the 'b' (binary) as part of the mode so that line endings
  // are not truncated.  POSIX ignores this.
  FILE* fileHandle = fopen(filename, "wb");
  if (fileHandle == NULL) {
    error("Could not open file '%s' for writing: %s\n",
          filename,
          strerror(errno));
    return EMBER_AF_OTA_STORAGE_ERROR;
  }

  uint8_t buffer[OTA_MAXIMUM_HEADER_LENGTH];
  uint8_t* bufferPtr = buffer;

  memcpy(bufferPtr, otaFileMagicNumberBytes, 4);
  bufferPtr += 4;
  bufferPtr[0] = (uint8_t)(header->headerVersion);
  bufferPtr[1] = (uint8_t)(header->headerVersion >> 8);
  bufferPtr += 2;

  mapHeaderFieldDefinitionToDataStruct(header);
  header->headerLength = calculateOtaFileHeaderLength(header);
  header->imageSize += header->headerLength;

  EmberAfOtaBootloadFileHeaderFieldIndex_t fieldIndex = HEADER_LENGTH_INDEX;
  while (fieldIndex < FIELD_INDEX_MAX) {
    const EmberAfOtaHeaderFieldDefinition *definition = emGetOtaHeaderFieldDefinition(header->headerVersion, fieldIndex);
    debug(config.memoryDebug,
          "Writing Header Field: %s, bufferPtr: 0x%08X\n",
          definition->name,
          (unsigned int)bufferPtr);

    bufferPtr = writeHeaderDataToBuffer(fieldIndex,
                                        header->headerVersion,
                                        bufferPtr);

    // I have run into stack corruption bugs in the past so this
    // is to try and catch any new issues.
    assert(bufferPtr - buffer <= OTA_MAXIMUM_HEADER_LENGTH);

    fieldIndex++;
  }
  unmapHeaderFieldDefinitions();

  size_t written = fwrite(buffer, 1, header->headerLength, fileHandle);
  if (written != header->headerLength) {
    error("Wrote only %d bytes but expected to write %d.\n",
          written,
          header->headerLength);
    fclose(fileHandle);
    return EMBER_AF_OTA_STORAGE_ERROR;
  }
  fflush(fileHandle);
  fclose(fileHandle);

  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

EmberAfOtaStorageStatus emAfOtaStorageAddImageFile(const char* filename)
{
  return (NULL == addImageFileToList(filename,
                                     false)    // print image info?
          ? EMBER_AF_OTA_STORAGE_ERROR
          : EMBER_AF_OTA_STORAGE_SUCCESS);
}

EmberAfOtaStorageStatus emberAfOtaStorageDeleteImageCallback(const EmberAfOtaImageId* id)
{
  OtaImage* image = imageSearchInternal(id);

  // EMAPPFWKV2-1169: if we find the image, we delete it and return success.
  // If we don't have the image, then we are ready to receive another one,
  // so we should return success.

  if (image == NULL) {
    if (config.printFileDiscoveryOrRemoval) {
      note("No such Image (Mfg ID: 0x%04X, Image ID: 0x%04X, Version: 0x%08X\n",
           id->manufacturerId,
           id->imageTypeId,
           id->firmwareVersion);
    }
    return EMBER_AF_OTA_STORAGE_SUCCESS;
  }
  if (config.printFileDiscoveryOrRemoval) {
    note("Image '%s' removed from storage list.  NOT deleted from disk.\n",
         image->filenameStart);
  }
  removeImage(image);
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

// Although the call is to "clear" temp data we actually end up allocating
// space for the filename and creating an empty file.
EmberAfOtaStorageStatus emberAfOtaStorageClearTempDataCallback(void)
{
  EmberAfOtaStorageStatus status = EMBER_AF_OTA_STORAGE_ERROR;
  FILE* fileHandle = NULL;

  if (!storageDeviceIsDirectory) {
    error("Cannot create temp. OTA data because storage device is a file, not a directory.\n");
    return EMBER_AF_OTA_STORAGE_ERROR;
  }

  if (storageDevice == NULL) {
    error("No storage device defined!");
    return EMBER_AF_OTA_STORAGE_ERROR;
  }

  if (tempStorageFilepath == NULL) {
    // Add 1 to make sure we have room for a NULL terminating character
    int tempFilepathLength = (strlen(storageDevice)
                              + strlen(tempStorageFile) + 1);
    if (tempFilepathLength > MAX_FILEPATH_LENGTH) {
      goto clearTempDataCallbackDone;
    }
    tempStorageFilepath = myMalloc(tempFilepathLength,
                                   "otaStorageCreateTempData(): tempFilepath");
    if (tempStorageFilepath == NULL) {
      goto clearTempDataCallbackDone;
    }
    snprintf(tempStorageFilepath,
             tempFilepathLength,
             "%s%s",
             storageDevice,
             tempStorageFile);
  }

  OtaImage* image = findImageByFilename(tempStorageFilepath);
  if (image) {
    removeImage(image);
  }
  // Windows requires the 'b' (binary) as part of the mode so that line endings
  // are not truncated.  POSIX ignores this.
  fileHandle = fopen(tempStorageFilepath,
                     "wb");  // truncate the file to zero length
  if (fileHandle == NULL) {
    error("Could not open temporary file '%s' for writing: %s\n",
          tempStorageFilepath,
          strerror(errno));
  } else {
    status = EMBER_AF_OTA_STORAGE_SUCCESS;
  }

  clearTempDataCallbackDone:
  // tempStorageFilepath is used throughout the life of the storage
  // device, so we only need to free it if there was an error,
  // or when the storage device changed.  We expect we can
  // only change the storage device if we first call emAfOtaStorageClose()
  // which frees the tempStorageFilepath data as well.
  if ((status != EMBER_AF_OTA_STORAGE_SUCCESS)
      && tempStorageFilepath) {
    myFree(tempStorageFilepath);
    tempStorageFilepath = NULL;
  }
  if (fileHandle) {
    fclose(fileHandle);
  }
  return status;
}

EmberAfOtaStorageStatus emberAfOtaStorageWriteTempDataCallback(uint32_t offset,
                                                               uint32_t length,
                                                               const uint8_t* data)
{
  if (tempStorageFilepath == NULL) {
    return EMBER_AF_OTA_STORAGE_ERROR;
  }
  return writeRawData(offset, tempStorageFilepath, length, data);
}

EmberAfOtaStorageStatus emberAfOtaStorageCheckTempDataCallback(uint32_t* returnOffset,
                                                               uint32_t* returnTotalSize,
                                                               EmberAfOtaImageId* returnOtaImageId)
{
  OtaImage* image;

  if (tempStorageFilepath == NULL) {
    return EMBER_AF_OTA_STORAGE_ERROR;
  }

  image = addImageFileToList(tempStorageFilepath, true);
  if (image == NULL) {
    return EMBER_AF_OTA_STORAGE_ERROR;
  }

  *returnTotalSize = image->header->imageSize;
  *returnOffset = image->fileSize;
  MEMSET(returnOtaImageId, 0, sizeof(EmberAfOtaImageId));
  *returnOtaImageId = emAfOtaStorageGetImageIdFromHeader(image->header);
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

EmberAfOtaStorageStatus emAfOtaStorageAppendImageData(const char* filename,
                                                      uint32_t length,
                                                      const uint8_t* data)
{
  OtaImage* image = findImageByFilename(filename);
  if (image != NULL) {
    // The file is already in use by the storage device, so it cannot
    // be modified.
    return EMBER_AF_OTA_STORAGE_ERROR;
  }

  return writeRawData(APPEND_OFFSET, filename, length, data);
}

int remainingAllocations(void)
{
  return allocations;
}

EmberAfOtaStorageStatus emberAfOtaStorageGetFullHeaderCallback(const EmberAfOtaImageId* id,
                                                               EmberAfOtaHeader* returnData)
{
  OtaImage* image = imageSearchInternal(id);
  if (image == NULL) {
    return EMBER_AF_OTA_STORAGE_ERROR;
  }
  MEMCOPY(returnData, image->header, sizeof(EmberAfOtaHeader));
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

uint32_t emberAfOtaStorageGetTotalImageSizeCallback(const EmberAfOtaImageId* id)
{
  OtaImage* image = imageSearchInternal(id);
  if (image == NULL) {
    return 0;
  }
  return image->fileSize;
}

EmberAfOtaStorageStatus emberAfOtaStorageFinishDownloadCallback(uint32_t offset)
{
  // Not used.  The OS filesystem keeps track of the latest download offset.
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

void emAfOtaStorageInfoPrint(void)
{
  note("Storage Module:     OTA POSIX Filesystem Storage Module\n");
  note("Storage Directory:  %s\n", defaultStorageDirectory);
}

uint32_t emberAfOtaStorageDriverMaxDownloadSizeCallback(void)
{
  // In theory we are limited by the local disk space, but for now
  // assume there is no limit.
  return 0xFFFFFFFFUL;
}

EmberAfOtaStorageStatus emberAfOtaStorageDriverPrepareToResumeDownloadCallback(void)
{
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

//==============================================================================
// Internal Functions
//==============================================================================

static EmberAfOtaStorageStatus createDefaultStorageDirectory(void)
{
  struct stat statInfo;
  int returnValue = stat(defaultStorageDirectory, &statInfo);
  if (returnValue == 0) {
    if (!S_ISDIR(statInfo.st_mode)) {
      error("Default storage directory '%s' is not a directory!\n",
            defaultStorageDirectory);
      return EMBER_AF_OTA_STORAGE_ERROR;
    }

    debug(config.fileDebug,
          "Default storage directory already exists '%s'\n",
          defaultStorageDirectory);
  } else {
    //   Does not exist, therefore we must create it.
    debug(config.fileDebug,
          "Creating default storage directory '%s'\n",
          defaultStorageDirectory);

    int status = portableMkdir(defaultStorageDirectory);

    if (status != 0) {
      error("Could not create default directory '%s': %s\n",
            defaultStorageDirectory,
            strerror(errno));
      return EMBER_AF_OTA_STORAGE_ERROR;
    }
  }
  return emAfOtaSetStorageDevice(defaultStorageDirectory);
}

static bool doEui64sMatch(const uint8_t* firstEui64,
                          const uint8_t* secondEui64)
{
  return (0 == MEMCOMPARE(firstEui64, secondEui64, EUI64_SIZE));
}

static OtaImage* findImageByFilename(const char* tempFilepath)
{
  OtaImage* ptr = imageListFirst;
  while (ptr != NULL) {
    if (0 == strcmp(ptr->filepath, tempFilepath)) {
      return ptr;
    }
    ptr = (OtaImage*)ptr->next;
  }
  return NULL;
}

static EmberAfOtaStorageStatus writeRawData(uint32_t offset,
                                            const char* filepath,
                                            uint32_t length,
                                            const uint8_t* data)
{
  // Windows requires the 'b' (binary) as part of the mode so that line endings
  // are not truncated.  POSIX ignores this.
  FILE* fileHandle = fopen(filepath,
                           "r+b");
  EmberAfOtaStorageStatus status =  EMBER_AF_OTA_STORAGE_ERROR;
  int whence = SEEK_SET;

  if (fileHandle == NULL) {
    error("Could not open file '%s' for writing: %s\n",
          filepath,
          strerror(errno));
    goto writeEnd;
  }

  if (offset == APPEND_OFFSET) {
    offset = 0;
    whence = SEEK_END;
  }

  if (0 != fseek(fileHandle, offset, whence)) {
    error("Could not seek to offset 0x%08X (%s) in file '%s': %s\n",
          offset,
          (whence == SEEK_END ? "SEEK_END" : "SEEK_SET"),
          filepath,
          strerror(errno));
    goto writeEnd;
  }

  size_t written = fwrite(data, 1, length, fileHandle);
  if (written != length) {
    error("Tried to write %d bytes but wrote %d\n", length, written);
    goto writeEnd;
  }
  status = EMBER_AF_OTA_STORAGE_SUCCESS;

  writeEnd:
  if (fileHandle) {
    fclose(fileHandle);
  }
  return status;
}

static void removeImage(OtaImage* image)
{
  OtaImage* before = (OtaImage*)image->prev;
  OtaImage* after = (OtaImage*)image->next;
  if (before) {
    before->next = (struct OtaImage*)after;
  }
  if (after) {
    after->prev = (struct OtaImage*)before;
  }
  if (image == imageListFirst) {
    imageListFirst = after;
  }
  if (image == imageListLast) {
    imageListLast = before;
  }
  freeOtaImage(image);
  imageCount--;
}

static void printEui64(uint8_t* eui64)
{
  note("%02X%02X%02X%02X%02X%02X%02X%02X",
       eui64[0],
       eui64[1],
       eui64[2],
       eui64[3],
       eui64[4],
       eui64[5],
       eui64[6],
       eui64[7]);
}

static void printHeaderInfo(const EmberAfOtaHeader* header)
{
  if (!config.printFileDiscoveryOrRemoval) {
    return;
  }

  //  printf("  Header Version:  0x%04X\n",
  //         header->headerVersion);
  //  printf("  Header Length:   0x%04X\n",
  //         header->headerLength);
  //  printf("  Field Control:   0x%04X\n",
  //         header->fieldControl);
  note("  Manufacturer ID: 0x%04X\n",
       header->manufacturerId);
  note("  Image Type ID:   0x%04X\n",
       header->imageTypeId);
  note("  Version:         0x%08X\n",
       header->firmwareVersion);
  note("  Header String:   %s\n",
       header->headerString);
  //  printf("\n");
}

static OtaImage* addImageFileToList(const char* filename,
                                    bool printImageInfo)
{
  OtaImage* newImage = (OtaImage*)myMalloc(sizeof(OtaImage),
                                           "addImageFileToList():OtaImage");
  if (newImage == NULL) {
    return NULL;
  }
  memset(newImage, 0, sizeof(OtaImage));

  struct stat statInfo;
  if (0 != stat(filename, &statInfo)) {
    myFree(newImage);
    return NULL;
  }
  newImage->fileSize = statInfo.st_size;

  int length = 1 + strnlen(filename, OTA_MAX_FILENAME_LENGTH);
  newImage->filepath = myMalloc(length, "filename");
  if (newImage->filepath == NULL) {
    goto dontAdd;
  }
  strncpy(newImage->filepath, filename, length);
  newImage->filepath[length - 1] = '\0';
  newImage->filenameStart = strrchr(newImage->filepath,
                                    '/');
  if (newImage->filenameStart == NULL) {
    newImage->filenameStart = newImage->filepath;
  } else {
    newImage->filenameStart++;  // +1 for the '/' character
  }

  newImage->header = readImageHeader(filename);
  if (newImage->header == NULL) {
    goto dontAdd;
  }

  EmberAfOtaImageId new = {
    newImage->header->manufacturerId,
    newImage->header->imageTypeId,
    INVALID_FIRMWARE_VERSION,
    INVALID_EUI64,
  };
  OtaImage* test;
  if (headerHasUpgradeFileDest(newImage->header)) {
    MEMCOPY(new.deviceSpecificFileEui64,
            &newImage->header->upgradeFileDestination,
            EUI64_SIZE);
  }
  test = imageSearchInternal(&new);

  if (test != NULL) {
    if (newImage->header->firmwareVersion
        < test->header->firmwareVersion) {
      note("Image '%s' has earlier version number (0x%08X) than existing one '%s' (0x%08X).  Skipping.\n",
           newImage->filenameStart,
           newImage->header->firmwareVersion,
           test->filenameStart,
           test->header->firmwareVersion);
      goto dontAdd;
    } else if (newImage->header->firmwareVersion
               > test->header->firmwareVersion) {
      note("Image '%s' has newer version number (0x%08X) than existing one '%s' (0x%08X).  Replacing existing one.\n",
           newImage->filenameStart,
           newImage->header->firmwareVersion,
           test->filenameStart,
           test->header->firmwareVersion);
      removeImage(test);
    } else { // Same version.
      if (headerHasUpgradeFileDest(newImage->header)
          && headerHasUpgradeFileDest(test->header)) {
        if (MEMCOMPARE(&newImage->header->upgradeFileDestination,
                       &test->header->upgradeFileDestination,
                       sizeof(test->header->upgradeFileDestination)) == 0) {
          note("Image '%s' has the same version number (0x%08X) and upgrade destination (",
               newImage->filenameStart,
               newImage->header->firmwareVersion);
          uint8_t *finger = (uint8_t *)&newImage->header->upgradeFileDestination;
          for (int i = emGetOtaHeaderFieldDefinition(newImage->header->headerVersion, UPGRADE_FILE_DESTINATION_INDEX)->length; i; i--) {
            note("%02X", finger[i - 1]);
          }
          note(") as an existing one\n");
          goto dontAdd;
        }
      } else {
        printf("Image '%s' has the same version number (0x%08X) as existing one.\n",
               newImage->filenameStart,
               newImage->header->firmwareVersion);
        goto dontAdd;
      }
    }
  }

  if (imageListFirst == NULL) {
    imageListFirst = newImage;
    imageListLast = newImage;
  } else {
    imageListLast->next = (struct OtaImage*)newImage;
    newImage->prev = (struct OtaImage*)imageListLast;
    imageListLast = newImage;
  }

  if (printImageInfo) {
    printHeaderInfo(newImage->header);
  }

  imageCount++;
  return newImage;

  dontAdd:
  freeOtaImage(newImage);
  return NULL;
}

// This function assumes that the file pointer is at the start of the
// file.  It will modify the pointer in the passed FILE*
static bool checkMagicNumber(FILE* fileHandle, bool printError)
{
  uint8_t magicNumber[4];
  if (1 != fread((void*)&magicNumber, 4, 1, fileHandle)) {
    return false;
  }
  if (0 != memcmp(magicNumber, otaFileMagicNumberBytes, 4)) {
    if (printError) {
      error("File has bad magic number.\n");
    }
    return false;
  }
  return true;
}

static EmberAfOtaStorageStatus initImageDirectory(void)
{
  DIR* dir = opendir(storageDevice);
  if (dir == NULL) {
    error("Could not open directory: %s\n", strerror(errno));
    return EMBER_AF_OTA_STORAGE_ERROR;
  }

  debug(config.fileDebug, "Opened Storage Directory: %s\n", storageDevice);

  struct dirent* dirEntry = readdir(dir);
  while (dirEntry != NULL) {
    FILE* fileHandle = NULL;
    debug(config.fileDebug, "Considering file '%s'\n", dirEntry->d_name);

    // +2 for trailing '/' and '\0'
    int pathLength = strlen(storageDevice) + strlen(dirEntry->d_name) + 2;
    if (pathLength > MAX_FILEPATH_LENGTH) {
      error("Filepath too long (max: %d) skipping file '%s'",
            MAX_FILEPATH_LENGTH,
            dirEntry->d_name);
      goto continueReadingDir;
    }
    char* filePath = myMalloc(pathLength, "initImageDirectory(): filepath");
    if (filePath == NULL) {
      error("Failed to allocate memory for filepath.\n");
      goto continueReadingDir;
    }

    sprintf(filePath, "%s%s",
            storageDevice,
            dirEntry->d_name);
    debug(config.fileDebug, "Full filepath: '%s'\n", filePath);

    struct stat buffer;
    if (0 != stat(filePath, &buffer)) {
      fprintf(stderr,
              "Error: Could not stat file '%s': %s\n",
              filePath,
              strerror(errno));
      goto continueReadingDir;
    } else if (S_ISDIR(buffer.st_mode) || !S_ISREG(buffer.st_mode)) {
      debug(config.fileDebug,
            "Ignoring '%s' because it is not a regular file.\n",
            dirEntry->d_name);

      goto continueReadingDir;
    }

    // NOTE:  dirent.d_name may have a limited length due to POSIX compliance.
    // Not sure if it will work for all possible filenames.  However
    // the length does NOT include the directory portion, so it should be
    // able to store most all filenames (<256 characters).

    // Windows requires the 'b' (binary) as part of the mode so that line endings
    // are not truncated.  POSIX ignores this.
    fileHandle = fopen(filePath, "rb");
    if (fileHandle == NULL) {
      error("Could not open file '%s' for reading: %s\n",
            filePath,
            strerror(errno));
      goto continueReadingDir;
    }

    if (!checkMagicNumber(fileHandle, false)) {
      goto continueReadingDir;
    }
    if (config.ignoreFilesWithUnderscorePrefix
        && dirEntry->d_name[0] == '_') {
      // As a means of making this program omit certain OTA files from
      // processing, we arbitrarily choose to ignore files starting with '_'.
      // This is done in part to be able to store multiple OTA files in
      // the same directory that have the same unique manufacturer and image
      // type ID.  Normally when this code finds a second image with the
      // same manufacturer and image type ID it picks the one with latest
      // version number.
      // By changing the file name we can keep the file intact and
      // have this code just skip it.
      printf("Ignoring OTA file '%s' since it starts with '_'.\n",
             dirEntry->d_name);
      goto continueReadingDir;
    }
    if (config.printFileDiscoveryOrRemoval) {
      note("Found OTA file '%s'\n", dirEntry->d_name);
    }

    // We don't really care about the return code because we want to keep trying
    // to add files.
    OtaImage* newImage = addImageFileToList(filePath, true);
    if (config.fileAddedHandler != NULL
        && newImage != NULL) {
      (config.fileAddedHandler)(newImage->header);
    }

    continueReadingDir:
    if (fileHandle) {
      fclose(fileHandle);
      fileHandle = NULL;
    }
    if (filePath) {
      myFree(filePath);
      filePath = NULL;
    }
    dirEntry = readdir(dir);
  }
  if (config.printFileDiscoveryOrRemoval) {
    printf("Found %d files\n\n", imageCount);
  }
  closedir(dir);
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

static void freeIfNotNull(void** ptr)
{
  if (*ptr != NULL) {
    myFree(*ptr);
    *ptr = NULL;
  }
}

static void freeOtaImage(OtaImage* image)
{
  if (image == NULL) {
    return;
  }
  freeIfNotNull((void**)&(image->header));
  freeIfNotNull((void**)&(image->filepath));
  myFree(image);
}

const EmberAfOtaHeaderFieldDefinition *emGetOtaHeaderFieldDefinition(uint16_t headerVersion, EmberAfOtaBootloadFileHeaderFieldIndex_t headerIndex)
{
  const EmberAfOtaHeaderFieldDefinition invalidField = { NULL, INVALID_FIELD, 0, 0 };
  if ((!isValidHeaderVersion(headerVersion)) && (headerIndex != MAGIC_NUMBER_INDEX) && (headerIndex != HEADER_VERSION_INDEX)) {
    return &otaHeaderFieldDefinitions[INVALID_FIELD_INDEX];
  }

  if ((headerIndex == UPGRADE_FILE_DESTINATION_INDEX) && (headerVersion == OTA_HEADER_VERSION_THREAD)) {
    static const EmberAfOtaHeaderFieldDefinition threadUpgradeFileDestination = { "Upgrade File Destination", BYTE_ARRAY_FIELD, UID_SIZE, DEVICE_SPECIFIC_FILE_PRESENT_MASK };
    return &threadUpgradeFileDestination;
  }

  return &otaHeaderFieldDefinitions[headerIndex];
}

static EmberAfOtaHeader* readImageHeader(const char* filename)
{
  EmberAfOtaHeader* header = NULL;
  // Windows requires the 'b' (binary) as part of the mode so that line endings
  // are not truncated.  POSIX ignores this.
  FILE* fileHandle = fopen(filename, "rb");
  if (fileHandle == NULL) {
    goto imageReadError;
  }

  header = (EmberAfOtaHeader*)myMalloc(sizeof(EmberAfOtaHeader), "readImageHeader():OtaImage");
  if (header == NULL) {
    goto imageReadError;
  }
  memset(header, 0, sizeof(EmberAfOtaHeader));
  if (!checkMagicNumber(fileHandle, true)) {
    goto imageReadError;
  }
  // In all the following code, we subtract 4 from the lengths because we have
  // already read the magic number and therefore do not need to include it in
  // our calculations.
  uint8_t buffer[OTA_MAXIMUM_HEADER_LENGTH - 4];
  uint8_t* bufferPtr = buffer;
  int dataRead = fread((void*)bufferPtr,
                       1,                             // block size (bytes)
                       OTA_MAXIMUM_HEADER_LENGTH - 4, // count
                       fileHandle);
  if (dataRead < (OTA_MINIMUM_HEADER_LENGTH - 4)) {
    error("OTA header is too short (length = %d but should be a minimum of %d)\n",
          dataRead,
          OTA_MINIMUM_HEADER_LENGTH - 4);
    goto imageReadError;
  }
  mapHeaderFieldDefinitionToDataStruct(header);

  // Read the Version and length first so we can use those to validate the rest
  // of the image.
  EmberAfOtaStorageStatus status;
  uint16_t headerVersionLength = emGetOtaHeaderFieldDefinition(0, HEADER_VERSION_INDEX)->length;
  status = readHeaderDataFromBuffer(HEADER_VERSION_INDEX,
                                    0, // we don't know the version yet
                                    bufferPtr,
                                    headerVersionLength,
                                    headerVersionLength);
  if (status != EMBER_AF_OTA_STORAGE_SUCCESS) {
    goto imageReadError;
  }
  if (!isValidHeaderVersion(header->headerVersion)) {
    error("Unknown header version number 0x%04X in file, cannot parse.\n", header->headerVersion);
    goto imageReadError;
  }
  bufferPtr += 2;  // header version field length
  uint16_t headerLengthLength = emGetOtaHeaderFieldDefinition(header->headerVersion, HEADER_LENGTH_INDEX)->length;
  status = readHeaderDataFromBuffer(HEADER_LENGTH_INDEX,
                                    header->headerVersion,
                                    bufferPtr,
                                    headerLengthLength,
                                    headerLengthLength);
  if (status != EMBER_AF_OTA_STORAGE_SUCCESS) {
    goto imageReadError;
  }
  bufferPtr += 2;  // header length field length

  // subtract 4 for length of "header length" and "header version" fields.
  int32_t lengthRemaining = header->headerLength - 4;
  dataRead -= 4;

  int fieldIndex = FIELD_CONTROL_INDEX;
  while (fieldIndex < FIELD_INDEX_MAX && dataRead > 0 && lengthRemaining > 0) {
    EmberAfOtaStorageStatus status = readHeaderDataFromBuffer(fieldIndex,
                                                              header->headerVersion,
                                                              bufferPtr,
                                                              lengthRemaining,
                                                              dataRead);
    if (status != EMBER_AF_OTA_STORAGE_SUCCESS) {
      goto imageReadError;
    }
    if (otaHeaderFieldLocations[fieldIndex].found == true) {
      uint16_t fieldLength = emGetOtaHeaderFieldDefinition(header->headerVersion, fieldIndex)->length;
      lengthRemaining -= fieldLength;
      bufferPtr += fieldLength;
      dataRead -= fieldLength;
    }
    fieldIndex++;
  }
  // Ensure Header string is NULL terminated.  The structure has one extra
  // byte than the EMBER_AF_OTA_MAX_HEADER_STRING_LENGTH
  header->headerString[EMBER_AF_OTA_MAX_HEADER_STRING_LENGTH] = '\0';

  fieldIndex = HEADER_VERSION_INDEX;
  while (fieldIndex < FIELD_INDEX_MAX) {
    const EmberAfOtaHeaderFieldDefinition *fieldPtr = emGetOtaHeaderFieldDefinition(header->headerVersion, fieldIndex);
    if (fieldPtr->maskForOptionalField == ALWAYS_PRESENT_MASK
        && !otaHeaderFieldLocations[fieldIndex].found) {
      error("Missing field '%s' from OTA header.\n", fieldPtr->name);
      goto imageReadError;
    }
    fieldIndex++;
  }

  unmapHeaderFieldDefinitions();
  fclose(fileHandle);

  //  printHeaderInfo(header);

  return header;

  imageReadError:
  unmapHeaderFieldDefinitions();
  freeIfNotNull((void**)&header);
  fclose(fileHandle);
  return NULL;
}

static EmberAfOtaStorageStatus readHeaderDataFromBuffer(EmberAfOtaBootloadFileHeaderFieldIndex_t fieldIndex,
                                                        uint16_t headerVersion,
                                                        uint8_t* bufferPtr,
                                                        int32_t headerLengthRemaining,
                                                        int32_t actualBufferDataRemaining)
{
  const EmberAfOtaHeaderFieldDefinition *definition = emGetOtaHeaderFieldDefinition(headerVersion, fieldIndex);
  if (definition->maskForOptionalField != ALWAYS_PRESENT_MASK) {
    uint16_t fieldControl = *(uint16_t*)(otaHeaderFieldLocations[FIELD_CONTROL_INDEX].location);
    if (!(fieldControl & definition->maskForOptionalField)) {
      // No more processing.
      return EMBER_AF_OTA_STORAGE_SUCCESS;
    }
  }
  if (headerLengthRemaining < definition->length
      || actualBufferDataRemaining < definition->length) {
    error("OTA Header does not contain enough data for field '%s'\n",
          definition->name);
    return EMBER_AF_OTA_STORAGE_ERROR;
  }

  if (definition->type == INTEGER_FIELD) {
    // Unfortunately we have to break up parsing of different integer lengths
    // into separate pieces of code because of the way the data
    // may be packed in the data structure.
    // Previously we tried to just use a generic 'uint32_t*' to point
    // to either the location of a 32-bit, 16-bit, or 8-bit value and then write
    // the data appropriately. However if the location could only store a 16-bit
    // value and we are referring to it as an uint32_t*, then we may write to the
    // wrong bytes depending on the how the data is packed.
    // Using the correct pointer based on the length sidesteps that problem.
    if (1 == definition->length) {
      uint8_t *value = (uint8_t*)(otaHeaderFieldLocations[fieldIndex].location);
      *value = bufferPtr[0];
    } else if (2 == definition->length) {
      uint16_t *value = (uint16_t*)(otaHeaderFieldLocations[fieldIndex].location);
      *value = (bufferPtr[0]
                + (bufferPtr[1] << 8));
    } else if (4 == definition->length) {
      uint32_t *value = (uint32_t*)(otaHeaderFieldLocations[fieldIndex].location);
      *value = (bufferPtr[0]
                + (bufferPtr[1] << 8)
                + (bufferPtr[2] << 16)
                + (bufferPtr[3] << 24));
    } else {
      error("Unsupported data value length '%d' for type '%s'.\n",
            definition->length,
            definition->name);
      return EMBER_AF_OTA_STORAGE_ERROR;
    }
  } else if (definition->type == BYTE_ARRAY_FIELD
             || definition->type == STRING_FIELD) {
    memcpy(otaHeaderFieldLocations[fieldIndex].location, bufferPtr, definition->length);
  } else {
    // Programatic error
    error("Unkown field type '%d'\n", definition->type);
    return EMBER_AF_OTA_STORAGE_ERROR;
  }
  otaHeaderFieldLocations[fieldIndex].found = true;
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

static void mapHeaderFieldDefinitionToDataStruct(EmberAfOtaHeader* header)
{
  otaHeaderFieldLocations[HEADER_VERSION_INDEX].location = &(header->headerVersion);
  otaHeaderFieldLocations[HEADER_LENGTH_INDEX].location = &(header->headerLength);
  otaHeaderFieldLocations[FIELD_CONTROL_INDEX].location = &(header->fieldControl);
  otaHeaderFieldLocations[MANUFACTURER_ID_INDEX].location = &(header->manufacturerId);
  otaHeaderFieldLocations[IMAGE_TYPE_INDEX].location = &(header->imageTypeId);
  otaHeaderFieldLocations[FIRMWARE_VERSION_INDEX].location = &(header->firmwareVersion);
  otaHeaderFieldLocations[ZIGBEE_STACK_VERSION_INDEX].location = &(header->zigbeeStackVersion);
  otaHeaderFieldLocations[TOTAL_IMAGE_SIZE_INDEX].location = &(header->imageSize);
  otaHeaderFieldLocations[SECURITY_CREDENTIALS_INDEX].location = &(header->securityCredentials);
  otaHeaderFieldLocations[MINIMUM_HARDWARE_VERSION_INDEX].location = &(header->minimumHardwareVersion);
  otaHeaderFieldLocations[MAXIMUM_HARDWARE_VERSION_INDEX].location = &(header->maximumHardwareVersion);

  // For byte arrays and strings, those are already pointers and we do not want
  // with another layer of indirection
  otaHeaderFieldLocations[HEADER_STRING_INDEX].location = header->headerString;
  otaHeaderFieldLocations[UPGRADE_FILE_DESTINATION_INDEX].location = &header->upgradeFileDestination;
}

static void unmapHeaderFieldDefinitions(void)
{
  for (int i = 0; i < FIELD_INDEX_MAX; i++) {
    otaHeaderFieldLocations[i].location = NULL;
    otaHeaderFieldLocations[i].found = false;
  }
}

static OtaImage* findImageById(const EmberAfOtaImageId* id)
{
  OtaImage* ptr = imageListFirst;
  while (ptr != NULL) {
    if (id->manufacturerId == ptr->header->manufacturerId
        && id->imageTypeId == ptr->header->imageTypeId
        && id->firmwareVersion == ptr->header->firmwareVersion) {
      return ptr;
    }
    ptr = (OtaImage*)ptr->next;
  }
  return NULL;
}

static uint8_t* writeHeaderDataToBuffer(EmberAfOtaBootloadFileHeaderFieldIndex_t fieldIndex,
                                        uint16_t headerVersion,
                                        uint8_t* bufferPtr)
{
  const EmberAfOtaHeaderFieldDefinition *definition = emGetOtaHeaderFieldDefinition(headerVersion, fieldIndex);
  if (definition->maskForOptionalField != ALWAYS_PRESENT_MASK) {
    uint16_t fieldControl = *(uint16_t*)(otaHeaderFieldLocations[FIELD_CONTROL_INDEX].location);
    if (!(fieldControl & definition->maskForOptionalField)) {
      debug(config.fieldDebug, "Skipping field %s\n", definition->name);

      // No more processing.
      return bufferPtr;
    }
  }

  debug(config.fieldDebug,
        "Writing field %s, type %d, length %d\n",
        definition->name,
        definition->type,
        definition->length);

  if (definition->type == BYTE_ARRAY_FIELD
      || definition->type == STRING_FIELD) {
    memcpy(bufferPtr, otaHeaderFieldLocations[fieldIndex].location, definition->length);
  } else if (definition->type == INTEGER_FIELD) {
    if (definition->length == 1) {
      uint8_t *value = otaHeaderFieldLocations[fieldIndex].location;
      bufferPtr[0] = *value;
    } else if (definition->length == 2) {
      uint16_t *value = otaHeaderFieldLocations[fieldIndex].location;
      bufferPtr[0] = (uint8_t)(*value);
      bufferPtr[1] = (uint8_t)(*value >> 8);
    } else if (definition->length == 4) {
      uint32_t *value = otaHeaderFieldLocations[fieldIndex].location;
      bufferPtr[0] = (uint8_t)(*value);
      bufferPtr[1] = (uint8_t)(*value >> 8);
      bufferPtr[2] = (uint8_t)(*value >> 16);
      bufferPtr[3] = (uint8_t)(*value >> 24);
    } else {
      assert(0);
    }
  }
  return (bufferPtr + definition->length);
}

static uint16_t calculateOtaFileHeaderLength(EmberAfOtaHeader* header)
{
  uint16_t length = 4; // the size of the magic number
  int fieldIndex = HEADER_VERSION_INDEX;
  while (fieldIndex < FIELD_INDEX_MAX) {
    const EmberAfOtaHeaderFieldDefinition *definition = emGetOtaHeaderFieldDefinition(header->headerVersion, fieldIndex);
    if (definition->maskForOptionalField == ALWAYS_PRESENT_MASK
        || (header->fieldControl & definition->maskForOptionalField)) {
      length += definition->length;
    }
    fieldIndex++;
  }
  return length;
}

static OtaImage* imageSearchInternal(const EmberAfOtaImageId* id)
{
  OtaImage* ptr = imageListFirst;
  OtaImage* newest = NULL;
  while (ptr != NULL) {
    /*
       note("imageSearchInternal: Considering file '%s' (MFG: 0x%04X, Image: 0x%04X)\n",
         ptr->filenameStart,
         ptr->header->manufacturerId,
         ptr->header->imageTypeId);
     */
    if (ptr->header->manufacturerId == id->manufacturerId
        && (ptr->header->imageTypeId == id->imageTypeId)) {
      if ((id->firmwareVersion == INVALID_FIRMWARE_VERSION)
          || (ptr->header->firmwareVersion == id->firmwareVersion)) {
        if (headerHasUpgradeFileDest(ptr->header)) {
          if ((ptr->header->headerVersion == OTA_HEADER_VERSION_ZIGBEE)
              && (doEui64sMatch(id->deviceSpecificFileEui64,
                                &ptr->header->upgradeFileDestination))) {
            // Because there is an exact match on the EUI64, we know
            // this is the only one that can match the request and can
            // return that now.
            return ptr;
          } else {
            assert(0); // TODO: We need to handle Thread OTA headers
          }
        } else {
          if (doEui64sMatch(emberAfInvalidImageId.deviceSpecificFileEui64,
                            id->deviceSpecificFileEui64)) {
            // Save this entry. There is no upgrade file dest in either the
            // search criteria or the file. This match may or may not be
            // the latest version number.
            newest = ptr;
          }
        }
      }
    }
    ptr = (OtaImage*)ptr->next;
  }
  return newest;
}

static EmberAfOtaImageId getIteratorImageId(void)
{
  if (iterator == NULL) {
    return emberAfInvalidImageId;
  }
  return emAfOtaStorageGetImageIdFromHeader(iterator->header);
}

uint32_t emAfOtaStorageGetSlot(void)
{
  return INVALID_SLOT;
}

//------------------------------------------------------------------------------
// DEBUG

static void* myMalloc(size_t size, const char* allocName)
{
  void* returnValue = malloc(size);
  if (returnValue != NULL) {
    allocations++;
    debug(config.memoryDebug,
          "[myMalloc] %s, %d bytes (0x%08X)\n",
          allocName, size, returnValue);
  }
  return returnValue;
}

static void myFree(void* ptr)
{
  debug(config.memoryDebug, "[myFree] 0x%08X\n", ptr);
  free(ptr);
  allocations--;
}

//------------------------------------------------------------------------------
// Print routines.

static void message(FILE* stream,
                    bool error,
                    const char* formatString,
                    va_list ap)
{
  if (messagePrefix) {
    fprintf(stream, "[%s] ", messagePrefix);
  }
  if (error) {
    fprintf(stream, "Error: ");
  }
  vfprintf(stream, formatString, ap);
  fflush(stream);
}

static void note(const char* formatString, ...)
{
  va_list ap = { 0 };
  va_start(ap, formatString);
  message(stdout, false, formatString, ap);
  va_end(ap);
}

static void debug(bool debugOn, const char* formatString, ...)
{
  if (debugOn) {
    va_list ap = { 0 };
    va_start(ap, formatString);
    message(stdout, false, formatString, ap);
    va_end(ap);
  }
}

static void error(const char* formatString, ...)
{
  va_list ap = { 0 };
  va_start(ap, formatString);
  message(stderr, true, formatString, ap);
  va_end(ap);
}

#endif  // defined(GATEWAY_APP)
