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
#include <ble/BleConfig.h>

#if CONFIG_NETWORK_LAYER_BLE

#include <cctype>
#include <cstdint>
#include <cstring>

#include "BleUUID.h"

namespace chip {
namespace Ble {

const ChipBleUUID CHIP_BLE_SVC_ID = { { // 0000FFF6-0000-1000-8000-00805F9B34FB
                                        0x00, 0x00, 0xFF, 0xF6, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34,
                                        0xFB } };

inline static uint8_t HexDigitToInt(const char c)
{
    return static_cast<uint8_t>(c >= '0' && c <= '9' ? c - '0' : tolower(c) - 'a' + 10);
}

bool UUIDsMatch(const ChipBleUUID * idOne, const ChipBleUUID * idTwo)
{
    if ((idOne == nullptr) || (idTwo == nullptr))
    {
        return false;
    }
    return (memcmp(idOne->bytes, idTwo->bytes, 16) == 0);
}

// Convert a string like "0000FFF6-0000-1000-8000-00805F9B34FB" to binary UUID
bool StringToUUID(const char * str, ChipBleUUID & uuid)
{
    constexpr size_t NUM_UUID_NIBBLES = sizeof(uuid.bytes) * 2;
    size_t nibbleId                   = 0;

    for (; *str; ++str)
    {
        if (*str == '-') // skip separators
            continue;

        if (!isxdigit(*str)) // invalid character!
            return false;

        if (nibbleId >= NUM_UUID_NIBBLES) // too long string!
            return false;

        uint8_t & byte = uuid.bytes[nibbleId / 2];
        if (nibbleId % 2 == 0)
            byte = static_cast<uint8_t>(HexDigitToInt(*str) << 4);
        else
            byte = static_cast<uint8_t>(byte | HexDigitToInt(*str));

        ++nibbleId;
    }

    // All bytes were initialized?
    return nibbleId == NUM_UUID_NIBBLES;
}

} /* namespace Ble */
} /* namespace chip */

#endif /* CONFIG_NETWORK_LAYER_BLE */
