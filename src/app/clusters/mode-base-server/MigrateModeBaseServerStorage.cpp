/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <app/clusters/mode-base-server/MigrateModeBaseServerStorage.h>
#include <app/data-model/Nullable.h>

namespace chip::app::Clusters::ModeBase {

namespace {

CHIP_ERROR
MigrateNullableUint8Attribute(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                              SafeAttributePersistenceProvider & safeProvider, AttributePersistenceProvider & dstProvider)
{
    // Read the value from the destination provider to check if it's already migrated
    uint8_t readBuf[sizeof(NumericAttributeTraits<uint8_t>::StorageType)];
    MutableByteSpan readSpan(readBuf);
    CHIP_ERROR err;
    VerifyOrReturnValue((err = dstProvider.ReadValue({ endpointId, clusterId, attributeId }, readSpan)) ==
                            CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND,
                        err);

    // Read the value from the safe provider if it's not already migrated
    DataModel::Nullable<uint8_t> value;
    VerifyOrReturnValue((err = safeProvider.ReadScalarValue({ endpointId, clusterId, attributeId }, value)) == CHIP_NO_ERROR,
                        (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND ? CHIP_NO_ERROR : err));

    NumericAttributeTraits<uint8_t>::StorageType storageValue;
    DataModel::NullableToStorage(value, storageValue);
    ReturnErrorOnFailure(dstProvider.WriteValue({ endpointId, clusterId, attributeId },
                                                ByteSpan(reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue))));
    ReturnErrorOnFailure(safeProvider.SafeDeleteValue({ endpointId, clusterId, attributeId }));

    return CHIP_NO_ERROR;
}

} // namespace

CHIP_ERROR MigrateModeBaseServerStorage(EndpointId endpointId, ClusterId clusterId, SafeAttributePersistenceProvider & safeProvider,
                                        AttributePersistenceProvider & dstProvider)
{
    static constexpr AttrMigrationData attributesToUpdate[] = {
        { Attributes::CurrentMode::Id, sizeof(uint8_t), true /* isScalar */ },
    };
    // We need to provide a buffer with enough space for the attributes that will be migrated.
    static constexpr size_t kBufferSize = MaxAttrMigrationValueSize(attributesToUpdate);
    static_assert(kBufferSize > 0, "All migration attributes have zero valueSize");
    uint8_t attributeBuffer[kBufferSize] = {};
    MutableByteSpan buffer(attributeBuffer);
    ReturnErrorOnFailure(MigrateFromSafeToAttributePersistenceProvider(safeProvider, dstProvider, { endpointId, clusterId },
                                                                       Span(attributesToUpdate), buffer));
    ReturnErrorOnFailure(
        MigrateNullableUint8Attribute(endpointId, clusterId, Attributes::StartUpMode::Id, safeProvider, dstProvider));
    ReturnErrorOnFailure(MigrateNullableUint8Attribute(endpointId, clusterId, Attributes::OnMode::Id, safeProvider, dstProvider));
    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters::ModeBase
