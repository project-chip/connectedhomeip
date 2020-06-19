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
 * @brief Routines for the Price Common plugin, which implement the common
 *        features of the Price cluster.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/util/util.h"
#include "price-common.h"
#include "enums.h"
#include "app/framework/plugin/price-common/price-common-time.h"

// Forward declaration
static void swap(uint8_t * a, uint8_t * b, uint16_t size);

void emberAfPluginPriceCommonSort(EmberAfPriceCommonInfo * commonInfos,
                                  uint8_t * dataArray,
                                  uint16_t dataArrayBlockSizeInByte,
                                  uint16_t dataArraySize)
{
  /* a[0] to a[dataArraySize-1] is the array to sort */
  uint16_t i, j;
  uint16_t iMin;

  /* advance the position through the entire array */
  for (j = 0; j < dataArraySize - 1; j++) {
    /* find the min element in the unsorted a[j .. dataArraySize-1] */

    /* assume the min is the first element */
    iMin = j;
    /* test against elements after j to find the smallest */
    for ( i = j + 1; i < dataArraySize; i++) {
      /* if this element is less, then it is the new minimum */
      if (commonInfos[i].valid && !commonInfos[iMin].valid) {
        iMin = i;
      } else if ( commonInfos[i].valid
                  && commonInfos[iMin].valid
                  && (commonInfos[i].startTime < commonInfos[iMin].startTime) ) {
        /* found new minimum; remember its index */
        iMin = i;
      } else {
        // MISRA requires ..else if.. to have terminating else.
      }
    }

    if (iMin != j) {
      // swap common infos
      swap((uint8_t *)&commonInfos[j], (uint8_t *)&commonInfos[iMin], sizeof(EmberAfPriceCommonInfo));
      // swap block size
      swap(dataArray + j * dataArrayBlockSizeInByte,
           dataArray + iMin * dataArrayBlockSizeInByte,
           dataArrayBlockSizeInByte);
    }
  }
}

void emberAfPluginPriceCommonUpdateDurationForOverlappingEvents(EmberAfPriceCommonInfo *commonInfos,
                                                                uint8_t numberOfEntries)
{
  uint8_t i;
  uint32_t endTime;
  if ( numberOfEntries > 1 ) {
    for ( i = 0; i < numberOfEntries - 1; i++ ) {
      if ( commonInfos[i].valid && commonInfos[i + 1].valid ) {
        endTime = commonInfos[i].startTime + commonInfos[i].durationSec;
        if ( endTime > commonInfos[i + 1].startTime ) {
          commonInfos[i].durationSec = commonInfos[i + 1].startTime - commonInfos[i].startTime;
        }
      }
    }
  }
}

uint32_t emberAfPluginPriceCommonSecondsUntilSecondIndexActive(EmberAfPriceCommonInfo *commonInfos,
                                                               uint8_t numberOfEntries)
{
  // This function assumes the commonInfos[] array is already sorted by start time.
  // It assumes index[0] is currently active, and checks index[1] to determine when
  // it will be active.  It returns how many seconds must elapse until index[1]
  // becomes active.
  uint32_t timeNow;
  uint32_t secondsUntilActive = 0xFFFFFFFFU;

  timeNow = emberAfGetCurrentTime();
  if ( (numberOfEntries > 1) && commonInfos[1].valid ) {
    if ( commonInfos[1].startTime >= timeNow ) {
      secondsUntilActive = commonInfos[1].startTime - timeNow;
    } else {
      secondsUntilActive = 0;
    }
  }
  return secondsUntilActive;
}

uint8_t emberAfPluginPriceCommonGetCommonMatchingOrUnusedIndex(EmberAfPriceCommonInfo *commonInfos,
                                                               uint8_t  numberOfEntries,
                                                               uint32_t newIssuerEventId,
                                                               uint32_t newStartTime,
                                                               bool expireTimedOut)
{
  uint32_t timeNow;
  uint32_t endTime;
  uint32_t smallestEventId = 0xFFFFFFFFU;
  uint8_t  smallestEventIdIndex = 0xFF;
  uint8_t i;

  // Search the commonInfos[] table for an entry that is:
  //  - Matching (same issuerEventId & providerId)
  //  - Invalid or expired
  //  - smallest eventId
  timeNow = emberAfGetCurrentTime();

  for ( i = 0; i < numberOfEntries; i++ ) {
    if ( commonInfos[i].durationSec != ZCL_PRICE_CLUSTER_DURATION_SEC_UNTIL_CHANGED ) {
      endTime = commonInfos[i].startTime + commonInfos[i].durationSec;
    } else {
      endTime = ZCL_PRICE_CLUSTER_END_TIME_NEVER;
    }

    if ( (!commonInfos[i].valid)
         || (expireTimedOut && (endTime <= timeNow)) ) {
      if ( smallestEventId > 0 ) {
        // Found index that is not valid.  Use the first invalid entry unless matching entry found.
        smallestEventId = 0;
        smallestEventIdIndex = i;
        commonInfos[i].valid = false;
      }
    } else if ( (commonInfos[i].startTime == newStartTime)
                && (commonInfos[i].issuerEventId == newIssuerEventId) ) {
      // Match found
      smallestEventIdIndex = i;
      break;
    } else if ( (commonInfos[i].issuerEventId < smallestEventId)
                && (commonInfos[i].issuerEventId < newIssuerEventId) ) {
      smallestEventId = commonInfos[i].issuerEventId;
      smallestEventIdIndex = i;
    } else {
      // MISRA requires ..else if.. to have terminating else.
    }
  }
  return smallestEventIdIndex;
}

uint8_t emberAfPluginPriceCommonFindValidEntries(uint8_t* validEntries,
                                                 uint8_t numberOfEntries,
                                                 EmberAfPriceCommonInfo* commonInfos,
                                                 uint32_t earliestStartTime,
                                                 uint32_t minIssuerEventId,
                                                 uint8_t numberOfCommands)
{
  uint8_t numberOfValidCommands = 0;
  uint32_t startTrackingLastStartTime = -1;
  uint8_t i = 0;
  uint8_t validEntryCount = 0;

  for (i = 0; i < numberOfEntries; i++) {
    if (commonInfos[i].valid) {
      validEntryCount++;
    }
  }
  emberAfPriceClusterPrintln("Total number of active entries in table: %d", validEntryCount);

  for (i = 0; i < numberOfEntries; i++) {
    if (commonInfos[i].valid == 0) {
      continue;
    }

    if ((minIssuerEventId != 0xFFFFFFFFU) && (minIssuerEventId > commonInfos[i].issuerEventId)) {
      continue;
    }

    if (earliestStartTime <= commonInfos[i].startTime) {
      validEntries[numberOfValidCommands] = i;
      numberOfValidCommands++;
    } else { // keep track of the startTime before currentTime.
             // in case there's no startTime after currentTime,
             // the last startTime will be valid.
      if (startTrackingLastStartTime == -1) {
        startTrackingLastStartTime = 1;
        validEntries[numberOfValidCommands] = i;
        numberOfValidCommands++;
      } else if (startTrackingLastStartTime == 1) { // update lastStartTime
        validEntries[numberOfValidCommands - 1] = i;
      } else {
        // MISRA requires ..else if.. to have terminating else.
      }
    }
  }

  if ( (numberOfCommands != 0x0)
       && (numberOfCommands < numberOfValidCommands) ) {
    numberOfValidCommands = numberOfCommands;
  }

  emberAfPriceClusterPrint("Valid entries index: ");
  for (i = 0; i < numberOfValidCommands; i++) {
    emberAfPriceClusterPrint("%d, ", validEntries[i]);
  }
  emberAfPriceClusterPrintln("");
  return numberOfValidCommands;
}

uint8_t emberAfPluginPriceCommonServerGetActiveIndex(EmberAfPriceCommonInfo *commonInfos,
                                                     uint8_t numberOfEntries)
{
  uint32_t timeNow;
  uint32_t endTime;
  uint32_t maxCurrentEventId = 0;
  uint8_t  maxCurrentEventIdIndex = 0xFF;
  uint8_t  i;

  // TODO:  This function doesn't work if the index+1 item has started up.
  // This function is fooled by index 0 and returns that as the active index.
  // Need to build in a means into all price commands to invalidate their entries
  // at the appropriate time (when index+1 starts up).
  // End TODO.

  timeNow = emberAfGetCurrentTime();
  for ( i = 0; i < numberOfEntries; i++ ) {
    if ( commonInfos[i].valid
         && (commonInfos[i].startTime <= timeNow) ) {
      if ( commonInfos[i].durationSec == ZCL_PRICE_CLUSTER_DURATION_SEC_UNTIL_CHANGED ) {
        endTime = ZCL_PRICE_CLUSTER_END_TIME_NEVER;
      } else {
        endTime = commonInfos[i].startTime + commonInfos[i].durationSec;
      }

      if ( (endTime >= timeNow)
           && (commonInfos[i].issuerEventId > maxCurrentEventId) ) {
        maxCurrentEventId = commonInfos[i].issuerEventId;
        maxCurrentEventIdIndex = i;
      }
    }
  }
  return maxCurrentEventIdIndex;
}

uint8_t emberAfPluginPriceCommonServerGetFutureIndex(EmberAfPriceCommonInfo *commonInfos,
                                                     uint8_t numberOfEntries,
                                                     uint32_t * secUntilFutureEvent)
{
  uint32_t minDeltaSec = 0xFFFFFFFFU;
  uint32_t minDeltaIndex = ZCL_PRICE_INVALID_INDEX;
  uint32_t deltaSec;
  uint32_t timeNow;
  uint8_t i;

  timeNow = emberAfGetCurrentTime();

  // Look for the nearest time of the next change starting either NOW, or in the future.
  for ( i = 0; i < numberOfEntries; i++ ) {
    if ( commonInfos[i].valid
         && (commonInfos[i].startTime >= timeNow) ) {
      deltaSec = commonInfos[i].startTime - timeNow;
      if ( deltaSec < minDeltaSec ) {
        minDeltaSec = deltaSec;
        minDeltaIndex = i;
      }
    }
  }

  if (secUntilFutureEvent != NULL) {
    *secUntilFutureEvent = minDeltaSec;
  }
  return minDeltaIndex;
}

static void swap(uint8_t * a, uint8_t * b, uint16_t size)
{
  uint8_t tmp;
  uint16_t i;
  for (i = 0; i < size; i++) {
    if (a[i] != b[i]) {
      tmp = a[i];
      a[i] = b[i];
      b[i] = tmp;
    }
  }
}
