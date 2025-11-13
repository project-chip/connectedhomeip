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
#include <app/persistence/DefaultAttributePersistenceProvider.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>

namespace chip {
namespace app {

CHIP_ERROR DefaultAttributePersistenceProvider::WriteValue(const ConcreteAttributePath & aPath, const ByteSpan & aValue)
{
    return StorageDelegateWrapper::WriteValue(
        DefaultStorageKeyAllocator::AttributeValue(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId), aValue);
}

CHIP_ERROR DefaultAttributePersistenceProvider::ReadValue(const ConcreteAttributePath & aPath, MutableByteSpan & aValue)
{
    return StorageDelegateWrapper::ReadValue(
        DefaultStorageKeyAllocator::AttributeValue(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId), aValue);
}

} // namespace app
} // namespace chip
