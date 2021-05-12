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

#pragma once

#include <map>
#include <string>

#include <core/CHIPPersistentStorageDelegate.h>

typedef void (*GetKeyValueFunct)(const char * key, size_t keyLen, void * value, uint16_t * size);
typedef void (*SetKeyValueFunct)(const char * key, size_t keyLen, const void * value, uint16_t size);
typedef void (*DeleteKeyValueFunct)(const char * key, size_t keyLen);

namespace chip {
namespace Controller {

class PythonPersistentStorageDelegate : public PersistentStorageDelegate
{
public:
    static PythonPersistentStorageDelegate & Instance() { return mInstance; }

    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override;
    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override;
    CHIP_ERROR SyncDeleteKeyValue(const char * key) override;

    void SetCallbacks(GetKeyValueFunct getKeyValueFunc, SetKeyValueFunct setKeyValueFunc, DeleteKeyValueFunct deleteKeyValueFunc)
    {
        mGetKeyValueFunc    = getKeyValueFunc;
        mSetKeyValueFunc    = setKeyValueFunc;
        mDeleteKeyValueFunc = deleteKeyValueFunc;
    }

private:
    PythonPersistentStorageDelegate() {}
    GetKeyValueFunct mGetKeyValueFunc       = nullptr;
    SetKeyValueFunct mSetKeyValueFunc       = nullptr;
    DeleteKeyValueFunct mDeleteKeyValueFunc = nullptr;

    static PythonPersistentStorageDelegate mInstance;
};

} // namespace Controller
} // namespace chip
