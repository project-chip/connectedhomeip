/*
 *
 * SPDX-FileCopyrightText: 2021-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppPreference.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>

#include <app_preference.h>
#include <tizen.h>

#include <lib/support/Base64.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {
namespace Internal {
namespace AppPreference {

CHIP_ERROR CheckData(const char * key)
{
    bool isExist = false;
    int err      = preference_is_existing(key, &isExist);
    VerifyOrReturnError(err == PREFERENCE_ERROR_NONE, CHIP_ERROR_INCORRECT_STATE);
    return isExist ? CHIP_NO_ERROR : CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
}

CHIP_ERROR GetData(const char * key, void * data, size_t dataSize, size_t * getDataSize, size_t offset)
{
    VerifyOrReturnError(data != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    char * encodedData = nullptr;
    // Make sure that string allocated by preference_get_string() will be freed
    std::unique_ptr<char, decltype(&::free)> _{ encodedData, &::free };

    int err = preference_get_string(key, &encodedData);
    if (err == PREFERENCE_ERROR_NO_KEY)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    if (err == PREFERENCE_ERROR_OUT_OF_MEMORY)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    if (err != PREFERENCE_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "Failed to get preference [%s]: %s", StringOrNullMarker(key), get_error_message(err));
        return CHIP_ERROR_INCORRECT_STATE;
    }

    size_t encodedDataSize = strlen(encodedData);

    Platform::ScopedMemoryBuffer<uint8_t> decodedData;
    size_t expectedMaxDecodedSize = BASE64_MAX_DECODED_LEN(encodedDataSize);
    VerifyOrReturnError(decodedData.Alloc(expectedMaxDecodedSize), CHIP_ERROR_NO_MEMORY);

    size_t decodedDataSize = Base64Decode(encodedData, static_cast<uint16_t>(encodedDataSize), decodedData.Get());
    VerifyOrReturnError(dataSize >= decodedDataSize - offset, CHIP_ERROR_BUFFER_TOO_SMALL);

    size_t copySize = std::min(dataSize, decodedDataSize - offset);
    if (getDataSize != nullptr)
    {
        *getDataSize = copySize;
    }
    ::memcpy(data, decodedData.Get() + offset, copySize);

    ChipLogDetail(DeviceLayer, "Get preference data: key=%s len=%u", key, static_cast<unsigned int>(copySize));
    ChipLogByteSpan(DeviceLayer, ByteSpan(reinterpret_cast<uint8_t *>(data), copySize));

    return CHIP_NO_ERROR;
}

CHIP_ERROR SaveData(const char * key, const void * data, size_t dataSize)
{
    // Expected size for null-terminated base64 string
    size_t expectedEncodedSize = BASE64_ENCODED_LEN(dataSize) + 1;

    Platform::ScopedMemoryBuffer<char> encodedData;
    VerifyOrReturnError(encodedData.Alloc(expectedEncodedSize), CHIP_ERROR_NO_MEMORY);

    size_t encodedDataSize = Base64Encode(static_cast<const uint8_t *>(data), static_cast<uint16_t>(dataSize), encodedData.Get());
    encodedData[encodedDataSize] = '\0';

    int err = preference_set_string(key, encodedData.Get());
    if (err == PREFERENCE_ERROR_OUT_OF_MEMORY)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    if (err != PREFERENCE_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "Failed to set preference [%s]: %s", StringOrNullMarker(key), get_error_message(err));
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ChipLogDetail(DeviceLayer, "Save preference data: key=%s len=%u", key, static_cast<unsigned int>(dataSize));
    ChipLogByteSpan(DeviceLayer, ByteSpan(reinterpret_cast<const uint8_t *>(data), dataSize));

    return CHIP_NO_ERROR;
}

CHIP_ERROR RemoveData(const char * key)
{
    int err = preference_remove(key);
    if (err == PREFERENCE_ERROR_NO_KEY)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    if (err != PREFERENCE_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "Failed to remove preference [%s]: %s", StringOrNullMarker(key), get_error_message(err));
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ChipLogProgress(DeviceLayer, "Remove preference data: key=%s", key);
    return CHIP_NO_ERROR;
}

} // namespace AppPreference
} // namespace Internal
} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
