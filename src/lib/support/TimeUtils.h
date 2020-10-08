/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *    @file
 *      Various utility functions for dealing with time and dates.
 *
 */

#pragma once

#include <stdint.h>

namespace chip {

enum
{
    kYearsPerCentury  = 100,
    kLeapYearInterval = 4,

    kMonthsPerYear = 12,

    kMaxDaysPerMonth = 31,

    kDaysPerWeek         = 7,
    kDaysPerStandardYear = 365,
    kDaysPerLeapYear     = kDaysPerStandardYear + 1,

    kHoursPerDay  = 24,
    kHoursPerWeek = kDaysPerWeek * kHoursPerDay,

    kMinutesPerHour = 60,
    kMinutesPerDay  = kHoursPerDay * kMinutesPerHour,

    kSecondsPerMinute       = 60,
    kSecondsPerHour         = kSecondsPerMinute * kMinutesPerHour,
    kSecondsPerDay          = kSecondsPerHour * kHoursPerDay,
    kSecondsPerWeek         = kSecondsPerDay * kDaysPerWeek,
    kSecondsPerStandardYear = kSecondsPerDay * kDaysPerStandardYear,

    kMillisecondPerSecond = 1000,

    kMicrosecondsPerSecond = 1000000
};

enum
{
    kJanuary   = 1,
    kFebruary  = 2,
    kMarch     = 3,
    kApril     = 4,
    kMay       = 5,
    kJune      = 6,
    kJuly      = 7,
    kAugust    = 8,
    kSeptember = 9,
    kOctober   = 10,
    kNovember  = 11,
    kDecember  = 12
};

enum
{
    // First year of the standard unix epoch.
    kEpochYear = 1970,

    // Last fully-representable year that can be stored in an unsigned 32-bit seconds-since-epoch value.
    kMaxYearInSecondsSinceEpoch32 = 2105,

    // Last fully-representable year that can be stored in an unsigned 32-bit days-since-epoch value.
    kMaxYearInDaysSinceEpoch32 = 28276
};

extern bool IsLeapYear(uint16_t year);
extern uint8_t DaysInMonth(uint16_t year, uint8_t month);
extern uint8_t FirstWeekdayOfYear(uint16_t year);
extern void OrdinalDateToCalendarDate(uint16_t year, uint16_t dayOfYear, uint8_t & month, uint8_t & dayOfMonth);
extern void CalendarDateToOrdinalDate(uint16_t year, uint8_t month, uint8_t dayOfMonth, uint16_t & dayOfYear);
extern bool CalendarDateToDaysSinceEpoch(uint16_t year, uint8_t month, uint8_t dayOfMonth, uint32_t & daysSinceEpoch);
extern void DaysSinceEpochToCalendarDate(uint32_t daysSinceEpoch, uint16_t & year, uint8_t & month, uint8_t & dayOfMonth);
extern void AdjustCalendarDate(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth, int32_t relativeDays);
extern bool CalendarTimeToSecondsSinceEpoch(uint16_t year, uint8_t month, uint8_t dayOfMonth, uint8_t hour, uint8_t minute,
                                            uint8_t second, uint32_t & secondsSinceEpoch);
extern void SecondsSinceEpochToCalendarTime(uint32_t secondsSinceEpoch, uint16_t & year, uint8_t & month, uint8_t & dayOfMonth,
                                            uint8_t & hour, uint8_t & minute, uint8_t & second);

/**
 *  @def secondsToMilliseconds
 *
 *  @brief
 *    Convert integer seconds to milliseconds.
 *
 */
inline uint32_t secondsToMilliseconds(uint32_t seconds)
{
    return (seconds * kMillisecondPerSecond);
}

} // namespace chip
