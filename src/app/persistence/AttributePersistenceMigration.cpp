#include "AttributePersistenceMigration.h"

namespace chip::app {

CHIP_ERROR MigrateFromSafeAttributePersistenceProvider(SafeAttributePersistenceProvider & safeProvider,
                                                       AttributePersistenceProvider & normProvider,
                                                       const ConcreteClusterPath & cluster, Span<const AttributeId> attributes,
                                                       MutableByteSpan & buffer)
{
    ChipError err        = CHIP_NO_ERROR;
    ChipError finalError = CHIP_NO_ERROR;

    if (attributes.size() > 1)
    {
        // We make a copy of the buffer so it can be resized
        // Still refers to same internal buffer though
        MutableByteSpan copyOfBuffer = buffer;

        // Quick check to see if migration already happened
        auto firstAttributePath = ConcreteAttributePath(cluster.mEndpointId, cluster.mClusterId, attributes[0]);
        err                     = normProvider.ReadValue(firstAttributePath, copyOfBuffer);
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

        auto attrPath = ConcreteAttributePath(cluster.mEndpointId, cluster.mClusterId, attr);
        // Read Value, will resize copyOfBuffer to read size
        err = safeProvider.SafeReadValue(attrPath, copyOfBuffer);
        if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            // The value does not exist so nothing to migrate, skip
            continue;
        }
        if (err != CHIP_NO_ERROR)
        {
            auto safePathKey = DefaultStorageKeyAllocator::SafeAttributeValue(cluster.mEndpointId, cluster.mClusterId, attr);
            ChipLogError(NotSpecified, "Migration Error - Error reading attribute %s - %" CHIP_ERROR_FORMAT, safePathKey.KeyName(),
                         err.Format());
            finalError = err; // make sure we report an error if any element fails
            continue;
        }

        err = normProvider.WriteValue(attrPath, copyOfBuffer);
        if (err != CHIP_NO_ERROR)
        {
            auto clusterKey = DefaultStorageKeyAllocator::AttributeValue(cluster.mEndpointId, cluster.mClusterId, attr);
            ChipLogError(NotSpecified, "Migration Error - Failed writing attribute %s - %" CHIP_ERROR_FORMAT, clusterKey.KeyName(),
                         err.Format());
            finalError = err; // make sure we report an error if any element fails
        }

        err = safeProvider.SafeDeleteValue(attrPath);
        if (err != CHIP_NO_ERROR)
        {
            auto safePathKey = DefaultStorageKeyAllocator::SafeAttributeValue(cluster.mEndpointId, cluster.mClusterId, attr);
            ChipLogError(NotSpecified, "Migration Error - Failed deleting attribute %s - %" CHIP_ERROR_FORMAT,
                         safePathKey.KeyName(), err.Format());
            finalError = err; // make sure we report an error if any element fails
        }
    }
    return finalError;
};
} // namespace chip::app
