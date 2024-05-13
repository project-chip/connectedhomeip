/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

    kMillisecondsPerSecond = 1000,

    kMicrosecondsPerSecond      = 1000000,
    kMicrosecondsPerMillisecond = 1000,

    kNanosecondsPerSecond      = 1000000000,
    kNanosecondsPerMillisecond = 1000000,
    kNanosecondsPerMicrosecond = 1000,
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

/* Unix epoch time.
 */
enum
{
    // First year of the standard unix epoch.
    kUnixEpochYear = 1970,

    // Last fully-representable year that can be stored in an unsigned 32-bit seconds-since-epoch value.
    kMaxYearInSecondsSinceUnixEpoch32 = 2105,

    // Last fully-representable year that can be stored in an unsigned 32-bit days-since-epoch value.
    kMaxYearInDaysSinceUnixEpoch32 = 28276
};

/* CHIP Epoch time.
 */
enum
{
    // Base year of the CHIP epoch time.
    kChipEpochBaseYear = 2000,

    // Last fully-representable year that can be stored in an unsigned 32-bit CHIP Epoch seconds value.
    kChipEpochMaxYear = 2135,

    // Offset, in days, from the Unix Epoch to the CHIP Epoch.
    kChipEpochDaysSinceUnixEpoch = 10957,

    // Offset, in non-leap seconds, from the Unix Epoch to the CHIP Epoch.
    kChipEpochSecondsSinceUnixEpoch = kChipEpochDaysSinceUnixEpoch * kSecondsPerDay,
};

// Difference in microseconds between Unix epoch (Jan 1 1970 00:00:00) and CHIP Epoch (Jan 1 2000 00:00:00).
constexpr uint64_t kChipEpochUsSinceUnixEpoch =
    static_cast<uint64_t>(kChipEpochSecondsSinceUnixEpoch) * chip::kMicrosecondsPerSecond;

bool IsLeapYear(uint16_t year);
uint8_t DaysInMonth(uint16_t year, uint8_t month);
uint8_t FirstWeekdayOfYear(uint16_t year);
void OrdinalDateToCalendarDate(uint16_t year, uint16_t dayOfYear, uint8_t & month, uint8_t & dayOfMonth);
void CalendarDateToOrdinalDate(uint16_t year, uint8_t month, uint8_t dayOfMonth, uint16_t & dayOfYear);
bool CalendarDateToDaysSinceUnixEpoch(uint16_t year, uint8_t month, uint8_t dayOfMonth, uint32_t & daysSinceEpoch);
bool DaysSinceUnixEpochToCalendarDate(uint32_t daysSinceEpoch, uint16_t & year, uint8_t & month, uint8_t & dayOfMonth);
bool AdjustCalendarDate(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth, int32_t relativeDays);
bool CalendarTimeToSecondsSinceUnixEpoch(uint16_t year, uint8_t month, uint8_t dayOfMonth, uint8_t hour, uint8_t minute,
                                         uint8_t second, uint32_t & secondsSinceEpoch);
void SecondsSinceUnixEpochToCalendarTime(uint32_t secondsSinceEpoch, uint16_t & year, uint8_t & month, uint8_t & dayOfMonth,
                                         uint8_t & hour, uint8_t & minute, uint8_t & second);

/**
 *  @brief Convert a calendar date and time to the number of seconds since CHIP Epoch (2000-01-01 00:00:00 UTC).
 *
 *  @note  This function makes no attempt to verify the correct range of any arguments other than year.
 *         Therefore callers must make sure the supplied values are valid prior to invocation.
 *
 *  @param year           Gregorian calendar year in the range 2000 to 2135.
 *  @param month          Month in standard form (1=January ... 12=December).
 *  @param dayOfMonth     Day-of-month in standard form (1=1st, 2=2nd, etc.).
 *  @param hour           Hour (0-23).
 *  @param minute         Minute (0-59).
 *  @param second         Second (0-59).
 *  @param chipEpochTime  Number of seconds since 2000-01-01 00:00:00 UTC.
 *
 *  @return   True if the date/time was converted successfully. False if the given year falls outside the
 *            representable range.
 */
bool CalendarToChipEpochTime(uint16_t year, uint8_t month, uint8_t dayOfMonth, uint8_t hour, uint8_t minute, uint8_t second,
                             uint32_t & chipEpochTime);

/**
 *  @brief Convert the number of seconds since CHIP Epoch (2000-01-01 00:00:00 UTC) to a calendar date and time.
 *
 *  @details  Input time values are limited to positive values up to 2^32-1. This limits the
 *            representable date range to the year 2135.
 *
 *  @param chipEpochTime  Number of seconds since 2000-01-01 00:00:00 UTC.
 *  @param year           Gregorian calendar year.
 *  @param month          Month in standard form (1=January ... 12=December).
 *  @param dayOfMonth     Day-of-month in standard form (1=1st, 2=2nd, etc.).
 *  @param hour           Hour (0-23).
 *  @param minute         Minute (0-59).
 *  @param second         Second (0-59).
 */
void ChipEpochToCalendarTime(uint32_t chipEpochTime, uint16_t & year, uint8_t & month, uint8_t & dayOfMonth, uint8_t & hour,
                             uint8_t & minute, uint8_t & second);

/**
 *  @brief Convert the number of seconds since Unix Epoch (1970-01-01 00:00:00 GMT TAI) to
 *         CHIP Epoch (2000-01-01 00:00:00 UTC).
 *
 *  @details  Input time values are limited to positive values up to 2^32-1. This limits the
 *            representable date range to the year 2135.
 *
 *  @param[in] unixEpochTimeSeconds  Number of seconds since 1970-01-01 00:00:00 GMT TAI.
 *  @param[out] outChipEpochTimeSeconds  Number of seconds since 2000-01-01 00:00:00 UTC.
 *
 *  @return   True if the time was converted successfully. False if the given Unix epoch time
 *            falls outside the representable range.
 */
bool UnixEpochToChipEpochTime(uint32_t unixEpochTimeSeconds, uint32_t & outChipEpochTimeSeconds);

/**
 *  @brief Convert the number of microseconds since CHIP Epoch (2000-01-01 00:00:00 UTC) to
 *         Unix Epoch (1970-01-01 00:00:00 GMT TAI).
 *
 *  @param[in] chipEpochTimeMicros  Number of microseconds since 2000-01-01 00:00:00 UTC.
 *  @param[out] outUnixEpochTimeMicros  Number of microseconds since 1970-01-01 00:00:00 GMT TAI.
 *
 *  @return   True if the time was converted successfully. False if the given CHIP epoch time
 *            falls outside the representable range.
 */
bool ChipEpochToUnixEpochMicros(uint64_t chipEpochTimeMicros, uint64_t & outUnixEpochTimeMicros);

/**
 *  @brief Convert the number of microseconds since Unix Epoch (1970-01-01 00:00:00 GMT TAI) to
 *         CHIP Epoch (2000-01-01 00:00:00 UTC).
 *
 *  @param[in] unixEpochTimeMicros  Number of microseconds since 1970-01-01 00:00:00 GMT TAI.
 *  @param[out] outChipEpochTimeMicros  Number of microseconds since 2000-01-01 00:00:00 UTC.
 *
 *  @return   True if the time was converted successfully. False if the given Unix epoch time
 *            falls outside the representable range.
 */
bool UnixEpochToChipEpochMicros(uint64_t unixEpochTimeMicros, uint64_t & outChipEpochTimeMicros);

/**
 *  @def SecondsToMilliseconds
 *
 *  @brief
 *    Convert integer seconds to milliseconds.
 *
 */
inline uint64_t SecondsToMilliseconds(uint32_t seconds)
{
    return (seconds * kMillisecondsPerSecond);
}

// For backwards-compatibility of public API.
[[deprecated("Use SecondsToMilliseconds")]] inline uint64_t secondsToMilliseconds(uint32_t seconds)
{
    return SecondsToMilliseconds(seconds);
}

} // namespace chip
