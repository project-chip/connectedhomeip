/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include "ThermostatClusterSetpoints.h"

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
#include <app/server/Server.h>
#include <clusters/Thermostat/Metadata.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

ThermostatCluster::ThermostatCluster(EndpointId endpointId, BitFlags<Thermostat::Feature> features,
                                     const OptionalAttributes & optionalAttributes) :
    DefaultServerCluster({ endpointId, Thermostat::Id }),
    mFeatures(features), mOptionalAttributes(optionalAttributes)
{
    auto hasHeating = mFeatures.Has(Feature::kHeating);
    auto hasCooling = mFeatures.Has(Feature::kCooling);
    if (hasHeating && hasCooling)
    {
        mControlSequenceOfOperation = ControlSequenceOfOperationEnum::kCoolingAndHeating;
    }
    else if (hasHeating)
    {
        mControlSequenceOfOperation = ControlSequenceOfOperationEnum::kHeatingOnly;
    }
    else if (hasCooling)
    {
        mControlSequenceOfOperation = ControlSequenceOfOperationEnum::kCoolingOnly;
    }
    mAtomicWriteSession.SetDelegate(this);
}

CHIP_ERROR ThermostatCluster::Startup(ServerClusterContext & context)
{
    ChipLogProgress(Zcl, "Starting up thermostat server cluster on endpoint %d", mPath.mEndpointId);
    AttributePersistence persistence(context.attributeStorage);
    LoadSetpoints(mSetpoints, persistence);

    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    ReturnErrorOnFailure(Server::GetInstance().GetFabricTable().AddFabricDelegate(this));
    return CHIP_NO_ERROR;
}

void ThermostatCluster::Shutdown(ClusterShutdownType type)
{
    mAtomicWriteSession.SetDelegate(nullptr);
    DefaultServerCluster::Shutdown(type);
    TEMPORARY_RETURN_IGNORED Server::GetInstance().GetFabricTable().RemoveFabricDelegate(this);
    ChipLogProgress(Zcl, "Shutting down thermostat server cluster on endpoint %d", mPath.mEndpointId);
}

CHIP_ERROR ThermostatCluster::Attributes(const ConcreteClusterPath & path,
                                         ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{

    ChipLogProgress(Zcl, "Fetching attributes hasOccupancy: %d, hasHeating: %d, hasCooling: %d, hasAuto: %d",
                    mFeatures.Has(Feature::kOccupancy), mFeatures.Has(Feature::kHeating), mFeatures.Has(Feature::kCooling),
                    mFeatures.Has(Feature::kAutoMode));

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
        { HasAttribute(ThermostatProgrammingOperationMode::Id), ThermostatProgrammingOperationMode::kMetadataEntry },

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
        { HasAttribute(ThermostatSuggestions::Id), ThermostatSuggestions::kMetadataEntry },
        { HasAttribute(CurrentThermostatSuggestion::Id), CurrentThermostatSuggestion::kMetadataEntry },
        { HasAttribute(ThermostatSuggestionNotFollowingReason::Id), ThermostatSuggestionNotFollowingReason::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(Thermostat::Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

Status ThermostatCluster::SetControlSequenceOfOperation(ControlSequenceOfOperationEnum controlSequenceOfOperation)
{
    if (!SetAttributeValue(mControlSequenceOfOperation, controlSequenceOfOperation,
                           app::Clusters::Thermostat::Attributes::ControlSequenceOfOperation::Id))
    {
        return Status::Failure;
    }
    return Status::Success;
}

Status ThermostatCluster::SetSystemMode(SystemModeEnum systemMode)
{
    switch (systemMode)
    {
    case SystemModeEnum::kOff:
        break;
    case SystemModeEnum::kAuto:
        if (!mFeatures.Has(Feature::kAutoMode))
            return Status::Failure;
        break;
    case SystemModeEnum::kCool:
        if (!mFeatures.Has(Feature::kCooling))
            return Status::Failure;
        break;
    case SystemModeEnum::kHeat:
        if (!mFeatures.Has(Feature::kHeating))
            return Status::Failure;
        break;
    case SystemModeEnum::kEmergencyHeat:
        if (!mFeatures.Has(Feature::kHeating))
            return Status::Failure;
        break;
    case SystemModeEnum::kPrecooling:
        if (!mFeatures.Has(Feature::kCooling))
            return Status::Failure;
        break;
    case SystemModeEnum::kFanOnly:
    case SystemModeEnum::kDry:
    case SystemModeEnum::kSleep:
        break;
    default:
        return Status::InvalidValue;
    }

    if (!SetAttributeValue(mSystemMode, systemMode, app::Clusters::Thermostat::Attributes::SystemMode::Id))
    {
        return Status::Failure;
    }
    return Status::Success;
}

Status ThermostatCluster::SetLocalTemperature(DataModel::Nullable<int16_t> localTemperature,
                                              DataModel::AttributeChangeType changeType)
{
    SetAttributeValue(mLocalTemperature, localTemperature, app::Clusters::Thermostat::Attributes::LocalTemperature::Id,
                      changeType);
    return Status::Success;
}

Status ThermostatCluster::SetRunningMode(ThermostatRunningModeEnum runningMode)
{
    if (!SetAttributeValue(mRunningMode, runningMode, app::Clusters::Thermostat::Attributes::ThermostatRunningMode::Id))
    {
        return Status::Failure;
    }
    return Status::Success;
}

Status ThermostatCluster::SetRunningState(BitMask<RelayStateBitmap> runningState)
{
    if (!SetAttributeValue(mRunningState, runningState, app::Clusters::Thermostat::Attributes::ThermostatRunningState::Id))
    {
        return Status::Failure;
    }
    return Status::Success;
}

void ThermostatCluster::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    mAtomicWriteSession.ResetAtomicWrite();
}

CHIP_ERROR ThermostatCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (mFeatures.Has(Feature::kMatterScheduleConfiguration))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::SetActiveScheduleRequest::kMetadataEntry }));
    }

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
    case Commands::SetActivePresetRequest::Id: {
        Commands::SetActivePresetRequest::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return SetActivePreset(request_data.presetHandle);
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
    case Commands::AddThermostatSuggestion::Id: {
        Commands::AddThermostatSuggestion::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return AddThermostatSuggestion(handler, request.path, request_data);
    }
    case Commands::RemoveThermostatSuggestion::Id: {
        Commands::RemoveThermostatSuggestion::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return RemoveThermostatSuggestion(handler, request.path, request_data);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

Status ThermostatCluster::OnAtomicWriteBegin(AttributeId attributeId)
{
    switch (attributeId)
    {
    case Presets::Id:
        if (mDelegate == nullptr)
        {
            return Status::InvalidInState;
        }
        mDelegate->InitializePendingPresets();
        break;
    default:
        break;
    }
    return Status::Success;
}

Status ThermostatCluster::OnAtomicWritePrecommit(AttributeId attributeId)
{
    switch (attributeId)
    {
    case Presets::Id:
        return PrecommitPresets();
    default:
        break;
    }
    return Status::Success;
}

Status ThermostatCluster::OnAtomicWriteCommit(AttributeId attributeId)
{
    switch (attributeId)
    {
    case Presets::Id: {
        if (mDelegate == nullptr)
        {
            return Status::InvalidInState;
        }
        ClusterStatusCode status(mDelegate->CommitPendingPresets());
        if (status.IsSuccess())
        {
            NotifyAttributeChanged(attributeId);
        }
        return status.GetStatus();
    }
    default:
        break;
    }
    return Status::Success;
}

Status ThermostatCluster::OnAtomicWriteRollback(AttributeId attributeId)
{
    switch (attributeId)
    {
    case Presets::Id:
        if (mDelegate == nullptr)
        {
            return Status::InvalidInState;
        }
        mDelegate->ClearPendingPresetList();
        break;
    default:
        break;
    }
    return Status::Success;
}

std::optional<System::Clock::Milliseconds16> ThermostatCluster::GetMaxAtomicWriteTimeout(chip::AttributeId attributeId)
{
    if (mDelegate == nullptr)
    {
        return std::nullopt;
    }
    return mDelegate->GetMaxAtomicWriteTimeout(attributeId);
}

void ThermostatCluster::OnAtomicWriteTimeout() {}

bool ThermostatCluster::HasAttribute(AttributeId attributeId)
{
    switch (attributeId)
    {
    case LocalTemperature::Id:
    case ControlSequenceOfOperation::Id:
    case SystemMode::Id:
        return true;
    case OutdoorTemperature::Id:
        return mOptionalAttributes.OutdoorTemperature;
    case Occupancy::Id:
        return mFeatures.Has(Feature::kOccupancy);
    case AbsMinHeatSetpointLimit::Id:
        return mOptionalAttributes.AbsMinHeatSetpointLimit;
    case AbsMaxHeatSetpointLimit::Id:
        return mOptionalAttributes.AbsMaxHeatSetpointLimit;
    case AbsMinCoolSetpointLimit::Id:
        return mOptionalAttributes.AbsMinCoolSetpointLimit;
    case AbsMaxCoolSetpointLimit::Id:
        return mOptionalAttributes.AbsMaxCoolSetpointLimit;
    case LocalTemperatureCalibration::Id:
        return mOptionalAttributes.LocalTemperatureCalibration;
    case OccupiedCoolingSetpoint::Id:
        return mFeatures.Has(Feature::kCooling);
    case OccupiedHeatingSetpoint::Id:
        return mFeatures.Has(Feature::kHeating);
    case UnoccupiedHeatingSetpoint::Id:
        return mFeatures.Has(Feature::kHeating) && mFeatures.Has(Feature::kOccupancy);
    case UnoccupiedCoolingSetpoint::Id:
        return mFeatures.Has(Feature::kCooling) && mFeatures.Has(Feature::kOccupancy);
    case MinHeatSetpointLimit::Id:
        return mOptionalAttributes.MinHeatSetpointLimit;
    case MaxHeatSetpointLimit::Id:
        return mOptionalAttributes.MaxHeatSetpointLimit;
    case MinCoolSetpointLimit::Id:
        return mOptionalAttributes.MinCoolSetpointLimit;
    case MaxCoolSetpointLimit::Id:
        return mOptionalAttributes.MaxCoolSetpointLimit;
    case MinSetpointDeadBand::Id:
        return mFeatures.Has(Feature::kAutoMode);
    case RemoteSensing::Id:
        return mOptionalAttributes.RemoteSensing;
    case ThermostatRunningMode::Id:
        return mOptionalAttributes.ThermostatRunningMode;
    case TemperatureSetpointHold::Id:
        return mOptionalAttributes.TemperatureSetpointHold;
    case TemperatureSetpointHoldDuration::Id:
        return mOptionalAttributes.TemperatureSetpointHoldDuration;
    case ThermostatRunningState::Id:
        return mOptionalAttributes.ThermostatRunningState;
    case SetpointChangeSource::Id:
        return mOptionalAttributes.SetpointChangeSource;
    case SetpointChangeAmount::Id:
        return mOptionalAttributes.SetpointChangeAmount;
    case SetpointChangeSourceTimestamp::Id:
        return mOptionalAttributes.SetpointChangeSourceTimestamp;
    case SetpointHoldExpiryTimestamp::Id:
        return mOptionalAttributes.SetpointHoldExpiryTimestamp;
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
    case ThermostatSuggestions::Id:
    case CurrentThermostatSuggestion::Id:
    case ThermostatSuggestionNotFollowingReason::Id:
        return mFeatures.Has(Feature::kThermostatSuggestions);
    default:
        return false;
    }
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
