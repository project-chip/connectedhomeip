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
#include <app/clusters/camera-av-settings-user-level-management-server/camera-av-settings-user-level-management-cluster.h>
#include <app/static-cluster-config/CameraAvSettingsUserLevelManagement.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

#include <cstdint>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement;
using chip::Protocols::InteractionModel::Status;

namespace {

static constexpr size_t kCameraAvSettingsUserLevelManagementFixedClusterCount =
    CameraAvSettingsUserLevelManagement::StaticApplicationConfig::kFixedClusterConfig.size();
static constexpr size_t kCameraAvSettingsUserLevelManagementMaxClusterCount =
    kCameraAvSettingsUserLevelManagementFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<CameraAvSettingsUserLevelMgmtServer> gServers[kCameraAvSettingsUserLevelManagementMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        ChipLogProgress(AppServer, "Registering Camera AV Settings User Level Management on endpoint %u, %d", endpointId, clusterInstanceIndex);
        gServers[clusterInstanceIndex].Create(endpointId, BitFlags<CameraAvSettingsUserLevelManagement::Feature>(featureMap));
        return gServers[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServers[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gServers[clusterInstanceIndex].Cluster();
    }
    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServers[clusterInstanceIndex].Destroy(); }
};

CameraAvSettingsUserLevelMgmtServer * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;
    return static_cast<CameraAvSettingsUserLevelMgmtServer *>(CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = CameraAvSettingsUserLevelManagement::Id,
            .fixedClusterInstanceCount = kCameraAvSettingsUserLevelManagementFixedClusterCount,
            .maxClusterInstanceCount   = kCameraAvSettingsUserLevelManagementMaxClusterCount,
        },
        integrationDelegate));
}

} // namespace
void MatterCameraAvSettingsUserLevelManagementClusterInitCallback(EndpointId endpointId)
{

    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = CameraAvSettingsUserLevelManagement::Id,
            .fixedClusterInstanceCount = kCameraAvSettingsUserLevelManagementFixedClusterCount,
            .maxClusterInstanceCount   = kCameraAvSettingsUserLevelManagementMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterCameraAvSettingsUserLevelManagementClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = CameraAvSettingsUserLevelManagement::Id,
            .fixedClusterInstanceCount = kCameraAvSettingsUserLevelManagementFixedClusterCount,
            .maxClusterInstanceCount   = kCameraAvSettingsUserLevelManagementMaxClusterCount,
        },
        integrationDelegate);
}

void MatterCameraAvSettingsUserLevelManagementPluginServerInitCallback() {}

void MatterCameraAvSettingsUserLevelManagementPluginServerShutdownCallback() {}

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvSettingsUserLevelManagement {

void SetDelegate(EndpointId endpointId, CameraAvSettingsUserLevelManagementDelegate * delegate)
{
    ChipLogProgress(AppServer, "Setting Camera AV Settings User Level Management delegate on endpoint %u", endpointId);

    if (CameraAvSettingsUserLevelMgmtServer * cluster = FindClusterOnEndpoint(endpointId); cluster != nullptr)
    {
        cluster->SetDelegate(delegate);
        TEMPORARY_RETURN_IGNORED cluster->Init();
    }
}

} // namespace CameraAvSettingsUserLevelManagement
} // namespace Clusters
} // namespace app
} // namespace chip
