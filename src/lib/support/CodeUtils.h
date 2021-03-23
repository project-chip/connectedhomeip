/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file defines and implements a number of miscellaneous
 *      templates for finding object minima and maxima and interface
 *      macros for assertion checking.
 *
 */

#pragma once

// TODO: Remove transitional includes (these headers are not used here)
#include <core/CHIPError.h>
#include <support/DieMacros.h>
#include <support/ErrorMacros.h>
#include <support/ErrorStr.h>
#include <support/ReturnMacros.h>
#include <support/logging/CHIPLogging.h>

namespace chip {

// Generic min() and max() functions
//
template <typename _T>
constexpr inline const _T & min(const _T & a, const _T & b)
{
    if (b < a)
        return b;

    return a;
}

template <typename _T>
constexpr inline const _T & max(const _T & a, const _T & b)
{
    if (a < b)
        return b;

    return a;
}

} // namespace chip

/**
 *  @def IgnoreUnusedVariable(aVariable)
 *
 *  @brief
 *    This casts the specified @a aVariable to void to quell any
 *    compiler-issued unused variable warnings or errors.
 *
 *  @code
 *  void foo (int err)
 *  {
 *      IgnoreUnusedVariable(err)
 *  }
 *  @endcode
 *
 */
#define IgnoreUnusedVariable(aVariable) ((void) (aVariable))

/**
 * @def ArraySize(aArray)
 *
 * @brief
 *   Returns the size of an array in number of elements.
 *
 * Example Usage:
 *
 * @code
 * int numbers[10];
 * SortNumbers(numbers, ArraySize(numbers));
 * @endcode
 *
 * @return      The size of an array in number of elements.
 */
#define ArraySize(a) (sizeof(a) / sizeof((a)[0]))

#if defined(__cplusplus) &&                                                                                                        \
    ((__cplusplus >= 201703L) || (defined(__GNUC__) && (__GNUC__ >= 7)) || (defined(__clang__)) && (__clang_major__ >= 4))
#define CHECK_RETURN_VALUE [[nodiscard]]
#elif defined(__GNUC__) && (__GNUC__ >= 4)
#define CHECK_RETURN_VALUE __attribute__((warn_unused_result))
#elif defined(_MSC_VER) && (_MSC_VER >= 1700)
#define CHECK_RETURN_VALUE _Check_return_
#else
#define CHECK_RETURN_VALUE
#endif

#if defined(__clang__)
#define FALLTHROUGH [[clang::fallthrough]]
#else
#define FALLTHROUGH (void) 0
#endif
