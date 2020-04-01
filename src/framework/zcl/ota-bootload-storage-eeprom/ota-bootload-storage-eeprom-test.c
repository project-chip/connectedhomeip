/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_HAL
#include CHIP_AF_API_EEPROM
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE
#include "../ota-bootload-storage-core/ota-bootload-storage-core-test.h"
#include "../ota-bootload-storage-core/ota-static-file-data.h"
#include "micro/unix/simulation/fake-eeprom.h"

// -----------------------------------------------------------------------------
// Stubs

// Cheat and include fake-eeprom.c directly so that we can workaround a build
// issue caused by an implicit declaration of simPrint.

void simPrint(char* format, ...)
{
  va_list argPointer;
  va_start(argPointer, format);
  vfprintf(stderr, format, argPointer);
  va_end(argPointer);
}

#include "micro/unix/simulation/fake-eeprom.c"

#define USE_STUB_halCommonIdleForMilliseconds
#define USE_STUB_emTasks
#define USE_STUB_emTaskCount
#include "stack/ip/stubs.c"

// -----------------------------------------------------------------------------
// Test stuff

static ChipZclOtaBootloadStorageStatus_t currentStorageStatus = CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_NULL;

void deletionTimePassesCallback(void)
{
  extern ChipEventControl chZclOtaBootloadStorageEepromEraseEventControl;
  extern void chZclOtaBootloadStorageEepromEraseEventHandler(void);
  const ChipEventData events[] = {
    { &chZclOtaBootloadStorageEepromEraseEventControl, chZclOtaBootloadStorageEepromEraseEventHandler, },
    { NULL, NULL, },
  };

  // The EEPROM storage implementation sets the erase event to active during in
  // the Delete() API call. Therefore, we should erase one of the pages without
  // any delay.
  microSetSystemTime(0);
  chipRunEvents(events);
  for (size_t i = 0;
       i < (chipAfPluginEepromInfo()->partSize
            / chipAfPluginEepromInfo()->pageSize);
       i++) {
    // We should wait for the page erase operation to complete before running
    // the erase event again. We run the erase operation one more times than
    // the page count, because we want to make sure that the erase of the last
    // page finished successfully.
    microSetSystemTime(halCommonGetInt32uMillisecondTick()
                       + chipAfPluginEepromInfo()->pageEraseMs);
    chipRunEvents(events);
  }
}

static void deleteCallback(ChipZclOtaBootloadStorageStatus_t storageStatus)
{
  currentStorageStatus = storageStatus;
}

static void testOneByteMissing(void)
{
  // If the OTA file contains an odd number of bytes starting at the first
  // tag data, then we may write a partial word (assuming a word size of 2) at
  // the very end of the file. This test case is to ensure that we flush those
  // partial words after we have written the whole OTA file.
  uint8_t otaFileData[] = STATIC_IMAGE_DATA;

  ChipZclOtaBootloadFileHeaderInfo_t fileHeaderInfo;
  assert(chipZclOtaBootloadFetchFileHeaderInfo(otaFileData, &fileHeaderInfo)
         == CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_VALID);

  // Change the file size (index 52) to an even length. This forces the file
  // length starting at the first tag data to be odd, since the header length
  // is odd.
  chipStoreLowHighInt32u(otaFileData + 52, fileHeaderInfo.fileSize - 1);

  // Create, write, and then read the whole file. This ensures that all bytes
  // are written, including the last partial word.
  uint8_t buffer[STATIC_IMAGE_DATA_SIZE];
  assert(chipZclOtaBootloadStorageCreate(&myFileSpec)
         == CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  assert(chipZclOtaBootloadStorageWrite(&myFileSpec,
                                         0,                           // offset
                                         otaFileData,
                                         fileHeaderInfo.fileSize - 1) // size
         == CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  assert(chipZclOtaBootloadStorageRead(&myFileSpec,
                                        0,                           // offset
                                        buffer,
                                        fileHeaderInfo.fileSize - 1) // size
         == CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  assert(memcmp(otaFileData, buffer, fileHeaderInfo.fileSize - 1) == 0);
}

void chipAfPluginEepromStateChangeCallback(HalEepromState oldEepromState, HalEepromState newEepromState)
{
}

int main(int argc, char *argv[])
{
  // See main.c for another example of this initialization
  setupFakeEeprom(((CHIP_AF_PLUGIN_OTA_BOOTLOAD_STORAGE_EEPROM_END_ADDRESS
                    - CHIP_AF_PLUGIN_OTA_BOOTLOAD_STORAGE_EEPROM_START_ADDRESS)
                   + 1),
                  CHIP_AF_PLUGIN_OTA_BOOTLOAD_STORAGE_EEPROM_START_ADDRESS,
                  FAKE_EEPROM_DEFAULT_PAGE_SIZE,
                  FAKE_EEPROM_DEFAULT_PAGE_ERASE_MS,
                  true,   /* pageEraseRequired? */
                  2);     /* wordSizeBytes */

  // From ota-bootload-storage-eeprom.c.
  extern void chZclOtaBootloadStorageEepromInitCallback(void);
  chZclOtaBootloadStorageEepromInitCallback();

  fprintf(stderr, "[%s ", argv[0]);

  runOtaBootloadStorageCoreTest(true); // runMultipleFilesTest?

  // Clear all the files so that we have a clean state to run the one byte
  // missing test.
  assert(chipZclOtaBootloadStorageDelete(&chipZclOtaBootloadFileSpecNull,
                                          deleteCallback)
         == CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  deletionTimePassesCallback();
  assert(currentStorageStatus == CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  fprintf(stderr, ".");

  testOneByteMissing();
  fprintf(stderr, ".");

  fprintf(stderr, " done]\n");

  return 0;
}
