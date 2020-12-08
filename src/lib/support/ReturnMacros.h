/*
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

#pragma once

#include <core/CHIPError.h>

/// Returns the error code if the expression returns something different
/// than CHIP_NO_ERROR
///
/// Use like:
///   ReturnErrorOnFailure(channel->SendMsg(msg));
#define ReturnErrorOnFailure(expr)                                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        CHIP_ERROR __err = (expr);                                                                                                 \
        if (__err != CHIP_NO_ERROR)                                                                                                \
        {                                                                                                                          \
            return __err;                                                                                                          \
        }                                                                                                                          \
    } while (false)

/// Returns from the void function if expression evaluates to false
///
/// Use like:
///   VerifyOrReturn(param != nullptr, LogError("param is nullptr"));
#define VerifyOrReturn(expr, ...)                                                                                                  \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expr))                                                                                                               \
        {                                                                                                                          \
            __VA_ARGS__;                                                                                                           \
            return;                                                                                                                \
        }                                                                                                                          \
    } while (false)

/// Returns a specified error code if expression evaluates to false
///
/// Use like:
///   VerifyOrReturnError(param != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
#define VerifyOrReturnError(expr, code)                                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(expr))                                                                                                               \
        {                                                                                                                          \
            return code;                                                                                                           \
        }                                                                                                                          \
    } while (false)

/// Returns a specified error code if expression evaluates to true
///
/// Use like:
///   ReturnErrorCodeIf(state == kInitialized, CHIP_NO_ERROR);
///   ReturnErrorCodeIf(state == kInitialized, CHIP_ERROR_INVALID_STATE);
#define ReturnErrorCodeIf(expr, code)                                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        if (expr)                                                                                                                  \
        {                                                                                                                          \
            return code;                                                                                                           \
        }                                                                                                                          \
    } while (false)
