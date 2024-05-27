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

#define _CHIP_BLE_BLE_H
#include "BleUUID.h"

#include <cctype>
#include <cstdint>
#include <cstring>

namespace chip {
namespace Ble {

const ChipBleUUID CHIP_BLE_SVC_ID = { { // 0000FFF6-0000-1000-8000-00805F9B34FB
                                        0x00, 0x00, 0xFF, 0xF6, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34,
                                        0xFB } };

bool UUIDsMatch(const ChipBleUUID * idOne, const ChipBleUUID * idTwo)
{
    if ((idOne == nullptr) || (idTwo == nullptr))
    {
        return false;
    }
    return (memcmp(idOne->bytes, idTwo->bytes, 16) == 0);
}

} /* namespace Ble */
} /* namespace chip */
