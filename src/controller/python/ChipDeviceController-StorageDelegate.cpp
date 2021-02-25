
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

#include <core/CHIPPersistentStorageDelegate.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace Controller {

void PythonPersistentStorageDelegate::SetDelegate(PersistentStorageResultDelegate * delegate)
{
    mDelegate = delegate;
}

void PythonPersistentStorageDelegate::GetKeyValue(const char * key)
{
    auto val = mStorage.find(key);
    if (val == mStorage.end())
    {
        mDelegate->OnStatus(key, PersistentStorageResultDelegate::Operation::kGET, CHIP_ERROR_KEY_NOT_FOUND);
        return;
    }

    mDelegate->OnValue(key, val->second.c_str());
}

CHIP_ERROR PythonPersistentStorageDelegate::GetKeyValue(const char * key, char * value, uint16_t & size)
{
    auto val = mStorage.find(key);
    if (val == mStorage.end())
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }
    if (size == 0 && value == nullptr)
    {
        size = val->second.size() + 1;
        return CHIP_NO_ERROR;
    }
    else if (size < val->second.size() + 1)
    {
        size = val->second.size() + 1;
        return CHIP_ERROR_NO_MEMORY;
    }

    memcpy(value, val->second.c_str(), val->second.size() + 1);
    return CHIP_NO_ERROR;
}

void PythonPersistentStorageDelegate::SetKeyValue(const char * key, const char * value)
{
    mStorage[key] = value;
    ChipLogDetail(Controller, "SetKeyValue: %s=%s", key, value);
    mDelegate->OnStatus(key, PersistentStorageResultDelegate::Operation::kSET, CHIP_NO_ERROR);
}

void PythonPersistentStorageDelegate::DeleteKeyValue(const char * key)
{
    mStorage.erase(key);
    mDelegate->OnStatus(key, PersistentStorageResultDelegate::Operation::kDELETE, CHIP_NO_ERROR);
}

} // namespace Controller
} // namespace chip
