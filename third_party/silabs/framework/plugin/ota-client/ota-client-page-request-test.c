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
 * @brief Page request test routines for the OTA Client plugin.
 *******************************************************************************
   ******************************************************************************/

#define EMBER_AF_PLUGIN_OTA_CLIENT_USE_PAGE_REQUEST
#define EMBER_AF_PLUGIN_OTA_CLIENT_PAGE_REQUEST_TIMEOUT_SECONDS 3

// This allows us to alter the page sizes dynamically so we don't have
// to worry about
#define PAGE_REQUEST_TEST

// Yes, include the C file directly because it gives us access to set local
// #defines and retrieve static variables we can't normally see.
#include "ota-client-page-request.c"

#include "app/framework/test/test-framework.h"
#include <assert.h>

//------------------------------------------------------------------------------
// Globals

// If you are debugging, you may want to turn off the big sizes and stick
// to smaller ones, especially when you enable debug printing.  It is very
// verbose.
static uint32_t totalImageSizes[] = { 1024, 2000, 50000, 192365 };

static const char* suffix[] = {
  "th",
  "st",
  "nd",
  "rd",
  "th",
  "th",
  "th",
  "th",
  "th",
  "th",
};

#define ALL_BLOCKS_MISSED 0xFFFF

EmberAfClusterCommand* emAfCurrentCommand = NULL;

//------------------------------------------------------------------------------
// Functions

static void printMask(void)
{
  uint8_t i;
  debug("Mask: ");
  for (i = 0; i < BITMASK_COUNT; i++) {
    debug("0x%2X ", blocksReceivedBitmask[i]);
  }
  debug("\n");
}

static const char* getSuffix(uint32_t number)
{
  uint32_t remainder = number % 10;
  return suffix[remainder];
}

// In order to create predictability in our tests, we simulate missing
// packets in a well-defined, periodic fashion.

static void pageRequestTestWithMissedBlocks(uint16_t missedBlockModulus)
{
  uint8_t i;
  uint32_t j;
  debug("\n");
  for (i = 0; i < sizeof(totalImageSizes) / sizeof(uint32_t); i++) {
    debug("\n*** Image size of %d. page size %d (",
          PAGE_SIZE,
          totalImageSizes[i]);
    if (missedBlockModulus) {
      debug("dropping every %d%s block",
            missedBlockModulus,
            getSuffix(missedBlockModulus));
    } else {
      debug("no missed blocks");
    }
    debug(") ***\n");
    uint32_t totalPages = totalImageSizes[i] / PAGE_SIZE;
    if (totalImageSizes[i] % PAGE_SIZE != 0) {
      totalPages++;
    }
    for (j = 0; j < totalPages; j++) {
      uint32_t currentOffset = j * PAGE_SIZE;
      uint32_t k;
      debug("  Page %d\n", j);
      assert(0 != emAfInitPageRequestClient(currentOffset,
                                            totalImageSizes[i]));
      for (k = currentOffset;
           k < (currentOffset + PAGE_SIZE)
           && (k < totalImageSizes[i])
           && missedBlockModulus != ALL_BLOCKS_MISSED;
           k += EM_AF_PAGE_REQUEST_BLOCK_SIZE) {
        debug("    Offset 0x%4X  ", k);
        if (missedBlockModulus
            && (((k / EM_AF_PAGE_REQUEST_BLOCK_SIZE)
                 % missedBlockModulus) == 0)) {
          debug("Simulating missed block %d.  ",
                ((k - (j * PAGE_SIZE)) / EM_AF_PAGE_REQUEST_BLOCK_SIZE));
          printMask();
        } else {
          assert(EM_AF_PAGE_REQUEST_ERROR
                 != emAfNoteReceivedBlockForPageRequestClient(k));
        }
      }
      emAfPageRequestTimerExpired();
      if (missedBlockModulus == ALL_BLOCKS_MISSED) {
        uint32_t offset;
        debug("  Simulating all blocks missed.\n");
        assert(EM_AF_PAGE_REQUEST_ERROR
               == emAfNextMissedBlockRequestOffset(&offset));
        assert(!emAfHandlingPageRequestClient());
        return;
      } else if (missedBlockModulus) {
        debug("  Timer expired, retrying missed packets\n");
        assert(emAfHandlingPageRequestClient());
      } else {
        // All packets received.
        assert(!emAfHandlingPageRequestClient());
      }

      // Retries
      while (EM_AF_RETRY_MISSED_PACKETS
             == emAfNextMissedBlockRequestOffset(&currentOffset)) {
        debug("    Offset 0x%4X  ", currentOffset);
        assert(EM_AF_PAGE_REQUEST_ERROR
               != emAfNoteReceivedBlockForPageRequestClient(currentOffset));
      }
      note(".");
      assert(!emAfHandlingPageRequestClient()); // should be complete
    }
  }
}

void pageRequestTest(void)
{
  uint8_t i;
  uint16_t pageSizes[] = { 1024, 2048, 4096 };
  for (i = 0; i < sizeof(pageSizes) / sizeof(uint16_t); i++) {
    PAGE_SIZE = pageSizes[i];
    MAX_BLOCKS_IN_PAGE = PAGE_SIZE / EM_AF_PAGE_REQUEST_BLOCK_SIZE;
    BITMASK_COUNT = MAX_BLOCKS_IN_PAGE / 16;

    // Simple page request with no missed blocks
    pageRequestTestWithMissedBlocks(0);

    // Every 4th block missed
    pageRequestTestWithMissedBlocks(4);

    // Every 16th block missed
    pageRequestTestWithMissedBlocks(16);

    pageRequestTestWithMissedBlocks(ALL_BLOCKS_MISSED);
  }
}
