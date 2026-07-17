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
#include <app/util/attribute-table.h>
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

LazyRegisteredServerCluster<CodegenThermostatCluster> gServers[kThermostatMaxClusterCount];

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
    { OutdoorTemperature::Id, OptionalAttributesBits::kOutdoorTemperature },
    { AbsMinHeatSetpointLimit::Id, OptionalAttributesBits::kAbsMinHeatSetpointLimit },
    { AbsMaxHeatSetpointLimit::Id, OptionalAttributesBits::kAbsMaxHeatSetpointLimit },
    { AbsMinCoolSetpointLimit::Id, OptionalAttributesBits::kAbsMinCoolSetpointLimit },
    { AbsMaxCoolSetpointLimit::Id, OptionalAttributesBits::kAbsMaxCoolSetpointLimit },
    { LocalTemperatureCalibration::Id, OptionalAttributesBits::kLocalTemperatureCalibration },
    { MinHeatSetpointLimit::Id, OptionalAttributesBits::kMinHeatSetpointLimit },
    { MaxHeatSetpointLimit::Id, OptionalAttributesBits::kMaxHeatSetpointLimit },
    { MinCoolSetpointLimit::Id, OptionalAttributesBits::kMinCoolSetpointLimit },
    { MaxCoolSetpointLimit::Id, OptionalAttributesBits::kMaxCoolSetpointLimit },
    { RemoteSensing::Id, OptionalAttributesBits::kRemoteSensing },
    { ThermostatRunningMode::Id, OptionalAttributesBits::kThermostatRunningMode },
    { TemperatureSetpointHold::Id, OptionalAttributesBits::kTemperatureSetpointHold },
    { TemperatureSetpointHoldDuration::Id, OptionalAttributesBits::kTemperatureSetpointHoldDuration },
    { ThermostatRunningState::Id, OptionalAttributesBits::kThermostatRunningState },
    { SetpointChangeSource::Id, OptionalAttributesBits::kSetpointChangeSource },
    { SetpointChangeAmount::Id, OptionalAttributesBits::kSetpointChangeAmount },
    { SetpointChangeSourceTimestamp::Id, OptionalAttributesBits::kSetpointChangeSourceTimestamp },
    { EmergencyHeatDelta::Id, OptionalAttributesBits::kEmergencyHeatDelta },
    { ACType::Id, OptionalAttributesBits::kACType },
    { ACCapacity::Id, OptionalAttributesBits::kACCapacity },
    { ACRefrigerantType::Id, OptionalAttributesBits::kACRefrigerantType },
    { ACCompressorType::Id, OptionalAttributesBits::kACCompressorType },
    { ACErrorCode::Id, OptionalAttributesBits::kACErrorCode },
    { ACLouverPosition::Id, OptionalAttributesBits::kACLouverPosition },
    { ACCoilTemperature::Id, OptionalAttributesBits::kACCoilTemperature },
    { ACCapacityformat::Id, OptionalAttributesBits::kACCapacityFormat },
    { SetpointHoldExpiryTimestamp::Id, OptionalAttributesBits::kSetpointHoldExpiryTimestamp },
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
        // Optional Attributes
        BitFlags<OptionalAttributesBits> optionalAttributes;
        for (const auto & mapping : kOptionalAttributeMap)
        {
            if (emberAfContainsAttribute(endpointId, Thermostat::Id, mapping.attributeId))
            {
                optionalAttributes.Set(mapping.bit);
            }
        }

        ThermostatCluster::Config config(endpointId, Server::GetInstance().GetFabricTable());
        config.WithFeatures(BitFlags<Thermostat::Feature>(featureMap)).WithOptionalAttributes(optionalAttributes);

        // Seed the setpoint family and mode attributes from the Ember/ZAP defaults.
        SeedFromDefault(AbsMinHeatSetpointLimit::GetDefault, endpointId, config.mAbsMinHeatSetpointLimit);
        SeedFromDefault(AbsMaxHeatSetpointLimit::GetDefault, endpointId, config.mAbsMaxHeatSetpointLimit);
        SeedFromDefault(AbsMinCoolSetpointLimit::GetDefault, endpointId, config.mAbsMinCoolSetpointLimit);
        SeedFromDefault(AbsMaxCoolSetpointLimit::GetDefault, endpointId, config.mAbsMaxCoolSetpointLimit);

        uint8_t numberOfSchedules = config.mNumberOfSchedules;
        if (NumberOfSchedules::GetDefault(endpointId, &numberOfSchedules) == Status::Success)
        {
            config.mNumberOfSchedules = numberOfSchedules;
        }

        uint8_t numberOfScheduleTransitions = config.mNumberOfScheduleTransitions;
        if (NumberOfScheduleTransitions::GetDefault(endpointId, &numberOfScheduleTransitions) == Status::Success)
        {
            config.mNumberOfScheduleTransitions = numberOfScheduleTransitions;
        }

        DataModel::Nullable<uint8_t> numberOfScheduleTransitionPerDay = config.mNumberOfScheduleTransitionPerDay;
        if (NumberOfScheduleTransitionPerDay::GetDefault(endpointId, numberOfScheduleTransitionPerDay) == Status::Success)
        {
            config.mNumberOfScheduleTransitionPerDay = numberOfScheduleTransitionPerDay;
        }

        gServers[clusterInstanceIndex].Create(config);
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

CHIP_ERROR CodegenThermostatCluster::Startup(ServerClusterContext & context)
{
    // Load values from ZAP defaults.
    // LocalTemperatureCalibration
    int8_t localTemperatureCalibration = mLocalTemperatureCalibration;
    if (LocalTemperatureCalibration::GetDefault(mPath.mEndpointId, &localTemperatureCalibration) == Status::Success)
    {
        mLocalTemperatureCalibration = localTemperatureCalibration;
    }

    // OccupiedCoolingSetpoint
    int16_t occupiedCoolingSetpoint = mOccupiedCoolingSetpoint;
    if (OccupiedCoolingSetpoint::GetDefault(mPath.mEndpointId, &occupiedCoolingSetpoint) == Status::Success)
    {
        mOccupiedCoolingSetpoint = occupiedCoolingSetpoint;
    }

    // OccupiedHeatingSetpoint
    int16_t occupiedHeatingSetpoint = mOccupiedHeatingSetpoint;
    if (OccupiedHeatingSetpoint::GetDefault(mPath.mEndpointId, &occupiedHeatingSetpoint) == Status::Success)
    {
        mOccupiedHeatingSetpoint = occupiedHeatingSetpoint;
    }

    // UnoccupiedCoolingSetpoint
    int16_t unoccupiedCoolingSetpoint = mUnoccupiedCoolingSetpoint;
    if (UnoccupiedCoolingSetpoint::GetDefault(mPath.mEndpointId, &unoccupiedCoolingSetpoint) == Status::Success)
    {
        mUnoccupiedCoolingSetpoint = unoccupiedCoolingSetpoint;
    }

    // UnoccupiedHeatingSetpoint
    int16_t unoccupiedHeatingSetpoint = mUnoccupiedHeatingSetpoint;
    if (UnoccupiedHeatingSetpoint::GetDefault(mPath.mEndpointId, &unoccupiedHeatingSetpoint) == Status::Success)
    {
        mUnoccupiedHeatingSetpoint = unoccupiedHeatingSetpoint;
    }

    // MinHeatSetpointLimit
    int16_t minHeatSetpointLimit = mMinHeatSetpointLimit;
    if (MinHeatSetpointLimit::GetDefault(mPath.mEndpointId, &minHeatSetpointLimit) == Status::Success)
    {
        mMinHeatSetpointLimit = minHeatSetpointLimit;
    }

    // MaxHeatSetpointLimit
    int16_t maxHeatSetpointLimit = mMaxHeatSetpointLimit;
    if (MaxHeatSetpointLimit::GetDefault(mPath.mEndpointId, &maxHeatSetpointLimit) == Status::Success)
    {
        mMaxHeatSetpointLimit = maxHeatSetpointLimit;
    }

    // MinCoolSetpointLimit
    int16_t minCoolSetpointLimit = mMinCoolSetpointLimit;
    if (MinCoolSetpointLimit::GetDefault(mPath.mEndpointId, &minCoolSetpointLimit) == Status::Success)
    {
        mMinCoolSetpointLimit = minCoolSetpointLimit;
    }

    // MaxCoolSetpointLimit
    int16_t maxCoolSetpointLimit = mMaxCoolSetpointLimit;
    if (MaxCoolSetpointLimit::GetDefault(mPath.mEndpointId, &maxCoolSetpointLimit) == Status::Success)
    {
        mMaxCoolSetpointLimit = maxCoolSetpointLimit;
    }

    // MinSetpointDeadBand
    int8_t minSetpointDeadBand = mMinSetpointDeadBand;
    if (MinSetpointDeadBand::GetDefault(mPath.mEndpointId, &minSetpointDeadBand) == Status::Success)
    {
        mMinSetpointDeadBand = minSetpointDeadBand;
    }

    // RemoteSensing
    BitMask<RemoteSensingBitmap> remoteSensing = mRemoteSensing;
    if (RemoteSensing::GetDefault(mPath.mEndpointId, &remoteSensing) == Status::Success)
    {
        mRemoteSensing = remoteSensing;
    }

    // ControlSequenceOfOperation
    ControlSequenceOfOperationEnum controlSequenceOfOperation = mControlSequenceOfOperation;
    if (ControlSequenceOfOperation::GetDefault(mPath.mEndpointId, &controlSequenceOfOperation) == Status::Success)
    {
        mControlSequenceOfOperation = controlSequenceOfOperation;
    }

    // SystemMode
    SystemModeEnum systemMode = mSystemMode;
    if (SystemMode::GetDefault(mPath.mEndpointId, &systemMode) == Status::Success)
    {
        mSystemMode = systemMode;
    }

    // ThermostatRunningMode
    ThermostatRunningModeEnum thermostatRunningMode = mThermostatRunningMode;
    if (ThermostatRunningMode::GetDefault(mPath.mEndpointId, &thermostatRunningMode) == Status::Success)
    {
        mThermostatRunningMode = thermostatRunningMode;
    }

    // TemperatureSetpointHold
    TemperatureSetpointHoldEnum temperatureSetpointHold = mTemperatureSetpointHold;
    if (TemperatureSetpointHold::GetDefault(mPath.mEndpointId, &temperatureSetpointHold) == Status::Success)
    {
        mTemperatureSetpointHold = temperatureSetpointHold;
    }

    // TemperatureSetpointHoldDuration
    DataModel::Nullable<uint16_t> temperatureSetpointHoldDuration = mTemperatureSetpointHoldDuration;
    if (TemperatureSetpointHoldDuration::GetDefault(mPath.mEndpointId, temperatureSetpointHoldDuration) == Status::Success)
    {
        mTemperatureSetpointHoldDuration = temperatureSetpointHoldDuration;
    }

    // ThermostatRunningState
    BitMask<RelayStateBitmap> thermostatRunningState = mThermostatRunningState;
    if (ThermostatRunningState::GetDefault(mPath.mEndpointId, &thermostatRunningState) == Status::Success)
    {
        mThermostatRunningState = thermostatRunningState;
    }

    // SetpointChangeSource
    SetpointChangeSourceEnum setpointChangeSource = mSetpointChangeSource;
    if (SetpointChangeSource::GetDefault(mPath.mEndpointId, &setpointChangeSource) == Status::Success)
    {
        mSetpointChangeSource = setpointChangeSource;
    }

    // SetpointChangeAmount
    DataModel::Nullable<int16_t> setpointChangeAmount = mSetpointChangeAmount;
    if (SetpointChangeAmount::GetDefault(mPath.mEndpointId, setpointChangeAmount) == Status::Success)
    {
        mSetpointChangeAmount = setpointChangeAmount;
    }

    // SetpointChangeSourceTimestamp
    uint32_t setpointChangeSourceTimestamp = mSetpointChangeSourceTimestamp;
    if (SetpointChangeSourceTimestamp::GetDefault(mPath.mEndpointId, &setpointChangeSourceTimestamp) == Status::Success)
    {
        mSetpointChangeSourceTimestamp = setpointChangeSourceTimestamp;
    }

    // EmergencyHeatDelta
    uint8_t emergencyHeatDelta = mEmergencyHeatDelta;
    if (EmergencyHeatDelta::GetDefault(mPath.mEndpointId, &emergencyHeatDelta) == Status::Success)
    {
        mEmergencyHeatDelta = emergencyHeatDelta;
    }

    // ACType
    ACTypeEnum acType = mACType;
    if (ACType::GetDefault(mPath.mEndpointId, &acType) == Status::Success)
    {
        mACType = acType;
    }

    // ACCapacity
    uint16_t acCapacity = mACCapacity;
    if (ACCapacity::GetDefault(mPath.mEndpointId, &acCapacity) == Status::Success)
    {
        mACCapacity = acCapacity;
    }

    // ACRefrigerantType
    ACRefrigerantTypeEnum acRefrigerantType = mACRefrigerantType;
    if (ACRefrigerantType::GetDefault(mPath.mEndpointId, &acRefrigerantType) == Status::Success)
    {
        mACRefrigerantType = acRefrigerantType;
    }

    // ACCompressorType
    ACCompressorTypeEnum acCompressorType = mACCompressorType;
    if (ACCompressorType::GetDefault(mPath.mEndpointId, &acCompressorType) == Status::Success)
    {
        mACCompressorType = acCompressorType;
    }

    // ACErrorCode
    BitMask<ACErrorCodeBitmap> acErrorCode = mACErrorCode;
    if (ACErrorCode::GetDefault(mPath.mEndpointId, &acErrorCode) == Status::Success)
    {
        mACErrorCode = acErrorCode;
    }

    // ACLouverPosition
    ACLouverPositionEnum acLouverPosition = mACLouverPosition;
    if (ACLouverPosition::GetDefault(mPath.mEndpointId, &acLouverPosition) == Status::Success)
    {
        mACLouverPosition = acLouverPosition;
    }

    // ACCoilTemperature
    DataModel::Nullable<int16_t> acCoilTemperature = mACCoilTemperature;
    if (ACCoilTemperature::GetDefault(mPath.mEndpointId, acCoilTemperature) == Status::Success)
    {
        mACCoilTemperature = acCoilTemperature;
    }

    // ACCapacityFormat
    ACCapacityFormatEnum acCapacityFormat = mACCapacityFormat;
    if (ACCapacityformat::GetDefault(mPath.mEndpointId, &acCapacityFormat) == Status::Success)
    {
        mACCapacityFormat = acCapacityFormat;
    }

    return ThermostatCluster::Startup(context);
}

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

Status Get(EndpointId endpoint, int16_t * value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    *value = cluster->GetOccupiedCoolingSetpoint();
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

Status Get(EndpointId endpoint, int16_t * value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    *value = cluster->GetOccupiedHeatingSetpoint();
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

Status Get(EndpointId endpoint, ControlSequenceOfOperationEnum * value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    *value = cluster->GetControlSequenceOfOperation();
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

Status Get(EndpointId endpoint, SystemModeEnum * value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    *value = cluster->GetSystemMode();
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

namespace AbsMinHeatSetpointLimit {

Status Get(EndpointId endpoint, int16_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetAbsMinHeatSetpointLimit();
    return Status::Success;
}

Status Set(EndpointId endpoint, int16_t value)
{
    using Traits = NumericAttributeTraits<int16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::Thermostat::Id, Id, writable, ZCL_TEMPERATURE_ATTRIBUTE_TYPE);
}

} // namespace AbsMinHeatSetpointLimit

namespace AbsMaxHeatSetpointLimit {

Status Get(EndpointId endpoint, int16_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetAbsMaxHeatSetpointLimit();
    return Status::Success;
}

Status Set(EndpointId endpoint, int16_t value)
{
    using Traits = NumericAttributeTraits<int16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::Thermostat::Id, Id, writable, ZCL_TEMPERATURE_ATTRIBUTE_TYPE);
}

} // namespace AbsMaxHeatSetpointLimit

namespace AbsMinCoolSetpointLimit {

Status Get(EndpointId endpoint, int16_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetAbsMinCoolSetpointLimit();
    return Status::Success;
}

Status Set(EndpointId endpoint, int16_t value)
{
    using Traits = NumericAttributeTraits<int16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::Thermostat::Id, Id, writable, ZCL_TEMPERATURE_ATTRIBUTE_TYPE);
}

} // namespace AbsMinCoolSetpointLimit

namespace AbsMaxCoolSetpointLimit {

Status Get(EndpointId endpoint, int16_t & value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    value = cluster->GetAbsMaxCoolSetpointLimit();
    return Status::Success;
}

Status Set(EndpointId endpoint, int16_t value)
{
    using Traits = NumericAttributeTraits<int16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::Thermostat::Id, Id, writable, ZCL_TEMPERATURE_ATTRIBUTE_TYPE);
}
} // namespace AbsMaxCoolSetpointLimit

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    ThermostatCluster * cluster = FindClusterOnEndpoint(endpoint);
    if (cluster == nullptr)
    {
        return Status::UnsupportedEndpoint;
    }
    *value = cluster->GetFeatureMap();
    return Status::Success;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::Thermostat::Id, Id, writable, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

} // namespace FeatureMap

} // namespace Attributes
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
