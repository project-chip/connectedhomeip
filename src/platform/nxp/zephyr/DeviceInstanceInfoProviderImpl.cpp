/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "DeviceInstanceInfoProviderImpl.h"
#include <lib/support/BytesToHex.h>

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CONFIG_CHIP_DEVICE_ROTATING_DEVICE_UID)
    static_assert(ConfigurationManager::kRotatingDeviceIDUniqueIDLength >= ConfigurationManager::kMinRotatingDeviceIDUniqueIDLength,
                  "Length of unique ID for rotating device ID is smaller than minimum.");

    ReturnErrorCodeIf(ConfigurationManager::kRotatingDeviceIDUniqueIDLength > uniqueIdSpan.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    size_t bytesLen = chip::Encoding::HexToBytes(CONFIG_CHIP_DEVICE_ROTATING_DEVICE_UID,
                                                 ConfigurationManager::kRotatingDeviceIDUniqueIDLength * 2, uniqueIdSpan.data(),
                                                 uniqueIdSpan.size());

    ReturnErrorCodeIf(bytesLen != ConfigurationManager::kRotatingDeviceIDUniqueIDLength, CHIP_ERROR_INVALID_STRING_LENGTH);
    uniqueIdSpan.reduce_size(bytesLen);

    return CHIP_NO_ERROR;
#endif // CHIP_ENABLE_ROTATING_DEVICE_ID

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace DeviceLayer
} // namespace chip
