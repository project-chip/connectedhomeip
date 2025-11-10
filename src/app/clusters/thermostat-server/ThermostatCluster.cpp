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
#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/server/Server.h>
#include <app/util/endpoint-config-api.h>
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

ThermostatCluster::ThermostatCluster(EndpointId endpointId, BitFlags<Thermostat::Feature> features) :
    DefaultServerCluster({ endpointId, Thermostat::Id }), mFeatures(features)
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
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    AttributePersistence persistence(context.attributeStorage);
    mSetpoints.LoadSetpointLimits(mPath.mEndpointId, persistence);

    Server::GetInstance().GetFabricTable().AddFabricDelegate(this);
    return CHIP_NO_ERROR;
}

void ThermostatCluster::Shutdown()
{
    DefaultServerCluster::Shutdown();
    Server::GetInstance().GetFabricTable().RemoveFabricDelegate(this);
    ChipLogProgress(Zcl, "Shutting down thermostat server cluster on endpoint %d", mPath.mEndpointId);
}

CHIP_ERROR ThermostatCluster::Attributes(const ConcreteClusterPath & path,
                                         ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    auto hasOccupancy = mFeatures.Has(Feature::kOccupancy);
    auto hasHeating   = mFeatures.Has(Feature::kHeating);
    auto hasCooling   = mFeatures.Has(Feature::kCooling);
    auto hasAuto      = mFeatures.Has(Feature::kAutoMode);

    ChipLogProgress(Zcl, "Fetching attributes hasOccupancy: %d, hasHeating: %d, hasCooling: %d, hasAuto: %d", hasOccupancy,
                    hasHeating, hasCooling, hasAuto);

    AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        // Setpoints
        { hasHeating, OccupiedHeatingSetpoint::kMetadataEntry },
        { hasCooling, OccupiedCoolingSetpoint::kMetadataEntry },
        { hasHeating && hasOccupancy, UnoccupiedHeatingSetpoint::kMetadataEntry },
        { hasCooling && hasOccupancy, UnoccupiedCoolingSetpoint::kMetadataEntry },

        // Setpoint Limits
        { hasHeating, AbsMinHeatSetpointLimit::kMetadataEntry },
        { hasHeating, AbsMaxHeatSetpointLimit::kMetadataEntry },
        { hasCooling, AbsMinCoolSetpointLimit::kMetadataEntry },
        { hasCooling, AbsMaxCoolSetpointLimit::kMetadataEntry },
        { hasHeating, MinHeatSetpointLimit::kMetadataEntry },
        { hasHeating, MaxHeatSetpointLimit::kMetadataEntry },
        { hasCooling, MinCoolSetpointLimit::kMetadataEntry },
        { hasCooling, MaxCoolSetpointLimit::kMetadataEntry },

        // Deadband
        { hasAuto, MinSetpointDeadBand::kMetadataEntry },

        // Feature-based State
        { hasOccupancy, Occupancy::kMetadataEntry },
        { hasAuto, ThermostatRunningMode::kMetadataEntry },

        // Other optional attributes
        { !mFeatures.Has(Feature::kLocalTemperatureNotExposed), LocalTemperatureCalibration::kMetadataEntry },
        { emberAfContainsAttribute(mPath.mEndpointId, Thermostat::Id, OutdoorTemperature::Id), OutdoorTemperature::kMetadataEntry },
        { emberAfContainsAttribute(mPath.mEndpointId, Thermostat::Id, RemoteSensing::Id), RemoteSensing::kMetadataEntry },
        { emberAfContainsAttribute(mPath.mEndpointId, Thermostat::Id, ThermostatRunningState::Id),
          ThermostatRunningState::kMetadataEntry },
        { emberAfContainsAttribute(mPath.mEndpointId, Thermostat::Id, SetpointChangeSource::Id),
          SetpointChangeSource::kMetadataEntry },
        { emberAfContainsAttribute(mPath.mEndpointId, Thermostat::Id, SetpointChangeAmount::Id),
          SetpointChangeAmount::kMetadataEntry },
        { emberAfContainsAttribute(mPath.mEndpointId, Thermostat::Id, SetpointChangeSourceTimestamp::Id),
          SetpointChangeSourceTimestamp::kMetadataEntry },
        { emberAfContainsAttribute(mPath.mEndpointId, Thermostat::Id, ThermostatProgrammingOperationMode::Id),
          ThermostatProgrammingOperationMode::kMetadataEntry },

        // Setpoint Holds
        { emberAfContainsAttribute(mPath.mEndpointId, Thermostat::Id, TemperatureSetpointHold::Id),
          TemperatureSetpointHold::kMetadataEntry },
        { emberAfContainsAttribute(mPath.mEndpointId, Thermostat::Id, TemperatureSetpointHoldDuration::Id),
          TemperatureSetpointHoldDuration::kMetadataEntry },
        { emberAfContainsAttribute(mPath.mEndpointId, Thermostat::Id, SetpointHoldExpiryTimestamp::Id),
          SetpointHoldExpiryTimestamp::kMetadataEntry },

        // Presets
        { mFeatures.Has(Feature::kPresets), PresetTypes::kMetadataEntry },
        { mFeatures.Has(Feature::kPresets), NumberOfPresets::kMetadataEntry },
        { mFeatures.Has(Feature::kPresets), ActivePresetHandle::kMetadataEntry },
        { mFeatures.Has(Feature::kPresets), Presets::kMetadataEntry },

        // Schedules
        { mFeatures.Has(Feature::kMatterScheduleConfiguration), ScheduleTypes::kMetadataEntry },
        { mFeatures.Has(Feature::kMatterScheduleConfiguration), NumberOfSchedules::kMetadataEntry },
        { mFeatures.Has(Feature::kMatterScheduleConfiguration), NumberOfScheduleTransitions::kMetadataEntry },
        { mFeatures.Has(Feature::kMatterScheduleConfiguration), NumberOfScheduleTransitionPerDay::kMetadataEntry },
        { mFeatures.Has(Feature::kMatterScheduleConfiguration), ActiveScheduleHandle::kMetadataEntry },
        { mFeatures.Has(Feature::kMatterScheduleConfiguration), Schedules::kMetadataEntry },

        // Suggestions
        { mFeatures.Has(Feature::kThermostatSuggestions), MaxThermostatSuggestions::kMetadataEntry },
        { mFeatures.Has(Feature::kThermostatSuggestions), ThermostatSuggestions::kMetadataEntry },
        { mFeatures.Has(Feature::kThermostatSuggestions), CurrentThermostatSuggestion::kMetadataEntry },
        { mFeatures.Has(Feature::kThermostatSuggestions), ThermostatSuggestionNotFollowingReason::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(Thermostat::Attributes::kMandatoryMetadata), Span(optionalAttributes));
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

        bool occupied = true;
        if (mFeatures.Has(Feature::kOccupancy))
        {
            occupied = mOccupancy.Has(OccupancyBitmap::kOccupied);
        }

        return mSetpoints.RaiseLowerSetpoint(request_data.amount, request_data.mode, mFeatures, occupied,
                                             mContext->attributeStorage);
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
        case Globals::AtomicRequestTypeEnum::kUnknownEnumValue:
            return Status::InvalidCommand;
        }
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
        return status.GetStatus();
    }
    default:
        break;
    }
    return Status::Success;
}

Status ThermostatCluster::OnAtomicWriteRollback(AttributeId attributeId)
{
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

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
