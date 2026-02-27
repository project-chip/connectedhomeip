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

#include <app/persistence/AttributePersistenceMigration.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

CHIP_ERROR MigrateFromSafeToAttributePersistenceProvider(SafeAttributePersistenceProvider & safeProvider,
                                                         AttributePersistenceProvider & dstProvider,
                                                         const ConcreteClusterPath & cluster,
                                                         Span<const AttrMigrationData> attributes, MutableByteSpan & buffer)
{
    CHIP_ERROR migrationError = CHIP_NO_ERROR;
    ConcreteAttributePath attrPath;

    for (const auto & entry : attributes)
    {
        attrPath = ConcreteAttributePath(cluster.mEndpointId, cluster.mClusterId, entry.attributeId);

        // Create a copy of the buffer to check if the value is already in the AttributePersistence.
        // If the attribute value is already stored in AttributePersistence, skip it.
        // Note: we assume any other error (e.g. including buffer too small) to be an indication that
        //       the key exists. The only case we migrate is if we are explicitly told "not found".
        MutableByteSpan readAttrBuffer = buffer;
        if (dstProvider.ReadValue(attrPath, readAttrBuffer) != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            continue;
        }

        // We make a copy of the buffer so it can be resized
        // Still refers to same internal buffer though
        // Read value from the safe provider, will resize copyOfBuffer to read size
        MutableByteSpan copyOfBuffer = buffer;
        // If there was an error reading from SafeAttribute, then we shouldn't try to write that value
        // to AttributePersistence
        ChipError attributeMigrationError = entry.migrator(attrPath, safeProvider, copyOfBuffer);
        if (attributeMigrationError != CHIP_NO_ERROR)
        {
            if(attributeMigrationError != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
            {
                migrationError = CHIP_ERROR_HAD_FAILURES;
                ChipLogError(DataManagement,
                            "AttributeMigration: Error reading SafeAttribute '" ChipLogFormatMEI "' from cluster '" ChipLogFormatMEI
                            "' (err=%" CHIP_ERROR_FORMAT ")",
                            ChipLogValueMEI(entry.attributeId), ChipLogValueMEI(cluster.mClusterId), attributeMigrationError.Format());

            }
            continue;
        }

        // Write value from SafeAttributePersistence into AttributePersistence
        attributeMigrationError = dstProvider.WriteValue(attrPath, copyOfBuffer);
        if (attributeMigrationError != CHIP_NO_ERROR)
        {
            migrationError = CHIP_ERROR_HAD_FAILURES;
            ChipLogError(DataManagement,
                         "AttributeMigration: Error writing Attribute '" ChipLogFormatMEI "' from cluster '" ChipLogFormatMEI
                         "' (err=%" CHIP_ERROR_FORMAT ")",
                         ChipLogValueMEI(entry.attributeId), ChipLogValueMEI(cluster.mClusterId), attributeMigrationError.Format());
        }

        // Always delete from the safe provider, this helps ensure that we only try to migrate the
        // persisted values once.
        attributeMigrationError = safeProvider.SafeDeleteValue(attrPath);
        if (attributeMigrationError != CHIP_NO_ERROR)
        {
            migrationError = CHIP_ERROR_HAD_FAILURES;
            ChipLogError(DataManagement,
                         "AttributeMigration: Error deleting SafeAttribute '" ChipLogFormatMEI "' from cluster '" ChipLogFormatMEI
                         "' (err=%" CHIP_ERROR_FORMAT ")",
                         ChipLogValueMEI(entry.attributeId), ChipLogValueMEI(cluster.mClusterId), attributeMigrationError.Format());
        }
    }
    return migrationError;
}

namespace DefaultMigrators {
CHIP_ERROR SafeValue(const ConcreteAttributePath & attrPath, SafeAttributePersistenceProvider & provider, MutableByteSpan & buffer)
{
    return provider.SafeReadValue(attrPath, buffer);
}
} // namespace DefaultMigrators
} // namespace chip::app
