/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>

#include <app/persistence/AttributePersistence.h>

namespace chip::app {

/**
 * @brief
 * This overload provides a simple function to migrate from the SafeAttributeProvider to the standard provider mechanism
 *
 * @param safeProvider A SafeAttributePersistenceProvider implementation to migrate from.
 * @param normProvider A standard AttributePersistenceProvider implementation to migrate to.
 * @param cluster      The concrete cluster path
 * @param attributes   The attributes that need to be migrated
 * @param buffer       An internal buffer used for temporary storage between the providers
 *
 * @return CHIP_NO_ERROR                        On successful migration.
 *         CHIP_ERROR_BUFFER_TOO_SMALL          Value was too big to swap using the chosen buffer size
 *
 *         If the code uses the default implementation the migration can only fail on long strings/lists (choose a bigger buffer)
 *
 *         Other errors are dependent of implementations of the providers, on any error it will continue
 *         and return last error encountered
 */
CHIP_ERROR MigrateFromSafeAttributePersistenceProvider(SafeAttributePersistenceProvider & safeProvider,
                                                       AttributePersistenceProvider & normProvider,
                                                       const ConcreteClusterPath & cluster, Span<const AttributeId> attributes,
                                                       MutableByteSpan & buffer);

/**
 * @brief
 * This overload provides a simple function to migrate from the SafeAttributeProvider to the standard provider mechanism over the
 * same storageDelegate.
 *
 *
 * @param attributeBufferSize  The size of the buffer to use to as swap memory between providers
 * @param cluster              The concrete cluster path
 * @param attributes           The attributes that need to be migrated
 * @param storageDelegate      The
 *
 * @return CHIP_NO_ERROR                        On successful migration.
 *         CHIP_ERROR_BUFFER_TOO_SMALL          Value was too big to swap using the chosen buffer size
 *
 *         If the code uses the default implementation the migration should only fail on long strings/lists (choose a bigger buffer)
 *
 *         Other errors are dependent of implementations of the providers, on any error it will continue
 *         and return last error encountered
 */
template <int attributeBufferSize = 255>
CHIP_ERROR MigrateFromSafeAttributePersistenceProvider(const ConcreteClusterPath & cluster, Span<const AttributeId> attributes,
                                                       PersistentStorageDelegate & storageDelegate)
{
    DefaultSafeAttributePersistenceProvider safeProvider;

    ReturnErrorOnFailure(safeProvider.Init(&storageDelegate));

    DefaultAttributePersistenceProvider normProvider;
    ReturnErrorOnFailure(normProvider.Init(&storageDelegate));

    uint8_t attributeBuffer[attributeBufferSize] = {};
    MutableByteSpan buffer(attributeBuffer);

    return MigrateFromSafeAttributePersistenceProvider(safeProvider, normProvider, cluster, attributes, buffer);
};

} // namespace chip::app
