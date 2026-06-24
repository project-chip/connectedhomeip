/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <type_traits>

namespace chip {

/**
 * Returns the number of 1 bits in the specified word (population count).
 *
 * The base template uses a portable software implementation. Explicit
 * specializations below use compiler builtins where available.
 */
template <typename T>
constexpr int PopCount(T word)
{
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "PopCount requires an unsigned integer type");
    int count = 0;
    while (word)
    {
        count++;
        word &= static_cast<T>(word - 1);
    }
    return count;
}

// GCC and Clang both define __GNUC__ and provide these builtins. They map
// directly to a single instruction on most architectures (e.g. x86 POPCNT,
// ARM VCNT). The three variants cover the three distinct widths that the
// standard guarantees: unsigned int, unsigned long, unsigned long long.
#if defined(__GNUC__)
template <>
constexpr inline int PopCount<unsigned int>(unsigned int word)
{
    return __builtin_popcount(word);
}

template <>
constexpr inline int PopCount<unsigned long>(unsigned long word)
{
    return __builtin_popcountl(word);
}

template <>
constexpr inline int PopCount<unsigned long long>(unsigned long long word)
{
    return __builtin_popcountll(word);
}
#endif // defined(__GNUC__)

// Narrow types delegate to unsigned int. Even in the software fallback,
// operating on a full int avoids repeated masking with a narrow type and
// lets the compiler use its natural word size. When the builtin specialization
// above is available they also benefit from it automatically.
template <>
constexpr inline int PopCount<unsigned char>(unsigned char word)
{
    return PopCount(static_cast<unsigned int>(word));
}

template <>
constexpr inline int PopCount<unsigned short>(unsigned short word)
{
    return PopCount(static_cast<unsigned int>(word));
}

} // namespace chip
