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

    DataModel::Nullable<uint8_t> startUpMode;
    CHIP_ERROR err = safeProvider.ReadScalarValue({ endpointId, clusterId, Attributes::StartUpMode::Id }, startUpMode);
    if (err == CHIP_NO_ERROR)
    {
        NumericAttributeTraits<uint8_t>::StorageType storageValue;
        DataModel::NullableToStorage(startUpMode, storageValue);
        ReturnErrorOnFailure(
            dstProvider.WriteValue({ endpointId, clusterId, Attributes::StartUpMode::Id },
                                   ByteSpan(reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue))));
    }
    else if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return err;
    }

    DataModel::Nullable<uint8_t> onMode;
    err = safeProvider.ReadScalarValue({ endpointId, clusterId, Attributes::OnMode::Id }, onMode);
    if (err == CHIP_NO_ERROR)
    {
        NumericAttributeTraits<uint8_t>::StorageType storageValue;
        DataModel::NullableToStorage(onMode, storageValue);
        ReturnErrorOnFailure(
            dstProvider.WriteValue({ endpointId, clusterId, Attributes::OnMode::Id },
                                   ByteSpan(reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue))));
    }
    else if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return err;
    }

    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters::ModeBase
