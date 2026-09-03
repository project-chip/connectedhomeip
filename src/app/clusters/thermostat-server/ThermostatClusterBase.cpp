/**
 *    Copyright (c) 2020-2026 Project CHIP Authors
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

#include "ThermostatClusterBase.h"
#include "PresetStructWithOwnedMembers.h"
#include "lib/support/logging/TextOnlyLogging.h"

#include <app/persistence/AttributePersistence.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/server-cluster/AttributeListBuilder.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <clusters/Thermostat/Metadata.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

ThermostatClusterBase::ThermostatClusterBase(EndpointId endpointId, const BitFlags<Thermostat::Feature> features,
                                             const Config & config, Thermostat::Delegate & delegate) :
    DefaultServerCluster({ endpointId, Thermostat::Id }),
    mFeatures(features), mConfig(config), mDelegate(delegate)
{
    ChipLogProgress(Zcl, "Starting up thermostat server cluster on endpoint %d", mPath.mEndpointId);
}

CHIP_ERROR ThermostatClusterBase::Startup(ServerClusterContext & context)
{
    ChipLogProgress(Zcl, "Starting up thermostat server cluster on endpoint %d", mPath.mEndpointId);
    return DefaultServerCluster::Startup(context);
}

void ThermostatClusterBase::Shutdown(ClusterShutdownType type)
{
    DefaultServerCluster::Shutdown(type);
    ChipLogProgress(Zcl, "Shutting down thermostat server cluster on endpoint %d", mPath.mEndpointId);
}

CHIP_ERROR ThermostatClusterBase::Attributes(const ConcreteClusterPath & path,
                                             ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mConfig.mOptionalAttributes.LocalTemperatureCalibration, LocalTemperatureCalibration::kMetadataEntry },
        { mConfig.mOptionalAttributes.OutdoorTemperature, OutdoorTemperature::kMetadataEntry },
        { mConfig.mOptionalAttributes.RemoteSensing, RemoteSensing::kMetadataEntry },
        { mConfig.mOptionalAttributes.ThermostatRunningMode, ThermostatRunningMode::kMetadataEntry },
        { mConfig.mOptionalAttributes.ThermostatRunningState, ThermostatRunningState::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(Thermostat::Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

ControlSequenceOfOperationEnum ThermostatClusterBase::GetControlSequenceOfOperation() const
{
    return mDelegate.GetControlSequenceOfOperation();
}

Status ThermostatClusterBase::SetControlSequenceOfOperation(ControlSequenceOfOperationEnum controlSequenceOfOperation)
{
    bool changed = false;
    if (auto err = mDelegate.SetControlSequenceOfOperation(controlSequenceOfOperation, changed); err != Status::Success)
    {
        return err;
    }
    if (changed)
    {
        NotifyAttributeChanged(ControlSequenceOfOperation::Id);
    }
    return Status::Success;
}

SystemModeEnum ThermostatClusterBase::GetSystemMode() const
{
    return mDelegate.GetSystemMode();
}

Status ThermostatClusterBase::SetSystemMode(SystemModeEnum systemMode)
{
    switch (systemMode)
    {
    case SystemModeEnum::kOff:
        break;
    case SystemModeEnum::kAuto:
        if (!mFeatures.Has(Feature::kAutoMode))
        {
            ChipLogDetail(Zcl, "Auto mode is not supported");
            return Status::ConstraintError;
        }
        break;
    case SystemModeEnum::kCool:
        if (!mFeatures.Has(Feature::kCooling))
        {
            ChipLogDetail(Zcl, "Cooling mode is not supported");
            return Status::ConstraintError;
        }
        break;
    case SystemModeEnum::kHeat:
        if (!mFeatures.Has(Feature::kHeating))
        {
            ChipLogDetail(Zcl, "Heating mode is not supported");
            return Status::ConstraintError;
        }
        break;
    case SystemModeEnum::kEmergencyHeat:
        if (!mFeatures.Has(Feature::kHeating))
        {
            ChipLogDetail(Zcl, "Emergency heat mode is not supported");
            return Status::ConstraintError;
        }
        break;
    case SystemModeEnum::kPrecooling:
        if (!mFeatures.Has(Feature::kCooling))
        {
            ChipLogDetail(Zcl, "Precooling mode is not supported");
            return Status::ConstraintError;
        }
        break;
    case SystemModeEnum::kFanOnly:
    case SystemModeEnum::kDry:
    case SystemModeEnum::kSleep:
        break;
    default:
        return Status::ConstraintError;
    }

    SystemModeEnum oldSystemMode = GetSystemMode();

    bool changed = false;
    if (auto err = mDelegate.SetSystemMode(systemMode, changed); err != Status::Success)
    {
        return err;
    }
    if (changed)
    {
        NotifyAttributeChanged(SystemMode::Id);
        GenerateSystemModeChangeEvent(Optional<SystemModeEnum>(oldSystemMode), systemMode);
    }
    return Status::Success;
}

DataModel::Nullable<temperature> ThermostatClusterBase::GetLocalTemperature() const
{
    return mDelegate.GetLocalTemperature();
}

Status ThermostatClusterBase::SetLocalTemperature(DataModel::Nullable<temperature> localTemperature,
                                                  DataModel::AttributeChangeType changeType)
{
    bool changed = false;
    if (auto err = mDelegate.SetLocalTemperature(localTemperature, changed); err != Status::Success)
    {
        return err;
    }
    if (changed)
    {
        NotifyAttributeChanged(LocalTemperature::Id, changeType);
        GenerateLocalTemperatureChangeEvent(localTemperature);
    }
    return Status::Success;
}

int8_t ThermostatClusterBase::GetLocalTemperatureCalibration() const
{
    return mDelegate.GetLocalTemperatureCalibration();
}

Status ThermostatClusterBase::SetLocalTemperatureCalibration(int8_t localTemperatureCalibration)
{
    bool changed = false;
    if (auto err = mDelegate.SetLocalTemperatureCalibration(localTemperatureCalibration, changed); err != Status::Success)
    {
        return err;
    }
    if (changed)
    {
        NotifyAttributeChanged(LocalTemperatureCalibration::Id);
    }
    return Status::Success;
}

ThermostatRunningModeEnum ThermostatClusterBase::GetRunningMode() const
{
    ThermostatRunningModeEnum runningMode = ThermostatRunningModeEnum::kUnknownEnumValue;
    if (auto status = mDelegate.GetRunningMode(runningMode); status != Status::Success)
    {
        ChipLogError(Zcl, "Failed to get running mode");
    }
    return runningMode;
}

Status ThermostatClusterBase::SetRunningMode(ThermostatRunningModeEnum runningMode)
{
    switch (runningMode)
    {
    case ThermostatRunningModeEnum::kOff:
        break;
    case ThermostatRunningModeEnum::kCool:
        if (!mFeatures.Has(Feature::kCooling))
        {
            ChipLogDetail(Zcl, "Cooling mode is not supported");
            return Status::ConstraintError;
        }
        break;
    case ThermostatRunningModeEnum::kHeat:
        if (!mFeatures.Has(Feature::kHeating))
        {
            ChipLogDetail(Zcl, "Heating mode is not supported");
            return Status::ConstraintError;
        }
        break;
    default:
        return Status::ConstraintError;
    }
    auto currentRunningMode = GetRunningMode();

    bool changed = false;
    if (auto err = mDelegate.SetRunningMode(runningMode, changed); err != Status::Success)
    {
        return err;
    }
    if (changed)
    {
        NotifyAttributeChanged(ThermostatRunningMode::Id);
        GenerateRunningModeChangeEvent(Optional<ThermostatRunningModeEnum>(currentRunningMode), runningMode);
    }
    return Status::Success;
}

BitMask<RelayStateBitmap> ThermostatClusterBase::GetRunningState() const
{
    BitMask<RelayStateBitmap> runningState = 0;
    if (auto status = mDelegate.GetRunningState(runningState); status != Status::Success)
    {
        ChipLogError(Zcl, "Failed to get running state");
    }
    return runningState;
}

Status ThermostatClusterBase::SetRunningState(BitMask<RelayStateBitmap> runningState)
{
    if (runningState.HasAny(RelayStateBitmap::kHeat, RelayStateBitmap::kHeatStage2) && !mFeatures.Has(Feature::kHeating))
    {
        ChipLogDetail(Zcl, "Heating relay state is not supported");
        return Status::ConstraintError;
    }
    if (runningState.HasAny(RelayStateBitmap::kCool, RelayStateBitmap::kCoolStage2) && !mFeatures.Has(Feature::kCooling))
    {
        ChipLogDetail(Zcl, "Cooling relay state is not supported");
        return Status::ConstraintError;
    }
    auto currentRunningState = GetRunningState();

    bool changed = false;
    if (auto err = mDelegate.SetRunningState(runningState, changed); err != Status::Success)
    {
        return err;
    }
    if (changed)
    {
        NotifyAttributeChanged(ThermostatRunningState::Id);
        GenerateRunningStateChangeEvent(Optional<BitMask<RelayStateBitmap>>(currentRunningState), runningState);
    }
    return Status::Success;
}

CHIP_ERROR ThermostatClusterBase::AcceptedCommands(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.AppendElements({
        Commands::SetpointRaiseLower::kMetadataEntry,
    });
}

CHIP_ERROR ThermostatClusterBase::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    return CHIP_NO_ERROR;
}

bool ThermostatClusterBase::IsActiveSetpoint(AttributeId attributeId) const
{
    if (IsOccupied())
    {
        return (attributeId == OccupiedHeatingSetpoint::Id || attributeId == OccupiedCoolingSetpoint::Id);
    }
    return (attributeId == UnoccupiedHeatingSetpoint::Id || attributeId == UnoccupiedCoolingSetpoint::Id);
}

bool ThermostatClusterBase::HasAttribute(AttributeId attributeId)
{
    switch (attributeId)
    {
    case LocalTemperature::Id:
    case ControlSequenceOfOperation::Id:
    case SystemMode::Id:
        return true;
    case OutdoorTemperature::Id:
        return mConfig.mOptionalAttributes.OutdoorTemperature;
    case Occupancy::Id:
        return mFeatures.Has(Feature::kOccupancy);
    case AbsMinHeatSetpointLimit::Id:
        return mConfig.mOptionalAttributes.AbsMinHeatSetpointLimit;
    case AbsMaxHeatSetpointLimit::Id:
        return mConfig.mOptionalAttributes.AbsMaxHeatSetpointLimit;
    case AbsMinCoolSetpointLimit::Id:
        return mConfig.mOptionalAttributes.AbsMinCoolSetpointLimit;
    case AbsMaxCoolSetpointLimit::Id:
        return mConfig.mOptionalAttributes.AbsMaxCoolSetpointLimit;
    case LocalTemperatureCalibration::Id:
        return mConfig.mOptionalAttributes.LocalTemperatureCalibration;
    case OccupiedCoolingSetpoint::Id:
        return mFeatures.Has(Feature::kCooling);
    case OccupiedHeatingSetpoint::Id:
        return mFeatures.Has(Feature::kHeating);
    case UnoccupiedHeatingSetpoint::Id:
        return mFeatures.Has(Feature::kHeating) && mFeatures.Has(Feature::kOccupancy);
    case UnoccupiedCoolingSetpoint::Id:
        return mFeatures.Has(Feature::kCooling) && mFeatures.Has(Feature::kOccupancy);
    case MinHeatSetpointLimit::Id:
        return mConfig.mOptionalAttributes.MinHeatSetpointLimit;
    case MaxHeatSetpointLimit::Id:
        return mConfig.mOptionalAttributes.MaxHeatSetpointLimit;
    case MinCoolSetpointLimit::Id:
        return mConfig.mOptionalAttributes.MinCoolSetpointLimit;
    case MaxCoolSetpointLimit::Id:
        return mConfig.mOptionalAttributes.MaxCoolSetpointLimit;
    case MinSetpointDeadBand::Id:
        return mFeatures.Has(Feature::kAutoMode);
    case RemoteSensing::Id:
        return mConfig.mOptionalAttributes.RemoteSensing;
    case ThermostatRunningMode::Id:
        return mConfig.mOptionalAttributes.ThermostatRunningMode;
    case TemperatureSetpointHold::Id:
        return mConfig.mOptionalAttributes.TemperatureSetpointHold;
    case TemperatureSetpointHoldDuration::Id:
        return mConfig.mOptionalAttributes.TemperatureSetpointHoldDuration;
    case ThermostatRunningState::Id:
        return mConfig.mOptionalAttributes.ThermostatRunningState;
    case SetpointChangeSource::Id:
        return mConfig.mOptionalAttributes.SetpointChangeSource;
    case SetpointChangeAmount::Id:
        return mConfig.mOptionalAttributes.SetpointChangeAmount;
    case SetpointChangeSourceTimestamp::Id:
        return mConfig.mOptionalAttributes.SetpointChangeSourceTimestamp;
    case SetpointHoldExpiryTimestamp::Id:
        return mConfig.mOptionalAttributes.SetpointHoldExpiryTimestamp;
    case PresetTypes::Id:
    case NumberOfPresets::Id:
    case ActivePresetHandle::Id:
    case Presets::Id:
        return mFeatures.Has(Feature::kPresets);
    case ScheduleTypes::Id:
    case NumberOfSchedules::Id:
    case NumberOfScheduleTransitions::Id:
    case NumberOfScheduleTransitionPerDay::Id:
    case ActiveScheduleHandle::Id:
    case Schedules::Id:
        return mFeatures.Has(Feature::kMatterScheduleConfiguration);
    case MaxThermostatSuggestions::Id:
    case Attributes::ThermostatSuggestions::Id:
    case CurrentThermostatSuggestion::Id:
    case ThermostatSuggestionNotFollowingReason::Id:
        return mFeatures.Has(Feature::kThermostatSuggestions);
    case Sensors::Id:
    case AvailableSensors::Id:
    case EnabledSensors::Id:
    case NumberOfSensorScheduleTransitions::Id:
    case SensorSchedule::Id:
        return mFeatures.Has(Feature::kThermostatSensors);
    default:
        return false;
    }
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
