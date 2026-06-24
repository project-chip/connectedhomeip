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

// We will use this to be able to set some values got from `Accessors::GetDefault` functions without failing, since the cluster will
// check the values to be valid using the delegate. After this the delegate will be set to the default delegate, which will make all subsequent `.Set*()` calls and write requests on the cluster to fail.
// Which will enforces the user to set a custom delegate before using the cluster.
struct AlwaysSuccessDelegate : public LaundryWasherControls::Delegate
{
    CHIP_ERROR GetSpinSpeedAtIndex(size_t, MutableCharSpan & spinSpeed) override
    {
        spinSpeed.reduce_size(0);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetSupportedRinseAtIndex(size_t, NumberOfRinsesEnum & numberOfRinses) override
    {
        numberOfRinses = NumberOfRinsesEnum::kNone;
        return CHIP_NO_ERROR;
    }
};
AlwaysSuccessDelegate gAlwaysSuccessDelegate;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        BitFlags<Feature> features(featureMap);
        LaundryWasherControlsCluster::Config config(features, gAlwaysSuccessDelegate);

        auto & server = gServers[clusterInstanceIndex];
        server.Create(endpointId, config);

        // Set values from ember storage.
        if (features.Has(Feature::kSpin))
        {
            DataModel::Nullable<uint8_t> spinSpeedCurrent;
            if (SpinSpeedCurrent::GetDefault(endpointId, spinSpeedCurrent) == Status::Success)
            {
                LogErrorOnFailure(server.Cluster().SetSpinSpeedCurrent(spinSpeedCurrent));
            }
        }

        if (features.Has(Feature::kRinse))
        {
            NumberOfRinsesEnum numberOfRinses;
            if (NumberOfRinses::GetDefault(endpointId, &numberOfRinses) == Status::Success)
            {
                LogErrorOnFailure(server.Cluster().SetNumberOfRinses(numberOfRinses));
            }
        }

        // Set the delegate to a default delegate, which will make all `.Set*()` calls and write requests on the cluster to fail.
        // This enforces the user to set a custom delegate before using the cluster.
        server.Cluster().SetDelegate(LaundryWasherControls::Delegate::DefaultInstance());

        return server.Registration();
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

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    if (cluster != nullptr)
    {
        cluster->SetDelegate(*delegate);
    }
}

CHIP_ERROR SetSpinSpeedCurrent(EndpointId endpointId, DataModel::Nullable<uint8_t> spinSpeedCurrent)
{
    auto cluster = FindClusterOnEndpoint(endpointId);
    return cluster != nullptr ? cluster->SetSpinSpeedCurrent(spinSpeedCurrent) : CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR GetSpinSpeedCurrent(EndpointId endpointId, DataModel::Nullable<uint8_t> & spinSpeedCurrent)
{
    auto cluster = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    spinSpeedCurrent = cluster->GetSpinSpeedCurrent();
    return CHIP_NO_ERROR;
}

CHIP_ERROR SetNumberOfRinses(EndpointId endpointId, NumberOfRinsesEnum newNumberOfRinses)
{
    auto cluster = FindClusterOnEndpoint(endpointId);
    return cluster != nullptr ? cluster->SetNumberOfRinses(newNumberOfRinses) : CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR GetNumberOfRinses(EndpointId endpointId, NumberOfRinsesEnum & numberOfRinses)
{
    auto cluster = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
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
