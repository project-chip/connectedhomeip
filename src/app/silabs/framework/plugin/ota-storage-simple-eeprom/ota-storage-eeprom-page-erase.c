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
 * @brief This code is intended for EEPROM devices that do not support
 * read-modify-write and must perform a page erase prior to writing data.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

#include "app/framework/plugin/ota-common/ota.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"
#include "app/framework/plugin/ota-client/ota-client.h"
#include EMBER_AF_API_EEPROM

//#define DEBUG_PRINT
#define OTA_STORAGE_EEPROM_INTERNAL_HEADER
#include "ota-storage-eeprom.h"
#undef OTA_STORAGE_EEPROM_INTERNAL_HEADER

#if (EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_READ_MODIFY_WRITE_SUPPORT == FALSE)

#if defined(EMBER_TEST)
 #include "hal/micro/unix/simulation/fake-eeprom.h"
#endif

//------------------------------------------------------------------------------
// Globals

static int32_t lastRecordedByteMaskIndex = -1;
static bool lastRecordedByteMaskIndexKnown = false;

static uint32_t currentEraseOffset;
static uint32_t endEraseOffset;
static bool newEraseOperation;

static EmberAfEventSleepControl storedSleepControl;

// this arbitrary size is to limit the amount we store on the call stack
#define BYTE_MASK_READ_SIZE 20

//------------------------------------------------------------------------------
// Forward declarations

#define startEraseOperation(begin, end)       \
  eraseOperation(true, /* start new erase? */ \
                 (begin),                     \
                 (end))

#define continueEraseOperation()                       \
  eraseOperation(false,   /* start new erase?       */ \
                 0,       /* begin offset (ignored) */ \
                 0)       /* end offset (ignored)   */ \

//------------------------------------------------------------------------------

// We want to get the log(2, PAGE_SIZE) so that we can use bitwise shifts
// instead of multiple and divide for various page size related operations.
// For the xap, 32-bit divide/modulus requires a software library and eats up
// a lot of flash.
static uint8_t determinePageSizeLog(void)
{
  uint8_t pageSizeLog;
  for (pageSizeLog = 0;
       (1 << pageSizeLog) < (emberAfPluginEepromInfo()->pageSize);
       pageSizeLog++) {
  }
  //  debugPrint("PageSizeLog: %d", pageSizeLog);
  return pageSizeLog;
}

static bool checkDelay(bool mustSetTimer)
{
  if (emberAfPluginEepromBusy() || mustSetTimer) {
    uint32_t delay = emberAfPluginEepromInfo()->pageEraseMs >> 2;
    if (delay == 0) {
      delay = 1;
    }
    debugPrint("Waiting %d ms for erase to complete.", delay);
    emberAfEventControlSetDelayMS(&emberAfPluginOtaStorageSimpleEepromPageEraseEventControl,
                                  delay);
    return true;
  }

  return false;
}

// Returns true for success (erase operation continuing or completed)
// Returns false for error (erase not started).
static bool eraseOperation(bool startNewErase,
                           uint32_t beginOffset,
                           uint32_t endOffset)
{
  bool success = true;

  EMBER_TEST_ASSERT(!startNewErase
                    || (startNewErase
                        && emberAfPluginOtaStorageSimpleEepromPageEraseEventControl.status == EMBER_EVENT_INACTIVE));

  // In case the first time we are called the EEPROM is busy,
  // we will delay.  However we haven't erased the first page
  // yet so we must take care not to increment the offset yet.

  if (startNewErase) {
    newEraseOperation = true;
    currentEraseOffset = beginOffset;
    endEraseOffset = endOffset;
    otaPrintln("Starting erase from offset 0x%4X to 0x%4X",
               beginOffset,
               endEraseOffset);
    storedSleepControl = emberAfGetDefaultSleepControlCallback();
    emberAfSetDefaultSleepControlCallback(EMBER_AF_STAY_AWAKE);
  }

  if (checkDelay(false)) {  // must set timer?
    return true;
  }

  if (!newEraseOperation) {
    currentEraseOffset += emberAfPluginEepromInfo()->pageSize;
  }

  if (currentEraseOffset < endEraseOffset) {
    uint8_t status;
    debugPrint("Erasing page %d of %d",
               (currentEraseOffset >> determinePageSizeLog()) + 1,
               (endEraseOffset >> determinePageSizeLog()));
    status = emberAfPluginEepromErase(currentEraseOffset, emberAfPluginEepromInfo()->pageSize);
    success = (status == EEPROM_SUCCESS);
    newEraseOperation = false;
    if (success) {
      checkDelay(true); // must set timer?
      return true;
    }
    otaPrintln("Could not start ERASE! (0x%X)", status);
  }

  emberAfSetDefaultSleepControl(storedSleepControl);

  otaPrintln("EEPROM Erase complete");

  if (!emAfOtaStorageCheckDownloadMetaData()) {
    // This was a full erase that wiped the meta-data.
    emAfOtaStorageWriteDownloadMetaData();
  }

  emberAfPluginOtaStorageSimpleEepromEraseCompleteCallback(success);
  return true;
}

static bool isMultipleOfPageSize(uint32_t address)
{
  uint32_t pageSizeBits = ((1 << determinePageSizeLog()) - 1);
  return ((pageSizeBits & address) == 0);
}

void emAfOtaStorageEepromInit(void)
{
  uint16_t expectedCapabilities = (EEPROM_CAPABILITIES_PAGE_ERASE_REQD
                                   | EEPROM_CAPABILITIES_ERASE_SUPPORTED);
  uint32_t spaceReservedForOta = (otaStorageEepromGetStorageEndAddress()
                                  - otaStorageEepromGetStorageStartAddress());
  const HalEepromInformationType *info = emberAfPluginEepromInfo();

  // NOTE: if the info pointer is NULL it's a good indicator that your data
  // flash isn't properly connected and jumpered in or that your bootloader
  // is too old to support EEPROM info.
  assert(info != NULL);
  assert(expectedCapabilities
         == (info->capabilitiesMask & expectedCapabilities));
  assert(isMultipleOfPageSize(otaStorageEepromGetStorageStartAddress()));
  assert(isMultipleOfPageSize(spaceReservedForOta));

  // Need to make sure that the bytemask used to store each
  // fully downloaded page is big enough to hold all the pages we have been
  // allocated.
  assert((MAX_BYTEMASK_LENGTH / emberAfPluginEepromGetWordSize())
         >= (spaceReservedForOta >> determinePageSizeLog()));
}

void emberAfPluginOtaStorageSimpleEepromPageEraseEventHandler(void)
{
  emberEventControlSetInactive(emberAfPluginOtaStorageSimpleEepromPageEraseEventControl);
  continueEraseOperation();
}

static int32_t getByteMaskIndexFromEeprom(void)
{
  uint8_t byteMask[BYTE_MASK_READ_SIZE];
  uint32_t readOffset = otaStorageEepromGetImageInfoStartAddress()
                        + SAVED_DOWNLOAD_OFFSET_INDEX;
  uint16_t byteMaskIndex;
  uint8_t wordSize = emberAfPluginEepromGetWordSize();

  for (byteMaskIndex = 0;
       byteMaskIndex < MAX_BYTEMASK_LENGTH;
       byteMaskIndex += BYTE_MASK_READ_SIZE,
       readOffset  += BYTE_MASK_READ_SIZE) {
    uint8_t i;
    uint8_t status = emberAfPluginEepromRead(readOffset,
                                             byteMask,
                                             BYTE_MASK_READ_SIZE);
    debugPrint("Bytemask read status: 0x%X", status);
    EMBER_TEST_ASSERT(status == 0);

    if (byteMaskIndex == 0 && byteMask[0] == 0xFF) {
      debugFlush();
      debugPrint("All bytes in bytemask erased, assuming index of -1");
      return -1;
    }

    for (i = 0; i < BYTE_MASK_READ_SIZE; i += wordSize) {
      if (byteMask[i] == 0xFF) {
        uint16_t index = (byteMaskIndex + i - 1) / wordSize;
        debugPrint("Last Download offset Bytemask index: %d",
                   index);
        return (index);
      }
    }
  }

  debugPrint("Error in determining bytemask index, assuming -1");
  return -1;
}

// The bytemask notes the real EEPROM offset of the pages that have been fully
// downloaded.  Each downloaded page is recorded as a 0 byte.  The OTA offset
// is determined based on the SOC_BOOTLOADING_SUPPORT.  In that case the
// first page is considered to be the combination of the portion of the OTA
// image at the bottom of the EEPROM space (the OTA header) and the first
// full flash page at the top of the EEPROM space (the EBL and its data).
// Without SOC bootloading support the OTA offset is equivalent to number
// of EEPROM pages written minus the overhead of the meta-data (namely this
// bytemask and some other data).
static uint32_t getOffsetFromByteMaskIndex(int32_t byteMaskIndex)
{
  // To convert to the number of fully written pages from the bytemask index
  // we must add 1.
  int32_t writtenPages = byteMaskIndex + 1;
  uint32_t otaOffset = (((uint32_t)(writtenPages)) << determinePageSizeLog());

  debugPrint("Unadjusted offset:    0x%4X", otaOffset);

  if (otaOffset != 0) {
#if defined(SOC_BOOTLOADING_SUPPORT)
    otaOffset += emAfGetEblStartOffset();
#else
    otaOffset -= OTA_HEADER_INDEX;
#endif
  }

  debugFlush();
  debugPrint("Last OTA Download offset: 0x%4X", otaOffset);
  debugFlush();

  return otaOffset;
}

static int32_t getByteMaskIndexFromOtaOffset(uint32_t otaOffset)
{
  int32_t adjustment;

#if defined(SOC_BOOTLOADING_SUPPORT)
  adjustment = emAfGetEblStartOffset();
#else
  adjustment = 0 - OTA_HEADER_INDEX;
#endif

  // debugPrint("Offset: 0x%4X, Adjustment: 0x%4X, EBL Start Offset: 0x%4X, Page Log: %d, Page Size: %d",
  //            otaOffset,
  //            adjustment,
  //            emAfGetEblStartOffset(),
  //            determinePageSizeLog(),
  //            emberAfPluginEepromInfo()->pageSize);

  if (otaOffset < (emberAfPluginEepromInfo()->pageSize + adjustment)) {
    return -1;
  }

  return (((otaOffset + adjustment) >> determinePageSizeLog()) - 1);
}

void emAfStorageEepromUpdateDownloadOffset(uint32_t otaOffsetNew, bool finalOffset)
{
  int32_t byteMaskIndexNew = getByteMaskIndexFromOtaOffset(otaOffsetNew);

  // debugPrint("Checking whether to update bytemask, New Offset: 0x%4X, new bytemask index: %d, old bytemask index: %d, final update: %c",
  //            otaOffsetNew,
  //            byteMaskIndexNew,
  //            lastRecordedByteMaskIndex,
  //            (finalOffset ? 'y' : 'n'));

  if (finalOffset
      && byteMaskIndexNew == lastRecordedByteMaskIndex) {
    byteMaskIndexNew++;
  }

  if (byteMaskIndexNew > lastRecordedByteMaskIndex) {
    uint8_t status;
    uint8_t byteArray[2] = { 0, 0 };

    debugFlush();
    debugPrint("Writing Last Download offset bytemask, new (old): %d (%d)",
               byteMaskIndexNew,
               lastRecordedByteMaskIndex);
    debugFlush();
    debugPrint("OTA Offsets, new (old): 0x%4X (0x%4X)",
               otaOffsetNew,
               getOffsetFromByteMaskIndex(lastRecordedByteMaskIndex));
    debugFlush();

    status = emberAfPluginEepromWrite(
      (otaStorageEepromGetImageInfoStartAddress()
       + SAVED_DOWNLOAD_OFFSET_INDEX
       + (byteMaskIndexNew
          * emberAfPluginEepromGetWordSize())),
      byteArray,
      emberAfPluginEepromGetWordSize());
    debugPrint("EEPROM Write status: 0x%X", status);
    EMBER_TEST_ASSERT(status == 0);

    lastRecordedByteMaskIndex = getByteMaskIndexFromEeprom();

    EMBER_TEST_ASSERT(lastRecordedByteMaskIndex == byteMaskIndexNew);
  }
}

void emAfOtaWipeStorageDevice(void)
{
  emberAfOtaStorageDriverInvalidateImageCallback();
}

EmberAfOtaStorageStatus emberAfOtaStorageDriverInvalidateImageCallback(void)
{
  lastRecordedByteMaskIndex = -1;
  lastRecordedByteMaskIndexKnown = false;

  return (startEraseOperation(otaStorageEepromGetStorageStartAddress(),
                              otaStorageEepromGetStorageEndAddress())
          ? EMBER_AF_OTA_STORAGE_OPERATION_IN_PROGRESS
          : EMBER_AF_OTA_STORAGE_ERROR);
}

uint32_t emberAfOtaStorageDriverRetrieveLastStoredOffsetCallback(void)
{
  if (!emAfOtaStorageCheckDownloadMetaData()) {
    return 0;
  }

  // Since retrieving the last download offset from the bytemask
  // may involve multiple halEepromRead() calls and this may be slow,
  // we cache the offset.

  if (!lastRecordedByteMaskIndexKnown) {
    lastRecordedByteMaskIndex = getByteMaskIndexFromEeprom();
    lastRecordedByteMaskIndexKnown = true;
  }
  return getOffsetFromByteMaskIndex(lastRecordedByteMaskIndex);
}

EmberAfOtaStorageStatus emberAfOtaStorageDriverPrepareToResumeDownloadCallback(void)
{
  uint32_t otaEepromStart;
  uint32_t pageOffsetStart;
  uint32_t pageSize;

  if (lastRecordedByteMaskIndex < 0) {
    return EMBER_AF_OTA_STORAGE_ERROR;
  }

  otaEepromStart = otaStorageEepromGetStorageStartAddress();
  pageSize = emberAfPluginEepromInfo()->pageSize;

  pageOffsetStart = (lastRecordedByteMaskIndex + 1) << determinePageSizeLog();

  return (startEraseOperation(otaEepromStart + pageOffsetStart,
                              otaEepromStart + pageOffsetStart + pageSize)
          ? EMBER_AF_OTA_STORAGE_OPERATION_IN_PROGRESS
          : EMBER_AF_OTA_STORAGE_ERROR);
}

#if defined(DEBUG_PRINT)
void emAfEepromTest(void)
{
  // This function works only for blocking IO calls

  uint16_t page = 0;
  uint8_t writeBuffer[16];
  uint16_t i;
  uint8_t status;

  uint16_t writes = emberAfPluginEepromInfo()->pageSize / 16;

  status = emberAfPluginEepromErase(page * emberAfPluginEepromInfo()->pageSize,
                                    emberAfPluginEepromInfo()->pageSize);
  if (status != 0) {
    debugPrint("Failed to erase page %d, status: 0x%X", page, status);
    return;
  }

  debugPrint("Number of writes: %d", writes);

  for (i = 0; i < writes; i++) {
    MEMSET(writeBuffer, i, 16);
    status = emberAfPluginEepromWrite(page + (i * 16),
                                      writeBuffer,
                                      16);
    debugPrint("Write address 0x%4X, length %d, status: 0x%X",
               page + i,
               16,
               status);
    if (status != 0) {
      return;
    }
  }
  debugPrint("All data written successfully.");
}
#endif

#endif // #if (EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_READ_MODIFY_WRITE_SUPPORT == FALSE)
