/*
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
#pragma once

#include <app/AttributePersistenceProvider.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

namespace chip {
namespace app {

/**
 * Default implementation of AttributePersistenceProvider.  This uses
 * PersistentStorageDelegate to store the attribute values.
 *
 * NOTE: SetAttributePersistenceProvider must still be called with an instance
 * of this class, since it can't be constructed automatically without knowing
 * what PersistentStorageDelegate is to be used.
 */
class DefaultAttributePersistenceProvider : public AttributePersistenceProvider, public SafeAttributePersistenceProvider
{
public:
    DefaultAttributePersistenceProvider() {}

    // Passed-in storage must outlive this object.
    CHIP_ERROR Init(PersistentStorageDelegate * storage)
    {
        if (storage == nullptr)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        mStorage = storage;
        return CHIP_NO_ERROR;
    }

    void Shutdown() {}

    // AttributePersistenceProvider implementation.
    CHIP_ERROR WriteValue(const ConcreteAttributePath & aPath, const ByteSpan & aValue) override;
    CHIP_ERROR ReadValue(const ConcreteAttributePath & aPath, const EmberAfAttributeMetadata * aMetadata,
                         MutableByteSpan & aValue) override;

    // SafeAttributePersistenceProvider implementation.
    CHIP_ERROR SafeWriteValue(const ConcreteAttributePath & aPath, const ByteSpan & aValue) override;
    CHIP_ERROR SafeReadValue(const ConcreteAttributePath & aPath, MutableByteSpan & aValue) override;

protected:
    PersistentStorageDelegate * mStorage;

private:
    CHIP_ERROR InternalWriteValue(const StorageKeyName & aKey, const ByteSpan & aValue);
    CHIP_ERROR InternalReadValue(const StorageKeyName & aKey, MutableByteSpan & aValue);
    CHIP_ERROR InternalReadValue(const StorageKeyName & aKey, EmberAfAttributeType aType, size_t aExpectedSize,
                                 MutableByteSpan & aValue);
};

} // namespace app
} // namespace chip
