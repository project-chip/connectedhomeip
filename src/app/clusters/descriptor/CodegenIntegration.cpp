/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/descriptor/descriptor-cluster.h>
#include <app/static-cluster-config/Descriptor.h>
#include <app/util/config.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Descriptor;

namespace {

static constexpr size_t kDescriptorFixedClusterCount =
    Descriptor::StaticApplicationConfig::kFixedClusterConfig.size();
static constexpr size_t kDescriptorMaxClusterCount =
    kDescriptorFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<DescriptorCluster> gServer;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate 
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned emberEndpointIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gServer.Create(endpointId, BitFlags<Descriptor::Feature>(featureMap));
        return gServer.Registration();
    }

    ServerClusterInterface & FindRegistration(unsigned emberEndpointIndex) override { return gServer.Cluster(); }
    void ReleaseRegistration(unsigned emberEndpointIndex) override { gServer.Destroy(); }
};
} // namespace

void emberAfDescriptorClusterServerInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = Descriptor::Id,
            .fixedClusterServerEndpointCount = kDescriptorFixedClusterCount,
            .maxEndpointCount                = kDescriptorMaxClusterCount,
            .fetchFeatureMap                 = true,
            .fetchOptionalAttributes         = false,
        },
        integrationDelegate);
}

void MatterDescriptorClusterServerShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = Descriptor::Id,
            .fixedClusterServerEndpointCount = kDescriptorFixedClusterCount,
            .maxEndpointCount                = kDescriptorMaxClusterCount,
        },
        integrationDelegate);
}

void MatterDescriptorPluginServerInitCallback() {}

void MatterDescriptorPluginServerShutdownCallback() {}