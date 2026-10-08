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

#include <app/clusters/chime-server/MigrateChimeServerStorage.h>
#include <clusters/Chime/Attributes.h>
#include <clusters/Chime/ClusterId.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Chime {

CHIP_ERROR MigrateChimeServerStorage(EndpointId endpointId, SafeAttributePersistenceProvider & safeProvider,
                                     AttributePersistenceProvider & dstProvider)
{
    static constexpr AttrMigrationData attributesToUpdate[] = {
        { Attributes::SelectedChime::Id, sizeof(uint8_t), true /* isScalar */ },
        { Attributes::Enabled::Id, sizeof(bool), true /* isScalar */ },
    };
    // We need to provide a buffer with enough space for the attributes that will be migrated.
    static constexpr size_t kBufferSize = MaxAttrMigrationValueSize(attributesToUpdate);
    static_assert(kBufferSize > 0, "All migration attributes have zero valueSize");
    uint8_t attributeBuffer[kBufferSize] = {};
    MutableByteSpan buffer(attributeBuffer);
    return MigrateFromSafeToAttributePersistenceProvider(safeProvider, dstProvider, { endpointId, Chime::Id },
                                                         Span(attributesToUpdate), buffer);
}

} // namespace Chime
} // namespace Clusters
} // namespace app
} // namespace chip
