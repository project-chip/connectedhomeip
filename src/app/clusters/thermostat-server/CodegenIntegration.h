/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/util/af-types.h>

#include <app-common/zap-generated/callback.h>
#include <lib/core/CHIPEncoding.h>

#include <app/server/Server.h>
#include <app/static-cluster-config/Thermostat.h>

#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/CodegenProcessingConfig.h>

#include "ThermostatCluster.h"
#include "ThermostatClusterWithFeatures.h"

namespace chip::app::Clusters::Thermostat {

constexpr size_t kThermostatFixedClusterCount = Thermostat::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kThermostatEndpointCount     = kThermostatFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

class BaseIntegrationDelegate : public CodegenClusterIntegration::Delegate
{
protected:
    ThermostatCluster::OptionalAttributes GetOptionalAttributes(EndpointId endpointId, BitFlags<Thermostat::Feature> features);
    ThermostatCluster::DefaultValues LoadDefaultValues(EndpointId endpointId, const BitFlags<Thermostat::Feature> & features);
};

template <std::size_t Size, typename Cluster>
class IntegrationDelegate : public BaseIntegrationDelegate {
public:
    // Declared and defined in one single place
    inline static std::array<LazyRegisteredServerCluster<Cluster>, Size> mClusters = {};

    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        const BitFlags<Thermostat::Feature> features(featureMap);
        const ThermostatCluster::OptionalAttributes optionalAttributes = GetOptionalAttributes(endpointId, features);
        const ThermostatCluster::DefaultValues defaultValues           = LoadDefaultValues(endpointId, features);

        ChipLogProgress(Zcl, "Creating thermostat cluster for endpoint %d", endpointId);
        mClusters[clusterInstanceIndex].Create(endpointId, features, optionalAttributes, defaultValues,
                                               Server::GetInstance().GetFabricTable());
        return mClusters[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(mClusters[clusterInstanceIndex].IsConstructed(), nullptr);
        return &mClusters[clusterInstanceIndex].Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override { mClusters[clusterInstanceIndex].Destroy(); }

    Cluster * FindClusterOnEndpoint(EndpointId endpointId)
    {
        for (auto & cluster : mClusters)
        {
            if (cluster.IsConstructed() && cluster.Cluster().GetPaths()[0].mEndpointId == endpointId)
            {
                return &cluster.Cluster();
            }
        }
        return nullptr;
    }

    Protocols::InteractionModel::Status SetDelegate(EndpointId endpoint, Thermostat::Delegate * delegate)
    {
        Cluster * cluster = FindClusterOnEndpoint(endpoint);
        if (cluster == nullptr)
        {
            ChipLogError(Zcl, "No thermostat cluster found for endpoint %d", endpoint);
            return Protocols::InteractionModel::Status::Failure;
        }
        cluster->SetDelegate(delegate);
        return Protocols::InteractionModel::Status::Success;
    }

   /* void ServerInit(EndpointId endpointId)
    {
        for (auto & cluster : mClusters)
        {
            if (cluster.IsConstructed() && cluster.Cluster().GetPaths()[0].mEndpointId == endpointId)
            {
                cluster.Cluster().ServerInit();
            }
        }
    }

    void ServerShutdown(EndpointId endpointId, MatterClusterShutdownType clusterShutdownType)
    {
        for (auto & cluster : mClusters)
        {
            if (cluster.IsConstructed() && cluster.Cluster().GetPaths()[0].mEndpointId == endpointId)
            {
                cluster.Cluster().ServerShutdown(clusterShutdownType);
            }
        }
    }*/
};

template <typename Cluster>
void ServerInit(EndpointId endpointId)
{
    IntegrationDelegate<kThermostatEndpointCount, Cluster> integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Thermostat::Id,
            .fixedClusterInstanceCount = kThermostatFixedClusterCount,
            .maxClusterInstanceCount   = kThermostatEndpointCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

template <typename Cluster>
void ServerShutdown(EndpointId endpointId, MatterClusterShutdownType clusterShutdownType)
{
    IntegrationDelegate<kThermostatEndpointCount, Cluster> integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Thermostat::Id,
            .fixedClusterInstanceCount = kThermostatFixedClusterCount,
            .maxClusterInstanceCount   = kThermostatEndpointCount,
        },
        integrationDelegate, clusterShutdownType);
}

template <typename Cluster>
ThermostatCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate<kThermostatEndpointCount, Cluster> integrationDelegate;
    return integrationDelegate.FindClusterOnEndpoint(endpointId);
}

inline ThermostatCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    return FindClusterOnEndpoint<DefaultThermostatCluster>(endpointId);
}

template <typename Cluster>
Protocols::InteractionModel::Status SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    IntegrationDelegate<kThermostatEndpointCount, Cluster> integrationDelegate;
    return integrationDelegate.SetDelegate(endpoint, delegate);
}

inline Protocols::InteractionModel::Status SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    return SetDefaultDelegate<DefaultThermostatCluster>(endpoint, delegate);
}

} // namespace chip::app::Clusters::Thermostat
