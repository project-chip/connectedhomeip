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

#include <app/persistence/AttributePersistenceMigration.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {

namespace Clusters {
namespace ResourceMonitoring {

/**
 * Migrates ResourceMonitoring cluster attributes from SafeAttributePersistenceProvider to AttributePersistenceProvider.
 *
 * @param endpointId The endpoint on which the cluster exists.
 * @param clusterId The ID of the ResourceMonitoring cluster (HepaFilterMonitoring or ActivatedCarbonFilterMonitoring).
 * @param safeProvider The source SafeAttributePersistenceProvider.
 * @param dstProvider The destination AttributePersistenceProvider.
 * @return CHIP_ERROR indicating success or failure of the migration.
 */
CHIP_ERROR MigrateResourceMonitoringServerStorage(EndpointId endpointId, ClusterId clusterId,
                                                  SafeAttributePersistenceProvider & safeProvider,
                                                  AttributePersistenceProvider & dstProvider);

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
