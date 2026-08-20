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

#include "ThermostatCluster.h"
#include "PresetStructWithOwnedMembers.h"

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

ThermostatCluster::ThermostatCluster(EndpointId endpointId, BitFlags<Thermostat::Feature> features, const Config & config,
                                     Thermostat::Delegate & delegate) :
    DefaultServerCluster({ endpointId, Thermostat::Id }),
    mFeatures(features), mConfig(config), mDelegate(delegate)
{
}

CHIP_ERROR ThermostatCluster::Startup(ServerClusterContext & context)
{
    ChipLogProgress(Zcl, "Starting up thermostat server cluster on endpoint %d", mPath.mEndpointId);
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    if (auto status = mConfig.mFabricTable.AddFabricDelegate(this); status != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to add fabric delegate to Thermostat Cluster");
    }
    mAtomicWriteSession.SetDelegate(this);
    return CHIP_NO_ERROR;
}

void ThermostatCluster::Shutdown(ClusterShutdownType type)
{
    mAtomicWriteSession.ResetAtomicWrite();
    mAtomicWriteSession.SetDelegate(nullptr);
    DefaultServerCluster::Shutdown(type);
    mConfig.mFabricTable.RemoveFabricDelegate(this);
    ChipLogProgress(Zcl, "Shutting down thermostat server cluster on endpoint %d", mPath.mEndpointId);
}

CHIP_ERROR ThermostatCluster::Attributes(const ConcreteClusterPath & path,
                                         ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        // Setpoints
        { HasAttribute(OccupiedHeatingSetpoint::Id), OccupiedHeatingSetpoint::kMetadataEntry },
        { HasAttribute(OccupiedCoolingSetpoint::Id), OccupiedCoolingSetpoint::kMetadataEntry },
        { HasAttribute(UnoccupiedHeatingSetpoint::Id), UnoccupiedHeatingSetpoint::kMetadataEntry },
        { HasAttribute(UnoccupiedCoolingSetpoint::Id), UnoccupiedCoolingSetpoint::kMetadataEntry },

        // Setpoint Limits
        { HasAttribute(AbsMinHeatSetpointLimit::Id), AbsMinHeatSetpointLimit::kMetadataEntry },
        { HasAttribute(AbsMaxHeatSetpointLimit::Id), AbsMaxHeatSetpointLimit::kMetadataEntry },
        { HasAttribute(AbsMinCoolSetpointLimit::Id), AbsMinCoolSetpointLimit::kMetadataEntry },
        { HasAttribute(AbsMaxCoolSetpointLimit::Id), AbsMaxCoolSetpointLimit::kMetadataEntry },
        { HasAttribute(MinHeatSetpointLimit::Id), MinHeatSetpointLimit::kMetadataEntry },
        { HasAttribute(MaxHeatSetpointLimit::Id), MaxHeatSetpointLimit::kMetadataEntry },
        { HasAttribute(MinCoolSetpointLimit::Id), MinCoolSetpointLimit::kMetadataEntry },
        { HasAttribute(MaxCoolSetpointLimit::Id), MaxCoolSetpointLimit::kMetadataEntry },

        // Deadband
        { HasAttribute(MinSetpointDeadBand::Id), MinSetpointDeadBand::kMetadataEntry },

        // Feature-based State
        { HasAttribute(Occupancy::Id), Occupancy::kMetadataEntry },
        { HasAttribute(ThermostatRunningMode::Id), ThermostatRunningMode::kMetadataEntry },

        // Other optional attributes
        { HasAttribute(LocalTemperatureCalibration::Id), LocalTemperatureCalibration::kMetadataEntry },
        { HasAttribute(OutdoorTemperature::Id), OutdoorTemperature::kMetadataEntry },
        { HasAttribute(RemoteSensing::Id), RemoteSensing::kMetadataEntry },
        { HasAttribute(ThermostatRunningState::Id), ThermostatRunningState::kMetadataEntry },
        { HasAttribute(SetpointChangeSource::Id), SetpointChangeSource::kMetadataEntry },
        { HasAttribute(SetpointChangeAmount::Id), SetpointChangeAmount::kMetadataEntry },
        { HasAttribute(SetpointChangeSourceTimestamp::Id), SetpointChangeSourceTimestamp::kMetadataEntry },

        // Setpoint Holds
        { HasAttribute(TemperatureSetpointHold::Id), TemperatureSetpointHold::kMetadataEntry },
        { HasAttribute(TemperatureSetpointHoldDuration::Id), TemperatureSetpointHoldDuration::kMetadataEntry },
        { HasAttribute(SetpointHoldExpiryTimestamp::Id), SetpointHoldExpiryTimestamp::kMetadataEntry },

        // Presets
        { HasAttribute(PresetTypes::Id), PresetTypes::kMetadataEntry },
        { HasAttribute(NumberOfPresets::Id), NumberOfPresets::kMetadataEntry },
        { HasAttribute(ActivePresetHandle::Id), ActivePresetHandle::kMetadataEntry },
        { HasAttribute(Presets::Id), Presets::kMetadataEntry },

        // Schedules
        { HasAttribute(ScheduleTypes::Id), ScheduleTypes::kMetadataEntry },
        { HasAttribute(NumberOfSchedules::Id), NumberOfSchedules::kMetadataEntry },
        { HasAttribute(NumberOfScheduleTransitions::Id), NumberOfScheduleTransitions::kMetadataEntry },
        { HasAttribute(NumberOfScheduleTransitionPerDay::Id), NumberOfScheduleTransitionPerDay::kMetadataEntry },
        { HasAttribute(ActiveScheduleHandle::Id), ActiveScheduleHandle::kMetadataEntry },
        { HasAttribute(Schedules::Id), Schedules::kMetadataEntry },

        // Suggestions
        { HasAttribute(MaxThermostatSuggestions::Id), MaxThermostatSuggestions::kMetadataEntry },
        { HasAttribute(Attributes::ThermostatSuggestions::Id), Attributes::ThermostatSuggestions::kMetadataEntry },
        { HasAttribute(CurrentThermostatSuggestion::Id), CurrentThermostatSuggestion::kMetadataEntry },
        { HasAttribute(ThermostatSuggestionNotFollowingReason::Id), ThermostatSuggestionNotFollowingReason::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(Thermostat::Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

ControlSequenceOfOperationEnum ThermostatCluster::GetControlSequenceOfOperation() const
{
    return mDelegate.GetControlSequenceOfOperation();
}

Status ThermostatCluster::SetControlSequenceOfOperation(ControlSequenceOfOperationEnum controlSequenceOfOperation)
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

SystemModeEnum ThermostatCluster::GetSystemMode() const
{
    return mDelegate.GetSystemMode();
}

Status ThermostatCluster::SetSystemMode(SystemModeEnum systemMode)
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

DataModel::Nullable<temperature> ThermostatCluster::GetLocalTemperature() const
{
    return mDelegate.GetLocalTemperature();
}

Status ThermostatCluster::SetLocalTemperature(DataModel::Nullable<temperature> localTemperature,
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

int8_t ThermostatCluster::GetLocalTemperatureCalibration() const
{
    return mDelegate.GetLocalTemperatureCalibration();
}

Status ThermostatCluster::SetLocalTemperatureCalibration(int8_t localTemperatureCalibration)
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

ThermostatRunningModeEnum ThermostatCluster::GetRunningMode() const
{
    return mDelegate.GetRunningMode();
}

Status ThermostatCluster::SetRunningMode(ThermostatRunningModeEnum runningMode)
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

BitMask<RelayStateBitmap> ThermostatCluster::GetRunningState() const
{
    return mDelegate.GetRunningState();
}

Status ThermostatCluster::SetRunningState(BitMask<RelayStateBitmap> runningState)
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

void ThermostatCluster::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    if (mAtomicWriteSession.InAtomicWrite(fabricIndex))
    {
        mAtomicWriteSession.ResetAtomicWrite();
    }
}

CHIP_ERROR ThermostatCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (mFeatures.Has(Feature::kPresets))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::SetActivePresetRequest::kMetadataEntry }));
    }

    if (mFeatures.Has(Feature::kPresets) || mFeatures.Has(Feature::kMatterScheduleConfiguration))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::AtomicRequest::kMetadataEntry }));
    }

    if (mFeatures.Has(Feature::kThermostatSuggestions))
    {
        ReturnErrorOnFailure(builder.AppendElements(
            { Commands::AddThermostatSuggestion::kMetadataEntry, Commands::RemoveThermostatSuggestion::kMetadataEntry }));
    }

    return builder.AppendElements({
        Commands::SetpointRaiseLower::kMetadataEntry,
    });
}

CHIP_ERROR ThermostatCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    if (mFeatures.Has(Feature::kPresets) || mFeatures.Has(Feature::kMatterScheduleConfiguration))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::AtomicResponse::Id }));
    }
    if (mFeatures.Has(Feature::kThermostatSuggestions))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::AddThermostatSuggestionResponse::Id }));
    }
    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus> ThermostatCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                              chip::TLV::TLVReader & input_arguments,
                                                                              CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::SetpointRaiseLower::Id: {
        Commands::SetpointRaiseLower::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return SetpointRaiseLower(request_data);
    }
    case Commands::AtomicRequest::Id: {
        Commands::AtomicRequest::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));

        switch (request_data.requestType)
        {
        case Globals::AtomicRequestTypeEnum::kBeginWrite:
            return mAtomicWriteSession.BeginAtomicWrite(handler, request.path, request_data);
        case Globals::AtomicRequestTypeEnum::kCommitWrite:
            return mAtomicWriteSession.CommitAtomicWrite(handler, request.path, request_data);
        case Globals::AtomicRequestTypeEnum::kRollbackWrite:
            return mAtomicWriteSession.RollbackAtomicWrite(handler, request.path, request_data);
        default:
            return Status::InvalidCommand;
        }
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

Status ThermostatCluster::OnAtomicWriteBegin(AttributeId attributeId)
{
    return Status::Success;
}

Status ThermostatCluster::OnAtomicWritePrecommit(AttributeId attributeId)
{
    return Status::Success;
}

Status ThermostatCluster::OnAtomicWriteCommit(AttributeId attributeId)
{
    return Status::Success;
}

Status ThermostatCluster::OnAtomicWriteRollback(AttributeId attributeId)
{
    return Status::Success;
}

std::optional<System::Clock::Milliseconds16> ThermostatCluster::GetMaxAtomicWriteTimeout(chip::AttributeId attributeId)
{
    return std::nullopt;
}

void ThermostatCluster::OnAtomicWriteTimeout() {}

bool ThermostatCluster::IsActiveSetpoint(AttributeId attributeId) const
{
    if (IsOccupied())
    {
        return (attributeId == OccupiedHeatingSetpoint::Id || attributeId == OccupiedCoolingSetpoint::Id);
    }
    return (attributeId == UnoccupiedHeatingSetpoint::Id || attributeId == UnoccupiedCoolingSetpoint::Id);
}

bool ThermostatCluster::HasAttribute(AttributeId attributeId)
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
    default:
        return false;
    }
}

Setpoints ThermostatCluster::GetSetpoints()
{
    Setpoints setpoints;
    setpoints.autoSupported      = mFeatures.Has(Feature::kAutoMode);
    setpoints.heatSupported      = mFeatures.Has(Feature::kHeating);
    setpoints.coolSupported      = mFeatures.Has(Feature::kCooling);
    setpoints.occupancySupported = mFeatures.Has(Feature::kOccupancy);
    mDelegate.LoadSetpoints(setpoints);
    return setpoints;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
