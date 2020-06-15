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
 * @brief Test harness code specific to the SOC.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/util/util.h"

#include "test-harness.h"

//------------------------------------------------------------------------------
// Globals

#if defined(CORTEXM3) && !defined(EZSP_HOST)
  #define HASH_CODE_SUPPORT
  #include "hal/micro/cortexm3/memmap.h"
  #include "hal/micro/cortexm3/memmap-tables.h"
#endif

//------------------------------------------------------------------------------
// Forward Declarations

EmberEventControl emberAfPluginTestHarnessFlashPageHashEventControl;

// Internal stack routines
void emResetApsFrameCounter(void);
void emTestHarnessAdvanceApsFrameCounter(void);

#if defined(HASH_CODE_SUPPORT)
static uint8_t pageRangeIndex;
static EmberAesMmoHashContext context;
static uint8_t pageWithinRangeIndex;
#endif

#define FLASH_PAGE_SIZE_35X 2048

// This adds very verbose printing that is useful only for debugging the
// hash calculation and making sure it matches em3xx_convert
//#define PRINT_INTERMEDIATE_HASH_RESULTS

//------------------------------------------------------------------------------
// Functions

void emAfTestHarnessResetApsFrameCounter(void)
{
  // Reseting the outgoing APS frame counter is non-standard and not
  // a good idea, especially on the TC.  This is necessary for Smart Energy
  // Key establihsment tests 15.39 and 15.40.  It is only necessary for a test
  // harness device.
  emResetApsFrameCounter();
}

void emAfTestHarnessAdvanceApsFrameCounter(void)
{
  emTestHarnessAdvanceApsFrameCounter();
}

#if defined(HASH_CODE_SUPPORT)
// The first page contains the AAT, which in turn contains the image stamp.
// To calculate the same hash as the em3xx_convert we must hash the AAT
// with a value of all F's for the image stamp.
static void hashTheFirstPage(uint8_t* address)
{
  const uint16_t offset = (// Base Address Table format
    +4                      // top of stack
    + 4                     // reset vector
    + 4                     // nmi handler
    + 4                     // hard fault handler
    + 2                     // type
    + 2                     // version
    + 4                     // vector table

    // AAT format
    + 1                     // platform info
    + 1                     // micro info
    + 1                     // phy info
    + 1                     // AAT Size
    + 2                     // Software Version
    + 2                     // Software Build
    + 4                     // timestamp
    + IMAGE_INFO_MAXLEN
    + 4                     // image CRC
    + (2 * 6)                // pageRange_t * 6 pages
    + 4                     // simEE bottom
    + 4                     // customer Application version
    + 4);                   // internal storage bottom

  uint8_t aatBuff[AAT_MAX_SIZE];
  MEMMOVE(aatBuff, address, AAT_MAX_SIZE);
  MEMSET(aatBuff + offset, 0xFF, IMAGE_STAMP_SIZE);

  assert(EMBER_SUCCESS == (emberAesMmoHashUpdate(&context,
                                                 AAT_MAX_SIZE,
                                                 aatBuff)));
#if defined(PRINT_INTERMEDIATE_HASH_RESULTS)
  emberAfCorePrint("AAT Hash Result: 0x");
  emberAfPrint8ByteBlocks(2, context.result, false);
#endif

  assert(EMBER_SUCCESS == (emberAesMmoHashUpdate(&context,
                                                 FLASH_PAGE_SIZE_35X - AAT_MAX_SIZE,
                                                 address + AAT_MAX_SIZE)));
#if defined(PRINT_INTERMEDIATE_HASH_RESULTS)
  emberAfCorePrint("Rest of 1st Page Hash Result: 0x");
  emberAfPrint8ByteBlocks(2, context.result, false);
#endif
}

#endif // HASH_CODE_SUPPORT

void emberAfPluginTestHarnessFlashPageHashEventHandler(void)
{
#if defined(HASH_CODE_SUPPORT)
  uint8_t pageBeg = halAppAddressTable.pageRanges[pageRangeIndex].begPg;
  uint8_t pageEnd = halAppAddressTable.pageRanges[pageRangeIndex].endPg;
  const uint8_t* address = (uint8_t*)(PAGE_TO_PROG_ADDR(pageBeg + pageWithinRangeIndex));

  emberEventControlSetInactive(emberAfPluginTestHarnessFlashPageHashEventControl);

#if defined(PRINT_INTERMEDIATE_HASH_RESULTS)
  emberAfCorePrintln("Processing Page Range Index %d, page %d, address: 0x%4X",
                     pageRangeIndex,
                     pageBeg + pageWithinRangeIndex,
                     (uint32_t)address);
#endif
  assert(EMBER_SUCCESS == (emberAesMmoHashUpdate(&context,
                                                 FLASH_PAGE_SIZE_35X,
                                                 address)));
#if defined(PRINT_INTERMEDIATE_HASH_RESULTS)
  emberAfCorePrint("Current Hash Result: 0x");
  emberAfPrint8ByteBlocks(2, context.result, false);
#endif

  pageWithinRangeIndex++;
  if ((pageBeg + pageWithinRangeIndex) > pageEnd) {
    pageRangeIndex++;
    if (pageRangeIndex >= 6
        || (halAppAddressTable.pageRanges[pageRangeIndex].begPg == 0xFF)) {
      const uint8_t unsetImageStamp[] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      };
      assert(EMBER_SUCCESS == emberAesMmoHashFinal(&context,
                                                   0,
                                                   NULL));
      emberAfCorePrintln("All pages in all ranges hashed.  Result: ");
      emberAfPrint8ByteBlocks(2, context.result, false);

      if (0 == MEMCOMPARE(unsetImageStamp, context.result, IMAGE_STAMP_SIZE)) {
        emberAfCorePrintln("Image stamp in AAT is unset (all F's).");
      } else {
        emberAfCorePrint("Image stamp in AAT: ");
        emberAfPrint8ByteBlocks(1, halAppAddressTable.imageStamp, false);
        emberAfCorePrintln("Image stamp %pmatch first 8 bytes in hash result.",
                           (0 == MEMCOMPARE(halAppAddressTable.imageStamp,
                                            context.result,
                                            IMAGE_STAMP_SIZE))
                           ? ""
                           : "DOES NOT ");
      }

      return;
    }
  }
  emberEventControlSetDelayMS(emberAfPluginTestHarnessFlashPageHashEventControl,
                              1);
#endif  // HASH_CODE_SUPPORT
}

void emAfTestHarnessStartImageStampCalculation(void)
{
#if defined(HASH_CODE_SUPPORT)
  uint8_t i;
  emberAesMmoHashInit(&context);

  emberAfCorePrintln("AAT Flash page ranges");
  for (i = 0; i < 6; i++) {
    emberAfCorePrintln("Index %d: Beg Page: %d, End Page: %d",
                       i,
                       halAppAddressTable.pageRanges[i].begPg,
                       halAppAddressTable.pageRanges[i].endPg);
  }

  pageRangeIndex = 0;
  pageWithinRangeIndex = 0;
  emberAfCorePrintln("Starting hash of all flash pages known by AAT.");
  hashTheFirstPage((uint8_t*)(PAGE_TO_PROG_ADDR(halAppAddressTable.pageRanges[0].begPg)));
  pageWithinRangeIndex++;
  emberEventControlSetActive(emberAfPluginTestHarnessFlashPageHashEventControl);
#else
  emberAfCorePrintln("Not supported on this micro.");
#endif
}

//------------------------------------------------------------------------------
