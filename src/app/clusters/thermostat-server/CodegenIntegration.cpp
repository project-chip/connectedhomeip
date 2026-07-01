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

struct OptionalAttributeMapping
{
    AttributeId attributeId;
    OptionalAttributesBits bit;
};

constexpr OptionalAttributeMapping kOptionalAttributeMap[] = {
    { OutdoorTemperature::Id,              OptionalAttributesBits::kOutdoorTemperature },
    { AbsMinHeatSetpointLimit::Id,         OptionalAttributesBits::kAbsMinHeatSetpointLimit },
    { AbsMaxHeatSetpointLimit::Id,         OptionalAttributesBits::kAbsMaxHeatSetpointLimit },
    { AbsMinCoolSetpointLimit::Id,         OptionalAttributesBits::kAbsMinCoolSetpointLimit },
    { AbsMaxCoolSetpointLimit::Id,         OptionalAttributesBits::kAbsMaxCoolSetpointLimit },
    { LocalTemperatureCalibration::Id,     OptionalAttributesBits::kLocalTemperatureCalibration },
    { MinHeatSetpointLimit::Id,            OptionalAttributesBits::kMinHeatSetpointLimit },
    { MaxHeatSetpointLimit::Id,            OptionalAttributesBits::kMaxHeatSetpointLimit },
    { MinCoolSetpointLimit::Id,            OptionalAttributesBits::kMinCoolSetpointLimit },
    { MaxCoolSetpointLimit::Id,            OptionalAttributesBits::kMaxCoolSetpointLimit },
    { RemoteSensing::Id,                   OptionalAttributesBits::kRemoteSensing },
    { ThermostatRunningMode::Id,           OptionalAttributesBits::kThermostatRunningMode },
    { TemperatureSetpointHold::Id,         OptionalAttributesBits::kTemperatureSetpointHold },
    { TemperatureSetpointHoldDuration::Id, OptionalAttributesBits::kTemperatureSetpointHoldDuration },
    { ThermostatRunningState::Id,          OptionalAttributesBits::kThermostatRunningState },
    { SetpointChangeSource::Id,            OptionalAttributesBits::kSetpointChangeSource },
    { SetpointChangeAmount::Id,            OptionalAttributesBits::kSetpointChangeAmount },
    { SetpointChangeSourceTimestamp::Id,   OptionalAttributesBits::kSetpointChangeSourceTimestamp },
    { EmergencyHeatDelta::Id,              OptionalAttributesBits::kEmergencyHeatDelta },
    { ACType::Id,                          OptionalAttributesBits::kACType },
    { ACCapacity::Id,                      OptionalAttributesBits::kACCapacity },
    { ACRefrigerantType::Id,               OptionalAttributesBits::kACRefrigerantType },
    { ACCompressorType::Id,                OptionalAttributesBits::kACCompressorType },
    { ACErrorCode::Id,                     OptionalAttributesBits::kACErrorCode },
    { ACLouverPosition::Id,                OptionalAttributesBits::kACLouverPosition },
    { ACCoilTemperature::Id,               OptionalAttributesBits::kACCoilTemperature },
    { ACCapacityformat::Id,                OptionalAttributesBits::kACCapacityFormat },
    { SetpointHoldExpiryTimestamp::Id,     OptionalAttributesBits::kSetpointHoldExpiryTimestamp },
};

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
        ThermostatCluster::StartupConfiguration config{};

        // Optional Attributes
        BitFlags<OptionalAttributesBits> optionalAttributes;
        for (const auto & mapping : kOptionalAttributeMap)
        {
            if (emberAfContainsAttribute(endpointId, Thermostat::Id, mapping.attributeId))
            {
                optionalAttributes.Set(mapping.bit);
            }
        }


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

        gServers[clusterInstanceIndex].Create(endpointId, featureMap, config, clusterContext, optionalAttributes);
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

} // namespace LocalTemperature

namespace OutdoorTemperature {

Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetOutdoorTemperature();
    return Status::Success;
}

Status Set(EndpointId endpoint, DataModel::Nullable<int16_t> value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetOutdoorTemperature(value);
    return Status::Success;
}

} // namespace OutdoorTemperature

namespace Occupancy {

Status Get(EndpointId endpoint, BitMask<OccupancyBitmap> & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetOccupancy();
    return Status::Success;
}

Status Set(EndpointId endpoint, BitMask<OccupancyBitmap> value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetOccupancy(value);
    return Status::Success;
}

} // namespace Occupancy

namespace LocalTemperatureCalibration {

Status Get(EndpointId endpoint, int8_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetLocalTemperatureCalibration();
    return Status::Success;
}

Status Set(EndpointId endpoint, int8_t value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetLocalTemperatureCalibration(value);
    return Status::Success;
}

} // namespace LocalTemperatureCalibration

namespace OccupiedCoolingSetpoint {

Status Get(EndpointId endpoint, int16_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetOccupiedCoolingSetpoint();
    return Status::Success;
}

Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetOccupiedCoolingSetpoint(value);
}

} // namespace OccupiedCoolingSetpoint

namespace OccupiedHeatingSetpoint {

Status Get(EndpointId endpoint, int16_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetOccupiedHeatingSetpoint();
    return Status::Success;
}

Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetOccupiedHeatingSetpoint(value);
}

} // namespace OccupiedHeatingSetpoint

namespace UnoccupiedCoolingSetpoint {

Status Get(EndpointId endpoint, int16_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetUnoccupiedCoolingSetpoint();
    return Status::Success;
}

Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetUnoccupiedCoolingSetpoint(value);
}

} // namespace UnoccupiedCoolingSetpoint

namespace UnoccupiedHeatingSetpoint {

Status Get(EndpointId endpoint, int16_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetUnoccupiedHeatingSetpoint();
    return Status::Success;
}

Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetUnoccupiedHeatingSetpoint(value);
}

} // namespace UnoccupiedHeatingSetpoint

namespace MinHeatSetpointLimit {

Status Get(EndpointId endpoint, int16_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetMinHeatSetpointLimit();
    return Status::Success;
}

Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetMinHeatSetpointLimit(value);
}

} // namespace MinHeatSetpointLimit

namespace MaxHeatSetpointLimit {

Status Get(EndpointId endpoint, int16_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetMaxHeatSetpointLimit();
    return Status::Success;
}

Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetMaxHeatSetpointLimit(value);
}

} // namespace MaxHeatSetpointLimit

namespace MinCoolSetpointLimit {

Status Get(EndpointId endpoint, int16_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetMinCoolSetpointLimit();
    return Status::Success;
}

Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetMinCoolSetpointLimit(value);
}

} // namespace MinCoolSetpointLimit

namespace MaxCoolSetpointLimit {

Status Get(EndpointId endpoint, int16_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetMaxCoolSetpointLimit();
    return Status::Success;
}

Status Set(EndpointId endpoint, int16_t value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetMaxCoolSetpointLimit(value);
}

} // namespace MaxCoolSetpointLimit

namespace MinSetpointDeadBand {

Status Get(EndpointId endpoint, int8_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetMinSetpointDeadband();
    return Status::Success;
}

Status Set(EndpointId endpoint, int8_t value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetMinSetpointDeadband(value);
}

} // namespace MinSetpointDeadBand

namespace RemoteSensing {

Status Get(EndpointId endpoint, BitMask<RemoteSensingBitmap> & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetRemoteSensing();
    return Status::Success;
}

Status Set(EndpointId endpoint, BitMask<RemoteSensingBitmap> value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetRemoteSensing(value);
}

} // namespace RemoteSensing

namespace ControlSequenceOfOperation {

Status Get(EndpointId endpoint, ControlSequenceOfOperationEnum & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetControlSequenceOfOperation();
    return Status::Success;
}

Status Set(EndpointId endpoint, ControlSequenceOfOperationEnum value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetControlSequenceOfOperation(value);
}

} // namespace ControlSequenceOfOperation

namespace SystemMode {

Status Get(EndpointId endpoint, SystemModeEnum & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetSystemMode();
    return Status::Success;
}

Status Set(EndpointId endpoint, SystemModeEnum value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    return cluster->SetSystemMode(value);
}

} // namespace SystemMode

namespace ThermostatRunningMode {

Status Get(EndpointId endpoint, ThermostatRunningModeEnum & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetThermostatRunningMode();
    return Status::Success;
}

Status Set(EndpointId endpoint, ThermostatRunningModeEnum value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetThermostatRunningMode(value);
    return Status::Success;
}

} // namespace ThermostatRunningMode

namespace TemperatureSetpointHold {

Status Get(EndpointId endpoint, TemperatureSetpointHoldEnum & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetTemperatureSetpointHold();
    return Status::Success;
}

Status Set(EndpointId endpoint, TemperatureSetpointHoldEnum value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetTemperatureSetpointHold(value);
    return Status::Success;
}

} // namespace TemperatureSetpointHold

namespace TemperatureSetpointHoldDuration {

Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetTemperatureSetpointHoldDuration();
    return Status::Success;
}

Status Set(EndpointId endpoint, DataModel::Nullable<uint16_t> value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetTemperatureSetpointHoldDuration(value);
    return Status::Success;
}

} // namespace TemperatureSetpointHoldDuration

namespace ThermostatRunningState {

Status Get(EndpointId endpoint, BitMask<RelayStateBitmap> & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetThermostatRunningState();
    return Status::Success;
}

Status Set(EndpointId endpoint, BitMask<RelayStateBitmap> value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetThermostatRunningState(value);
    return Status::Success;
}

} // namespace ThermostatRunningState

namespace SetpointChangeSource {

Status Get(EndpointId endpoint, SetpointChangeSourceEnum & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetSetpointChangeSource();
    return Status::Success;
}

Status Set(EndpointId endpoint, SetpointChangeSourceEnum value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetSetpointChangeSource(value);
    return Status::Success;
}

} // namespace SetpointChangeSource

namespace SetpointChangeAmount {

Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetSetpointChangeAmount();
    return Status::Success;
}

Status Set(EndpointId endpoint, DataModel::Nullable<int16_t> value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetSetpointChangeAmount(value);
    return Status::Success;
}

} // namespace SetpointChangeAmount

namespace SetpointChangeSourceTimestamp {

Status Get(EndpointId endpoint, uint32_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetSetpointChangeSourceTimestamp();
    return Status::Success;
}

Status Set(EndpointId endpoint, uint32_t value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetSetpointChangeSourceTimestamp(value);
    return Status::Success;
}

} // namespace SetpointChangeSourceTimestamp

namespace EmergencyHeatDelta {

Status Get(EndpointId endpoint, uint8_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetEmergencyHeatDelta();
    return Status::Success;
}

Status Set(EndpointId endpoint, uint8_t value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetEmergencyHeatDelta(value);
    return Status::Success;
}

} // namespace EmergencyHeatDelta

namespace ACType {

Status Get(EndpointId endpoint, ACTypeEnum & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetACType();
    return Status::Success;
}

Status Set(EndpointId endpoint, ACTypeEnum value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetACType(value);
    return Status::Success;
}

} // namespace ACType

namespace ACCapacity {

Status Get(EndpointId endpoint, uint16_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetACCapacity();
    return Status::Success;
}

Status Set(EndpointId endpoint, uint16_t value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetACCapacity(value);
    return Status::Success;
}

} // namespace ACCapacity

namespace ACRefrigerantType {

Status Get(EndpointId endpoint, ACRefrigerantTypeEnum & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetACRefrigerantType();
    return Status::Success;
}

Status Set(EndpointId endpoint, ACRefrigerantTypeEnum value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetACRefrigerantType(value);
    return Status::Success;
}

} // namespace ACRefrigerantType

namespace ACCompressorType {

Status Get(EndpointId endpoint, ACCompressorTypeEnum & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetACCompressorType();
    return Status::Success;
}

Status Set(EndpointId endpoint, ACCompressorTypeEnum value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetACCompressorType(value);
    return Status::Success;
}

} // namespace ACCompressorType

namespace ACErrorCode {

Status Get(EndpointId endpoint, BitMask<ACErrorCodeBitmap> & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetACErrorCode();
    return Status::Success;
}

Status Set(EndpointId endpoint, BitMask<ACErrorCodeBitmap> value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetACErrorCode(value);
    return Status::Success;
}

} // namespace ACErrorCode

namespace ACLouverPosition {

Status Get(EndpointId endpoint, ACLouverPositionEnum & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetACLouverPosition();
    return Status::Success;
}

Status Set(EndpointId endpoint, ACLouverPositionEnum value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetACLouverPosition(value);
    return Status::Success;
}

} // namespace ACLouverPosition

namespace ACCoilTemperature {

Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetACCoilTemperature();
    return Status::Success;
}

Status Set(EndpointId endpoint, DataModel::Nullable<int16_t> value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetACCoilTemperature(value);
    return Status::Success;
}

} // namespace ACCoilTemperature

namespace ACCapacityformat {

Status Get(EndpointId endpoint, ACCapacityFormatEnum & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetACCapacityFormat();
    return Status::Success;
}

Status Set(EndpointId endpoint, ACCapacityFormatEnum value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetACCapacityFormat(value);
    return Status::Success;
}

} // namespace ACCapacityformat

namespace NumberOfSchedules {

Status Get(EndpointId endpoint, uint8_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetNumberOfSchedules();
    return Status::Success;
}

Status Set(EndpointId endpoint, uint8_t value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetNumberOfSchedules(value);
    return Status::Success;
}

} // namespace NumberOfSchedules

namespace NumberOfScheduleTransitions {

Status Get(EndpointId endpoint, uint8_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetNumberOfScheduleTransitions();
    return Status::Success;
}

Status Set(EndpointId endpoint, uint8_t value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetNumberOfScheduleTransitions(value);
    return Status::Success;
}

} // namespace NumberOfScheduleTransitions

namespace NumberOfScheduleTransitionPerDay {

Status Get(EndpointId endpoint, DataModel::Nullable<uint8_t> & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetNumberOfScheduleTransitionPerDay();
    return Status::Success;
}

Status Set(EndpointId endpoint, DataModel::Nullable<uint8_t> value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetNumberOfScheduleTransitionPerDay(value);
    return Status::Success;
}

} // namespace NumberOfScheduleTransitionPerDay

namespace SetpointHoldExpiryTimestamp {

Status Get(EndpointId endpoint, DataModel::Nullable<uint32_t> & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetSetpointHoldExpiryTimestamp();
    return Status::Success;
}

Status Set(EndpointId endpoint, DataModel::Nullable<uint32_t> value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    cluster->SetSetpointHoldExpiryTimestamp(value);
    return Status::Success;
}

} // namespace SetpointHoldExpiryTimestamp
} // namespace Attributes
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
