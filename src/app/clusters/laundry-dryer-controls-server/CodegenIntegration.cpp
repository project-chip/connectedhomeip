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
#include <app/util/attribute-storage.h>
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

static_assert(kLaundryDryerControlsFixedClusterCount == MATTER_DM_LAUNDRY_DRYER_CONTROLS_CLUSTER_SERVER_ENDPOINT_COUNT,
              "LaundryDryerControls static cluster config must match ZAP server endpoint count");
static_assert(kLaundryDryerControlsMaxClusterCount <= kEmberInvalidEndpointIndex, "LaundryDryerControls cluster table size error");

// Proxy delegate used only by the codegen integration layer.
//
// LaundryDryerControlsCluster requires a Delegate & at construction, while Ember/ZAP applications
// register their real delegate separately via SetDefaultDelegate - possibly before the cluster is
// created, possibly after, possibly never.
//
// With no application delegate, it reports an empty SupportedDrynessLevels list, which in turn makes
// every non-null SelectedDrynessLevel write fail validation with ConstraintError.
class IntegrationDelegateWrapper final : public LaundryDryerControls::Delegate
{
public:
    void SetWrapped(LaundryDryerControls::Delegate * wrapped) { mWrapped = wrapped; }

    CHIP_ERROR GetSupportedDrynessLevelAtIndex(size_t index, DrynessLevelEnum & supportedDryness) override
    {
        VerifyOrReturnError(mWrapped != nullptr, CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
        return mWrapped->GetSupportedDrynessLevelAtIndex(index, supportedDryness);
    }

private:
    LaundryDryerControls::Delegate * mWrapped = nullptr;
};

struct ClusterWithDelegate
{
    IntegrationDelegateWrapper integrationDelegateWrapper;
    LazyRegisteredServerCluster<LaundryDryerControlsCluster> server;
};

ClusterWithDelegate gClusters[kLaundryDryerControlsMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gClusters[clusterInstanceIndex].server.Create(endpointId, gClusters[clusterInstanceIndex].integrationDelegateWrapper);
        return gClusters[clusterInstanceIndex].server.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gClusters[clusterInstanceIndex].server.IsConstructed(), nullptr);
        return &gClusters[clusterInstanceIndex].server.Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gClusters[clusterInstanceIndex].server.Destroy(); }
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

// Resolves the endpoint to its cluster slot via ember metadata rather than via FindClusterOnEndpoint,
// so the delegate can be registered whether or not the cluster has been created yet.
void SetDelegate(EndpointId endpoint, Delegate & delegate)
{
    uint16_t index = emberAfGetClusterServerEndpointIndex(endpoint, LaundryDryerControls::Id,
                                                          MATTER_DM_LAUNDRY_DRYER_CONTROLS_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (index >= kLaundryDryerControlsMaxClusterCount)
    {
        ChipLogError(Zcl, "LaundryDryerControls cluster on endpoint %u not found", static_cast<unsigned>(endpoint));
        return;
    }

    gClusters[index].integrationDelegateWrapper.SetWrapped(&delegate);
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
