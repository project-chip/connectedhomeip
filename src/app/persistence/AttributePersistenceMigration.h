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
#include <app/persistence/AttributePersistence.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>

#include <cstring>

namespace chip::app {

using SafeAttributeMigrator = CHIP_ERROR (*)(ConcreteAttributePath attrPath, SafeAttributePersistenceProvider & provider,
                                             MutableByteSpan & buffer);
using AttrMigrationData     = std::pair<const AttributeId, SafeAttributeMigrator>;
/**
 * @brief
 * This function migrates attribute values from the SafeAttributeProvider to the standard provider mechanism.
 *
 * Each attribute is first checked in the standard provider. If the value is already present, the attribute
 * is skipped to avoid overwriting a newer runtime value. Otherwise, the attribute is read from the safe
 * provider. If a value is not found in the safe provider, it is skipped.
 * When a value is found, it is always deleted from the safe provider after the read, regardless of whether
 * the write to the standard provider succeeds. This ensures each attribute is only migrated once and avoids
 * overwriting newer runtime values with stale persisted data on subsequent startups.
 *
 * The user should ensure that the provided buffer has enough capacity for the attributes to be migrated.
 *
 * @param safeProvider A SafeAttributePersistenceProvider implementation to migrate from.
 * @param normProvider A standard AttributePersistenceProvider implementation to migrate to.
 * @param cluster      The concrete cluster path
 * @param attributes   The attributes that need to be migrated
 * @param buffer       An internal buffer used for temporary storage between the providers
 *
 * @return CHIP_NO_ERROR                        On successful migration.
 *         CHIP_ERROR_HAD_FAILURES              There were errors during migration, some attributes might not be migrated.
 *
 */
CHIP_ERROR MigrateFromSafeAttributePersistenceProvider(SafeAttributePersistenceProvider & safeProvider,
                                                       AttributePersistenceProvider & normProvider,
                                                       const ConcreteClusterPath & cluster,
                                                       Span<const AttrMigrationData> attributes, MutableByteSpan & buffer);

/**
 * @brief
 * This overload provides a simple function to migrate from the SafeAttributeProvider to the standard provider mechanism over
 * the same storageDelegate.
 *
 *
 * @param attributeBufferSize  The size of the buffer to use as temporary storage between providers
 * @param cluster              The concrete cluster path
 * @param attributes           The attributes that need to be migrated
 * @param storageDelegate      The storage delegate used for persistence
 *
 * @return CHIP_NO_ERROR                        On successful migration.
 *         CHIP_ERROR_HAD_FAILURES              There were errors during migration, some attributes might not be migrated.
 *
 */
template <int attributeBufferSize = 255>
CHIP_ERROR MigrateFromSafeAttributePersistenceProvider(const ConcreteClusterPath & cluster,
                                                       Span<const AttrMigrationData> attributes,
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

namespace DefaultMigrators {
template <class T>
static CHIP_ERROR ScalarValue(ConcreteAttributePath attrPath, SafeAttributePersistenceProvider & provider, MutableByteSpan & buffer)
{
    VerifyOrReturnError(sizeof(T) <= buffer.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    T value;
    ReturnErrorOnFailure(provider.ReadScalarValue(attrPath, value));
    buffer.reduce_size(sizeof(T));
    memcpy(buffer.data(), &value, sizeof(T));
    return CHIP_NO_ERROR;
}

CHIP_ERROR SafeValue(ConcreteAttributePath attrPath, SafeAttributePersistenceProvider & provider, MutableByteSpan & buffer);
}; // namespace DefaultMigrators

} // namespace chip::app
