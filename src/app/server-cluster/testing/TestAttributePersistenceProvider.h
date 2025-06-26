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

#include <app/StorageDelegateWrapper.h>
#include <app/util/persistence/AttributePersistenceProvider.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

namespace chip {

/**
 * Default implementation of AttributePersistenceProvider.  This uses
 * PersistentStorageDelegate to store the attribute values.
 *
 * NOTE: SetAttributePersistenceProvider must still be called with an instance
 * of this class, since it can't be constructed automatically without knowing
 * what PersistentStorageDelegate is to be used.
 */
class TestAttributePersistenceProvider : protected app::StorageDelegateWrapper, public app::AttributePersistenceProvider
{
public:
    TestAttributePersistenceProvider() {};

    CHIP_ERROR Init(PersistentStorageDelegate * storage) { return StorageDelegateWrapper::Init(storage); }

    // AttributePersistenceProvider implementation.
    CHIP_ERROR WriteValue(const app::ConcreteAttributePath & aPath, const ByteSpan & aValue) override 
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ReadValue(const app::ConcreteAttributePath & aPath, const EmberAfAttributeMetadata * aMetadata,
                         MutableByteSpan & aValue) override
    {
        return CHIP_NO_ERROR;
    }
    // PersistenceStorageDelegate accessor
    TestPersistentStorageDelegate & GetPersistenceStorageDelegate() { return mContext; };

    TestPersistentStorageDelegate mContext;

private:
    CHIP_ERROR InternalReadValue(const StorageKeyName & aKey, EmberAfAttributeType aType, size_t aExpectedSize,
                                 MutableByteSpan & aValue);
};

} // namespace chip
