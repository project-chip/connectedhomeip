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
#include <lib/core/CHIPEncoding.h>
#include <lib/support/logging/CHIPLogging.h>
#include <nlbyteorder.h>

#include <cstring>

namespace {

using namespace ::chip;
using namespace chip::app;

/**
 * Validates that the given size is a supported scalar width (1, 2, 4, or 8 bytes).
 * On big-endian systems, also performs an in-place little-endian to host byte-order swap.
 * On little-endian systems, no swap is needed so only the size validation is performed.
 */
CHIP_ERROR HostSwapBySize(uint8_t * data, size_t size)
{
    switch (size)
    {
#if (NLBYTEORDER == NLBYTEORDER_BIG_ENDIAN)
    case 1:
        // Single byte, no swap needed.
        return CHIP_NO_ERROR;
    case 2: {
        uint16_t val;
        memcpy(&val, data, sizeof(val));
        val = chip::Encoding::LittleEndian::HostSwap16(val);
        memcpy(data, &val, sizeof(val));
        return CHIP_NO_ERROR;
    }
    case 4: {
        uint32_t val;
        memcpy(&val, data, sizeof(val));
        val = chip::Encoding::LittleEndian::HostSwap32(val);
        memcpy(data, &val, sizeof(val));
        return CHIP_NO_ERROR;
    }
    case 8: {
        uint64_t val;
        memcpy(&val, data, sizeof(val));
        val = chip::Encoding::LittleEndian::HostSwap64(val);
        memcpy(data, &val, sizeof(val));
        return CHIP_NO_ERROR;
    }
#else
    case 1:
    case 2:
    case 4:
    case 8:
        return CHIP_NO_ERROR;
#endif
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

CHIP_ERROR MigrateValueFromSafe(const ConcreteAttributePath & attrPath, SafeAttributePersistenceProvider & provider,
                                MutableByteSpan & buffer, size_t valueSize, bool isScalar)
{

    VerifyOrReturnError(valueSize <= buffer.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

#if (NLBYTEORDER == NLBYTEORDER_LITTLE_ENDIAN)
    // On little-endian systems, no byte swap is needed for scalar values.
    // Read directly into the output buffer and validate the size.
    ReturnErrorOnFailure(provider.SafeReadValue(attrPath, buffer));
    VerifyOrReturnError(buffer.size() == valueSize, CHIP_ERROR_INCORRECT_STATE);
    // For scalar values in LE the function HostSwapBySize just checks that a valid size was provided.
    if (isScalar)
    {
        ReturnErrorOnFailure(HostSwapBySize(buffer.data(), valueSize));
    }
#else

    // On big-endian, no need to swap non-scalar values.
    // For non-scalar values, just return the raw data.
    if (!isScalar)
    {
        return provider.SafeReadValue(attrPath, buffer);
    }

    // For scalar values, read into a temp buffer, byte-swap, then copy out.
    uint8_t attrData[sizeof(uint64_t)];
    MutableByteSpan tempVal(attrData);

    ReturnErrorOnFailure(provider.SafeReadValue(attrPath, tempVal));
    VerifyOrReturnError(tempVal.size() == valueSize, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(HostSwapBySize(tempVal.data(), valueSize));
    memcpy(buffer.data(), tempVal.data(), valueSize);
    buffer.reduce_size(valueSize);
#endif
    return CHIP_NO_ERROR;
}
} // namespace
namespace chip::app {

CHIP_ERROR MigrateFromSafeToAttributePersistenceProvider(SafeAttributePersistenceProvider & safeProvider,
                                                         AttributePersistenceProvider & dstProvider,
                                                         const ConcreteClusterPath & cluster,
                                                         Span<const AttrMigrationData> attributes, MutableByteSpan buffer)
{
    bool hadMigrationErrors = false;
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
        // If there was an error reading from SafeAttributePersistence, then we shouldn't try to write that value
        // to AttributePersistence
        ChipError attributeMigrationError =
            MigrateValueFromSafe(attrPath, safeProvider, copyOfBuffer, entry.valueSize, entry.isScalar);
        if (attributeMigrationError != CHIP_NO_ERROR)
        {
            // If the value was not found in SafeAttributePersistence, it means that it was already migrated or that
            // there wasn't a value stored for this attribute in the first place, so we skip it.
            if (attributeMigrationError != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
            {
                hadMigrationErrors = true;
                ChipLogError(DataManagement,
                             "AttributeMigration: Error reading SafeAttribute '" ChipLogFormatMEI
                             "' from cluster '" ChipLogFormatMEI "' (err=%" CHIP_ERROR_FORMAT ")",
                             ChipLogValueMEI(entry.attributeId), ChipLogValueMEI(cluster.mClusterId),
                             attributeMigrationError.Format());
            }
            continue;
        }

        // Delete from the safe provider immediately after a successful read, so we only try to
        // migrate the persisted values once and avoid re-migrating after a reset.
        attributeMigrationError = safeProvider.SafeDeleteValue(attrPath);
        if (attributeMigrationError != CHIP_NO_ERROR)
        {
            hadMigrationErrors = true;
            ChipLogError(DataManagement,
                         "AttributeMigration: Error deleting SafeAttribute '" ChipLogFormatMEI "' from cluster '" ChipLogFormatMEI
                         "' (err=%" CHIP_ERROR_FORMAT ")",
                         ChipLogValueMEI(entry.attributeId), ChipLogValueMEI(cluster.mClusterId), attributeMigrationError.Format());
        }

        // Write value from SafeAttributePersistence into AttributePersistence
        attributeMigrationError = dstProvider.WriteValue(attrPath, copyOfBuffer);
        if (attributeMigrationError != CHIP_NO_ERROR)
        {
            hadMigrationErrors = true;
            ChipLogError(DataManagement,
                         "AttributeMigration: Error writing Attribute '" ChipLogFormatMEI "' from cluster '" ChipLogFormatMEI
                         "' (err=%" CHIP_ERROR_FORMAT ")",
                         ChipLogValueMEI(entry.attributeId), ChipLogValueMEI(cluster.mClusterId), attributeMigrationError.Format());
        }
    }
    return hadMigrationErrors ? CHIP_ERROR_HAD_FAILURES : CHIP_NO_ERROR;
}
} // namespace chip::app
