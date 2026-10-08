/*
 *
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

#include <app/SafeAttributePersistenceProvider.h>
#include <app/clusters/resource-monitoring-server/CodegenResourceMonitoringCluster.h>
#include <app/clusters/resource-monitoring-server/MigrateResourceMonitoringServerStorage.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

CHIP_ERROR CodegenResourceMonitoringCluster::Startup(ServerClusterContext & context)
{
    // Migrate attributes for this cluster from SafeAttribute to AttributePersistence.
    // This is done at Startup time when the persistence providers are guaranteed to be available.
    SafeAttributePersistenceProvider * srcProvider = GetSafeAttributePersistenceProvider();
    AttributePersistenceProvider & dstProvider     = context.attributeStorage;

    if (srcProvider != nullptr)
    {
        LogErrorOnFailure(ResourceMonitoring::MigrateResourceMonitoringServerStorage(mPath.mEndpointId, mPath.mClusterId,
                                                                                     *srcProvider, dstProvider));
    }

    return ResourceMonitoringCluster::Startup(context);
}

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
