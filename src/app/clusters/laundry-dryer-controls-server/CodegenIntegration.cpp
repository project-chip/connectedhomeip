/**
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

#include "CodegenIntegration.h"

#include <app/data-model/Nullable.h>
#include <app/static-cluster-config/LaundryDryerControls.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LaundryDryerControls;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr size_t kLaundryDryerControlsFixedClusterCount = LaundryDryerControls::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kLaundryDryerControlsMaxClusterCount =
    kLaundryDryerControlsFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<LaundryDryerControlsCluster> gServers[kLaundryDryerControlsMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gServers[clusterInstanceIndex].Create(endpointId);
        return gServers[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServers[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gServers[clusterInstanceIndex].Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServers[clusterInstanceIndex].Destroy(); }
};

} // namespace

void MatterLaundryDryerControlsPluginServerInitCallback() {}

void MatterLaundryDryerControlsClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = LaundryDryerControls::Id,
            .fixedClusterInstanceCount = kLaundryDryerControlsFixedClusterCount,
            .maxClusterInstanceCount   = kLaundryDryerControlsMaxClusterCount,
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterLaundryDryerControlsClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = LaundryDryerControls::Id,
            .fixedClusterInstanceCount = kLaundryDryerControlsFixedClusterCount,
            .maxClusterInstanceCount   = kLaundryDryerControlsMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

namespace chip::app::Clusters::LaundryDryerControls {

namespace LaundryDryerControlsServer {

// Delegate should be valid until the cluster on the endpoint is destroyed. This will probably happen at the end of the program.
void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    VerifyOrDie(delegate != nullptr);
    SetDelegate(endpoint, *delegate);
}

void SetDelegate(EndpointId endpoint, Delegate & delegate)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    if (cluster != nullptr)
    {
        cluster->SetDelegate(delegate);
    }
    else
    {
        ChipLogError(Zcl, "LaundryDryerControls cluster on endpoint %d not found", endpoint);
    }
}

Status SetSelectedDrynessLevel(EndpointId endpointId, DrynessLevelEnum newSelectedDrynessLevel)
{
    auto cluster = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnValue(cluster != nullptr, Status::Failure);
    return cluster->SetSelectedDrynessLevel(DataModel::MakeNullable(newSelectedDrynessLevel));
}

Status GetSelectedDrynessLevel(EndpointId endpointId, DataModel::Nullable<DrynessLevelEnum> & selectedDrynessLevel)
{
    auto cluster = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnValue(cluster != nullptr, Status::Failure);
    selectedDrynessLevel = cluster->GetSelectedDrynessLevel();
    return Status::Success;
}

} // namespace LaundryDryerControlsServer

LaundryDryerControlsCluster * FindClusterOnEndpoint(EndpointId endpoint)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * serverCluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpoint,
            .clusterId                 = LaundryDryerControls::Id,
            .fixedClusterInstanceCount = kLaundryDryerControlsFixedClusterCount,
            .maxClusterInstanceCount   = kLaundryDryerControlsMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<LaundryDryerControlsCluster *>(serverCluster);
}

} // namespace chip::app::Clusters::LaundryDryerControls
