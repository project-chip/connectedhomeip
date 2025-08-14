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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-cluster.h>
#include <app/static-cluster-config/PushAvStreamTransport.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

#include <cstdint>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PushAvStreamTransport::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {

static constexpr size_t kPushAvStreamTransportFixedClusterCount =
    PushAvStreamTransport::StaticApplicationConfig::kFixedClusterConfig.size();
static constexpr size_t kPushAvStreamTransportMaxClusterCount =
    kPushAvStreamTransportFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<PushAvStreamTransportServer> gServers[kPushAvStreamTransportMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned zeroBasedArrayIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        ChipLogProgress(AppServer, "Registering Push AV Stream Transport on endpoint %u, %d", endpointId, zeroBasedArrayIndex);
        gServers[zeroBasedArrayIndex].Create(endpointId, BitFlags<PushAvStreamTransport::Feature>(featureMap));
        return gServers[zeroBasedArrayIndex].Registration();
    }

    ServerClusterInterface & FindRegistration(unsigned zeroBasedArrayIndex) override
    {
        return gServers[zeroBasedArrayIndex].Cluster();
    }
    void DestroyRegistration(unsigned zeroBasedArrayIndex) override { gServers[zeroBasedArrayIndex].Destroy(); }
};

} // namespace
void emberAfPushAvStreamTransportClusterServerInitCallback(EndpointId endpointId)
{

    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = PushAvStreamTransport::Id,
            .fixedClusterServerEndpointCount = kPushAvStreamTransportFixedClusterCount,
            .maxEndpointCount                = kPushAvStreamTransportMaxClusterCount,
            .fetchFeatureMap                 = true,
            .fetchOptionalAttributes         = false,
        },
        integrationDelegate);
}

void MatterPushAvStreamTransportClusterServerShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = PushAvStreamTransport::Id,
            .fixedClusterServerEndpointCount = kPushAvStreamTransportFixedClusterCount,
            .maxEndpointCount                = kPushAvStreamTransportMaxClusterCount,
        },
        integrationDelegate);
}

void MatterPushAvStreamTransportPluginServerInitCallback() {}

void MatterPushAvStreamTransportPluginServerShutdownCallback() {}

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

void SetDelegate(EndpointId endpointId, PushAvStreamTransportDelegate * delegate)
{
    ChipLogProgress(AppServer, "Setting Push AV Stream Transport delegate on endpoint %u", endpointId);
    uint16_t arrayIndex = 0;
    if (!FindEndpointWithLog(endpointId, arrayIndex))
    {
        return;
    }
    gServers[arrayIndex].Cluster().SetDelegate(endpointId, delegate);
    gServers[arrayIndex].Cluster().Init();
}

} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
