/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <cstdint>

#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <platform/KeyValueStoreManager.h>

namespace chip {

// Sample version of PersistentStorageDelegate that relies only on a KeyValueStoreManager
class KvsPersistentStorageDelegate : public PersistentStorageDelegate
{
public:
    KvsPersistentStorageDelegate()          = default;
    virtual ~KvsPersistentStorageDelegate() = default;

    // No copy, move or assignment.
    KvsPersistentStorageDelegate(const KvsPersistentStorageDelegate &)  = delete;
    KvsPersistentStorageDelegate(const KvsPersistentStorageDelegate &&) = delete;
    KvsPersistentStorageDelegate & operator=(const KvsPersistentStorageDelegate &) = delete;

    CHIP_ERROR Init(DeviceLayer::PersistedStorage::KeyValueStoreManager * kvsManager)
    {
        if (kvsManager == nullptr)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        mKvsManager = kvsManager;

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override
    {
        VerifyOrReturnError(mKvsManager != nullptr, CHIP_ERROR_INCORRECT_STATE);

        uint8_t emptyPlaceholder = 0;
        if (buffer == nullptr)
        {
            if (size != 0)
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            // When size is zero, let's give a non-nullptr to the KVS backend
            buffer = &emptyPlaceholder;
        }

        size_t bytesRead = 0;
        CHIP_ERROR err   = mKvsManager->Get(key, buffer, size, &bytesRead);

        // Update size only if it made sense
        if ((CHIP_ERROR_BUFFER_TOO_SMALL == err) || (CHIP_NO_ERROR == err))
        {
            size = CanCastTo<uint16_t>(bytesRead) ? static_cast<uint16_t>(bytesRead) : 0;
        }

        return err;
    }

    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override
    {
        VerifyOrReturnError(mKvsManager != nullptr, CHIP_ERROR_INCORRECT_STATE);

        uint8_t placeholderForEmpty = 0;
        if (value == nullptr)
        {
            if (size == 0)
            {
                value = &placeholderForEmpty;
            }
            else
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }
        return mKvsManager->Put(key, value, size);
    }

    CHIP_ERROR SyncDeleteKeyValue(const char * key) override
    {
        VerifyOrReturnError(mKvsManager != nullptr, CHIP_ERROR_INCORRECT_STATE);
        return mKvsManager->Delete(key);
    }

protected:
    DeviceLayer::PersistedStorage::KeyValueStoreManager * mKvsManager = nullptr;
};

} // namespace chip
