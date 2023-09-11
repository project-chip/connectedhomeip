/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    CHIP_ERROR InternalReadValue(const StorageKeyName & aKey, EmberAfAttributeType aType, size_t aSize, MutableByteSpan & aValue);
};

} // namespace app
} // namespace chip
