#include "AttributePersistenceMigration.h"

namespace chip::app {
// Only available in little endian for the moment
CHIP_ERROR MigrateFromSafeAttributePersistanceProvider(EndpointId endpointId, ClusterId clusterId, Span<AttributeId> attributes,
                                                       MutableByteSpan & buffer, PersistentStorageDelegate & storageDelegate)
{

    ChipError err;
    DefaultSafeAttributePersistenceProvider safeProvider;
    safeProvider.Init(&storageDelegate);
    DefaultAttributePersistenceProvider normProvider;
    normProvider.Init(&storageDelegate);

    for (auto attr : attributes)
    {
        // We make a copy of the buffer so it can be resized
        MutableByteSpan copyOfBuffer = buffer;

        auto safePath = DefaultStorageKeyAllocator::SafeAttributeValue(endpointId, clusterId, attr);
        auto attrPath = ConcreteAttributePath(endpointId, clusterId, attr);
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
