#include "AttributePersistenceMigration.h"

namespace chip::app {

CHIP_ERROR MigrateFromSafeAttributePersistanceProvider(SafeAttributePersistenceProvider & safeProvider,
                                                       AttributePersistenceProvider & normProvider, ConcreteClusterPath path,
                                                       Span<AttributeId> attributes, MutableByteSpan & buffer)
{
    ChipError err;
    for (auto attr : attributes)
    {
        // We make a copy of the buffer so it can be resized
        // Still refers to same internal buffer though
        MutableByteSpan copyOfBuffer = buffer;

        auto safePath = DefaultStorageKeyAllocator::SafeAttributeValue(path.mEndpointId, path.mClusterId, attr);
        auto attrPath = ConcreteAttributePath(path.mEndpointId, path.mClusterId, attr);
        // Read Value, will resize copyOfBuffer to read size
        err = safeProvider.SafeReadValue(attrPath, copyOfBuffer);
        if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            // The value does not exist
            continue;
        }
        if (err != CHIP_NO_ERROR)
        {
            // ChipLogError(Unspecified, "Error reading attribute %s - %" CHIP_ERROR_FORMAT, safePath.KeyName(), err);
            continue;
        }

        ReturnErrorOnFailure(normProvider.WriteValue(attrPath, copyOfBuffer));
        // do nothing with this error
        err = safeProvider.SafeDeleteValue(attrPath);
    }
    return err;
};
} // namespace chip::app
