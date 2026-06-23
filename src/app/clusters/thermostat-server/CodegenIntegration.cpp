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

/// Reads the Ember/ZAP-configured default for a single int16_t attribute, leaving the existing value
/// untouched if no default is configured (so the spec default from StartupConfiguration is kept).
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

        // Seed the setpoint family and mode attributes from the Ember/ZAP defaults. Other optional
        // scalar attributes fall back to their spec defaults from StartupConfiguration (TODO(3a-follow-up):
        // seed the remaining optional scalars from ZAP for full parity).
        SeedFromDefault(AbsMinHeatSetpointLimit::GetDefault, endpointId, config.absMinHeatSetpointLimit);
        SeedFromDefault(AbsMaxHeatSetpointLimit::GetDefault, endpointId, config.absMaxHeatSetpointLimit);
        SeedFromDefault(AbsMinCoolSetpointLimit::GetDefault, endpointId, config.absMinCoolSetpointLimit);
        SeedFromDefault(AbsMaxCoolSetpointLimit::GetDefault, endpointId, config.absMaxCoolSetpointLimit);
        SeedFromDefault(MinHeatSetpointLimit::GetDefault, endpointId, config.minHeatSetpointLimit);
        SeedFromDefault(MaxHeatSetpointLimit::GetDefault, endpointId, config.maxHeatSetpointLimit);
        SeedFromDefault(MinCoolSetpointLimit::GetDefault, endpointId, config.minCoolSetpointLimit);
        SeedFromDefault(MaxCoolSetpointLimit::GetDefault, endpointId, config.maxCoolSetpointLimit);
        SeedFromDefault(OccupiedHeatingSetpoint::GetDefault, endpointId, config.occupiedHeatingSetpoint);
        SeedFromDefault(OccupiedCoolingSetpoint::GetDefault, endpointId, config.occupiedCoolingSetpoint);
        SeedFromDefault(UnoccupiedHeatingSetpoint::GetDefault, endpointId, config.unoccupiedHeatingSetpoint);
        SeedFromDefault(UnoccupiedCoolingSetpoint::GetDefault, endpointId, config.unoccupiedCoolingSetpoint);

        int8_t deadBand = config.minSetpointDeadBand;
        if (MinSetpointDeadBand::GetDefault(endpointId, &deadBand) == Status::Success)
        {
            config.minSetpointDeadBand = deadBand;
        }

        ControlSequenceOfOperationEnum controlSeq = config.controlSequenceOfOperation;
        if (ControlSequenceOfOperation::GetDefault(endpointId, &controlSeq) == Status::Success)
        {
            config.controlSequenceOfOperation = controlSeq;
        }

        SystemModeEnum systemMode = config.systemMode;
        if (SystemMode::GetDefault(endpointId, &systemMode) == Status::Success)
        {
            config.systemMode = systemMode;
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

// Each per-endpoint ThermostatCluster now registers itself as a FabricTable::Delegate in its Startup(),
// so there is no global fabric-delegate registration to do here.
void MatterThermostatPluginServerInitCallback() {}

void MatterThermostatPluginServerShutdownCallback() {}

// The generated endpoint_config function array (chipFuncArrayThermostatServer) still references these
// Ember per-cluster lifecycle hooks because the Thermostat attributes remain ZAP/RAM-declared. For the
// code-driven cluster they are no-ops: endpoint init/registration happens in MatterThermostatClusterInitCallback,
// attribute validation / change reporting now happen in ThermostatCluster::WriteAttribute, and atomic-write
// teardown happens in ThermostatCluster::Shutdown (the Ember write path is no longer used for this cluster once
// it is registered with the codegen data model provider).
void emberAfThermostatClusterServerInitCallback(EndpointId endpoint) {}

void MatterThermostatClusterServerShutdownCallback(EndpointId endpoint) {}

void MatterThermostatClusterServerAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath) {}

chip::Protocols::InteractionModel::Status
MatterThermostatClusterServerPreAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath,
                                                         EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    return chip::Protocols::InteractionModel::Status::Success;
}

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

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
