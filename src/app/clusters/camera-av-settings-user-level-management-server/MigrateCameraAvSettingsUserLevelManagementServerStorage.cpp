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

#include <app/clusters/camera-av-settings-user-level-management-server/CameraAvSettingsUserLevelManagementCluster.h>
#include <app/clusters/camera-av-settings-user-level-management-server/CameraAvSettingsUserLevelManagementConstants.h>
#include <app/clusters/camera-av-settings-user-level-management-server/MigrateCameraAvSettingsUserLevelManagementServerStorage.h>
#include <app/persistence/AttributePersistenceMigration.h>
#include <clusters/CameraAvSettingsUserLevelManagement/Attributes.h>
#include <clusters/CameraAvSettingsUserLevelManagement/Ids.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement;
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement::Attributes;

namespace {

static constexpr AttrMigrationData kAttributesToMigrate[] = {
    { MPTZPosition::Id, Clusters::kMptzPositionStructMaxSerializedSize, false /* isScalar */ },
    { MPTZPresets::Id, kMaxMPTZPresetsSerializedSize, false /* isScalar */ },
    { DPTZStreams::Id, kMaxDPTZStreamsSerializedSize, false /* isScalar */ },
};

static constexpr size_t kBufferSize = MaxAttrMigrationValueSize(kAttributesToMigrate);
static_assert(kBufferSize > 0, "All migration attributes have zero valueSize");

// File-scope buffer avoids a large stack allocation for MPTZPresets migration.
static uint8_t sAttributeBuffer[kBufferSize];

} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvSettingsUserLevelManagement {

CHIP_ERROR MigrateCameraAvSettingsUserLevelManagementServerStorage(EndpointId endpointId,
                                                                   SafeAttributePersistenceProvider & safeProvider,
                                                                   AttributePersistenceProvider & dstProvider)
{
    MutableByteSpan buffer(sAttributeBuffer);
    return MigrateFromSafeToAttributePersistenceProvider(safeProvider, dstProvider,
                                                         ConcreteClusterPath(endpointId, CameraAvSettingsUserLevelManagement::Id),
                                                         Span<const AttrMigrationData>(kAttributesToMigrate), buffer);
}

} // namespace CameraAvSettingsUserLevelManagement
} // namespace Clusters
} // namespace app
} // namespace chip
