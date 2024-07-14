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
#include <app/util/ember-strings.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>

namespace chip {
namespace app {

CHIP_ERROR DefaultAttributePersistenceProvider::InternalWriteValue(const StorageKeyName & aKey, const ByteSpan & aValue)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // TODO: we may want to have a small cache for values that change a lot, so
    //  we only write them once a bunch of changes happen or on timer or
    //  shutdown.
    if (!CanCastTo<uint16_t>(aValue.size()))
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    return mStorage->SyncSetKeyValue(aKey.KeyName(), aValue.data(), static_cast<uint16_t>(aValue.size()));
}

CHIP_ERROR DefaultAttributePersistenceProvider::InternalReadValue(const StorageKeyName & aKey, MutableByteSpan & aValue)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint16_t size = static_cast<uint16_t>(min(aValue.size(), static_cast<size_t>(UINT16_MAX)));
    ReturnErrorOnFailure(mStorage->SyncGetKeyValue(aKey.KeyName(), aValue.data(), size));
    aValue.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultAttributePersistenceProvider::InternalReadValue(const StorageKeyName & aKey, EmberAfAttributeType aType,
                                                                  size_t aExpectedSize, MutableByteSpan & aValue)
{
    ReturnErrorOnFailure(InternalReadValue(aKey, aValue));
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
    return InternalWriteValue(DefaultStorageKeyAllocator::AttributeValue(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId),
                              aValue);
}

CHIP_ERROR DefaultAttributePersistenceProvider::ReadValue(const ConcreteAttributePath & aPath,
                                                          const EmberAfAttributeMetadata * aMetadata, MutableByteSpan & aValue)
{
    return InternalReadValue(DefaultStorageKeyAllocator::AttributeValue(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId),
                             aMetadata->attributeType, aMetadata->size, aValue);
}

CHIP_ERROR DefaultAttributePersistenceProvider::SafeWriteValue(const ConcreteAttributePath & aPath, const ByteSpan & aValue)
{
    return InternalWriteValue(
        DefaultStorageKeyAllocator::SafeAttributeValue(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId), aValue);
}

CHIP_ERROR DefaultAttributePersistenceProvider::SafeReadValue(const ConcreteAttributePath & aPath, MutableByteSpan & aValue)
{
    return InternalReadValue(
        DefaultStorageKeyAllocator::SafeAttributeValue(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId), aValue);
}

namespace {

AttributePersistenceProvider * gAttributeSaver = nullptr;

} // anonymous namespace

/**
 * Gets the global attribute saver.
 *
 * Note: When storing cluster attributes that are managed via AttributeAccessInterface, it is recommended to
 * use SafeAttributePersistenceProvider. See AttributePersistenceProvider and SafeAttributePersistenceProvider
 * class documentation for more information.
 */
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

namespace {

SafeAttributePersistenceProvider * gSafeAttributeSaver = nullptr;

} // anonymous namespace

/**
 * Gets the global attribute safe saver.
 */
SafeAttributePersistenceProvider * GetSafeAttributePersistenceProvider()
{
    return gSafeAttributeSaver;
}

void SetSafeAttributePersistenceProvider(SafeAttributePersistenceProvider * aProvider)
{
    if (aProvider != nullptr)
    {
        gSafeAttributeSaver = aProvider;
    }
}

} // namespace app
} // namespace chip
