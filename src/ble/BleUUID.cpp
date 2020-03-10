/*
 *
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
#include <BleLayer/BleConfig.h>

#if CONFIG_NETWORK_LAYER_BLE

#include <stdint.h>
#include <string.h>

#include "BleUUID.h"

namespace nl {
namespace Ble {

const WeaveBleUUID WEAVE_BLE_SVC_ID = { { // 0000FEAF-0000-1000-8000-00805F9B34FB
                                          0x00, 0x00, 0xFE, 0xAF, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34,
                                          0xFB } };

bool UUIDsMatch(const WeaveBleUUID * idOne, const WeaveBleUUID * idTwo)
{
    if ((idOne == NULL) || (idTwo == NULL))
    {
        return false;
    }
    return (memcmp(idOne->bytes, idTwo->bytes, 16) == 0);
}

} /* namespace Ble */
} /* namespace nl */

#endif /* CONFIG_NETWORK_LAYER_BLE */
