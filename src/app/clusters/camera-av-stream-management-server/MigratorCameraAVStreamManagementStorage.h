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
namespace CameraAvStreamManagement {

/**
 * Migrates persisted attribute data for the CameraAVStreamManagement cluster from
 * SafeAttributePersistenceProvider storage to the standard AttributePersistenceProvider.
 *
 * Only scalar attributes and Viewport/StreamUsagePriorities are migrated. Allocated stream
 * lists are excluded because they are large and ephemeral.
 */
CHIP_ERROR MigratorCameraAVStreamManagementStorage(EndpointId endpointId, SafeAttributePersistenceProvider & safeProvider,
                                                   AttributePersistenceProvider & dstProvider);

} // namespace CameraAvStreamManagement
} // namespace Clusters
} // namespace app
} // namespace chip
