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
 * @brief Time-related routines for the Price Common plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/time-util.h"
#include "price-common-time.h"

uint32_t emberAfPluginPriceCommonClusterGetAdjustedStartTime(uint32_t startTimeUtc,
                                                             uint8_t durationType)
{
  // According to the SE spec, for non-MINUTE times, the duration control indicates if the duration
  // shall run from the START or END of the respective day, week, or month.
  // See SE-1.2a.09 - section D.4.2.4.2 (Publish Block Period command, start time description)
  // for more details.
  //   Start time set to 00:00:00 on applicable date for START_TIME, or to 23:59:59 on applicable
  //   date for end time.
  EmberAfTimeStruct time;
  uint32_t adjustedStartTimeUtc;
  uint8_t durationTimebase = (durationType & 0x0F);
  uint8_t durationControl  = (durationType & 0xF0) >> 4;
  uint8_t weekday;

  // startTime of 0x00 means "now"
  if (startTimeUtc == 0x00) {
    return emberAfGetCurrentTime();
  }

  // Set adjusted time the same for DURATION_MINS, or if any inputs are invalid.
  adjustedStartTimeUtc = startTimeUtc;

  if ( durationTimebase != EMBER_ZCL_BLOCK_PERIOD_DURATION_TYPE_TIMEBASE_MINUTES ) {
    // Adjust start time based on Start/End of day, week, month
    emberAfFillTimeStructFromUtc(startTimeUtc, &time);
    if ( durationTimebase == EMBER_ZCL_BLOCK_PERIOD_DURATION_TYPE_TIMEBASE_DAYS ) {
      // Set adjusted start time to start or end of the current day
      time.hours = 0;
      time.minutes = 0;
      time.seconds = 0;
      adjustedStartTimeUtc = emberAfGetUtcFromTimeStruct(&time);
      if ( durationControl == END_OF_TIMEBASE ) {
        adjustedStartTimeUtc += (SECONDS_IN_DAY - 1);
      }
    } else if ( durationTimebase == EMBER_ZCL_BLOCK_PERIOD_DURATION_TYPE_TIMEBASE_WEEKS ) {
      // First adjust to start of current day.
      time.hours = 0;
      time.minutes = 0;
      time.seconds = 0;
      adjustedStartTimeUtc = emberAfGetUtcFromTimeStruct(&time);
      // now adjust that to the start of the week
      // weekday ranges from 0 to 6.
      weekday = emberAfGetWeekdayFromUtc(adjustedStartTimeUtc);
      while ( weekday > 0 ) {
        adjustedStartTimeUtc -= SECONDS_IN_DAY;
        weekday--;
      }
      if ( durationControl == END_OF_TIMEBASE ) {
        // Jump to last second of current week.
        adjustedStartTimeUtc += (SECONDS_IN_WEEK - 1);
      }
    } else if ( durationTimebase == EMBER_ZCL_BLOCK_PERIOD_DURATION_TYPE_TIMEBASE_MONTHS ) {
      // Get beginning of current month.
      time.hours = 0;
      time.minutes = 0;
      time.seconds = 0;
      time.day = 1;
      if ( durationControl == END_OF_TIMEBASE ) {
        // Calculate time at beginning of next month, then subtract 1 second
        // to get the end of the current month.
        time.month++;
        if ( time.month >= 13 ) {
          time.month = 1;
          time.year++;
        }
        adjustedStartTimeUtc = emberAfGetUtcFromTimeStruct(&time) - 1;
      } else {
        adjustedStartTimeUtc = emberAfGetUtcFromTimeStruct(&time);
      }
    } else {
      // MISRA requires ..else if.. to have terminating else.
    }
  }
  return adjustedStartTimeUtc;
}

uint32_t emberAfPluginPriceCommonClusterConvertDurationToSeconds(uint32_t startTimeUtc,
                                                                 uint32_t duration,
                                                                 uint8_t durationType)
{
  // Convert the duration from durationType units (minutes, days, weeks, etc) into seconds.
  EmberAfTimeStruct time;
  uint8_t durationTimebase = (durationType & 0x0F);
  uint8_t durationControl  = (durationType & 0xF0) >> 4;
  uint32_t endTimeUtc;

  if ( duration == DURATION_FOREVER_U32 ) {
    duration = 0xFFFFFFFFU - startTimeUtc;
  } else {
    if ( durationTimebase == EMBER_ZCL_BLOCK_PERIOD_DURATION_TYPE_TIMEBASE_MINUTES ) {
      duration *= SECONDS_IN_MINUTE;
    } else if ( durationTimebase == EMBER_ZCL_BLOCK_PERIOD_DURATION_TYPE_TIMEBASE_DAYS ) {
      duration *= SECONDS_IN_DAY;
    } else if ( durationTimebase == EMBER_ZCL_BLOCK_PERIOD_DURATION_TYPE_TIMEBASE_WEEKS ) {
      duration *= SECONDS_IN_WEEK;
    } else if ( durationTimebase == EMBER_ZCL_BLOCK_PERIOD_DURATION_TYPE_TIMEBASE_MONTHS ) {
      // Convert startTime into date stamp.
      // Add number of months, then convert back into UTC.
      // Calculate delta.
      // Be sure to start with the adjusted start time!!
      //   Recall 1 month starting at beginning of month has the # days of current month.
      //   1 month starting at end of month has the # days of the next month.
      startTimeUtc = emberAfPluginPriceCommonClusterGetAdjustedStartTime(startTimeUtc, durationType);

      emberAfFillTimeStructFromUtc(startTimeUtc, &time);
      while ( duration > 12 ) {
        time.year++;
        duration -= 12;
      }
      time.month += duration;
      if ( time.month > 12 ) {
        time.month -= 12;
        time.year++;
      }
      if ( durationControl == END_OF_TIMEBASE ) {
        // Update time day to the last day of the month
        uint8_t day = emberAfGetNumberDaysInMonth(&time);
        time.day = day;
      }

      endTimeUtc = emberAfGetUtcFromTimeStruct(&time);
      duration = endTimeUtc - startTimeUtc;
    } else {
      // MISRA requires ..else if.. to have terminating else.
    }
  }
  return duration;
}
