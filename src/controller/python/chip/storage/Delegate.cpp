
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

#include "Delegate.h"

#include <cstring>
#include <map>
#include <string>

#include <core/CHIPPersistentStorageDelegate.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

using namespace chip::Controller;

namespace chip {
namespace Controller {

PythonPersistentStorageDelegate PythonPersistentStorageDelegate::mInstance;

CHIP_ERROR PythonPersistentStorageDelegate::SyncGetKeyValue(const char * key, void * value, uint16_t & size)
{
    VerifyOrReturnError(mGetKeyValueFunc != nullptr, CHIP_ERROR_INCORRECT_STATE);
    uint16_t readSize = size;
    mGetKeyValueFunc(key, strlen(key), value, &readSize);

    if (readSize == UINT16_MAX)
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    if (readSize > size)
    {
        size = readSize;
        return CHIP_ERROR_NO_MEMORY;
    }

    size = readSize;
    return CHIP_NO_ERROR;
}

CHIP_ERROR PythonPersistentStorageDelegate::SyncSetKeyValue(const char * key, const void * value, uint16_t size)
{
    VerifyOrReturnError(mSetKeyValueFunc != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mSetKeyValueFunc(key, strlen(key), value, size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PythonPersistentStorageDelegate::SyncDeleteKeyValue(const char * key)
{
    VerifyOrReturnError(mDeleteKeyValueFunc != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mDeleteKeyValueFunc(key, strlen(key));
    return CHIP_NO_ERROR;
}

} // namespace Controller
} // namespace chip

extern "C" {

void pychip_PythonPersistentStorageDelegate_SetCallbacks(GetKeyValueFunct getKeyValueFunc, SetKeyValueFunct setKeyValueFunc,
                                                         DeleteKeyValueFunct deleteKeyValueFunc)
{
    PythonPersistentStorageDelegate::Instance().SetCallbacks(getKeyValueFunc, setKeyValueFunc, deleteKeyValueFunc);
}
}
