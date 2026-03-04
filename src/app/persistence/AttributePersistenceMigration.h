/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/persistence/AttributePersistenceProvider.h>
#include <app/SafeAttributePersistenceProvider.h>

#include <cstring>

namespace chip::app {

using SafeAttributeMigrator = CHIP_ERROR (*)(const ConcreteAttributePath & attrPath, SafeAttributePersistenceProvider & provider,
                                             MutableByteSpan & buffer);

struct AttrMigrationData
{
    AttributeId attributeId;
    SafeAttributeMigrator migrator;
};
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
 * This function will return CHIP_ERROR_HAD_FAILURES when there are errors reading/deleting from the SafeAttributePersistence
 * or when the write to the destination provider failed.
 *
 * The user should ensure that the provided buffer has enough capacity for the attributes to be migrated.
 *
 * @param safeProvider A SafeAttributePersistenceProvider implementation to migrate from.
 * @param dstProvider A standard AttributePersistenceProvider implementation to migrate to.
 * @param cluster      The concrete cluster path
 * @param attributes   The attributes that need to be migrated
 * @param buffer       An internal buffer used for temporary storage between the providers
 *
 * @return CHIP_NO_ERROR                        On successful migration.
 *         CHIP_ERROR_HAD_FAILURES              There were errors during migration, some attributes might not be migrated.
 *
 */
CHIP_ERROR MigrateFromSafeToAttributePersistenceProvider(SafeAttributePersistenceProvider & safeProvider,
                                                         AttributePersistenceProvider & dstProvider,
                                                         const ConcreteClusterPath & cluster,
                                                         Span<const AttrMigrationData> attributes, MutableByteSpan buffer);

namespace DefaultMigrators {
template <class T>
CHIP_ERROR ScalarValue(const ConcreteAttributePath & attrPath, SafeAttributePersistenceProvider & provider,
                       MutableByteSpan & buffer)
{
    VerifyOrReturnError(sizeof(T) <= buffer.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    T value;
    ReturnErrorOnFailure(provider.ReadScalarValue(attrPath, value));
    buffer.reduce_size(sizeof(T));
    memcpy(buffer.data(), &value, sizeof(T));
    return CHIP_NO_ERROR;
}

CHIP_ERROR SafeValue(const ConcreteAttributePath & attrPath, SafeAttributePersistenceProvider & provider, MutableByteSpan & buffer);
}; // namespace DefaultMigrators

} // namespace chip::app
