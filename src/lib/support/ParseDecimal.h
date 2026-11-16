/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

#include <charconv>
#include <system_error>
#include <type_traits>

namespace chip {

/**
 * Parses all of `str` as a base-10 integer.
 *
 * Accepts a leading '-' only for signed T. Rejects an empty string, whitespace, '+',
 * trailing characters and values that don't fit in T. Leaves `out` unchanged on failure.
 */
template <typename T>
bool ParseDecimal(CharSpan str, T & out)
{
    static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>, "ParseDecimal requires an integer type");

    // An empty span may have a null data() pointer.
    VerifyOrReturnValue(!str.empty(), false);
    T value;
    auto [parsedEnd, error] = std::from_chars(str.begin(), str.end(), value);
    VerifyOrReturnValue(error == std::errc() && parsedEnd == str.end(), false);
    out = value;
    return true;
}

/// Same as ParseDecimal(CharSpan, T &), for a null-terminated string. Returns false if `str` is null.
template <typename T>
bool ParseDecimal(const char * str, T & out)
{
    return ParseDecimal(CharSpan::fromCharString(str), out);
}

} // namespace chip
