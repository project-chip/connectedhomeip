/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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

#ifndef _CHIP_BLE_BLE_H
#error "Please include <ble/Ble.h> instead!"
#endif

#include <cstddef>
#include <cstdint>
#include <utility>

namespace chip {
namespace Ble {

// Type to represent 128-bit BLE UUIDs. 16-bit short UUIDs may be combined with
// the Bluetooth Base UUID to form full 128-bit UUIDs as described in the
// Service Discovery Protocol (SDP) definition, part of the Bluetooth Core
// Specification.
struct ChipBleUUID
{
    uint8_t bytes[16];
};

// UUID of CHIP BLE service. Exposed for use in scan filter.
extern const ChipBleUUID CHIP_BLE_SVC_ID;

namespace {
constexpr bool isValidHexChar(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

constexpr uint8_t HexDigitToInt(const char c)
{
    if (c >= '0' && c <= '9')
        return static_cast<uint8_t>(c - '0');
    else
        return static_cast<uint8_t>((c >= 'a' ? c - 'a' : c - 'A') + 10);
}
} // namespace

bool UUIDsMatch(const ChipBleUUID * idOne, const ChipBleUUID * idTwo);

/*
 * StringToUUID converts a string representation of a UUID to a binary UUID.
 * The string representation must be in the format "0000FFF6-0000-1000-8000-00805F9B34FB".
 * The function returns a pair of a boolean indicating whether the conversion was successful
 * and the binary UUID.
 *
 */
template <size_t N>
constexpr std::pair<bool, ChipBleUUID> StringToUUID(const char (&str)[N])
{
    constexpr size_t UUID_LEN         = 16;
    constexpr size_t NUM_UUID_NIBBLES = UUID_LEN * 2;
    static_assert(N >= NUM_UUID_NIBBLES);
    ChipBleUUID uuid{};

    size_t nibbleId = 0;
    for (size_t i = 0; i < N - 1; ++i)
    {
        if (str[i] == '-')
            continue;
        if (!isValidHexChar(str[i]))
            return { false, {} };
        if (nibbleId >= NUM_UUID_NIBBLES)
            return { false, {} };
        uint8_t & byte = uuid.bytes[nibbleId / 2];
        if (nibbleId % 2 == 0)
            byte = static_cast<uint8_t>(HexDigitToInt(str[i]) << 4);
        else
            byte = static_cast<uint8_t>(byte | HexDigitToInt(str[i]));
        ++nibbleId;
    }
    return { nibbleId == NUM_UUID_NIBBLES, uuid };
}

#define StringToUUIDConstexpr(str)                                                                                                 \
    [&]() {                                                                                                                        \
        constexpr std::pair<bool, ChipBleUUID> res = StringToUUID(str);                                                            \
        static_assert(res.first, "Argument: \"" #str "\" is not valid hex string");                                                \
        return res.second;                                                                                                         \
    }();

#define StringToUUIDConstexprGlobal(str)                                                                                           \
    []() {                                                                                                                         \
        constexpr std::pair<bool, ChipBleUUID> res = StringToUUID(str);                                                            \
        static_assert(res.first, "Argument: \"" #str "\" is not valid hex string");                                                \
        return res.second;                                                                                                         \
    }();

} /* namespace Ble */
} /* namespace chip */
