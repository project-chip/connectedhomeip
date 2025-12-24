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
#include <app/clusters/push-av-stream-transport-server/PushAVStreamTransportCluster.h>
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
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        ChipLogProgress(AppServer, "Registering Push AV Stream Transport on endpoint %u, %d", endpointId, clusterInstanceIndex);
        gServers[clusterInstanceIndex].Create(endpointId, BitFlags<PushAvStreamTransport::Feature>(featureMap));
        return gServers[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServers[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gServers[clusterInstanceIndex].Cluster();
    }
    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServers[clusterInstanceIndex].Destroy(); }
};

PushAvStreamTransportServer * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;
    return static_cast<PushAvStreamTransportServer *>(CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = PushAvStreamTransport::Id,
            .fixedClusterInstanceCount = kPushAvStreamTransportFixedClusterCount,
            .maxClusterInstanceCount   = kPushAvStreamTransportMaxClusterCount,
        },
        integrationDelegate));
}

} // namespace
void MatterPushAvStreamTransportClusterInitCallback(EndpointId endpointId)
{

    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = PushAvStreamTransport::Id,
            .fixedClusterInstanceCount = kPushAvStreamTransportFixedClusterCount,
            .maxClusterInstanceCount   = kPushAvStreamTransportMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterPushAvStreamTransportClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = PushAvStreamTransport::Id,
            .fixedClusterInstanceCount = kPushAvStreamTransportFixedClusterCount,
            .maxClusterInstanceCount   = kPushAvStreamTransportMaxClusterCount,
        },
        integrationDelegate, shutdownType);
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

    if (PushAvStreamTransportServer * cluster = FindClusterOnEndpoint(endpointId); cluster != nullptr)
    {
        cluster->SetDelegate(delegate);
        TEMPORARY_RETURN_IGNORED cluster->Init();
    }
}

void SetTLSClientManagementDelegate(EndpointId endpointId, TlsClientManagementDelegate * delegate)
{
    ChipLogProgress(AppServer, "Setting TLS Client Management delegate on endpoint %u", endpointId);
    if (PushAvStreamTransportServer * cluster = FindClusterOnEndpoint(endpointId); cluster != nullptr)
    {
        cluster->SetTLSClientManagementDelegate(delegate);
    }
}

void SetTlsCertificateManagementDelegate(EndpointId endpointId, TlsCertificateManagementDelegate * delegate)
{
    ChipLogProgress(AppServer, "Setting TLS Certificate Management delegate on endpoint %u", endpointId);
    uint16_t arrayIndex =
        emberAfGetClusterServerEndpointIndex(endpointId, PushAvStreamTransport::Id, kPushAvStreamTransportFixedClusterCount);
    if (arrayIndex >= kPushAvStreamTransportMaxClusterCount)
    {
        return;
    }

    if (!gServers[arrayIndex].IsConstructed())
    {
        ChipLogError(AppServer, "Push AV Stream transport is NOT yet constructed. Cannot set TLS Certificate Management delegate");
        return;
    }
    gServers[arrayIndex].Cluster().SetTlsCertificateManagementDelegate(delegate);
}
} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
