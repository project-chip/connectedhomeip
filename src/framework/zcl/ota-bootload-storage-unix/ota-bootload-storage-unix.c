/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#include CHIP_AF_API_BUFFER_MANAGEMENT
#include CHIP_AF_API_EVENT_QUEUE
#include CHIP_AF_API_ZCL_CORE
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE
#include "ota-bootload-storage-unix.h"

#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#else
  #define chipAfPluginOtaBootloadStorageUnixPrint(...)
  #define chipAfPluginOtaBootloadStorageUnixPrintln(...)
  #define chipAfPluginOtaBootloadStorageUnixFlush()
  #define chipAfPluginOtaBootloadStorageUnixDebugExec(x)
  #define chipAfPluginOtaBootloadStorageUnixPrintBuffer(buffer, len, withSpace)
  #define chipAfPluginOtaBootloadStorageUnixPrintString(buffer)
#endif

#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>

// This is a POSIX filesystem implementation of the ota-bootload-storage-core
// API. It uses stdio FILE objects to read/write OTA data to an actual file on
// disk. This implementation is slightly inefficient because of the need to seek
// around a file, but it doesn't look like this takes too much extra time (even
// on a Raspberry Pi system). It uses a linked list and malloc to store as many
// files as the system will allow.

// -----------------------------------------------------------------------------
// Constants

#define MAX_FILENAME_SIZE 256

#define FILENAME_FORMAT "%04X-%04X-%08X.ota"

// Arbitrary value. We can update this to whatever it needs to be.
#define MAX_FILE_SIZE 10000000

#define TAG_OVERHEAD 6 /* 2-byte ID, 4-byte length */
#define TAG_ID_UPGRADE_IMAGE 0x0000

// -----------------------------------------------------------------------------
// Types

typedef struct File_tag {
  ChipZclOtaBootloadFileHeaderInfo_t headerInfo;
  char name[MAX_FILENAME_SIZE + 1]; // add a byte for the NUL-terminator
  FILE *file;
  uint16_t totalHeaderSize;
  struct File_tag *next;
} File_t;

// -----------------------------------------------------------------------------
// Globals

static ChipZclOtaBootloadStorageInfo_t storageInfo = {
  .maximumFileSize = MAX_FILE_SIZE,
  .fileCount = 0,
};
static File_t *files = NULL;

// -----------------------------------------------------------------------------
// Internal API

#if 0
static void logPrint(const char *api,
                     const ChipZclOtaBootloadFileSpec_t *fileSpec,
                     size_t offset,
                     const uint8_t *data,
                     size_t dataLength)
{
  chipAfPluginOtaBootloadStorageUnixPrint("%s(m=%04X t=%04X v=%08X, o=%zu, l=%zu, [ ",
                                           api,
                                           fileSpec->manufacturerCode,
                                           fileSpec->type,
                                           fileSpec->version,
                                           offset,
                                           dataLength);
  if (dataLength > MAX_FILE_SIZE) {
    chipAfPluginOtaBootloadStorageUnixPrintln("hugeSize! ])");
    return;
  }
  for (size_t i = 0; i < dataLength; i++) {
    chipAfPluginOtaBootloadStorageUnixPrint("%02x ", data[i]);
  }
  chipAfPluginOtaBootloadStorageUnixPrintln("])");
}
#else
  #define logPrint(...)
#endif

// -------------------------------------
// Storage management
bool chipZclOtaBootloadStorageUnixWriteFileHeader(FILE *file,
                                                   ChipZclOtaBootloadFileHeaderInfo_t *fileHeaderInfo,
                                                   size_t imageDataSize)
{
  // The OTA file is little-endian.
  uint8_t headerData[CHIP_ZCL_OTA_BOOTLOAD_HEADER_MAX_SIZE];
  ChipZclOtaBootloadFileStatus_t status;
  if ((status = chipZclOtaBootloadStoreFileHeaderInfo(headerData, fileHeaderInfo, imageDataSize))
      != CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_VALID) {
    chipAfPluginOtaBootloadStorageUnixPrintln("StoreFileHeaderInfo status: %d", status);
    return true;
  }

  assert(fseek(file, 0, SEEK_SET) != -1); // TODO: what if this fails?

  chipAfPluginOtaBootloadStorageUnixPrintln("HeaderSize: %d", fileHeaderInfo->headerSize);
  return (fwrite(headerData, 1, fileHeaderInfo->headerSize, file) != fileHeaderInfo->headerSize);
}

bool chipZclOtaBootloadStorageUnixReadFileHeader(FILE *file,
                                                  ChipZclOtaBootloadFileHeaderInfo_t *fileHeaderInfo)
{
  // The OTA file is little-endian.
  uint8_t headerData[CHIP_ZCL_OTA_BOOTLOAD_HEADER_MAX_SIZE];
  uint8_t *finger = headerData;

  assert(fseek(file, 0, SEEK_SET) != -1);

  #define PREHEADER_SIZE 8
  if (fread(headerData, 1, PREHEADER_SIZE, file) != PREHEADER_SIZE) {
    chipAfPluginOtaBootloadStorageUnixPrint("Bad preheader read ");
    return true;
  }

  // Magic number.
  if (CHIP_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER != chipFetchLowHighInt32u(finger)) {
    chipAfPluginOtaBootloadStorageUnixPrint("Bad magic (%d != %d) ", CHIP_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER, chipFetchLowHighInt32u(finger));
    return true;
  }
  finger += sizeof(uint32_t);

  // Header version.
  if (CHIP_ZCL_OTA_BOOTLOAD_FILE_VERSION != chipFetchLowHighInt16u(finger)) {
    chipAfPluginOtaBootloadStorageUnixPrint("Bad version (%d != %d)", CHIP_ZCL_OTA_BOOTLOAD_FILE_VERSION, chipFetchLowHighInt16u(finger));
    return true;
  }
  finger += sizeof(uint16_t);

  // Total Header length
  uint16_t totalHeaderSize = chipFetchLowHighInt16u(finger);
  finger += sizeof(uint16_t);

  uint16_t readCount = 0;
  if ((readCount = fread(finger, 1, totalHeaderSize - PREHEADER_SIZE, file)) != (totalHeaderSize - PREHEADER_SIZE)) {
    chipAfPluginOtaBootloadStorageUnixPrint("Bad total header read (%d != %d) ", readCount, (totalHeaderSize - PREHEADER_SIZE));
    return true;
  }

  if (chipZclOtaBootloadFetchFileHeaderInfo(headerData, fileHeaderInfo)
      != CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_VALID) {
    chipAfPluginOtaBootloadStorageUnixPrint("Bad file status info");
    return true;
  }
  assert(fseek(file, 0, SEEK_END) != -1);
  if (ftell(file) != fileHeaderInfo->fileSize) {
    chipAfPluginOtaBootloadStorageUnixPrint("Header reported size (%ld) and actual file size (%d) mismatch",
                                             ftell(file),
                                             fileHeaderInfo->fileSize);
    return true;
  }

  chipAfPluginOtaBootloadStorageUnixPrint("m=0x%04X t=0x%04X v=0x%08X", fileHeaderInfo->spec.manufacturerCode, fileHeaderInfo->spec.type, fileHeaderInfo->spec.version);
  return false;
}

static File_t *createFile(const char *fileName, const ChipZclOtaBootloadFileSpec_t *fileSpec)
{
  if (strncmp(fileName + strlen(fileName) - 4, ".ota", 4) != 0) {
    return NULL;
  }
  File_t *newFile = malloc(sizeof(File_t));
  if (newFile == NULL) {
    return NULL;
  }
  strncpy(newFile->name, fileName, MAX_FILENAME_SIZE);
  newFile->file = fopen((char *)newFile->name, access(newFile->name, F_OK) == 0 ? "r+" : "w+");
  if (newFile->file == NULL) {
    free(newFile);
    return NULL;
  }

  if (chipZclOtaBootloadFileSpecsAreEqual(fileSpec, &chipZclOtaBootloadFileSpecNull)) {
    newFile->headerInfo.spec = chipZclOtaBootloadFileSpecNull;
    if (chipZclOtaBootloadStorageUnixReadFileHeader(newFile->file, &(newFile->headerInfo))) {
      uint32_t manufCode, type, version;
      if (sscanf(newFile->name, FILENAME_FORMAT, &(manufCode), &(type), &(version)) != 3) {
        fclose(newFile->file);
        free(newFile);
        return NULL;
      } else {
        chipAfPluginOtaBootloadStorageUnixPrint("Keeping anyway ");
        newFile->headerInfo.spec.manufacturerCode = manufCode;
        newFile->headerInfo.spec.type = type;
        newFile->headerInfo.spec.version = version;
        assert(fseek(newFile->file, 0, SEEK_END) != -1); // TODO: what if this fails?
        assert((newFile->headerInfo.fileSize = ftell(newFile->file)) != -1);
        assert(fseek(newFile->file, 0, SEEK_SET) != -1); // TODO: what if this fails?
      }
    }
  } else {
    newFile->headerInfo.spec = *fileSpec;
    newFile->headerInfo.fileSize = 0;
  }

  // Add new file to the head of the list for simplicity.
  newFile->next = files;
  files = newFile;
  storageInfo.fileCount++;

  return newFile;
}

static bool closeFile(File_t *file, bool remove)
{
  int err = fclose(file->file);
  if (err == -1) {
    return false;
  }

  if (remove) {
    err = unlink((char *)file->name);
    if (err == -1) {
      return false;
    }
  }

  if (file == files) {
    files = file->next;
  } else {
    File_t *tmpFile = files;
    while (tmpFile != NULL && tmpFile->next != file) tmpFile = tmpFile->next;
    assert(tmpFile != NULL); // what to do if not in file list? Where'd it come from?
    tmpFile->next = file->next;
  }

  free(file);
  storageInfo.fileCount--;

  return true;
}

static bool closeAllFiles(bool remove)
{
  while (files != NULL) {
    if (!closeFile(files, remove)) {
      return false;
    }
  }
  return true;
}

// -------------------------------------
// File management

static bool readOrWrite(const File_t *file,
                        size_t offset,
                        void *data,
                        size_t dataLength,
                        bool read)
{
  int err = fseek(file->file, offset, SEEK_SET);
  if (err == -1) {
    return false;
  }

  size_t readOrWrittenBytes = (read
                               ? fread(data, 1, dataLength, file->file)
                               : fwrite(data, 1, dataLength, file->file));
  return (readOrWrittenBytes == dataLength);
}

static void loadFiles(void)
{
  if (files != NULL) {
    closeAllFiles(false);
  }

  // TODO: do we want to use scandir(3) here?
  DIR *dir = opendir(".");
  assert(dir != NULL); // TODO: what if this fails?

  struct dirent *ent;
  while ((ent = readdir(dir)) != NULL) {
    chipAfPluginOtaBootloadStorageUnixPrint("%s: ", ent->d_name);
    createFile(ent->d_name, &chipZclOtaBootloadFileSpecNull);
    chipAfPluginOtaBootloadStorageUnixPrintln("");
  }
  chipAfPluginOtaBootloadStorageUnixPrintln("Load files found %d files", storageInfo.fileCount);

  assert(closedir(dir) == 0); // TODO: what if this fails?
}

static File_t *findFileHelper(const ChipZclOtaBootloadFileSpec_t *fileSpec)
{
  for (File_t *file = files; file != NULL; file = file->next) {
    if (chipZclOtaBootloadFileSpecsAreEqual(&file->headerInfo.spec, fileSpec)) {
      return file;
    }
  }
  return NULL;
}

static File_t *findFile(const ChipZclOtaBootloadFileSpec_t *fileSpec)
{
  File_t *ret = findFileHelper(fileSpec);
  if ((ret == NULL) || (access(ret->name, F_OK) != 0)) {
    loadFiles();
    ret = findFileHelper(fileSpec);
  }
  return ret;
}

// -----------------------------------------------------------------------------
// API

void chZclOtaBootloadStorageUnixInitCallback(void)
{
  loadFiles();
}

void chipZclOtaBootloadStorageGetInfo(ChipZclOtaBootloadStorageInfo_t *info,
                                       ChipZclOtaBootloadFileSpec_t *returnedFiles,
                                       size_t returnedFilesMaxCount)
{
  loadFiles(); // force a rescan of the folder
  *info = storageInfo;
  for (File_t *file = files;
       returnedFiles != NULL && file != NULL && returnedFilesMaxCount > 0;
       file = file->next, returnedFilesMaxCount--) {
    *returnedFiles++ = file->headerInfo.spec;
  }
}

ChipZclOtaBootloadStorageStatus_t chipZclOtaBootloadStorageFind(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                                  ChipZclOtaBootloadStorageFileInfo_t *fileInfo)
{
  File_t *file = findFile(fileSpec);
  if (file == NULL) {
    return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
  }

  fileInfo->size = file->headerInfo.fileSize;

  return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS;
}

ChipZclOtaBootloadStorageStatus_t chipZclOtaBootloadStorageCreate(const ChipZclOtaBootloadFileSpec_t *fileSpec)
{
  File_t *file = findFile(fileSpec);
  if (file != NULL) {
    // We already are storing a file with this file spec! We can't store two
    // different files with the same file spec, because this would violate the
    // bijection from fileSpec's to actual files in storage.
    return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
  }

  char fileName[MAX_FILENAME_SIZE + 1];
  sprintf(fileName, FILENAME_FORMAT, fileSpec->manufacturerCode, fileSpec->type, fileSpec->version);
  file = createFile(fileName, fileSpec);
  if (file == NULL) {
    // If we can't create the file, then a system error happened, most likely.
    return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_SPACE;
  }

  return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS;
}

ChipZclOtaBootloadStorageStatus_t chipZclOtaBootloadStorageRead(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                                  size_t offset,
                                                                  void *data,
                                                                  size_t dataLength)
{
  logPrint("Read", fileSpec, offset, data, dataLength);

  File_t *file = findFile(fileSpec);
  if (file == NULL) {
    chipAfPluginOtaBootloadStorageUnixPrintln("Read: Invalid file");
    return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
  }

  if (offset + dataLength > file->headerInfo.fileSize) {
    chipAfPluginOtaBootloadStorageUnixPrintln("Read: Out of range");
    return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_RANGE;
  }

  return (readOrWrite(file,
                      offset,
                      data,
                      dataLength,
                      true) // read?
          ? CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS
          : CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_FAILED);
}

ChipZclOtaBootloadStorageStatus_t chipZclOtaBootloadStorageWrite(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                                   size_t offset,
                                                                   const void *data,
                                                                   size_t dataLength)
{
  logPrint("Write", fileSpec, offset, data, dataLength);

  File_t *file = findFile(fileSpec);
  if (file == NULL) {
    chipAfPluginOtaBootloadStorageUnixPrintln("Write: Invalid file");
    return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
  }

  // Only allow sequential writes (see OTA Bootload Storage Core API).
  if (offset != file->headerInfo.fileSize) {
    chipAfPluginOtaBootloadStorageUnixPrintln("Write: offset out of range (got: %d, expected: %d)",
                                               offset,
                                               file->headerInfo.fileSize);
    return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_RANGE;
  }

  if (offset + dataLength > storageInfo.maximumFileSize) {
    chipAfPluginOtaBootloadStorageUnixPrintln("Write: length out of range");
    return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_RANGE;
  }

  bool success = readOrWrite(file,
                             offset,
                             (void *)data,
                             dataLength,
                             false); // read?
  if (success) {
    file->headerInfo.fileSize += dataLength;
  }
  chipAfPluginOtaBootloadStorageUnixPrintln("Write: success = %d", success);
  return (success
          ? CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS
          : CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_FAILED);
}

ChipZclOtaBootloadStorageStatus_t chipZclOtaBootloadStorageDelete(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                                    ChipZclOtaBootloadStorageDeleteCallback callback)
{
  logPrint("Delete", fileSpec, 0, NULL, 0);

  // If the NULL file spec is passed, we delete all files.
  File_t *file = NULL;
  if (!chipZclOtaBootloadFileSpecsAreEqual(fileSpec,
                                            &chipZclOtaBootloadFileSpecNull)) {
    file = findFile(fileSpec);
    if (file == NULL) {
      return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
    }
  }

  ChipZclOtaBootloadStorageStatus_t storageStatus
    = ((file != NULL ? closeFile(file, true) : closeAllFiles(true))
       ? CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS
       : CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_FAILED);

  (*callback)(storageStatus);

  return storageStatus;
}
