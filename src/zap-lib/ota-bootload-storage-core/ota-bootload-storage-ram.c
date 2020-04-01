/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include "ota-bootload-storage-core.h"

// This is a RAM implementation of the ota-bootload-storage-core API. It is not
// meant to be released, and it is only used for testing.

// -----------------------------------------------------------------------------
// Globals

// This switch is so that other unit tests can run tests on scenarios where
// calls to this implementation's emberZclOtaBootloadStorageWrite() API fail.
bool emZclOtaBootloadStorageRamWriteSuccess = true;

// -----------------------------------------------------------------------------
// Constants

#define MAX_FILES 1
#define MAX_FILE_SIZE 2048

// -----------------------------------------------------------------------------
// Types

typedef struct {
  EmberZclOtaBootloadFileSpec_t spec;
  EmberZclOtaBootloadStorageFileInfo_t info;
  uint8_t data[MAX_FILE_SIZE];
} File_t;
static File_t files[MAX_FILES];

// -----------------------------------------------------------------------------
// Internal API

static size_t getFileCount(void)
{
  size_t count = 0;
  for (size_t i = 0; i < MAX_FILES; i++) {
    if (!emberZclOtaBootloadFileSpecsAreEqual(&files[i].spec,
                                              &emberZclOtaBootloadFileSpecNull)) {
      count++;
    }
  }
  return count;
}

static File_t *findFile(const EmberZclOtaBootloadFileSpec_t *fileSpec)
{
  for (size_t i = 0; i < MAX_FILES; i++) {
    if (emberZclOtaBootloadFileSpecsAreEqual(fileSpec, &files[i].spec)) {
      return &files[i];
    }
  }
  return NULL;
}

#define findUnusedFile() findFile(&emberZclOtaBootloadFileSpecNull)
#define deleteAllFiles() emZclOtaBootloadStorageInitCallback()

// -----------------------------------------------------------------------------
// API

void emZclOtaBootloadStorageInitCallback(void)
{
  for (size_t i = 0; i < MAX_FILES; i++) {
    files[i].spec = emberZclOtaBootloadFileSpecNull;
  }
}

void emberZclOtaBootloadStorageGetInfo(EmberZclOtaBootloadStorageInfo_t *info,
                                       EmberZclOtaBootloadFileSpec_t *returnedFiles,
                                       size_t returnedFilesMaxCount)
{
  info->maximumFileSize = MAX_FILE_SIZE;
  info->fileCount = getFileCount();
  if (returnedFiles != NULL) {
    for (size_t i = 0, j = 0; i < MAX_FILES && j < returnedFilesMaxCount; i++) {
      if (!emberZclOtaBootloadFileSpecsAreEqual(&files[i].spec,
                                                &emberZclOtaBootloadFileSpecNull)) {
        returnedFiles[j] = files[i].spec;
        j++;
      }
    }
  }
}

EmberZclOtaBootloadStorageStatus_t emberZclOtaBootloadStorageFind(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                                  EmberZclOtaBootloadStorageFileInfo_t *fileInfo)
{
  File_t *file = findFile(fileSpec);
  if (file == NULL) {
    return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
  }

  *fileInfo = file->info;

  return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS;
}

EmberZclOtaBootloadStorageStatus_t emberZclOtaBootloadStorageCreate(const EmberZclOtaBootloadFileSpec_t *fileSpec)
{
  File_t *file = findFile(fileSpec);
  if (file != NULL) {
    // We already are storing a file with this file spec! We can't store two
    // different files with the same file spec, because this would violate the
    // bijection from fileSpec's to actual files in storage.
    return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
  }

  file = findUnusedFile();
  if (file == NULL) {
    // There isn't enough space for this file!
    return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_SPACE;
  }

  file->spec = *fileSpec;
  file->info.size = 0;
  MEMSET(file->data, 0xFF, sizeof(file->data));

  return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS;
}

EmberZclOtaBootloadStorageStatus_t emberZclOtaBootloadStorageRead(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                                  size_t offset,
                                                                  void *data,
                                                                  size_t dataLength)
{
  File_t *file = findFile(fileSpec);
  if (file == NULL) {
    return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
  }

  if (offset + dataLength > file->info.size) {
    return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_RANGE;
  }

  MEMMOVE(data, file->data + offset, dataLength);

  return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS;
}

EmberZclOtaBootloadStorageStatus_t emberZclOtaBootloadStorageWrite(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                                   size_t offset,
                                                                   const void *data,
                                                                   size_t dataLength)
{
  if (!emZclOtaBootloadStorageRamWriteSuccess) {
    return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_FAILED;
  }

  File_t *file = findFile(fileSpec);
  if (file == NULL) {
    return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
  }

  if (file->info.size + dataLength > sizeof(file->data)) {
    return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_RANGE;
  }

  MEMMOVE(file->data + offset, data, dataLength);
  file->info.size += dataLength;

  return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS;
}

EmberZclOtaBootloadStorageStatus_t emberZclOtaBootloadStorageDelete(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                                    EmberZclOtaBootloadStorageDeleteCallback callback)
{
  File_t *file = NULL;
  if (!emberZclOtaBootloadFileSpecsAreEqual(fileSpec,
                                            &emberZclOtaBootloadFileSpecNull)) {
    file = findFile(fileSpec);
    if (file == NULL) {
      return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE;
    }
  }

  if (file == NULL) {
    // Delete all files.
    deleteAllFiles();
  } else {
    file->spec = emberZclOtaBootloadFileSpecNull;
  }

  (*callback)(EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);

  return EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS;
}
