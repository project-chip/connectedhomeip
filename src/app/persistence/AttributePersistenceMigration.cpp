#include <app/persistence/AttributePersistenceMigration.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

CHIP_ERROR MigrateFromSafeAttributePersistenceProvider(SafeAttributePersistenceProvider & safeProvider,
                                                       AttributePersistenceProvider & normProvider,
                                                       const ConcreteClusterPath & cluster,
                                                       Span<const std::pair<const AttributeId, SafeAttributeMigrator>> attributes,
                                                       MutableByteSpan & buffer)
{
    CHIP_ERROR migrationError = CHIP_NO_ERROR;
    ConcreteAttributePath attrPath;

    for (const auto & [attr, migrator] : attributes)
    {
        ChipError attributeMigrationError = CHIP_NO_ERROR;
        attrPath                          = ConcreteAttributePath(cluster.mEndpointId, cluster.mClusterId, attr);

        // Create a copy of the buffer to check if the value is already in the AttributePersistence
        // If the attribute value is already stored in AttributePersistence, skip it.
        MutableByteSpan readAttrBuffer = buffer;
        if (normProvider.ReadValue(attrPath, readAttrBuffer) != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            continue;
        }

        // We make a copy of the buffer so it can be resized
        // Still refers to same internal buffer though
        // Read value from the safe provider, will resize copyOfBuffer to read size
        MutableByteSpan copyOfBuffer = buffer;
        attributeMigrationError      = migrator(attrPath, safeProvider, copyOfBuffer);
        if (attributeMigrationError == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            // Attribute not in safe provider, nothing to migrate
            continue;
        }

        // Always delete from the safe provider to ensure we only attempt migration once.
        // This avoids overwriting a newer runtime value with a stale persisted one on the next startup.
        RETURN_SAFELY_IGNORED safeProvider.SafeDeleteValue(attrPath);

        // If there was an error reading from SafeAttribute, then we shouldn't try to write that value
        // to AttributePersistence
        if (attributeMigrationError != CHIP_NO_ERROR)
        {
            migrationError = CHIP_ERROR_HAD_FAILURES;
            ChipLogError(NotSpecified,
                         "AttributeMigration: Error reading SafeAttribute '" ChipLogFormatMEI "' from cluster '" ChipLogFormatMEI
                         "' (err=%" CHIP_ERROR_FORMAT ")",
                         ChipLogValueMEI(attr), ChipLogValueMEI(cluster.mClusterId), attributeMigrationError.Format());
            continue;
        }

        // Write value from SafeAttributePersistence into AttributePersistence
        attributeMigrationError = normProvider.WriteValue(attrPath, copyOfBuffer);
        if (attributeMigrationError != CHIP_NO_ERROR)
        {
            migrationError = CHIP_ERROR_HAD_FAILURES;
            ChipLogError(NotSpecified,
                         "AttributeMigration: Error writing Attribute '" ChipLogFormatMEI "' from cluster '" ChipLogFormatMEI
                         "' (err=%" CHIP_ERROR_FORMAT ")",
                         ChipLogValueMEI(attr), ChipLogValueMEI(cluster.mClusterId), attributeMigrationError.Format());
        }
    }
    return migrationError;
};

namespace DefaultMigrators {
CHIP_ERROR SafeValue(ConcreteAttributePath attrPath, SafeAttributePersistenceProvider & provider, MutableByteSpan & buffer)
{
    return provider.SafeReadValue(attrPath, buffer);
}
} // namespace DefaultMigrators
} // namespace chip::app
