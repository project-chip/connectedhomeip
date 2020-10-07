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

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>

#include <core/CHIPCore.h>

#include "TimeUtils.h"

namespace chip {

enum
{
    // Number of days during the invariant part of the year (after the leap day).
    kDaysFromMarch1ToDecember31 = 306,

    // Number of years in a Gregorian "cycle", where a cycle is the 400-year period
    // over which the Gregorian calendar repeats.
    kYearsPerCycle = 400,

    // Total number of days within cycle.
    kDaysPerCycle = 146097,

    // Total number of days between 0000/03/01 and 1970/01/01.
    kEpochOffsetDays = 719468
};

/* Returns the number of days between January 1st and March 1st for a given year.
 */
static inline uint8_t DaysToMarch1(uint16_t year)
{
    if (IsLeapYear(year))
        return 60;
    return 59;
}

/* Converts a March-based month number (0=March, 1=April, etc.) to a March-1st based day of year (0=March 1st, 1=March 2nd, etc.).
 *
 * NOTE: This is based on the math described in http://howardhinnant.github.io/date_algorithms.html.
 */
static uint16_t MarchBasedMonthToDayOfYear(uint8_t month)
{
    return (153 * month + 2) / 5;
}

/* Converts a March-1st based day of year (0=March 1st, 1=March 2nd, etc.) to a March-based month number (0=March, 1=April, etc.).
 */
static uint8_t MarchBasedDayOfYearToMonth(uint16_t dayOfYear)
{
    return (5 * dayOfYear + 2) / 153;
}

/**
 *  @def IsLeapYear
 *
 *  @brief
 *    Returns true if the given year is a leap year according to the Gregorian calendar.
 *
 *  @param year
 *    Gregorian calendar year.
 *
 */
bool IsLeapYear(uint16_t year)
{
    return (year % kLeapYearInterval) == 0 && ((year % kYearsPerCentury) != 0 || (year % kYearsPerCycle) == 0);
}

/**
 *  @def DaysInMonth
 *
 *  @brief
 *    Returns the number of days in the given month/year.
 *
 *  @param year
 *    Gregorian calendar year.
 *
 *  @param month
 *    Month in standard form (1=January ... 12=December).
 *
 *  @return
 *    Number of days in the given month.
 */
uint8_t DaysInMonth(uint16_t year, uint8_t month)
{
    static const uint8_t daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (month == kFebruary && IsLeapYear(year))
        return 29;
    if (month >= kJanuary && month <= kDecember)
        return daysInMonth[month - 1];
    return 0;
}

/**
 *  @def FirstWeekdayOfYear
 *
 *  @brief
 *    Returns the day of the week for January 1st of the given year.
 *
 *  @param year
 *    Gregorian calendar year.
 *
 *  @return
 *    The day-of-week (0=Sunday...6=Saturday).
 */
uint8_t FirstWeekdayOfYear(uint16_t year)
{
    // Compute the day of the week for the first day of the given year using Gauss' algorithm.
    return (1 + 5 * ((year - 1) % kLeapYearInterval) + 4 * ((year - 1) % kYearsPerCentury) + 6 * ((year - 1) % kYearsPerCycle)) %
        kDaysPerWeek;
}

/**
 *  @def OrdinalDateToCalendarDate
 *
 *  @brief
 *    Convert an ordinal date (year/day-of-year) to a calendar date.
 *
 *  @param year
 *    Gregorian calendar year.
 *
 *  @param dayOfYear
 *    Ordinal day of year, base 1 (1=January 1st, 2=January 2nd, etc.).
 *
 *  @param month
 *    [OUTPUT] Corresponding month in standard form (1=January ... 12=December).
 *
 *  @param dayOfMonth
 *    [OUTPUT] Corresponding day-of-month in standard form (1=1st, 2=2nd, etc.).
 *
 */
void OrdinalDateToCalendarDate(uint16_t year, uint16_t dayOfYear, uint8_t & month, uint8_t & dayOfMonth)
{
    uint8_t daysToMarch1 = DaysToMarch1(year);

    // Make dayOfYear base 0.
    dayOfYear -= 1;

    // Adjust dayOfYear to a March 1st base (i.e. 0 = March 1, 1 = March 2, etc.).  This numbers January
    // and February at the end of the range, with the benefit that day numbering is identical between
    // standard and leap years with the exception of the leap day itself.
    if (dayOfYear < daysToMarch1)
        dayOfYear += kDaysFromMarch1ToDecember31;
    else
        dayOfYear -= daysToMarch1;

    // Compute a March-based month number (i.e. 0=March...11=February) from the day of year.  This is based
    // on the logic in http://howardhinnant.github.io/date_algorithms.html.
    month = MarchBasedDayOfYearToMonth(dayOfYear);

    // Compute the days from March 1st to the start of the corresponding month.
    uint16_t daysFromMarch1ToStartOfMonth = MarchBasedMonthToDayOfYear(month);

    // Compute the day of month in standard form (1=1st, 2=2nd, etc.).
    dayOfMonth = dayOfYear - daysFromMarch1ToStartOfMonth + 1;

    // Convert the month number to standard form (1=January...12=December).
    month = month + (month < 10 ? 3 : -9);
}

/**
 *  @def CalendarDateToOrdinalDate
 *
 *  @brief
 *    Convert an calendar date to ordinal form (year/day-of-year).
 *
 *  @param year
 *    Gregorian calendar year.
 *
 *  @param month
 *    Month in standard form (1=January ... 12=December).
 *
 *  @param dayOfMonth
 *    Day-of-month in standard form (1=1st, 2=2nd, etc.).
 *
 *  @param dayOfYear
 *    [OUTPUT] Ordinal day of year, base 1 (1=January 1st, 2=January 2nd, etc.).
 *
 */
void CalendarDateToOrdinalDate(uint16_t year, uint8_t month, uint8_t dayOfMonth, uint16_t & dayOfYear)
{
    // Convert month to a March-based month number (i.e. 0=March, 1=April, ...11=February).
    month = month + (month > kFebruary ? -3 : 9);

    // Compute the days from March 1st to the start of the corresponding month.
    dayOfYear = MarchBasedMonthToDayOfYear(month);

    // Adjust dayOfYear to be January-based (0=January 1st, 1=January 2nd...).
    if (dayOfYear < kDaysFromMarch1ToDecember31)
        dayOfYear += DaysToMarch1(year);
    else
        dayOfYear -= kDaysFromMarch1ToDecember31;

    // Add in day of month, converting to base 1 in the process.
    dayOfYear += dayOfMonth;
}

/**
 *  @def CalendarDateToDaysSinceEpoch
 *
 *  @brief
 *    Convert a calendar date to the number of days since 1970-01-01.
 *
 *  @param year
 *    Gregorian calendar year in the range 1970 to 28276.
 *
 *  @param month
 *    Month in standard form (1=January ... 12=December).
 *
 *  @param dayOfMonth
 *    Day-of-month in standard form (1=1st, 2=2nd, etc.).
 *
 *  @param daysSinceEpoch
 *    [OUTPUT] Number of days since 1970-01-01.
 *
 *  @return
 *    True if the date was converted successfully.  False if the given year falls outside the
 *    representable range.
 *
 *  @note
 *    This function makes no attempt to verify the correct range of any arguments other than year.
 *    Therefore callers must make sure the supplied values are valid prior to calling the function.
 */
bool CalendarDateToDaysSinceEpoch(uint16_t year, uint8_t month, uint8_t dayOfMonth, uint32_t & daysSinceEpoch)
{
    // NOTE: This algorithm is based on the logic described in http://howardhinnant.github.io/date_algorithms.html.

    // Return immediately if the year is out of range.
    if (year < kEpochYear || year > kMaxYearInDaysSinceEpoch32)
    {
        daysSinceEpoch = UINT32_MAX;
        return false;
    }

    // Adjust the year and month to be March-based (i.e. 0=March, 1=April, ...11=February).
    if (month <= kFebruary)
    {
        year--;
        month += 9;
    }
    else
        month -= 3;

    // Compute the days from March 1st to the start of the specified day.
    uint16_t dayOfYear = MarchBasedMonthToDayOfYear(month) + (dayOfMonth - 1);

    // Compute the 400-year Gregorian "cycle" within which the given year falls.
    uint16_t cycle = year / kYearsPerCycle;

    // Compute the relative year within the cycle.
    uint32_t yearOfCycle = year - (cycle * kYearsPerCycle);

    // Compute the relative day within the cycle, accounting for leap-years.
    uint32_t dayOfCycle =
        (yearOfCycle * kDaysPerStandardYear) + dayOfYear - (yearOfCycle / kYearsPerCentury) + (yearOfCycle / kLeapYearInterval);

    // Compute the total number of days since the start of the logical calendar (0000-03-01).
    uint32_t daysSinceCalendarStart = (cycle * kDaysPerCycle) + dayOfCycle;

    // Adjust the days value to be days since 1970-01-01.
    daysSinceEpoch = daysSinceCalendarStart - kEpochOffsetDays;

    return true;
}

/**
 *  @def DaysSinceEpochToCalendarDate
 *
 *  @brief
 *    Convert the number of days since 1970-01-01 to a calendar date.
 *
 *  @param daysSinceEpoch
 *    Number of days since 1970-01-01.
 *
 *  @param year
 *    [OUTPUT] Gregorian calendar year.
 *
 *  @param month
 *    [OUTPUT] Month in standard form (1=January ... 12=December).
 *
 *  @param dayOfMonth
 *    [OUTPUT] Day-of-month in standard form (1=1st, 2=2nd, etc.).
 *
 */
void DaysSinceEpochToCalendarDate(uint32_t daysSinceEpoch, uint16_t & year, uint8_t & month, uint8_t & dayOfMonth)
{
    // NOTE: This algorithm is based on the logic described in http://howardhinnant.github.io/date_algorithms.html.

    // Adjust days value to be relative to 0000-03-01.
    daysSinceEpoch += kEpochOffsetDays;

    // Compute the 400-year Gregorian cycle in which the given day resides.
    uint32_t cycle = daysSinceEpoch / kDaysPerCycle;

    // Compute the relative day within the cycle.
    uint32_t dayOfCycle = daysSinceEpoch - (cycle * kDaysPerCycle);

    // Compute the relative year within the cycle, adjusting for leap-years.
    uint16_t yearOfCycle = (dayOfCycle - dayOfCycle / 1460 + dayOfCycle / 36524 - dayOfCycle / 146096) / kDaysPerStandardYear;

    // Compute the relative day with the year.
    uint16_t dayOfYear =
        dayOfCycle - (yearOfCycle * kDaysPerStandardYear + yearOfCycle / kLeapYearInterval - yearOfCycle / kYearsPerCentury);

    // Compute a March-based month number (i.e. 0=March...11=February) from the day of year.
    month = MarchBasedDayOfYearToMonth(dayOfYear);

    // Compute the days from March 1st to the start of the corresponding month.
    uint16_t daysFromMarch1ToStartOfMonth = MarchBasedMonthToDayOfYear(month);

    // Compute the day of month in standard form (1=1st, 2=2nd, etc.).
    dayOfMonth = dayOfYear - daysFromMarch1ToStartOfMonth + 1;

    // Convert the month number to standard form (1=January...12=December).
    month = month + (month < 10 ? 3 : -9);

    // Compute the year, adjusting for the standard start of year (January).
    year = yearOfCycle + cycle * kYearsPerCycle;
    if (month <= kFebruary)
        year++;
}

/**
 *  @def AdjustCalendarDate
 *
 *  @brief
 *    Adjust a calendar date by a given number of days (positive or negative).
 *
 *  @param year
 *    [INPUT/OUTPUT] Gregorian calendar year.
 *
 *  @param month
 *    [INPUT/OUTPUT] Month in standard form (1=January ... 12=December).
 *
 *  @param dayOfMonth
 *    [INPUT/OUTPUT] Day-of-month in standard form (1=1st, 2=2nd, etc.).
 *
 *  @param relativeDays
 *    Number of days to add/subtract from given calendar date.
 *
 *  @note
 *    Given date must be equal to or greater than 1970-01-01.
 */
void AdjustCalendarDate(uint16_t & year, uint8_t & month, uint8_t & dayOfMonth, int32_t relativeDays)
{
    uint32_t daysSinceEpoch;

    CalendarDateToDaysSinceEpoch(year, month, dayOfMonth, daysSinceEpoch);
    daysSinceEpoch += relativeDays;
    DaysSinceEpochToCalendarDate(daysSinceEpoch, year, month, dayOfMonth);
}

/**
 *  @def CalendarTimeToSecondsSinceEpoch
 *
 *  @brief
 *    Convert a calendar date and time to the number of seconds since 1970-01-01 00:00:00 UTC.
 *
 *  @details
 *    This function is roughly equivalent to the POSIX gmtime() function with the exception
 *    that the output time value is limited to positive values up to 2^32-1.  This limits the
 *    representable date range to the year 2105.
 *
 *  @note
 *    This function makes no attempt to verify the correct range of any arguments other than year.
 *    Therefore callers must make sure the supplied values are valid prior to invocation.
 *
 *  @param secondsSinceEpoch
 *    Number of seconds since 1970-01-01 00:00:00 UTC.  Note: this value is compatible with
 *    *positive* values of the POSIX time_t value up to the year 2105.
 *
 *  @param year
 *    Gregorian calendar year in the range 1970 to 2105.
 *
 *  @param month
 *    Month in standard form (1=January ... 12=December).
 *
 *  @param dayOfMonth
 *    Day-of-month in standard form (1=1st, 2=2nd, etc.).
 *
 *  @param hour
 *    Hour (0-23).
 *
 *  @param minute
 *    Minute (0-59).
 *
 *  @param second
 *    Second (0-59).
 *
 *  @return
 *    True if the date/time was converted successfully.  False if the given year falls outside the
 *    representable range.
 */
bool CalendarTimeToSecondsSinceEpoch(uint16_t year, uint8_t month, uint8_t dayOfMonth, uint8_t hour, uint8_t minute, uint8_t second,
                                     uint32_t & secondsSinceEpoch)
{
    uint32_t daysSinceEpoch;

    // Return immediately if the year is out of range.
    if (year < kEpochYear || year > kMaxYearInSecondsSinceEpoch32)
    {
        secondsSinceEpoch = UINT32_MAX;
        return false;
    }

    CalendarDateToDaysSinceEpoch(year, month, dayOfMonth, daysSinceEpoch);

    secondsSinceEpoch = (daysSinceEpoch * kSecondsPerDay) + (hour * kSecondsPerHour) + (minute * kSecondsPerMinute) + second;

    return true;
}

/**
 *  @def SecondsSinceEpochToCalendarTime
 *
 *  @brief
 *    Convert the number of seconds since 1970-01-01 00:00:00 UTC to a calendar date and time.
 *
 *  @details
 *    This function is roughly equivalent to the POSIX mktime() function, with the following
 *    exceptions:
 *
 *    - Input time values are limited to positive values up to 2^32-1.  This limits the
 *    representable date range to the year 2105.
 *
 *    - The output time is always UTC (unlike mktime() which outputs time in the process's
 *    configured timezone).
 *
 *  @param secondsSinceEpoch
 *    Number of seconds since 1970-01-01 00:00:00 UTC.  Note: this value is compatible with
 *    *positive* values of the POSIX time_t value up to the year 2105.
 *
 *  @param year
 *    [OUTPUT] Gregorian calendar year.
 *
 *  @param month
 *    [OUTPUT] Month in standard form (1=January ... 12=December).
 *
 *  @param dayOfMonth
 *    [OUTPUT] Day-of-month in standard form (1=1st, 2=2nd, etc.).
 *
 *  @param hour
 *    [OUTPUT] Hour (0-23).
 *
 *  @param minute
 *    [OUTPUT] Minute (0-59).
 *
 *  @param second
 *    [OUTPUT] Second (0-59).
 */
void SecondsSinceEpochToCalendarTime(uint32_t secondsSinceEpoch, uint16_t & year, uint8_t & month, uint8_t & dayOfMonth,
                                     uint8_t & hour, uint8_t & minute, uint8_t & second)
{
    uint32_t daysSinceEpoch = secondsSinceEpoch / kSecondsPerDay;
    uint32_t timeOfDay      = secondsSinceEpoch - (daysSinceEpoch * kSecondsPerDay);

    DaysSinceEpochToCalendarDate(daysSinceEpoch, year, month, dayOfMonth);

    hour = static_cast<uint8_t>(timeOfDay / kSecondsPerHour);
    timeOfDay -= (hour * kSecondsPerHour);
    minute = static_cast<uint8_t>(timeOfDay / kSecondsPerMinute);
    timeOfDay -= (minute * kSecondsPerMinute);
    second = static_cast<uint8_t>(timeOfDay);
}

} // namespace chip
