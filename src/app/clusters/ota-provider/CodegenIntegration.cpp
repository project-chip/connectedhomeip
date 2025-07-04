/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <app/clusters/ota-provider/ota-provider-cluster.h>
#include <app/static-cluster-config/OtaSoftwareUpdateProvider.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

#include <cstdint>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

static constexpr size_t kOtaProviderFixedClusterCount =
    OtaSoftwareUpdateProvider::StaticApplicationConfig::kFixedClusterConfig.size();
static constexpr size_t kOtaProviderMaxClusterCount = kOtaProviderFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<OtaProviderServer> gServers[kOtaProviderMaxClusterCount];

// Find the 0-based array index corresponding to the given endpoint id.
// Log an error if not found.
bool findEndpointWithLog(EndpointId endpointId, uint16_t & outArrayIndex)
{
    uint16_t arrayIndex =
        emberAfGetClusterServerEndpointIndex(endpointId, OtaSoftwareUpdateProvider::Id, kOtaProviderFixedClusterCount);

    if (arrayIndex >= kOtaProviderMaxClusterCount)
    {
        ChipLogError(AppServer, "Cound not find endpoint index for endpoint %u", endpointId);
        return false;
    }
    return true;
}

} // namespace

void emberAfOtaSoftwareUpdateProviderClusterInitCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    if (!findEndpointWithLog(endpointId, arrayIndex))
    {
        return;
    }
    gServers[arrayIndex].Create(endpointId);
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServers[arrayIndex].Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register OTA on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
}

void emberAfOtaSoftwareUpdateProviderClusterShutdownCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    if (!findEndpointWithLog(endpointId, arrayIndex))
    {
        return;
    }

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServers[arrayIndex].Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister OTA on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
    gServers[arrayIndex].Destroy();
}

void MatterOtaSoftwareUpdateProviderPluginServerInitCallback() {}

void MatterOtaSoftwareUpdateProviderPluginServerShutdownCallback() {}

namespace chip {
namespace app {
namespace Clusters {
namespace OTAProvider {

void SetDelegate(EndpointId endpointId, OTAProviderDelegate * delegate)
{
    uint16_t arrayIndex = 0;
    if (!findEndpointWithLog(endpointId, arrayIndex))
    {
        return;
    }
    gServers[arrayIndex].Cluster().SetDelegate(delegate);
}

} // namespace OTAProvider
} // namespace Clusters
} // namespace app
} // namespace chip
