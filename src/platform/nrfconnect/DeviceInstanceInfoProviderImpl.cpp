/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
