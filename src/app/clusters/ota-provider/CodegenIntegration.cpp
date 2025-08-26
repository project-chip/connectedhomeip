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
#include <data-model-providers/codegen/ClusterIntegration.h>

#include <cstdint>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

static constexpr size_t kOtaProviderFixedClusterCount =
    OtaSoftwareUpdateProvider::StaticApplicationConfig::kFixedClusterConfig.size();
static constexpr size_t kOtaProviderMaxClusterCount = kOtaProviderFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<OtaProviderServer> gServers[kOtaProviderMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned emberEndpointIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gServers[emberEndpointIndex].Create(endpointId);
        return gServers[emberEndpointIndex].Registration();
    }

    ServerClusterInterface & FindRegistration(unsigned emberEndpointIndex) override
    {
        return gServers[emberEndpointIndex].Cluster();
    }
    void ReleaseRegistration(unsigned emberEndpointIndex) override { gServers[emberEndpointIndex].Destroy(); }
};

} // namespace

void emberAfOtaSoftwareUpdateProviderClusterServerInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = OtaSoftwareUpdateProvider::Id,
            .fixedClusterServerEndpointCount = kOtaProviderFixedClusterCount,
            .maxEndpointCount                = kOtaProviderMaxClusterCount,
            .fetchFeatureMap                 = false,
            .fetchOptionalAttributes         = false,
        },
        integrationDelegate);
}

void MatterOtaSoftwareUpdateProviderClusterServerShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = OtaSoftwareUpdateProvider::Id,
            .fixedClusterServerEndpointCount = kOtaProviderFixedClusterCount,
            .maxEndpointCount                = kOtaProviderMaxClusterCount,
        },
        integrationDelegate);
}

void MatterOtaSoftwareUpdateProviderPluginServerInitCallback() {}

void MatterOtaSoftwareUpdateProviderPluginServerShutdownCallback() {}

namespace chip {
namespace app {
namespace Clusters {
namespace OTAProvider {

void SetDelegate(EndpointId endpointId, OTAProviderDelegate * delegate)
{
    uint16_t arrayIndex = emberAfGetClusterServerEndpointIndex(endpointId, OtaSoftwareUpdateProvider::Id,
                                                               static_cast<uint16_t>(kOtaProviderFixedClusterCount));
    VerifyOrReturn(arrayIndex < kOtaProviderMaxClusterCount);
    gServers[arrayIndex].Cluster().SetDelegate(delegate);
}

} // namespace OTAProvider
} // namespace Clusters
} // namespace app
} // namespace chip
