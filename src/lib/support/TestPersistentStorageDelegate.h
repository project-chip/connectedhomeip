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

#include <algorithm>
#include <credentials/FabricTable.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/DLLUtil.h>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace chip {

class TestPersistentStorageDelegate : public PersistentStorageDelegate
{
public:
    TestPersistentStorageDelegate() {}

    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override
    {
        if (mErrorKeys.find(std::string(key)) != mErrorKeys.end())
        {
            return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }
        bool contains = mStorage.find(key) != mStorage.end();
        VerifyOrReturnError(contains, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

        std::vector<uint8_t> & value = mStorage[key];
        uint16_t value_size          = static_cast<uint16_t>(value.size());
        VerifyOrReturnError(value_size <= size, CHIP_ERROR_BUFFER_TOO_SMALL);

        size = std::min(value_size, size);
        memcpy(buffer, value.data(), size);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override
    {
        if (mErrorKeys.find(std::string(key)) != mErrorKeys.end())
        {
            return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }
        const uint8_t * bytes = static_cast<const uint8_t *>(value);
        mStorage[key]         = std::vector<uint8_t>(bytes, bytes + size);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SyncDeleteKeyValue(const char * key) override
    {
        if (mErrorKeys.find(std::string(key)) != mErrorKeys.end())
        {
            return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }
        bool contains = mStorage.find(key) != mStorage.end();
        VerifyOrReturnError(contains, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
        mStorage.erase(key);
        return CHIP_NO_ERROR;
    }

    void AddErrorKey(const std::string & key) { mErrorKeys.insert(key); }

    void ClearErrorKey() { mErrorKeys.clear(); }

protected:
    std::map<std::string, std::vector<uint8_t>> mStorage;
    std::set<std::string> mErrorKeys;
};

} // namespace chip
