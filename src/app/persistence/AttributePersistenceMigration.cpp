#include "AttributePersistenceMigration.h"

namespace chip::app {

CHIP_ERROR MigrateFromSafeAttributePersistenceProvider(SafeAttributePersistenceProvider & safeProvider,
                                                       AttributePersistenceProvider & normProvider,
                                                       const ConcreteClusterPath & path, Span<AttributeId> attributes,
                                                       MutableByteSpan & buffer)
{
    ChipError err        = CHIP_NO_ERROR;
    ChipError finalError = CHIP_NO_ERROR;

    if (attributes.size() > 1)
    {
        // Quick check to see if migration already happened
        auto firstAttributePath = ConcreteAttributePath(path.mEndpointId, path.mClusterId, attributes[0]);
        err                     = normProvider.ReadValue(firstAttributePath, buffer);
        // If the attribute is already in the standard Attribute provider it means that a migration already happened
        if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            return CHIP_NO_ERROR;
        }
    }

    for (auto attr : attributes)
    {
        // We make a copy of the buffer so it can be resized
        // Still refers to same internal buffer though
        MutableByteSpan copyOfBuffer = buffer;

        auto attrPath = ConcreteAttributePath(path.mEndpointId, path.mClusterId, attr);
        // Read Value, will resize copyOfBuffer to read size
        err = safeProvider.SafeReadValue(attrPath, copyOfBuffer);
        if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            // The value does not exist so nothing to migrate, skip
            continue;
        }
        if (err != CHIP_NO_ERROR)
        {
            auto safePathKey = DefaultStorageKeyAllocator::SafeAttributeValue(path.mEndpointId, path.mClusterId, attr);
            ChipLogError(NotSpecified, "Migration Error - Error reading attribute %s - %" CHIP_ERROR_FORMAT, safePathKey.KeyName(),
                         err.Format());
            finalError = err; // make sure we report an error if any element fails
            continue;
        }

        auto pathKey = DefaultStorageKeyAllocator::AttributeValue(path.mEndpointId, path.mClusterId, attr);

        err = normProvider.WriteValue(attrPath, copyOfBuffer);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Migration Error - Failed writing attribute %s - %" CHIP_ERROR_FORMAT, pathKey.KeyName(),
                         err.Format());
            finalError = err; // make sure we report an error if any element fails
        }

        err = safeProvider.SafeDeleteValue(attrPath);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Migration Error - Failed deleting attribute %s - %" CHIP_ERROR_FORMAT, pathKey.KeyName(),
                         err.Format());
            finalError = err; // make sure we report an error if any element fails
        }
    }
    return finalError;
};
} // namespace chip::app
