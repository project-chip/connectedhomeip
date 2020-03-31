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
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE
#include "../ota-bootload-storage-core/ota-bootload-storage-core-test.h"
#include "../ota-bootload-storage-core/ota-static-file-data.h"
#include "ota-bootload-storage-unix.h"

#include <unistd.h>

// -----------------------------------------------------------------------------
// Stubs

void deletionTimePassesCallback(void)
{
  // With the POSIX based implementation, the Delete() API returns
  // synchronously.
}

// -----------------------------------------------------------------------------
// Test stuff

static EmberZclOtaBootloadStorageStatus_t currentStorageStatus = EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_NULL;

static void deleteCompleteCallback(EmberZclOtaBootloadStorageStatus_t storageStatus)
{
  currentStorageStatus = storageStatus;
}

static void testFileCount(size_t count)
{
  EmberZclOtaBootloadStorageInfo_t storageInfo;
  emberZclOtaBootloadStorageGetInfo(&storageInfo, NULL, 0);
  assert(storageInfo.fileCount == count);
}

static void testFileSpec(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                         const size_t expectedSize)
{
  EmberZclOtaBootloadStorageInfo_t storageInfo;
  EmberZclOtaBootloadFileSpec_t storedFileSpec;
  emberZclOtaBootloadStorageGetInfo(&storageInfo, &storedFileSpec, 1);
  assert(emberZclOtaBootloadFileSpecsAreEqual(fileSpec, &storedFileSpec));
  EmberZclOtaBootloadStorageFileInfo_t fileInfo;
  assert(emberZclOtaBootloadStorageFind(fileSpec, &fileInfo)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  assert(fileInfo.size == expectedSize);
}

static void testMultipleFiles(void)
{
  const EmberZclOtaBootloadFileSpec_t files[] = {
    {
      .manufacturerCode = 0x0001,
      .type = 0x1000,
      .version = 0x10101010,
    },
    {
      .manufacturerCode = 0x0002,
      .type = 0x2000,
      .version = 0x20202020,
    },
    {
      .manufacturerCode = 0x0003,
      .type = 0x3000,
      .version = 0x30303030,
    },
  };

  // Add all of the files.
  for (size_t i = 0; i < COUNTOF(files); i++) {
    assert(emberZclOtaBootloadStorageCreate(&files[i])
           == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
    testFileCount(i + 1);
  }

  // Write something to each of the files.
  for (size_t i = 0; i < COUNTOF(files); i++) {
    assert(emberZclOtaBootloadStorageWrite(&files[i],
                                           0,  // offset
                                           &i, // data
                                           1)  // dataLength
           == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  }

  // Delete the second file, then the third file, and then the first file. This
  // is to test removal from the linked list.
  for (size_t i = 0; i < COUNTOF(files); i++) {
    size_t realIndex = (i + 1)  % COUNTOF(files);
    assert(emberZclOtaBootloadStorageDelete(&files[realIndex],
                                            deleteCompleteCallback)
           == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
    assert(currentStorageStatus == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
    testFileCount(COUNTOF(files) - 1 - i);
  }
}

static void testDeleteAllFiles(void)
{
  assert(emberZclOtaBootloadStorageDelete(&emberZclOtaBootloadFileSpecNull,
                                          deleteCompleteCallback)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  assert(currentStorageStatus == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  testFileCount(0);
}

static void testFileReload(void)
{
  // Start out with no files.
  testFileCount(0);

  // Create a file on disk to be reloaded by the plugin. This file format comes
  // from ota-bootload-storage-unix.c.
  EmberZclOtaBootloadFileSpec_t spec = {
    .manufacturerCode = 0x1020,
    .type = 0x3040,
    .version = 0x50607080,
  };
  FILE *file = fopen("1020-3040-50607080.ota", "w");
  assert(file != NULL);
  assert(access("1020-3040-50607080.ota", F_OK) == 0);
  EmberZclOtaBootloadFileHeaderInfo_t fileHeaderInfo;
  emberZclOtaBootloadInitFileHeaderInfo(&fileHeaderInfo);
  fileHeaderInfo.spec = spec;
  assert(emberZclOtaBootloadStorageUnixWriteFileHeader(file, &fileHeaderInfo, 0) == false);
  assert(fclose(file) == 0);

  // Reload the file on disk.
  // From ota-bootload-storage-unix.c.
  extern void emZclOtaBootloadStorageUnixInitCallback(void);
  emZclOtaBootloadStorageUnixInitCallback();

  // One file should exist, and then file spec should come from the file format.
  testFileCount(1);
  testFileSpec(&fileHeaderInfo.spec, fileHeaderInfo.headerSize);
}

int main(int argc, char *argv[])
{
  // From ota-bootload-storage-unix.c.
  extern void emZclOtaBootloadStorageUnixInitCallback(void);
  emZclOtaBootloadStorageUnixInitCallback();

  fprintf(stderr, "[%s ", argv[0]);

  // Delete all of the OTA files so our test starts from a known state.
  testDeleteAllFiles();
  fprintf(stderr, ".");

  // Make sure we can successfully reload a file from disk.
  testFileReload();
  testDeleteAllFiles();
  fprintf(stderr, ".");

  // Test the linked list of files.
  testMultipleFiles();
  testDeleteAllFiles();
  fprintf(stderr, ".");

  // Create a file, and then make sure the file exists on disk. This file name
  // comes from emberZclOtaBootloadStorageCreate in ota-bootload-storage-unix.c.
  EmberZclOtaBootloadFileSpec_t spec = {
    .manufacturerCode = 0x1002,
    .type = 0x5678,
    .version = 0x00000006,
  };
  assert(emberZclOtaBootloadStorageCreate(&spec)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  assert(access("1002-5678-00000006.ota", F_OK) == 0);
  assert(emberZclOtaBootloadStorageCreate(&spec)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE);
  fprintf(stderr, ".");
  testFileSpec(&spec, 0);

  // Delete all of the OTA files so our test starts from a known state.
  testDeleteAllFiles();
  fprintf(stderr, ".");

  // Don't run the multiple files test, since the UNIX OTA Storage implementation
  // allows for any number of OTA files.
  runOtaBootloadStorageCoreTest(false); // runMultipleFilesTest?

  // Delete all of the OTA files so our test doesn't leave any garbage behind.
  testDeleteAllFiles();
  fprintf(stderr, ".");

  fprintf(stderr, " done]\n");

  return 0;
}
