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

#include <app/clusters/push-av-stream-transport-server/MigratePushAvStreamTransportServerStorage.h>

#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-storage.h>
#include <clusters/PushAvStreamTransport/AttributeIds.h>
#include <clusters/PushAvStreamTransport/ClusterId.h>
#include <lib/support/ScopedMemoryBuffer.h>

namespace chip::app::Clusters::PushAvStreamTransport {

CHIP_ERROR MigratePushAvStreamTransportServerStorage(EndpointId endpointId, SafeAttributePersistenceProvider & safeProvider,
                                                     AttributePersistenceProvider & dstProvider)
{
    static constexpr AttrMigrationData attributesToUpdate[] = {
        { Attributes::CurrentConnections::Id, kMaxCurrentConnectionsSerializedSize, false /* isScalar */ },
    };
    static constexpr size_t kBufferSize = MaxAttrMigrationValueSize(attributesToUpdate);

    // kBufferSize scales with CHIP_CONFIG_MAX_FABRICS * CHIP_CONFIG_MAX_NUM_PUSH_TRANSPORTS, too large for the stack.
    Platform::ScopedMemoryBuffer<uint8_t> attributeBuffer;
    VerifyOrReturnError(attributeBuffer.Alloc(kBufferSize), CHIP_ERROR_NO_MEMORY);

    return MigrateFromSafeToAttributePersistenceProvider(safeProvider, dstProvider, { endpointId, PushAvStreamTransport::Id },
                                                         Span(attributesToUpdate),
                                                         MutableByteSpan(attributeBuffer.Get(), kBufferSize));
}

} // namespace chip::app::Clusters::PushAvStreamTransport
