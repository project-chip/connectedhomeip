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

#include <app/SafeAttributePersistenceProvider.h>
#include <app/StorageDelegateWrapper.h>

namespace chip {
namespace app {

class DefaultSafeAttributePersistenceProvider : protected StorageDelegateWrapper, public SafeAttributePersistenceProvider
{
public:
    DefaultSafeAttributePersistenceProvider() = default;

    CHIP_ERROR Init(PersistentStorageDelegate * storage) { return StorageDelegateWrapper::Init(storage); }

    CHIP_ERROR SafeWriteValue(const ConcreteAttributePath & aPath, const ByteSpan & aValue) override
    {
        return StorageDelegateWrapper::WriteValue(
            DefaultStorageKeyAllocator::SafeAttributeValue(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId), aValue);
    }

    CHIP_ERROR SafeReadValue(const ConcreteAttributePath & aPath, MutableByteSpan & aValue) override
    {
        return StorageDelegateWrapper::ReadValue(
            DefaultStorageKeyAllocator::SafeAttributeValue(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId), aValue);
    }
};

} // namespace app
} // namespace chip
