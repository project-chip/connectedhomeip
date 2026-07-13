/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/static-cluster-config/LaundryWasherControls.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LaundryWasherControls;
using namespace chip::app::Clusters::LaundryWasherControls::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr size_t kLaundryWasherControlsFixedClusterCount =
    LaundryWasherControls::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kLaundryWasherControlsMaxClusterCount =
    kLaundryWasherControlsFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<LaundryWasherControlsCluster> gServers[kLaundryWasherControlsMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        BitFlags<Feature> features(featureMap);
        LaundryWasherControlsCluster::Config config(features);

        gServers[clusterInstanceIndex].Create(endpointId, config);

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

void MatterLaundryWasherControlsPluginServerInitCallback() {}

void MatterLaundryWasherControlsClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = LaundryWasherControls::Id,
            .fixedClusterInstanceCount = kLaundryWasherControlsFixedClusterCount,
            .maxClusterInstanceCount   = kLaundryWasherControlsMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterLaundryWasherControlsClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = LaundryWasherControls::Id,
            .fixedClusterInstanceCount = kLaundryWasherControlsFixedClusterCount,
            .maxClusterInstanceCount   = kLaundryWasherControlsMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

namespace chip::app::Clusters::LaundryWasherControls {

namespace LaundryWasherControlsServer {

// Delegate should be valid until cluster on the endpoint is destroyed. This will probably happen at the end of the program.
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
        ChipLogError(Zcl, "LaundryWasherControls cluster on endpoint %d not found", endpoint);
    }
}

CHIP_ERROR SetSpinSpeedCurrent(EndpointId endpointId, DataModel::Nullable<uint8_t> spinSpeedCurrent)
{
    auto cluster = FindClusterOnEndpoint(endpointId);
    return cluster != nullptr ? cluster->SetSpinSpeedCurrent(spinSpeedCurrent) : CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR GetSpinSpeedCurrent(EndpointId endpointId, DataModel::Nullable<uint8_t> & spinSpeedCurrent)
{
    auto cluster = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    spinSpeedCurrent = cluster->GetSpinSpeedCurrent();
    return CHIP_NO_ERROR;
}

CHIP_ERROR SetNumberOfRinses(EndpointId endpointId, NumberOfRinsesEnum newNumberOfRinses)
{
    auto cluster = FindClusterOnEndpoint(endpointId);
    return cluster != nullptr ? cluster->SetNumberOfRinses(newNumberOfRinses) : CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR GetNumberOfRinses(EndpointId endpointId, NumberOfRinsesEnum & numberOfRinses)
{
    auto cluster = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    numberOfRinses = cluster->GetNumberOfRinses();
    return CHIP_NO_ERROR;
}

} // namespace LaundryWasherControlsServer

LaundryWasherControlsCluster * FindClusterOnEndpoint(EndpointId endpoint)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * serverCluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpoint,
            .clusterId                 = LaundryWasherControls::Id,
            .fixedClusterInstanceCount = kLaundryWasherControlsFixedClusterCount,
            .maxClusterInstanceCount   = kLaundryWasherControlsMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<LaundryWasherControlsCluster *>(serverCluster);
}

} // namespace chip::app::Clusters::LaundryWasherControls
