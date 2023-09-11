/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
