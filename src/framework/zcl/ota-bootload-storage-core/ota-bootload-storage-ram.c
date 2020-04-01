/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_ZCL_CORE
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include "ota-bootload-storage-core.h"

// This is a RAM implementation of the ota-bootload-storage-core API. It is not
// meant to be released, and it is only used for testing.

// -----------------------------------------------------------------------------
// Globals

// This switch is so that other unit tests can run tests on scenarios where
// calls to this implementation's chipZclOtaBootloadStorageWrite() API fail.
bool chZclOtaBootloadStorageRamWriteSuccess = true;

// -----------------------------------------------------------------------------
// Constants

#define MAX_FILES 1
#define MAX_FILE_SIZE 2048

// -----------------------------------------------------------------------------
// Types

typedef struct {
  ChipZclOtaBootloadFileSpec_t spec;
  ChipZclOtaBootloadStorageFileInfo_t info;
  uint8_t data[MAX_FILE_SIZE];
} File_t;
static File_t files[MAX_FILES];

// -----------------------------------------------------------------------------
// Internal API

static size_t getFileCount(void)
{
  size_t count = 0;
  for (size_t i = 0; i < MAX_FILES; i++) {
    if (!chipZclOtaBootloadFileSpecsAreEqual(&files[i].spec,
                                              &chipZclOtaBootloadFileSpecNull)) {
      count++;
    }
  }
  return count;
}

static File_t *findFile(const ChipZclOtaBootloadFileSpec_t *fileSpec)
{
  for (size_t i = 0; i < MAX_FILES; i++) {
    if (chipZclOtaBootloadFileSpecsAreEqual(fileSpec, &files[i].spec)) {
      return &files[i];
    }
  }
  return NULL;
}

#define findUnusedFile() findFile(&chipZclOtaBootloadFileSpecNull)
#define deleteAllFiles() chZclOtaBootloadStorageInitCallback()

// -----------------------------------------------------------------------------
// API

void chZclOtaBootloadStorageInitCallback(void)
{
  for (size_t i = 0; i < MAX_FILES; i++) {
    files[i].spec = chipZclOtaBootloadFileSpecNull;
  }
}

void chipZclOtaBootloadStorageGetInfo(ChipZclOtaBootloadStorageInfo_t *info,
                                       ChipZclOtaBootloadFileSpec_t *returnedFiles,
                                       size_t returnedFilesMaxCount)
{
  info->maximumFileSize = MAX_FILE_SIZE;
  info->fileCount = getFileCount();
  if (returnedFiles != NULL) {
    for (size_t i = 0, j = 0; i < MAX_FILES && j < returnedFilesMaxCount; i++) {
      if (!chipZclOtaBootloadFileSpecsAreEqual(&files[i].spec,
                                                &chipZclOtaBootloadFileSpecNull)) {
        returnedFiles[j] = files[i].spec;
        j++;
      }
    }
  }
}

ChipZclOtaBootloadStorageStatus_t chipZclOtaBootloadStorageFind(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                                  ChipZclOtaBootloadStorageFileInfo_t *fileInfo)
{
  File_t *file = findFile(fileSpec);
  if (file == NULL) {
    return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
  }

  *fileInfo = file->info;

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

  file = findUnusedFile();
  if (file == NULL) {
    // There isn't enough space for this file!
    return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_SPACE;
  }

  file->spec = *fileSpec;
  file->info.size = 0;
  MEMSET(file->data, 0xFF, sizeof(file->data));

  return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS;
}

ChipZclOtaBootloadStorageStatus_t chipZclOtaBootloadStorageRead(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                                  size_t offset,
                                                                  void *data,
                                                                  size_t dataLength)
{
  File_t *file = findFile(fileSpec);
  if (file == NULL) {
    return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
  }

  if (offset + dataLength > file->info.size) {
    return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_RANGE;
  }

  MEMMOVE(data, file->data + offset, dataLength);

  return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS;
}

ChipZclOtaBootloadStorageStatus_t chipZclOtaBootloadStorageWrite(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                                   size_t offset,
                                                                   const void *data,
                                                                   size_t dataLength)
{
  if (!chZclOtaBootloadStorageRamWriteSuccess) {
    return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_FAILED;
  }

  File_t *file = findFile(fileSpec);
  if (file == NULL) {
    return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
  }

  if (file->info.size + dataLength > sizeof(file->data)) {
    return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_RANGE;
  }

  MEMMOVE(file->data + offset, data, dataLength);
  file->info.size += dataLength;

  return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS;
}

ChipZclOtaBootloadStorageStatus_t chipZclOtaBootloadStorageDelete(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                                    ChipZclOtaBootloadStorageDeleteCallback callback)
{
  File_t *file = NULL;
  if (!chipZclOtaBootloadFileSpecsAreEqual(fileSpec,
                                            &chipZclOtaBootloadFileSpecNull)) {
    file = findFile(fileSpec);
    if (file == NULL) {
      return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
    }
  }

  if (file == NULL) {
    // Delete all files.
    deleteAllFiles();
  } else {
    file->spec = chipZclOtaBootloadFileSpecNull;
  }

  (*callback)(CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);

  return CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS;
}
