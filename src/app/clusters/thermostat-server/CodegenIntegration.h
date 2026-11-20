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
#include <platform/DefaultTimerDelegate.h>

#include <app/server/Server.h>
#include <app/static-cluster-config/Thermostat.h>

#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/CodegenProcessingConfig.h>

#include "ThermostatCluster.h"
#include "ThermostatClusterBase.h"

namespace chip::app::Clusters::Thermostat {

constexpr size_t kThermostatFixedClusterCount = Thermostat::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kThermostatEndpointCount     = kThermostatFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

DefaultTimerDelegate & GetDefaultTimerDelegate();

/**
 * Base delegate for the Thermostat cluster.
 *
 * It provides the default implementation of GetOptionalAttributes, which is used to
 * determine which optional attributes should be enabled for the cluster.
 */
class BaseIntegrationDelegate : public CodegenClusterIntegration::Delegate
{
protected:
    OptionalAttributes GetOptionalAttributes(EndpointId endpointId, BitFlags<Thermostat::Feature> features);
};

/**
 * A helper struct to store the cluster instances.
 *
 * @tparam Size The maximum number of cluster instances.
 * @tparam Cluster The type of the cluster.
 */
template <std::size_t Size, typename Cluster>
struct ClusterStorage
{
    inline static std::array<LazyRegisteredServerCluster<Cluster>, Size> mClusters = {};
};

/**
 * Integration delegate for the Thermostat cluster.
 *
 * @tparam Size The maximum number of cluster instances.
 * @tparam Cluster The type of the cluster.
 * @tparam ...Delegates The types of the delegates.
 */
template <std::size_t Size, typename Cluster, typename... Delegates>
class IntegrationDelegate : public BaseIntegrationDelegate
{
public:
    explicit IntegrationDelegate(Delegates &... delegates) : mDelegates(delegates...) {}

    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        const BitFlags<Thermostat::Feature> features(featureMap);
        const OptionalAttributes optionalAttributes = GetOptionalAttributes(endpointId, features);

        ThermostatClusterBase::Config config(optionalAttributes, GetDefaultTimerDelegate());

        ChipLogProgress(Zcl, "Creating thermostat cluster for endpoint %d", endpointId);
        if constexpr (sizeof...(Delegates) > 0)
        {
            std::apply(
                [&](auto &... dels) {
                    ClusterStorage<Size, Cluster>::mClusters[clusterInstanceIndex].Create(endpointId, features, config, dels...);
                },
                mDelegates);
        }
        return ClusterStorage<Size, Cluster>::mClusters[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        if (!ClusterStorage<Size, Cluster>::mClusters[clusterInstanceIndex].IsConstructed())
        {
            return nullptr;
        }
        return &ClusterStorage<Size, Cluster>::mClusters[clusterInstanceIndex].Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override
    {
        ClusterStorage<Size, Cluster>::mClusters[clusterInstanceIndex].Destroy();
    }

    static Cluster * FindClusterOnEndpoint(EndpointId endpointId)
    {
        for (auto & cluster : ClusterStorage<Size, Cluster>::mClusters)
        {
            if (cluster.IsConstructed() && cluster.Cluster().GetPaths()[0].mEndpointId == endpointId)
            {
                return &cluster.Cluster();
            }
        }
        return nullptr;
    }

private:
    std::tuple<Delegates &...> mDelegates;
};

/**
 * Initialize the Thermostat cluster on a given endpoint with the given delegates.
 */
template <typename ClusterT, typename... DelegateArgs>
void ServerInit(EndpointId endpointId, DelegateArgs &... delegates)
{
    static_assert(sizeof...(DelegateArgs) > 0, "ServerInit requires at least one delegate");
    IntegrationDelegate<kThermostatEndpointCount, ClusterT, DelegateArgs...> integrationDelegate(delegates...);

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

/**
 * A convenience overload that deduces the cluster type from the delegate arguments.
 */
template <typename... DelegateArgs>
void ServerInit(EndpointId endpointId, DelegateArgs &... delegates)
{
    ServerInit<ThermostatCluster<std::decay_t<DelegateArgs>...>, DelegateArgs...>(endpointId,
                                                                                  std::forward<DelegateArgs &>(delegates)...);
}

/**
 * Uninitialize the Thermostat cluster on a given endpoint with the given cluster type.
 */
template <typename ClusterT>
void ServerShutdown(EndpointId endpointId, MatterClusterShutdownType clusterShutdownType)
{
    IntegrationDelegate<kThermostatEndpointCount, ClusterT> integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Thermostat::Id,
            .fixedClusterInstanceCount = kThermostatFixedClusterCount,
            .maxClusterInstanceCount   = kThermostatEndpointCount,
        },
        integrationDelegate, clusterShutdownType);
}

/**
 * A convenience overload that deduces the cluster type from the delegate arguments.
 */
template <typename ClusterT>
ClusterT * FindClusterOnEndpoint(EndpointId endpointId)
{
    return IntegrationDelegate<kThermostatEndpointCount, ClusterT>::FindClusterOnEndpoint(endpointId);
}

} // namespace chip::app::Clusters::Thermostat
