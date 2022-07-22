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

#pragma once

#include <lib/support/BitFlags.h>
#include <lib/support/BitMask.h>
#include <lib/support/Span.h>

#include <type_traits>

namespace chip {
namespace app {
namespace DataModel {

/**
 * IsBasicType checks whether the given type can be encoded into TLV as a single value. numeric values, as well as bitmaps, char
 * strings and octet strings are basic types, see spec 7.18.1.
 */
template <typename X>
struct IsBasicType
{
    static constexpr bool value = std::is_integral<X>::value || std::is_floating_point<X>::value || std::is_enum<X>::value;
};

template <typename X>
struct IsBasicType<BitFlags<X>>
{
    static constexpr bool value = true;
};

template <typename X>
struct IsBasicType<BitMask<X>>
{
    static constexpr bool value = true;
};

template <>
struct IsBasicType<ByteSpan>
{
    static constexpr bool value = true;
};

template <>
struct IsBasicType<CharSpan>
{
    static constexpr bool value = true;
};

} // namespace DataModel
} // namespace app
} // namespace chip
