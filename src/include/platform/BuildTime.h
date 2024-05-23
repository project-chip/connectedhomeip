/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

// Adapted from https://stackoverflow.com/questions/11697820/how-to-use-date-and-time-predefined-macros-in-as-two-integers-then-stri

#pragma once

#include <platform/CHIPDeviceConfig.h>

// Expects input as provided by __DATE__ and __TIME__ macros.
//
// Example of __DATE__ string: "Jul 27 2012"
// Example of __DATE__ string: "Jul  2 2012" -> note leading whitespace for single-digit day of month
// Example of __TIME__ string: "21:06:19"

#define COMPUTE_BUILD_YEAR(_date)                                                                                                  \
    static_cast<uint16_t>(((_date)[7] - '0') * 1000 + ((_date)[8] - '0') * 100 + ((_date)[9] - '0') * 10 + ((_date)[10] - '0'))

#define COMPUTE_BUILD_DAY(_date) static_cast<uint8_t>((((_date)[4] >= '0') ? ((_date)[4] - '0') * 10 : 0) + ((_date)[5] - '0'))

#define BUILD_MONTH_IS_JAN(_date) ((_date)[0] == 'J' && (_date)[1] == 'a')
#define BUILD_MONTH_IS_FEB(_date) ((_date)[0] == 'F')
#define BUILD_MONTH_IS_MAR(_date) ((_date)[0] == 'M' && (_date)[1] == 'a' && (_date)[2] == 'r')
#define BUILD_MONTH_IS_APR(_date) ((_date)[0] == 'A' && (_date)[1] == 'p')
#define BUILD_MONTH_IS_MAY(_date) ((_date)[0] == 'M' && (_date)[1] == 'a' && (_date)[2] == 'y')
#define BUILD_MONTH_IS_JUN(_date) ((_date)[0] == 'J' && (_date)[1] == 'u' && (_date)[2] == 'n')
#define BUILD_MONTH_IS_JUL(_date) ((_date)[0] == 'J' && (_date)[1] == 'u' && (_date)[2] == 'l')
#define BUILD_MONTH_IS_AUG(_date) ((_date)[0] == 'A' && (_date)[1] == 'u')
#define BUILD_MONTH_IS_SEP(_date) ((_date)[0] == 'S')
#define BUILD_MONTH_IS_OCT(_date) ((_date)[0] == 'O')
#define BUILD_MONTH_IS_NOV(_date) ((_date)[0] == 'N')
#define BUILD_MONTH_IS_DEC(_date) ((_date)[0] == 'D')

#define COMPUTE_BUILD_MONTH(_date)                                                                                                 \
    ((BUILD_MONTH_IS_JAN(_date))       ? 1                                                                                         \
         : (BUILD_MONTH_IS_FEB(_date)) ? 2                                                                                         \
         : (BUILD_MONTH_IS_MAR(_date)) ? 3                                                                                         \
         : (BUILD_MONTH_IS_APR(_date)) ? 4                                                                                         \
         : (BUILD_MONTH_IS_MAY(_date)) ? 5                                                                                         \
         : (BUILD_MONTH_IS_JUN(_date)) ? 6                                                                                         \
         : (BUILD_MONTH_IS_JUL(_date)) ? 7                                                                                         \
         : (BUILD_MONTH_IS_AUG(_date)) ? 8                                                                                         \
         : (BUILD_MONTH_IS_SEP(_date)) ? 9                                                                                         \
         : (BUILD_MONTH_IS_OCT(_date)) ? 10                                                                                        \
         : (BUILD_MONTH_IS_NOV(_date)) ? 11                                                                                        \
         : (BUILD_MONTH_IS_DEC(_date)) ? 12                                                                                        \
                                       : /* error default */ 99)

#define COMPUTE_BUILD_HOUR(_time) static_cast<uint8_t>(((_time)[0] - '0') * 10 + (_time)[1] - '0')
#define COMPUTE_BUILD_MIN(_time) static_cast<uint8_t>(((_time)[3] - '0') * 10 + (_time)[4] - '0')
#define COMPUTE_BUILD_SEC(_time) static_cast<uint8_t>(((_time)[6] - '0') * 10 + (_time)[7] - '0')

#define BUILD_DATE_IS_BAD(_date) ((_date) == nullptr || strlen(_date) < strlen("Jan 01 2000") || (_date)[0] == '?')
#define BUILD_TIME_IS_BAD(_time) ((_time) == nullptr || strlen(_time) < strlen("00:00:00") || (_time)[0] == '?')
