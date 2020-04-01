/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include "ota-bootload-storage-core.h"
#include "ota-bootload-storage-core-test.h"
#include "ota-static-file-data.h"

// This is a generic test that can be run on any implementation of the
// ota-bootload-storage-core API.

// The values for myFileSpec are obtained from ota-static-file-data.h.
const EmberZclOtaBootloadFileSpec_t myFileSpec = {
  .manufacturerCode = 0x1002,
  .type             = 0x5678,
  .version          = 0x00000005,
};
static const EmberZclOtaBootloadFileSpec_t myOtherFileSpec = {
  .manufacturerCode = 1234,
  .type             = 5678,
  .version          = 0xD00DF00D,
};
static size_t offset = 0;
static uint8_t readData[STATIC_IMAGE_DATA_SIZE];
static const uint8_t writeData[STATIC_IMAGE_DATA_SIZE] = STATIC_IMAGE_DATA;
static EmberZclOtaBootloadStorageStatus_t currentStorageStatus = EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_NULL;

static void deleteCallback(EmberZclOtaBootloadStorageStatus_t storageStatus)
{
  currentStorageStatus = storageStatus;
}

static size_t getMaxFileSize(void)
{
  EmberZclOtaBootloadStorageInfo_t info;
  emberZclOtaBootloadStorageGetInfo(&info, NULL, 0);
  return info.maximumFileSize;
}

static void testValidOtaHeader(const uint8_t *bytes)
{
  EmberZclOtaBootloadFileHeaderInfo_t fileHeaderInfo;
  assert(emberZclOtaBootloadFetchFileHeaderInfo(bytes, &fileHeaderInfo)
         == EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_VALID);
  assert(fileHeaderInfo.version == EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION);
  assert(fileHeaderInfo.headerSize == STATIC_IMAGE_DATA_HEADER_SIZE);
  assert(fileHeaderInfo.spec.manufacturerCode == myFileSpec.manufacturerCode);
  assert(fileHeaderInfo.spec.type == myFileSpec.type);
  assert(fileHeaderInfo.spec.version == myFileSpec.version);
  assert(fileHeaderInfo.stackVersion
         == EMBER_ZCL_OTA_BOOTLOAD_STACK_VERSION_IP);
  assert(memcmp(fileHeaderInfo.string,
                STATIC_IMAGE_HEADER_STRING,
                EMBER_ZCL_OTA_BOOTLOAD_HEADER_STRING_SIZE)
         == 0);
  assert(fileHeaderInfo.fileSize == STATIC_IMAGE_DATA_SIZE);
  assert(fileHeaderInfo.securityCredentialVersion
         == EMBER_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_IP);

  // See EmberZclOtaBootloadFileHeaderInfo_t doxygen.
  for (size_t i = 0; i < EMBER_ZCL_UID_SIZE; i++) {
    assert(fileHeaderInfo.destination.bytes[i] == 0xFF);
  }
  assert(fileHeaderInfo.hardwareVersionRange.minimum
         == EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL);
  assert(fileHeaderInfo.hardwareVersionRange.maximum
         == EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL);
}

static void testInvalidOtaHeader(void)
{
  EmberZclOtaBootloadFileHeaderInfo_t fileHeaderInfo;
  uint8_t image[STATIC_IMAGE_DATA_SIZE] = STATIC_IMAGE_DATA;
  size_t index;

  // Magic number (index 0 in the OTA header).
  index = 0;
  for (size_t i = 0;
       i < EMBER_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER_SIZE;
       i++, index++) {
    image[index] += (i + 1);
    assert(emberZclOtaBootloadFetchFileHeaderInfo(image, &fileHeaderInfo)
           == EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_MAGIC_NUMBER);
    image[index] -= (i + 1);
  }

  // File version (index 4 in the OTA header).
  index = 4;
  for (size_t i = 0; i < sizeof(fileHeaderInfo.version); i++, index++) {
    image[index] += (i + 1);
    assert(emberZclOtaBootloadFetchFileHeaderInfo(image, &fileHeaderInfo)
           == EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_VERSION);
    image[index] -= (i + 1);
  }

  // Header length (index 6 in the OTA header).
  index = 6;
  for (size_t i = 0; i < sizeof(fileHeaderInfo.headerSize); i++, index++) {
    image[index] += (i + 1);
    assert(emberZclOtaBootloadFetchFileHeaderInfo(image, &fileHeaderInfo)
           == EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_HEADER_SIZE);
    image[index] -= (i + 1);
  }

  // Stack version (index 18 in the OTA header).
  index = 18;
  for (size_t i = 0; i < sizeof(fileHeaderInfo.stackVersion); i++) {
    image[index] += (i + 1);
    assert(emberZclOtaBootloadFetchFileHeaderInfo(image, &fileHeaderInfo)
           == EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_STACK_VERSION);
    image[index] -= (i + 1);
  }

  // Security credential (index 56 in the OTA header).
  index = 56;
  for (size_t i = 0; i < sizeof(fileHeaderInfo.securityCredentialVersion); i++) {
    image[index] += (i + 1);
    assert(emberZclOtaBootloadFetchFileHeaderInfo(image, &fileHeaderInfo)
           == EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_SECURITY_CREDENTIAL_VERSION);
    image[index] += (i - 1);
  }
}

static void testFileCount(size_t count)
{
  EmberZclOtaBootloadStorageInfo_t info;
  EmberZclOtaBootloadFileSpec_t fileSpec;
  emberZclOtaBootloadStorageGetInfo(&info, &fileSpec, 1);
  assert(info.fileCount == count);
  if (count > 0) {
    assert(emberZclOtaBootloadFileSpecsAreEqual(&fileSpec, &myFileSpec));
  }
}

static void testFileSize()
{
  EmberZclOtaBootloadStorageFileInfo_t fileInfo;
  assert(emberZclOtaBootloadStorageFind(&myFileSpec, &fileInfo)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  assert(fileInfo.size == offset);
}

static void testReadWrite(void)
{
  const size_t dataLengths[] = { 16, 8, 16, };
  for (size_t i = 0; i < COUNTOF(dataLengths); i++) {
    assert(emberZclOtaBootloadStorageWrite(&myFileSpec,
                                           offset,
                                           writeData + offset,
                                           dataLengths[i])
           == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
    assert(emberZclOtaBootloadStorageRead(&myFileSpec,
                                          offset,
                                          readData + offset,
                                          dataLengths[i])
           == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);

    // Since the EEPROM plugin caches partial word writes, we may not be able to
    // read back all of the bytes that we supposedly wrote. In the worst case,
    // there will be two bytes missing, one at the end of the EEPROM address
    // space, and one at the beginning of the EEPROM address space.
    assert(memcmp(readData + offset,
                  writeData + offset,
                  dataLengths[i] - 2)
           == 0);

    offset += dataLengths[i];
    testFileSize();
  }
}

static void testInvalidFile(void)
{
  EmberZclOtaBootloadStorageFileInfo_t fileInfo;
  assert(emberZclOtaBootloadStorageRead(&myFileSpec,
                                        0,         // offset, whatever
                                        readData,
                                        10)        // dataLength, whatever
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE);
  assert(emberZclOtaBootloadStorageWrite(&myFileSpec,
                                         0,         // offset, whatever
                                         writeData,
                                         10)        // dataLength, whatever
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE);
  assert(emberZclOtaBootloadStorageDelete(&myFileSpec, deleteCallback)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE);
  assert(emberZclOtaBootloadStorageFind(&myFileSpec, &fileInfo)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE);

  assert(emberZclOtaBootloadStorageDelete(&emberZclOtaBootloadFileSpecNull,
                                          deleteCallback)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
}

void runOtaBootloadStorageCoreTest(bool runMultipleFilesTest)
{
  // Test (for sanity) that the writeData contains a valid OTA header.
  testValidOtaHeader(writeData);
  fprintf(stderr, ".");

  // Test (for insanity, and because this is the best place to do it) invalid
  // header data.
  testInvalidOtaHeader();
  fprintf(stderr, ".");

  // Test calling functions when no files exist.
  testInvalidFile();
  testFileCount(0);
  fprintf(stderr, ".");

  // Test file creation.
  assert(emberZclOtaBootloadStorageCreate(&myFileSpec)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  testFileCount(1);
  testFileSize();
  fprintf(stderr, ".");

  // Test reading when there is nothing to read.
  assert(emberZclOtaBootloadStorageRead(&myFileSpec,
                                        0,         // offset, at the beginning
                                        readData,
                                        10)        // dataLength, whatever
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_RANGE);
  assert(emberZclOtaBootloadStorageRead(&myFileSpec,
                                        100,       // offset, in the middle
                                        readData,
                                        10)        // dataLength, whatever
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_RANGE);
  fprintf(stderr, ".");

  // Test reading and writing successfully.
  testReadWrite();
  fprintf(stderr, ".");

  // Test creating a file that has a different file spec than the one currently
  // created. Make sure the original still exists
  if (runMultipleFilesTest) {
    assert(emberZclOtaBootloadStorageCreate(&myOtherFileSpec)
           == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_SPACE);
  }
  testReadWrite();
  testFileCount(1);
  fprintf(stderr, ".");

  // Test creating a file that has a different file spec than the one currently
  // created. Make sure the original still exists
  assert(emberZclOtaBootloadStorageCreate(&myFileSpec)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE);
  testReadWrite();
  testFileCount(1);
  fprintf(stderr, ".");

  // Test deleting a file. Make sure the file is really gone.
  assert(emberZclOtaBootloadStorageDelete(&myFileSpec, deleteCallback)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  deletionTimePassesCallback();
  assert(currentStorageStatus == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  testInvalidFile();
  testFileCount(0);
  fprintf(stderr, ".");

  // Test creating a file again after deleting it.
  assert(emberZclOtaBootloadStorageCreate(&myFileSpec)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  offset = 0;
  testFileSize();
  testFileCount(1);
  fprintf(stderr, ".");

  // Test deleting all the files.
  assert(emberZclOtaBootloadStorageDelete(&emberZclOtaBootloadFileSpecNull,
                                          deleteCallback)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  deletionTimePassesCallback();
  assert(currentStorageStatus == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  testInvalidFile();
  testFileCount(0);
  fprintf(stderr, ".");

  // Test creating a file again after deleting it (again).
  assert(emberZclOtaBootloadStorageCreate(&myFileSpec)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  offset = 0;
  testFileSize();
  testFileCount(1);
  fprintf(stderr, ".");

  // Test writing too much.
  assert(emberZclOtaBootloadStorageWrite(&myFileSpec,
                                         offset,
                                         writeData,
                                         getMaxFileSize() + 1)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_RANGE);
  testFileSize();
  fprintf(stderr, ".");

  // Test writing and reading the whole file.
  assert(emberZclOtaBootloadStorageWrite(&myFileSpec,
                                         0,
                                         writeData,
                                         STATIC_IMAGE_DATA_SIZE)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  assert(emberZclOtaBootloadStorageRead(&myFileSpec,
                                        0,
                                        readData,
                                        STATIC_IMAGE_DATA_SIZE)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  assert(memcmp(readData, writeData, STATIC_IMAGE_DATA_SIZE) == 0);
  offset = STATIC_IMAGE_DATA_SIZE;
  testFileSize();
  fprintf(stderr, ".");

  // Test that the OTA header we read is still legit.
  testValidOtaHeader(readData);
  fprintf(stderr, ".");
}
