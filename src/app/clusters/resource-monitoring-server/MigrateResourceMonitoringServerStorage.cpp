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

#include <app/clusters/resource-monitoring-server/MigrateResourceMonitoringServerStorage.h>
#include <clusters/ActivatedCarbonFilterMonitoring/Attributes.h>
#include <clusters/HepaFilterMonitoring/Attributes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

CHIP_ERROR MigrateResourceMonitoringServerStorage(EndpointId endpointId, ClusterId clusterId,
                                                  SafeAttributePersistenceProvider & safeProvider,
                                                  AttributePersistenceProvider & dstProvider)
{
    // LastChangedTime attribute ID is the same for both HepaFilterMonitoring and ActivatedCarbonFilterMonitoring
    static_assert(HepaFilterMonitoring::Attributes::LastChangedTime::Id ==
                  ActivatedCarbonFilterMonitoring::Attributes::LastChangedTime::Id);

    static constexpr AttrMigrationData attributesToUpdate[] = {
        { HepaFilterMonitoring::Attributes::LastChangedTime::Id, sizeof(uint32_t), true /* isScalar */ },
    };

    // We need to provide a buffer with enough space for the attributes that will be migrated.
    static constexpr size_t kBufferSize = MaxAttrMigrationValueSize(attributesToUpdate);
    static_assert(kBufferSize > 0, "All migration attributes have zero valueSize");
    uint8_t attributeBuffer[kBufferSize] = {};
    MutableByteSpan buffer(attributeBuffer);

    return MigrateFromSafeToAttributePersistenceProvider(safeProvider, dstProvider, { endpointId, clusterId },
                                                         Span(attributesToUpdate), buffer);
}

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
