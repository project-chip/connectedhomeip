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

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {
namespace Internal {
namespace AppPreference {

static CHIP_ERROR __IsKeyExist(const char * key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int preErr     = PREFERENCE_ERROR_NONE;
    bool isExist   = false;

    preErr = preference_is_existing(key, &isExist);
    if (preErr != PREFERENCE_ERROR_NONE)
    {
        err = CHIP_ERROR_INCORRECT_STATE;
    }
    else if (isExist == false)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return err;
}

CHIP_ERROR CheckData(const char * key)
{
    return __IsKeyExist(key);
}

CHIP_ERROR GetData(const char * key, void * data, size_t dataSize, size_t * getDataSize, size_t offset)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    int preErr                    = PREFERENCE_ERROR_NONE;
    char * encodedData            = NULL;
    uint8_t * decodedData         = NULL;
    size_t encodedDataSize        = 0;
    size_t encodedDataPaddingSize = 0;
    size_t decodedDataSize        = 0;
    size_t expectedDecodedSize    = 0;
    size_t copy_size              = 0;

    VerifyOrExit(data != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = __IsKeyExist(key);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogProgress(DeviceLayer, "Not found data [%s]", key));

    preErr = preference_get_string(key, &encodedData);
    VerifyOrExit(preErr == PREFERENCE_ERROR_NONE, err = CHIP_ERROR_INCORRECT_STATE);
    encodedDataSize = strlen(encodedData);

    if ((encodedDataSize > 0) && (encodedData[encodedDataSize - 1] == '='))
    {
        encodedDataPaddingSize++;
        if ((encodedDataSize > 1) && (encodedData[encodedDataSize - 2] == '='))
            encodedDataPaddingSize++;
    }
    expectedDecodedSize = ((encodedDataSize - encodedDataPaddingSize) * 3) / 4;

    decodedData = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(expectedDecodedSize));
    VerifyOrExit(decodedData != NULL, err = CHIP_ERROR_INCORRECT_STATE);

    decodedDataSize = Base64Decode(encodedData, static_cast<uint16_t>(encodedDataSize), decodedData);

    copy_size = min(dataSize, decodedDataSize - offset);
    if (getDataSize != NULL)
    {
        *getDataSize = copy_size;
    }
    memset(data, 0, dataSize);
    memcpy(data, decodedData + offset, copy_size);

    ChipLogProgress(DeviceLayer, "Get data [%s:%s]", key, (char *) data);

    chip::Platform::MemoryFree(decodedData);

    VerifyOrExit(dataSize >= decodedDataSize - offset, err = CHIP_ERROR_BUFFER_TOO_SMALL);

exit:
    free(encodedData);
    return err;
}

CHIP_ERROR SaveData(const char * key, const uint8_t * data, size_t dataSize)
{
    CHIP_ERROR err             = CHIP_NO_ERROR;
    int preErr                 = PREFERENCE_ERROR_NONE;
    char * encodedData         = NULL;
    size_t encodedDataSize     = 0;
    size_t expectedEncodedSize = ((dataSize + 3) * 4) / 3;

    err = __IsKeyExist(key);
    if (err == CHIP_NO_ERROR)
    {
        VerifyOrExit(RemoveData(key) == CHIP_NO_ERROR, err = CHIP_ERROR_INCORRECT_STATE);
    }
    err = CHIP_NO_ERROR;

    encodedData = static_cast<char *>(chip::Platform::MemoryAlloc(expectedEncodedSize));
    VerifyOrExit(encodedData != NULL, err = CHIP_ERROR_INCORRECT_STATE);

    encodedDataSize              = Base64Encode(data, static_cast<uint16_t>(dataSize), encodedData);
    encodedData[encodedDataSize] = 0;

    preErr = preference_set_string(key, encodedData);
    if (preErr == PREFERENCE_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Save data [%s:%s]", key, data);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "FAIL: set string [%s]", get_error_message(preErr));
        err = CHIP_ERROR_INCORRECT_STATE;
    }

    chip::Platform::MemoryFree(encodedData);

exit:
    return err;
}

CHIP_ERROR RemoveData(const char * key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int preErr     = PREFERENCE_ERROR_NONE;

    preErr = preference_remove(key);
    if (preErr == PREFERENCE_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Remove data [%s]", key);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "FAIL: remove preference [%s]", get_error_message(preErr));
        err = CHIP_ERROR_INCORRECT_STATE;
    }

    return err;
}

} // namespace AppPreference
} // namespace Internal
} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
