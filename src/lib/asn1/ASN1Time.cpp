/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *      This file implements Abstract Syntax Notation One (ASN.1) Time functions.
 *
 */

#include <getopt.h>
#include <inttypes.h>
#include <limits.h>
#include <memory>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <ctype.h>
#include <stdio.h>

#include <lib/asn1/ASN1.h>
#include <lib/support/TimeUtils.h>

namespace chip {
namespace ASN1 {

namespace {

/**
 * @brief Parses the two first characters C-string interpreting its content as a two base-10 digits number.
 *        The C-string pointer is advansed by two characters.
 */
uint8_t atoi2(const char *& buf)
{
    uint8_t val = static_cast<uint8_t>((buf[0] - '0') * 10 + (buf[1] - '0'));
    buf += 2;
    return val;
}

/**
 * @brief Converts two low significan base-10 digits of an integer value (val % 100) to a two character C-string.
 *        The C-string pointer is advansed by two characters.
 */
void itoa2(uint32_t val, char *& buf)
{
    buf[1] = static_cast<char>('0' + (val % 10));
    val /= 10;
    buf[0] = static_cast<char>('0' + (val % 10));
    buf += 2;
}

} // anonymous namespace

CHIP_ERROR ASN1UniversalTime::ImportFrom_ASN1_TIME_string(const CharSpan & asn1_time)
{
    const char * p    = asn1_time.data();
    const size_t size = asn1_time.size();

    VerifyOrReturnError(p != nullptr, ASN1_ERROR_INVALID_STATE);
    VerifyOrReturnError(size == kASN1UTCTimeStringLength || size == kASN1GeneralizedTimeStringLength,
                        ASN1_ERROR_UNSUPPORTED_ENCODING);
    VerifyOrReturnError(p[size - 1] == 'Z', ASN1_ERROR_UNSUPPORTED_ENCODING);
    for (size_t i = 0; i < size - 1; i++)
    {
        VerifyOrReturnError(isdigit(p[i]), ASN1_ERROR_INVALID_ENCODING);
    }

    if (size == kASN1GeneralizedTimeStringLength)
    {
        Year = static_cast<uint16_t>(atoi2(p) * 100 + atoi2(p));
    }
    else
    {
        Year = atoi2(p);
        Year = static_cast<uint16_t>(Year + ((Year >= 50) ? 1900 : 2000));
    }

    Month  = atoi2(p);
    Day    = atoi2(p);
    Hour   = atoi2(p);
    Minute = atoi2(p);
    Second = atoi2(p);

    VerifyOrReturnError(Month > 0 && Month <= kMonthsPerYear, ASN1_ERROR_INVALID_ENCODING);
    VerifyOrReturnError(Day > 0 && Day <= kMaxDaysPerMonth, ASN1_ERROR_INVALID_ENCODING);
    VerifyOrReturnError(Hour < kHoursPerDay, ASN1_ERROR_INVALID_ENCODING);
    VerifyOrReturnError(Minute < kMinutesPerHour, ASN1_ERROR_INVALID_ENCODING);
    VerifyOrReturnError(Second < kSecondsPerMinute, ASN1_ERROR_INVALID_ENCODING);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1UniversalTime::ExportTo_ASN1_TIME_string(MutableCharSpan & asn1_time) const
{
    char * p = asn1_time.data();

    VerifyOrReturnError(p != nullptr, ASN1_ERROR_INVALID_STATE);

    // X.509/RFC5280 mandates that times before 2050 UTC must be encoded as ASN.1 UTCTime values, while
    // times equal or greater than 2050 must be encoded as GeneralizedTime values.  The only difference
    // (in the context of X.509 DER) is that GeneralizedTimes are encoded with a 4 digit year, while
    // UTCTimes are encoded with a two-digit year.
    if (Year < 1950 || Year >= 2050)
    {
        VerifyOrReturnError(asn1_time.size() >= kASN1GeneralizedTimeStringLength, ASN1_ERROR_UNDERRUN);
        itoa2(Year / 100, p);
    }
    else
    {
        VerifyOrReturnError(asn1_time.size() >= kASN1UTCTimeStringLength, ASN1_ERROR_UNDERRUN);
    }

    itoa2(Year, p);
    itoa2(Month, p);
    itoa2(Day, p);
    itoa2(Hour, p);
    itoa2(Minute, p);
    itoa2(Second, p);
    *p = 'Z';

    asn1_time.reduce_size(static_cast<size_t>(p - asn1_time.data() + 1));

    return CHIP_NO_ERROR;
}

bool ASN1UniversalTime::ExportTo_UnixTime(uint32_t & unixEpoch)
{
    return CalendarTimeToSecondsSinceUnixEpoch(Year, Month, Day, Hour, Minute, Second, unixEpoch);
}

} // namespace ASN1
} // namespace chip
