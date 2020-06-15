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
 * @brief
 *******************************************************************************
   ******************************************************************************/

#include "../include/af.h"
#include "af-main.h"
#include "common.h"
#include "time-util.h"

#ifdef EMBER_AF_PLUGIN_TIME_SERVER
#include "app/framework/plugin/time-server/time-server.h"
#endif

#define mYEAR_IS_LEAP_YEAR(year)  ( ((year) % 4 == 0) && (((year) % 100 != 0) || ((year) % 400 == 0)) )

const uint8_t emberAfDaysInMonth[] =
{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

uint8_t emberAfGetNumberDaysInMonth(EmberAfTimeStruct *time)
{
  uint8_t daysInMonth = 0;
  if ( (time->month > 0) && (time->month < 13) ) {
    daysInMonth = emberAfDaysInMonth[time->month - 1];
    if ( mYEAR_IS_LEAP_YEAR(time->year) && (time->month == 2) ) {
      daysInMonth++;
    }
  }
  return daysInMonth;
}

void emberAfCopyDate(uint8_t *data, uint16_t index, EmberAfDate *src)
{
  data[index]   = src->year;
  data[index + 1] = src->month;
  data[index + 2] = src->dayOfMonth;
  data[index + 3] = src->dayOfWeek;
}

int8_t emberAfCompareDates(EmberAfDate* date1, EmberAfDate* date2)
{
  uint32_t val1 = emberAfEncodeDate(date1);
  uint32_t val2 = emberAfEncodeDate(date2);
  return (val1 == val2) ? 0 : ((val1 < val2) ? -1 : 1);
}

void emberAfFillTimeStructFromUtc(uint32_t utcTime,
                                  EmberAfTimeStruct* returnTime)
{
  bool isLeapYear = true; // 2000 was a leap year
  uint32_t i;
  uint32_t daysSince2000 = utcTime / SECONDS_IN_DAY;
  uint32_t secToday = utcTime - (daysSince2000 * SECONDS_IN_DAY);
  returnTime->hours = (uint8_t)(secToday / SECONDS_IN_HOUR);
  returnTime->minutes = (uint8_t)((secToday
                                   - (returnTime->hours * SECONDS_IN_HOUR)) / 60);
  returnTime->seconds = (uint8_t)(secToday
                                  - ((returnTime->hours * SECONDS_IN_HOUR)
                                     + (returnTime->minutes * 60)));
  returnTime->year = 2000;
  returnTime->month = 1;
  returnTime->day = 1;

  // march through the calendar, counting months, days and years
  // being careful to account for leapyears.
  for (i = 0; i < daysSince2000; i++) {
    uint8_t daysInMonth;
    if (isLeapYear && (returnTime->month == 2)) {
      daysInMonth = 29;
    } else {
      daysInMonth = emberAfDaysInMonth[returnTime->month - 1];
    }
    if (daysInMonth == returnTime->day) {
      returnTime->month++;
      returnTime->day = 1;
    } else {
      returnTime->day++;
    }
    if (returnTime->month == 13) {
      returnTime->year++;
      returnTime->month = 1;
      if (returnTime->year % 4 == 0
          && (returnTime->year % 100 != 0
              || (returnTime->year % 400 == 0))) {
        isLeapYear = true;
      } else {
        isLeapYear = false;
      }
    }
  }
}

uint32_t emberAfGetUtcFromTimeStruct(EmberAfTimeStruct *time)
{
  // Construct a UTC timestamp given an EmberAfTimeStruct structure.
  uint32_t utcTime = 0;
  uint16_t daysThisYear;
  uint32_t i;

  if ( (time == NULL) || (time->year < 2000) || (time->month == 0)
       || (time->month > 12) || (time->day == 0) || (time->day > 31) ) {
    return 0xFFFFFFFFU;    // Invalid parameters
  }

  for ( i = 2000; i < time->year; i++ ) {
    utcTime += (365 * SECONDS_IN_DAY);
    if ( mYEAR_IS_LEAP_YEAR(i) ) {
      utcTime += SECONDS_IN_DAY;
    }
  }
  emberAfAppPrintln("Seconds in %d years=%d", i, utcTime);
  // Utc Time now reflects seconds up to Jan 1 00:00:00 of current year.
  daysThisYear = 0;
  for ( i = 0; i < time->month - 1; i++ ) {
    daysThisYear += emberAfDaysInMonth[i];
    if ( (i == 1) && (mYEAR_IS_LEAP_YEAR(time->year)) ) {
      daysThisYear++;
    }
  }
  daysThisYear += time->day - 1;
  utcTime += SECONDS_IN_DAY * daysThisYear;
  emberAfAppPrintln("daysThisYear=%d, total Sec=%d (0x%4x)", daysThisYear, utcTime, utcTime);

  // Utc Time now reflects seconds up to last completed day of current year.
  for ( i = 0; i < time->hours; i++ ) {
    utcTime += SECONDS_IN_HOUR;
  }
  //for( i=0; i<time->minutes; i++ ){
  //iutcTime += 60;
  //}
  utcTime += (60 * time->minutes);
  utcTime += time->seconds;
  return utcTime;
}

#define SECONDS_IN_WEEK  (SECONDS_IN_DAY * 7)
// Determine which day of the week it is, from a given utc timestamp.
// Return 0=MON, 1=TUES, etc.
uint8_t emberAfGetWeekdayFromUtc(uint32_t utcTime)
{
  uint8_t dayIndex;
  utcTime %= SECONDS_IN_WEEK;

  for ( dayIndex = 0; dayIndex < 7; dayIndex++ ) {
    if ( utcTime < SECONDS_IN_DAY ) {
      break;
    }
    utcTime -= SECONDS_IN_DAY;
  }
  // Note:  Jan 1, 2000 is a SATURDAY.
  // Do some translation work so 0=MONDAY, 5=SATURDAY, 6=SUNDAY
  if ( dayIndex < 2 ) {
    dayIndex += 5;
  } else {
    dayIndex -= 2;
  }
  return dayIndex;
}

void emberAfDecodeDate(uint32_t src, EmberAfDate* dest)
{
  dest->year       = (uint8_t)((src & 0xFF000000U) >> 24);
  dest->month      = (uint8_t)((src & 0x00FF0000U) >> 16);
  dest->dayOfMonth = (uint8_t)((src & 0x0000FF00U) >>  8);
  dest->dayOfWeek  = (uint8_t) (src & 0x000000FFU);
}

uint32_t emberAfEncodeDate(EmberAfDate* date)
{
  uint32_t result = ((((uint32_t)date->year) << 24)
                     + (((uint32_t)date->month) << 16)
                     + (((uint32_t)date->dayOfMonth) << 8)
                     + (((uint32_t)date->dayOfWeek)));
  return result;
}

// emberAfPrintTime expects to be passed a ZigBee time which is the number
// of seconds since the year 2000, it prints out a human readable time
// from that value.
void emberAfPrintTime(uint32_t utcTime)
{
#ifdef EMBER_AF_PRINT_ENABLE
  EmberAfTimeStruct time;
  emberAfFillTimeStructFromUtc(utcTime, &time);
  emberAfPrintln(emberAfPrintActiveArea,
                 "UTC time: %d/%d/%d %d:%d:%d (%4x)",
                 time.month,
                 time.day,
                 time.year,
                 time.hours,
                 time.minutes,
                 time.seconds,
                 utcTime);
#endif //EMBER_AF_PRINT_ENABLE
}

void emberAfPrintTimeIsoFormat(uint32_t utcTime)
{
#ifdef EMBER_AF_PRINT_ENABLE
  EmberAfTimeStruct time;
  emberAfFillTimeStructFromUtc(utcTime, &time);
  emberAfPrint(emberAfPrintActiveArea,
               "%d-%d-%d %d:%d:%d",
               time.year,
               time.month,
               time.day,
               time.hours,
               time.minutes,
               time.seconds);
#endif //EMBER_AF_PRINT_ENABLE
}

void emberAfPrintDate(const EmberAfDate * date)
{
#ifdef EMBER_AF_PRINT_ENABLE
  uint32_t zigbeeDate = ((((uint32_t)date->year) << 24)
                         + (((uint32_t)date->month) << 16)
                         + (((uint32_t)date->dayOfMonth) << 8)
                         + (((uint32_t)date->dayOfWeek)));

  emberAfPrint(emberAfPrintActiveArea,
               "0x%4X (%d/%p%d/%p%d)",
               zigbeeDate,
               date->year + 1900,
               (date->month < 10 ? "0" : ""),
               date->month,
               (date->dayOfMonth < 10 ? "0" : ""),
               date->dayOfMonth);
#endif //EMBER_AF_PRINT_ENABLE
}

void emberAfPrintDateln(const EmberAfDate * date)
{
  emberAfPrintDate(date);
  emberAfPrintln(emberAfPrintActiveArea, "");
}

// *******************************************************
// emberAfPrintTime and emberAfSetTime are convienience methods for setting
// and displaying human readable times.

// Expects to be passed a ZigBee time which is the number of seconds
// since the year 2000
void emberAfSetTime(uint32_t utcTime)
{
#ifdef EMBER_AF_PLUGIN_TIME_SERVER
  emAfTimeClusterServerSetCurrentTime(utcTime);
#endif //EMBER_AF_PLUGIN_TIME_SERVER
  emberAfSetTimeCallback(utcTime);
}

uint32_t emberAfGetCurrentTime(void)
{
#ifdef EMBER_AF_PLUGIN_TIME_SERVER
  return emAfTimeClusterServerGetCurrentTime();
#else
  return emberAfGetCurrentTimeCallback();
#endif
}
