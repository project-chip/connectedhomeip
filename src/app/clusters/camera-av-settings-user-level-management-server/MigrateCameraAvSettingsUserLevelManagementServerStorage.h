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

#pragma once

#include <app/SafeAttributePersistenceProvider.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvSettingsUserLevelManagement {

/**
 * Migrates persisted attribute data for the CameraAvSettingsUserLevelManagement cluster from
 * SafeAttributePersistenceProvider storage to the standard AttributePersistenceProvider.
 *
 * MPTZPosition, MPTZPresets, and DPTZStreams are migrated as opaque TLV blobs.
 */
CHIP_ERROR MigrateCameraAvSettingsUserLevelManagementServerStorage(EndpointId endpointId,
                                                                   SafeAttributePersistenceProvider & safeProvider,
                                                                   AttributePersistenceProvider & dstProvider);

} // namespace CameraAvSettingsUserLevelManagement
} // namespace Clusters
} // namespace app
} // namespace chip
