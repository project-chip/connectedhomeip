/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
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
    enum class LoggingLevel : unsigned
    {
        kDisabled            = 0,
        kLogMutation         = 1,
        kLogMutationAndReads = 2,
    };

    TestPersistentStorageDelegate() {}

    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override
    {
        if (mLoggingLevel >= LoggingLevel::kLogMutationAndReads)
        {
            ChipLogDetail(Test, "TestPersistentStorageDelegate::SyncGetKeyValue: Get key '%s'", StringOrNullMarker(key));
        }

        CHIP_ERROR err = SyncGetKeyValueInternal(key, buffer, size);

        if (mLoggingLevel >= LoggingLevel::kLogMutationAndReads)
        {
            if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
            {
                ChipLogDetail(Test, "--> TestPersistentStorageDelegate::SyncGetKeyValue: Key '%s' not found",
                              StringOrNullMarker(key));
            }
            else if (err == CHIP_ERROR_PERSISTED_STORAGE_FAILED)
            {
                ChipLogDetail(Test, "--> TestPersistentStorageDelegate::SyncGetKeyValue: Key '%s' is a poison key",
                              StringOrNullMarker(key));
            }
        }

        return err;
    }

    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override
    {
        if (mLoggingLevel >= LoggingLevel::kLogMutation)
        {
            ChipLogDetail(Test, "TestPersistentStorageDelegate::SyncSetKeyValue, Set key '%s' with data size %u", key,
                          static_cast<unsigned>(size));
        }

        CHIP_ERROR err = SyncSetKeyValueInternal(key, value, size);

        if (mLoggingLevel >= LoggingLevel::kLogMutationAndReads)
        {
            if (err == CHIP_ERROR_PERSISTED_STORAGE_FAILED)
            {
                ChipLogDetail(Test, "--> TestPersistentStorageDelegate::SyncSetKeyValue: Key '%s' is a poison key",
                              StringOrNullMarker(key));
            }
        }

        return err;
    }

    CHIP_ERROR SyncDeleteKeyValue(const char * key) override
    {
        if (mLoggingLevel >= LoggingLevel::kLogMutation)
        {
            ChipLogDetail(Test, "TestPersistentStorageDelegate::SyncDeleteKeyValue, Delete key '%s'", StringOrNullMarker(key));
        }
        CHIP_ERROR err = SyncDeleteKeyValueInternal(key);

        if (mLoggingLevel >= LoggingLevel::kLogMutation)
        {
            if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
            {
                ChipLogDetail(Test, "--> TestPersistentStorageDelegate::SyncDeleteKeyValue: Key '%s' not found",
                              StringOrNullMarker(key));
            }
            else if (err == CHIP_ERROR_PERSISTED_STORAGE_FAILED)
            {
                ChipLogDetail(Test, "--> TestPersistentStorageDelegate::SyncDeleteKeyValue: Key '%s' is a poison key",
                              StringOrNullMarker(key));
            }
        }

        return err;
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

    /**
     * @return the number of keys currently written in storage
     */
    virtual size_t GetNumKeys() { return mStorage.size(); }

    /**
     * @return a set of all the keys stored
     */
    virtual std::set<std::string> GetKeys()
    {
        std::set<std::string> keys;

        for (auto it = mStorage.begin(); it != mStorage.end(); ++it)
        {
            keys.insert(it->first);
        }

        return keys;
    }

    /**
     * @brief Determine if storage has a given key
     *
     * @param key - key to find (case-sensitive)
     * @return true if key is present in storage, false otherwise
     */
    virtual bool HasKey(const std::string & key) { return (mStorage.find(key) != mStorage.end()); }

    /**
     * @brief Set the logging verbosity for debugging
     *
     * @param loggingLevel - logging verbosity level to set
     */
    virtual void SetLoggingLevel(LoggingLevel loggingLevel) { mLoggingLevel = loggingLevel; }

    /**
     * @brief Dump a list of all storage keys, sorted alphabetically
     */
    virtual void DumpKeys()
    {
        ChipLogError(Test, "TestPersistentStorageDelegate::DumpKeys: %u keys", static_cast<unsigned>(GetNumKeys()));

        auto allKeys = GetKeys();
        std::vector<std::string> allKeysSorted(allKeys.cbegin(), allKeys.cend());
        std::sort(allKeysSorted.begin(), allKeysSorted.end());

        for (const std::string & key : allKeysSorted)
        {
            (void) key.c_str(); // Guard against log level disabling  error logging which would make `key` unused.
            ChipLogError(Test, "  -> %s", key.c_str());
        }
    }

protected:
    virtual CHIP_ERROR SyncGetKeyValueInternal(const char * key, void * buffer, uint16_t & size)
    {
        ReturnErrorCodeIf(((buffer == nullptr) && (size != 0)), CHIP_ERROR_INVALID_ARGUMENT);

        // Making sure poison keys are not accessed
        if (mPoisonKeys.find(std::string(key)) != mPoisonKeys.end())
        {
            return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }

        bool contains = HasKey(key);
        VerifyOrReturnError(contains, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

        std::vector<uint8_t> & value = mStorage[key];
        size_t valueSize             = value.size();
        if (!CanCastTo<uint16_t>(valueSize))
        {
            return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }

        uint16_t valueSizeUint16 = static_cast<uint16_t>(valueSize);
        ReturnErrorCodeIf(size == 0 && valueSizeUint16 == 0, CHIP_NO_ERROR);
        ReturnErrorCodeIf(buffer == nullptr, CHIP_ERROR_BUFFER_TOO_SMALL);

        uint16_t sizeToCopy = std::min(size, valueSizeUint16);

        size = sizeToCopy;
        memcpy(buffer, value.data(), size);
        return size < valueSizeUint16 ? CHIP_ERROR_BUFFER_TOO_SMALL : CHIP_NO_ERROR;
    }

    virtual CHIP_ERROR SyncSetKeyValueInternal(const char * key, const void * value, uint16_t size)
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

    virtual CHIP_ERROR SyncDeleteKeyValueInternal(const char * key)
    {
        // Make sure poison keys are not accessed
        if (mPoisonKeys.find(std::string(key)) != mPoisonKeys.end())
        {
            return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }

        bool contains = HasKey(key);
        VerifyOrReturnError(contains, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
        mStorage.erase(key);
        return CHIP_NO_ERROR;
    }

    std::map<std::string, std::vector<uint8_t>> mStorage;
    std::set<std::string> mPoisonKeys;
    LoggingLevel mLoggingLevel = LoggingLevel::kDisabled;
};

} // namespace chip
