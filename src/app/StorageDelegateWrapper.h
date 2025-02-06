/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

namespace chip {
namespace app {

/**
 * Wraps around a PersistentStorageDelegate to perform ByteSpan I/O over StorageKey
 */
class StorageDelegateWrapper
{
public:
    StorageDelegateWrapper() = default;

    // Passed-in storage must outlive this object.
    CHIP_ERROR Init(PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        mStorage = storage;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR WriteValue(const StorageKeyName & aKey, const ByteSpan & aValue);
    CHIP_ERROR ReadValue(const StorageKeyName & aKey, MutableByteSpan & aValue);

private:
    PersistentStorageDelegate * mStorage;
};

} // namespace app
} // namespace chip
