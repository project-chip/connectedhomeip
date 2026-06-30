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

#include <app/clusters/thermostat-server/CodegenIntegration.h>
#include <app/clusters/thermostat-server/ThermostatCluster.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/server/Server.h>
#include <app/static-cluster-config/Thermostat.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace Protocols::InteractionModel;

namespace {

constexpr size_t kThermostatFixedClusterCount = Thermostat::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kThermostatMaxClusterCount   = kThermostatFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<ThermostatCluster> gServers[kThermostatMaxClusterCount];

// Delegates set by the application (via SetDefaultDelegate) keyed by cluster instance index. The
// application may set its delegate before or after the cluster is registered, so the value is
// retained here and also pushed onto the live cluster instance when available.
Thermostat::Delegate * gDelegateTable[kThermostatMaxClusterCount] = { nullptr };

/// Reads the Ember/ZAP configured default for a single int16_t attribute.
void SeedFromDefault(Status (*getDefault)(EndpointId, int16_t *), EndpointId endpointId, int16_t & target)
{
    int16_t value = target;
    if (getDefault(endpointId, &value) == Status::Success)
    {
        target = value;
    }
}

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        ThermostatCluster::StartupConfiguration config;

        // Seed the setpoint family and mode attributes from the Ember/ZAP defaults.
        SeedFromDefault(AbsMinHeatSetpointLimit::GetDefault, endpointId, config.absMinHeatSetpointLimit);
        SeedFromDefault(AbsMaxHeatSetpointLimit::GetDefault, endpointId, config.absMaxHeatSetpointLimit);
        SeedFromDefault(AbsMinCoolSetpointLimit::GetDefault, endpointId, config.absMinCoolSetpointLimit);
        SeedFromDefault(AbsMaxCoolSetpointLimit::GetDefault, endpointId, config.absMaxCoolSetpointLimit);

        uint8_t numberOfSchedules = config.numberOfSchedules;
        if (NumberOfSchedules::GetDefault(endpointId, &numberOfSchedules) == Status::Success)
        {
            config.numberOfSchedules = numberOfSchedules;
        }

        uint8_t numberOfScheduleTransitions = config.numberOfScheduleTransitions;
        if (NumberOfScheduleTransitions::GetDefault(endpointId, &numberOfScheduleTransitions) == Status::Success)
        {
            config.numberOfScheduleTransitions = numberOfScheduleTransitions;
        }

        DataModel::Nullable<uint8_t> numberOfScheduleTransitionPerDay = config.numberOfScheduleTransitionPerDay;
        if (NumberOfScheduleTransitionPerDay::GetDefault(endpointId, numberOfScheduleTransitionPerDay) == Status::Success)
        {
            config.numberOfScheduleTransitionPerDay = numberOfScheduleTransitionPerDay;
        }

        ThermostatCluster::Context clusterContext{ Server::GetInstance().GetFabricTable() };
        gServers[clusterInstanceIndex].Create(endpointId, featureMap, config, clusterContext);
        gServers[clusterInstanceIndex].Cluster().SetDelegate(gDelegateTable[clusterInstanceIndex]);
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

void MatterThermostatClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Thermostat::Id,
            .fixedClusterInstanceCount = kThermostatFixedClusterCount,
            .maxClusterInstanceCount   = kThermostatMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterThermostatClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Thermostat::Id,
            .fixedClusterInstanceCount = kThermostatFixedClusterCount,
            .maxClusterInstanceCount   = kThermostatMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}


void MatterThermostatPluginServerInitCallback() {}

void MatterThermostatPluginServerShutdownCallback() {}

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

ThermostatCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * cluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = Thermostat::Id,
            .fixedClusterInstanceCount = kThermostatFixedClusterCount,
            .maxClusterInstanceCount   = kThermostatMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<ThermostatCluster *>(cluster);
}

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t index = emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, kThermostatFixedClusterCount);
    return (index >= MATTER_ARRAY_SIZE(gDelegateTable)) ? nullptr : gDelegateTable[index];
}

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t index = emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, kThermostatFixedClusterCount);
    VerifyOrReturn(index < MATTER_ARRAY_SIZE(gDelegateTable));

    gDelegateTable[index] = delegate;
    if (delegate != nullptr)
    {
        delegate->SetEndpointId(endpoint);
    }

    // If the cluster instance already exists, update its delegate too (handles SetDefaultDelegate being
    // called after the endpoint has been brought up).
    if (gServers[index].IsConstructed())
    {
        gServers[index].Cluster().SetDelegate(delegate);
    }
}
namespace Attributes {

namespace LocalTemperature {

Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetLocalTemperature();
    return Status::Success;
}

Status Set(EndpointId endpoint, DataModel::Nullable<int16_t> value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetLocalTemperature(value);
    return Status::Success;
}

} // namespace FanMode
} // namespace Attributes
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
