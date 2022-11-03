/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <app/AttributePersistenceProvider.h>
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
class DefaultAttributePersistenceProvider : public AttributePersistenceProvider
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
    CHIP_ERROR WriteValue(const ConcreteAttributePath & aPath, const EmberAfAttributeMetadata * aMetadata,
                          const ByteSpan & aValue) override;
    CHIP_ERROR ReadValue(const ConcreteAttributePath & aPath, const EmberAfAttributeMetadata * aMetadata,
                         MutableByteSpan & aValue) override;

protected:
    PersistentStorageDelegate * mStorage;
};

} // namespace app
} // namespace chip
