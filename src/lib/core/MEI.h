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

#pragma once

#include <cstdint>

namespace chip {

namespace MEI {
typedef uint16_t PrefixType;
typedef uint16_t SuffixType;
typedef uint32_t Type;

/**
 * 7.18.2.1 MEI Encoding
 * Suffix 0x0000 - 0xFFFE : Item 0 to 65534
 * Suffix 0xFFFF          : Wildcard
 */
constexpr Type kSuffixMask = 0x0000'FFFF;
constexpr Type kPrefixMask = 0xFFFF'0000;
constexpr Type kWildcard   = 0xFFFF'FFFF;

constexpr SuffixType GetSuffix(Type id)
{
    return static_cast<SuffixType>(id & kSuffixMask);
}

constexpr PrefixType GetPrefix(Type id)
{
    return static_cast<PrefixType>((id & kPrefixMask) >> 16);
}

} // namespace MEI

} // namespace chip
