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

#include <app/DefaultAttributePersistenceProvider.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>

namespace chip {
namespace app {

CHIP_ERROR DefaultAttributePersistenceProvider::WriteValue(const ConcreteAttributePath & aPath,
                                                           const EmberAfAttributeMetadata * aMetadata, const ByteSpan & aValue)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // TODO: we may want to have a small cache for values that change a lot, so
    // we only write them once a bunch of changes happen or on timer or
    // shutdown.
    DefaultStorageKeyAllocator key;
    if (!CanCastTo<uint16_t>(aValue.size()))
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    return mStorage->SyncSetKeyValue(key.AttributeValue(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId), aValue.data(),
                                     static_cast<uint16_t>(aValue.size()));
}

CHIP_ERROR DefaultAttributePersistenceProvider::ReadValue(const ConcreteAttributePath & aPath,
                                                          const EmberAfAttributeMetadata * aMetadata, MutableByteSpan & aValue)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);

    DefaultStorageKeyAllocator key;
    uint16_t size = static_cast<uint16_t>(min(aValue.size(), static_cast<size_t>(UINT16_MAX)));
    ReturnErrorOnFailure(mStorage->SyncGetKeyValue(key.AttributeValue(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId),
                                                   aValue.data(), size));
    EmberAfAttributeType type = aMetadata->attributeType;
    if (emberAfIsStringAttributeType(type))
    {
        // Ensure that we've read enough bytes that we are not ending up with
        // un-initialized memory.  Should have read length + 1 (for the length
        // byte).
        VerifyOrReturnError(size >= emberAfStringLength(aValue.data()) + 1, CHIP_ERROR_INCORRECT_STATE);
    }
    else if (emberAfIsLongStringAttributeType(type))
    {
        // Ensure that we've read enough bytes that we are not ending up with
        // un-initialized memory.  Should have read length + 2 (for the length
        // bytes).
        VerifyOrReturnError(size >= emberAfLongStringLength(aValue.data()) + 2, CHIP_ERROR_INCORRECT_STATE);
    }
    else
    {
        // Ensure we got the expected number of bytes for all other types.
        VerifyOrReturnError(size == aMetadata->size, CHIP_ERROR_INCORRECT_STATE);
    }
    aValue.reduce_size(size);
    return CHIP_NO_ERROR;
}

namespace {

AttributePersistenceProvider * gAttributeSaver = nullptr;

} // anonymous namespace

AttributePersistenceProvider * GetAttributePersistenceProvider()
{
    return gAttributeSaver;
}

void SetAttributePersistenceProvider(AttributePersistenceProvider * aProvider)
{
    if (aProvider != nullptr)
    {
        gAttributeSaver = aProvider;
    }
}

} // namespace app
} // namespace chip
