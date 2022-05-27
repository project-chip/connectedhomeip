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

#include "AppPreference.h"
#include <app_preference.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPMem.h>
#include <memory>

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
        ChipLogError(DeviceLayer, "Failed to get preference [%s]: %s", key, get_error_message(err));
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

    ChipLogProgress(DeviceLayer, "Get data [%s:%.*s]", key, static_cast<int>(copySize), static_cast<char *>(data));
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
        ChipLogError(DeviceLayer, "Failed to set preference [%s]: %s", key, get_error_message(err));
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ChipLogProgress(DeviceLayer, "Save data [%s:%.*s]", key, static_cast<int>(dataSize), static_cast<const char *>(data));
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
        ChipLogError(DeviceLayer, "Failed to remove preference [%s]: %s", key, get_error_message(err));
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ChipLogProgress(DeviceLayer, "Remove data [%s]", key);
    return CHIP_NO_ERROR;
}

} // namespace AppPreference
} // namespace Internal
} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
