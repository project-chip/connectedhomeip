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

#include <app/clusters/electrical-distribution-server/CodegenIntegration.h>

#include <app/util/generic-callbacks.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/CodegenProcessingConfig.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalDistribution {

CHIP_ERROR Instance::Init()
{
    ReturnErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration()));
    mRegistered = true;
    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    VerifyOrReturn(mRegistered);
    mRegistered = false;

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster());
    if (err != CHIP_NO_ERROR)
    {
#if CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        ChipLogError(AppServer, "Failed to unregister Electrical Distribution cluster: %" CHIP_ERROR_FORMAT, err.Format());
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
    }
}

} // namespace ElectricalDistribution
} // namespace Clusters
} // namespace app
} // namespace chip

// Electrical Distribution is listed in CodeDrivenClusters, so the generated dispatch calls these
// for every endpoint that enables the cluster. They are intentionally empty: the application owns
// the cluster lifetime via ElectricalDistribution::Instance, because the attribute values are
// application data that the generated static configuration cannot supply.
void MatterElectricalDistributionClusterInitCallback(chip::EndpointId) {}

void MatterElectricalDistributionClusterShutdownCallback(chip::EndpointId, MatterClusterShutdownType) {}
