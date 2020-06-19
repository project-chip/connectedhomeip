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
 * @brief APIs and defines for the Price Common plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_PRICE_COMMON_H
#define SILABS_PRICE_COMMON_H

#include "price-common-time.h"

#ifndef EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT
  #define EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT (1)
#endif

#define EVENT_ID_UNSPECIFIED                          (0xFFFFFFFFUL)
#define TARIFF_TYPE_UNSPECIFIED                       (0xFF)
#define ZCL_PRICE_CLUSTER_DURATION16_UNTIL_CHANGED    (0xFFFF)
#define ZCL_PRICE_CLUSTER_DURATION_SEC_UNTIL_CHANGED  (0xFFFFFFFFUL)
#define ZCL_PRICE_CLUSTER_END_TIME_NEVER              (0xFFFFFFFFUL)
#define ZCL_PRICE_CLUSTER_NUMBER_OF_EVENTS_ALL        (0x00)
#define ZCL_PRICE_CLUSTER_START_TIME_NOW              (0x00000000UL)
#define ZCL_PRICE_CLUSTER_WILDCARD_ISSUER_ID          (0xFFFFFFFFUL)
#define ZCL_PRICE_INVALID_ENDPOINT_INDEX              (0xFF)
#define ZCL_PRICE_INVALID_INDEX                       (0xFF)

typedef struct {
  uint32_t startTime;

  /* Using uint32_t since int24u doesn't exist.  MEASURED IN SECONDS.
   * FOREVER or UNTIL_CHANGED should set this to ZCL_PRICE_CLUSTER_DURATION_SEC_UNTIL_CHANGED.
   * Some commands might not use this field.
   */
  uint32_t durationSec;

  uint32_t issuerEventId;
  bool valid;

  /* Flag showing if actions are required by the current entry, such
   * as updating attributes and etc, are still pending. Usage of this flag is
   * optional.
   */
  bool actionsPending;
} EmberAfPriceCommonInfo;

/**
 * @brief Returns the best matching or other index to use for inserting new data
 *
 * @param commonInfos An array of EmberAfPriceCommonInfo structures whose data is used to find
 * the best available index.
 * @param numberOfEntries The number of entries in the EmberAfPriceCommonInfo array.
 * @param newIssuerEventId The issuerEventId of the new data. This is used to see if a match is present.
 * @param newStartTime The startTime of the new data.
 * @param expireTimedOut Treats any timed-out entries as invalid if set to true.
 * @return The best index - either a matching index, if found, or an invalid or timed out index.
 *
 **/
uint8_t emberAfPluginPriceCommonGetCommonMatchingOrUnusedIndex(EmberAfPriceCommonInfo *commonInfos,
                                                               uint8_t  numberOfEntries,
                                                               uint32_t newIssuerEventId,
                                                               uint32_t newStartTime,
                                                               bool expireTimedOut);

/**
 * @brief Sorts Price related data structures.
 *
 * This semi-generic sorting function can be used to sort all structures that
 * utilizes the EmberAfPriceCommonInfo data type.
 *
 * @param commonInfos The destination address to which the command should be sent.
 * @param dataArray The source endpoint used in the transmission.
 * @param dataArrayBlockSizeInByte The source endpoint used in the transmission.
 * @param dataArraySize The source endpoint used in the transmission.
 *
 **/
void emberAfPluginPriceCommonSort(EmberAfPriceCommonInfo * commonInfos,
                                  uint8_t * dataArray,
                                  uint16_t dataArrayBlockSizeInByte,
                                  uint16_t dataArraySize);

/**
 * @brief Updates durations to avoid overlapping the next event.
 *
 * @param commonInfos An array of EmberAfPriceCommonInfo structures that will be evaluated.
 * @param numberOfEntries The number of entries in the EmberAfPriceCommonInfo array.
 *
 **/
void emberAfPluginPriceCommonUpdateDurationForOverlappingEvents(EmberAfPriceCommonInfo *commonInfos,
                                                                uint8_t numberOfEntries);

/**
 * @brief Determines the time until the next index becomes active.
 *
 * This function assumes the commonInfos[] array is already sorted by startTime from earliest to latest.
 *
 * @param commonInfos An array of EmberAfPriceCommonInfo structures that will be evaluated.
 * @param numberOfEntries The number of entries in the EmberAfPriceCommonInfo array.
 *
 **/
uint32_t emberAfPluginPriceCommonSecondsUntilSecondIndexActive(EmberAfPriceCommonInfo *commonInfos,
                                                               uint8_t numberOfEntries);

/**
 * @brief Find valid entries in the EmberAfPriceCommonInfo structure array.
 *
 * @param validEntries An array of the same size as the EmberAfPriceCommonInfo
 * array that will store the valid flag for each entry (true or false).
 * @param numberOfEntries The number of entries in the validEntries array and the commonInfos array.
 * @param commonInfos The EmberAfPriceCommonInfo array that will be searched for valid entries.
 * @param earliestStartTime A minimum start time such that all valid entries have a start
 * time greater than or equal to this value.
 * @minIssuerEventId A minimum event ID such that all valid entries have an issuerEventId
 * greater than or equal to this.
 * @numberOfRequestedCommands The maximum number of valid entries to be returned.
 * @return The number of valid commands found in the commonInfos array.
 *
 **/
uint8_t emberAfPluginPriceCommonFindValidEntries(uint8_t* validEntries,
                                                 uint8_t numberOfEntries,
                                                 EmberAfPriceCommonInfo* commonInfos,
                                                 uint32_t earliestStartTime,
                                                 uint32_t minIssuerEventId,
                                                 uint8_t numberOfCommands);

/**
 * @brief Returns the index of the active entry in the EmberAfPriceCommonInfo array.
 *
 * Search through array for the most recent active entry. "Issuer Event Id" has higher
 * priority than "start time".
 *
 * @param commonInfos The EmberAfPriceCommonInfo array that will be searched for an active entry.
 * @param numberOfEntries The number of entries in the commonInfo array.
 * @return The index of the active entry, or 0xFF if an active entry is not found.
 *
 **/
uint8_t emberAfPluginPriceCommonServerGetActiveIndex(EmberAfPriceCommonInfo *commonInfos,
                                                     uint8_t numberOfEntries);

/**
 * @brief Returns the index to the most recent entry that will become active in the future.
 *
 * @param commonInfos The EmberAfPriceCommonInfo array that will be searched for the entry.
 * @numberOfEntries The number of entries in the commonInfo array.
 * @secUntilFutureEvent The output pointer to the number of seconds until the next
 *                      active entry.
 *
 * @return The index of the next-active entry, or 0xFF if an active entry is not found.
 *
 **/
uint8_t emberAfPluginPriceCommonServerGetFutureIndex(EmberAfPriceCommonInfo *commonInfos,
                                                     uint8_t numberOfEntries,
                                                     uint32_t * secUntilFutureEvent);
#endif  // #ifndef _PRICE_COMMON_H_
