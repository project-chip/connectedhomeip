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
#include <app/util/persistence/DefaultAttributePersistenceProvider.h>

#include <app/util/ember-strings.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>

namespace chip {
namespace app {

CHIP_ERROR DefaultAttributePersistenceProvider::InternalReadValue(const StorageKeyName & aKey, EmberAfAttributeType aType,
                                                                  size_t aExpectedSize, MutableByteSpan & aValue)
{
    ReturnErrorOnFailure(StorageDelegateWrapper::ReadValue(aKey, aValue));
    size_t size = aValue.size();
    if (emberAfIsStringAttributeType(aType))
    {
        // Ensure that we've read enough bytes that we are not ending up with
        // un-initialized memory.  Should have read length + 1 (for the length
        // byte).
        VerifyOrReturnError(size >= 1 && size - 1 >= emberAfStringLength(aValue.data()), CHIP_ERROR_INCORRECT_STATE);
    }
    else if (emberAfIsLongStringAttributeType(aType))
    {
        // Ensure that we've read enough bytes that we are not ending up with
        // un-initialized memory.  Should have read length + 2 (for the length
        // bytes).
        VerifyOrReturnError(size >= 2 && size - 2 >= emberAfLongStringLength(aValue.data()), CHIP_ERROR_INCORRECT_STATE);
    }
    else
    {
        // Ensure we got the expected number of bytes for all other types.
        VerifyOrReturnError(size == aExpectedSize, CHIP_ERROR_INVALID_ARGUMENT);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultAttributePersistenceProvider::WriteValue(const ConcreteAttributePath & aPath, const ByteSpan & aValue)
{
    return StorageDelegateWrapper::WriteValue(
        DefaultStorageKeyAllocator::AttributeValue(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId), aValue);
}

CHIP_ERROR DefaultAttributePersistenceProvider::ReadValue(const ConcreteAttributePath & aPath,
                                                          const EmberAfAttributeMetadata * aMetadata, MutableByteSpan & aValue)
{
    return InternalReadValue(DefaultStorageKeyAllocator::AttributeValue(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId),
                             aMetadata->attributeType, aMetadata->size, aValue);
}

} // namespace app
} // namespace chip
