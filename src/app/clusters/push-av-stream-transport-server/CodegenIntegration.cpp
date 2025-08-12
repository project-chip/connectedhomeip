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
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

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

// Find the 0-based array index corresponding to the given endpoint id.
// Log an error if not found.
bool FindEndpointWithLog(EndpointId endpointId, uint16_t & outArrayIndex)
{
    uint16_t arrayIndex =
        emberAfGetClusterServerEndpointIndex(endpointId, PushAvStreamTransport::Id, kPushAvStreamTransportFixedClusterCount);

    if (arrayIndex >= kPushAvStreamTransportMaxClusterCount)
    {
        ChipLogError(AppServer, "Cound not find endpoint index for endpoint %u", endpointId);
        return false;
    }
    return true;
}

} // namespace
void emberAfPushAvStreamTransportClusterServerInitCallback(EndpointId endpointId)
{

    uint16_t arrayIndex = 0;
    if (!FindEndpointWithLog(endpointId, arrayIndex))
    {
        return;
    }
    uint32_t rawFeatureMap;
    if (FeatureMap::Get(endpointId, &rawFeatureMap) != Status::Success)
    {
        ChipLogError(AppServer, "Failed to get feature map for endpoint %u", endpointId);
        rawFeatureMap = 0;
    }
    ChipLogProgress(AppServer, "Registering Push AV Stream Transport on endpoint %u, %d", endpointId, arrayIndex);
    gServers[arrayIndex].Create(endpointId, BitFlags<PushAvStreamTransport::Feature>(rawFeatureMap));
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServers[arrayIndex].Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register Push AV Stream Transport on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
}

void MatterPushAvStreamTransportClusterServerShutdownCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    if (!FindEndpointWithLog(endpointId, arrayIndex))
    {
        return;
    }

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServers[arrayIndex].Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister Push AV Stream Transport on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
    gServers[arrayIndex].Cluster().Deinit();
    gServers[arrayIndex].Destroy();
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
