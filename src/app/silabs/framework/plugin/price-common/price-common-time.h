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
 * @brief Time-related APIs and defines for the Price Common plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_PRICE_COMMON_TIME_H
#define SILABS_PRICE_COMMON_TIME_H

enum {
  START_OF_TIMEBASE = 0x00,
  END_OF_TIMEBASE = 0x01
};

enum {
  DURATION_TYPE_MINS = 0x00,
  DURATION_TYPE_DAYS_START   = 0x01,
  DURATION_TYPE_DAYS_END     = 0x11,
  DURATION_TYPE_WEEKS_START  = 0x02,
  DURATION_TYPE_WEEKS_END    = 0x12,
  DURATION_TYPE_MONTHS_START = 0x03,
  DURATION_TYPE_MONTHS_END   = 0x13,
};

/**
 * @brief Converts the duration to a number of seconds based on the duration type parameter.
 *
 */
uint32_t emberAfPluginPriceCommonClusterConvertDurationToSeconds(uint32_t startTimeUtc, uint32_t duration, uint8_t durationType);

/**
 * @brief Calculates a new UTC start time value based on the duration type parameter.
 *
 */
uint32_t emberAfPluginPriceCommonClusterGetAdjustedStartTime(uint32_t startTimeUtc, uint8_t durationType);

#endif // #ifndef _PRICE_COMMON_TIME_H_
