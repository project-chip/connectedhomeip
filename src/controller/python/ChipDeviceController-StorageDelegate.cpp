
/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "controller/python/ChipDeviceController-StorageDelegate.h"

#include <cstring>
#include <map>
#include <string>

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace Controller {

CHIP_ERROR PythonPersistentStorageDelegate::SyncGetKeyValue(const char * key, void * value, uint16_t & size)
{
    auto val = mStorage.find(key);
    if (val == mStorage.end())
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    if (value == nullptr)
    {
        size = 0;
    }

    uint16_t neededSize = val->second.size();
    if (size == 0)
    {
        size = neededSize;
        return CHIP_ERROR_NO_MEMORY;
    }

    if (size < neededSize)
    {
        memcpy(value, val->second.data(), size);
        size = neededSize;
        return CHIP_ERROR_NO_MEMORY;
    }

    memcpy(value, val->second.data(), neededSize);
    size = neededSize;
    return CHIP_NO_ERROR;
}

CHIP_ERROR PythonPersistentStorageDelegate::SyncSetKeyValue(const char * key, const void * value, uint16_t size)
{
    mStorage[key] = std::string(static_cast<const char *>(value), size);
    ChipLogDetail(Controller, "SyncSetKeyValue on %s", key);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PythonPersistentStorageDelegate::SyncDeleteKeyValue(const char * key)
{
    mStorage.erase(key);
    return CHIP_NO_ERROR;
}

namespace Python {

CHIP_ERROR StorageAdapter::SyncGetKeyValue(const char * key, void * value, uint16_t & size)
{
    ChipLogDetail(Controller, "StorageAdapter::GetKeyValue: Key = %s, Value = %p (%u)", key, value, size);

    uint16_t tmpSize = size;

    mGetKeyCb(mContext, key, (char *) value, &tmpSize);

    if (tmpSize == 0)
    {
        ChipLogDetail(Controller, "Key Not Found\n");
        return CHIP_ERROR_KEY_NOT_FOUND;
    }
    else if (size < tmpSize)
    {
        ChipLogDetail(Controller, "Buf not big enough\n");
        size = tmpSize;
        return CHIP_ERROR_NO_MEMORY;
    }
    else
    {
        ChipLogDetail(Controller, "Key Found %d\n", tmpSize);
        size = tmpSize;
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR StorageAdapter::SyncSetKeyValue(const char * key, const void * value, uint16_t size)
{
    ChipLogDetail(Controller, "StorageAdapter::SetKeyValue: Key = %s, Value = %p (%u)", key, value, size);
    mStorage[key] = std::string(static_cast<const char *>(value), size);
    mSetKeyCb(mContext, key, value, size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR StorageAdapter::SyncDeleteKeyValue(const char * key)
{
    mDeleteKeyCb(mContext, key);
    return CHIP_NO_ERROR;
}

} // namespace Python
} // namespace Controller
} // namespace chip
