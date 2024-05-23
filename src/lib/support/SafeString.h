/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      Utilities for safely working with strings.
 *
 */

#pragma once

#include <lib/support/CodeUtils.h>
#include <utility>

namespace chip {

/**
 * A function that determines, at compile time, the maximal length (not
 * counting the null terminator) of a list of literal C strings.  Suitable for
 * determining sizes of buffers that might need to hold any of the given
 * strings.
 *
 * Do NOT pass things that are not string literals to this function.
 *
 * Use like:
 *   constexpr size_t maxLen = MaxStringLength("abc", "defhij", "something");
 */
constexpr size_t MaxStringLength()
{
    return 0;
}

template <size_t FirstLength, typename... RestOfTypes>
constexpr size_t MaxStringLength(const char (&)[FirstLength], RestOfTypes &&... aArgs)
{
    // Subtract 1 because we are not counting the null-terminator.
    return max(FirstLength - 1, MaxStringLength(std::forward<RestOfTypes>(aArgs)...));
}

/**
 * A function that determines, at compile time, the total length (not
 * counting the null terminator) of a list of literal C strings. Suitable for
 * determining sizes of buffers that might need to hold all of the given
 * strings.
 *
 * Do NOT pass things that are not string literals to this function.
 *
 * Use like:
 *   constexpr size_t totalLen = TotalStringLength("abc", "defhij", "something");
 */
constexpr size_t TotalStringLength()
{
    return 0;
}

template <size_t FirstLength, typename... RestOfTypes>
constexpr size_t TotalStringLength(const char (&)[FirstLength], RestOfTypes &&... aArgs)
{
    // Subtract 1 because we are not counting the null-terminator.
    return FirstLength - 1 + TotalStringLength(std::forward<RestOfTypes>(aArgs)...);
}

} // namespace chip
