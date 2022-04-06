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
#include <lib/support/SafeInt.h>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace chip {

/**
 * Implementation of PersistentStorageDelegate suitable for unit tests,
 * where persistence lasts for the object's lifetime and where all data is retained
 * is memory.
 *
 * This version also has "poison keys" which, if accessed, yield an error. This can
 * be used in unit tests to make sure a module making use of the PersistentStorageDelegate
 * does not access some particular keys which should remain untouched by underlying
 * logic.
 */
class TestPersistentStorageDelegate : public PersistentStorageDelegate
{
public:
    TestPersistentStorageDelegate() {}

    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override
    {
        if ((buffer == nullptr) && (size != 0))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        // Making sure poison keys are not accessed
        if (mPoisonKeys.find(std::string(key)) != mPoisonKeys.end())
        {
            return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }

        bool contains = mStorage.find(key) != mStorage.end();
        VerifyOrReturnError(contains, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

        std::vector<uint8_t> & value = mStorage[key];
        size_t valueSize             = value.size();
        if (size < valueSize)
        {
            size = CanCastTo<uint16_t>(valueSize) ? static_cast<uint16_t>(valueSize) : 0;
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }

        size = static_cast<uint16_t>(valueSize);
        memcpy(buffer, value.data(), size);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override
    {
        // Make sure poison keys are not accessed
        if (mPoisonKeys.find(std::string(key)) != mPoisonKeys.end())
        {
            return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }

        // Handle empty values
        if (value == nullptr)
        {
            if (size == 0)
            {
                mStorage[key] = std::vector<uint8_t>();
                return CHIP_NO_ERROR;
            }

            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        // Handle non-empty values

        const uint8_t * bytes = static_cast<const uint8_t *>(value);
        mStorage[key]         = std::vector<uint8_t>(bytes, bytes + size);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SyncDeleteKeyValue(const char * key) override
    {
        // Make sure poison keys are not accessed
        if (mPoisonKeys.find(std::string(key)) != mPoisonKeys.end())
        {
            return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }

        bool contains = mStorage.find(key) != mStorage.end();
        VerifyOrReturnError(contains, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
        mStorage.erase(key);
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Adds a "poison key": a key that, if read/written, implies some bad
     *        behavior occurred.
     *
     * @param key - Poison key to add to the set.
     */
    virtual void AddPoisonKey(const std::string & key) { mPoisonKeys.insert(key); }

    /**
     * @brief Clear all "poison keys"
     *
     */
    virtual void ClearPoisonKeys() { mPoisonKeys.clear(); }

    /**
     * @brief Reset entire contents back to empty. This does NOT clear the "poison keys"
     *
     */
    virtual void ClearStorage() { mStorage.clear(); }

protected:
    std::map<std::string, std::vector<uint8_t>> mStorage;
    std::set<std::string> mPoisonKeys;
};

} // namespace chip
