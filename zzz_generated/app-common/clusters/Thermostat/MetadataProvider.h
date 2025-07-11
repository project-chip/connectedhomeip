// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Thermostat (cluster code: 513/0x201)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/Thermostat/Ids.h>
#include <clusters/Thermostat/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::Thermostat::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::Thermostat::Attributes;
        switch (attributeId)
        {
        case LocalTemperature::Id:
            return LocalTemperature::kMetadataEntry;
        case OutdoorTemperature::Id:
            return OutdoorTemperature::kMetadataEntry;
        case Occupancy::Id:
            return Occupancy::kMetadataEntry;
        case AbsMinHeatSetpointLimit::Id:
            return AbsMinHeatSetpointLimit::kMetadataEntry;
        case AbsMaxHeatSetpointLimit::Id:
            return AbsMaxHeatSetpointLimit::kMetadataEntry;
        case AbsMinCoolSetpointLimit::Id:
            return AbsMinCoolSetpointLimit::kMetadataEntry;
        case AbsMaxCoolSetpointLimit::Id:
            return AbsMaxCoolSetpointLimit::kMetadataEntry;
        case PICoolingDemand::Id:
            return PICoolingDemand::kMetadataEntry;
        case PIHeatingDemand::Id:
            return PIHeatingDemand::kMetadataEntry;
        case HVACSystemTypeConfiguration::Id:
            return HVACSystemTypeConfiguration::kMetadataEntry;
        case LocalTemperatureCalibration::Id:
            return LocalTemperatureCalibration::kMetadataEntry;
        case OccupiedCoolingSetpoint::Id:
            return OccupiedCoolingSetpoint::kMetadataEntry;
        case OccupiedHeatingSetpoint::Id:
            return OccupiedHeatingSetpoint::kMetadataEntry;
        case UnoccupiedCoolingSetpoint::Id:
            return UnoccupiedCoolingSetpoint::kMetadataEntry;
        case UnoccupiedHeatingSetpoint::Id:
            return UnoccupiedHeatingSetpoint::kMetadataEntry;
        case MinHeatSetpointLimit::Id:
            return MinHeatSetpointLimit::kMetadataEntry;
        case MaxHeatSetpointLimit::Id:
            return MaxHeatSetpointLimit::kMetadataEntry;
        case MinCoolSetpointLimit::Id:
            return MinCoolSetpointLimit::kMetadataEntry;
        case MaxCoolSetpointLimit::Id:
            return MaxCoolSetpointLimit::kMetadataEntry;
        case MinSetpointDeadBand::Id:
            return MinSetpointDeadBand::kMetadataEntry;
        case RemoteSensing::Id:
            return RemoteSensing::kMetadataEntry;
        case ControlSequenceOfOperation::Id:
            return ControlSequenceOfOperation::kMetadataEntry;
        case SystemMode::Id:
            return SystemMode::kMetadataEntry;
        case ThermostatRunningMode::Id:
            return ThermostatRunningMode::kMetadataEntry;
        case StartOfWeek::Id:
            return StartOfWeek::kMetadataEntry;
        case NumberOfWeeklyTransitions::Id:
            return NumberOfWeeklyTransitions::kMetadataEntry;
        case NumberOfDailyTransitions::Id:
            return NumberOfDailyTransitions::kMetadataEntry;
        case TemperatureSetpointHold::Id:
            return TemperatureSetpointHold::kMetadataEntry;
        case TemperatureSetpointHoldDuration::Id:
            return TemperatureSetpointHoldDuration::kMetadataEntry;
        case ThermostatProgrammingOperationMode::Id:
            return ThermostatProgrammingOperationMode::kMetadataEntry;
        case ThermostatRunningState::Id:
            return ThermostatRunningState::kMetadataEntry;
        case SetpointChangeSource::Id:
            return SetpointChangeSource::kMetadataEntry;
        case SetpointChangeAmount::Id:
            return SetpointChangeAmount::kMetadataEntry;
        case SetpointChangeSourceTimestamp::Id:
            return SetpointChangeSourceTimestamp::kMetadataEntry;
        case OccupiedSetback::Id:
            return OccupiedSetback::kMetadataEntry;
        case OccupiedSetbackMin::Id:
            return OccupiedSetbackMin::kMetadataEntry;
        case OccupiedSetbackMax::Id:
            return OccupiedSetbackMax::kMetadataEntry;
        case UnoccupiedSetback::Id:
            return UnoccupiedSetback::kMetadataEntry;
        case UnoccupiedSetbackMin::Id:
            return UnoccupiedSetbackMin::kMetadataEntry;
        case UnoccupiedSetbackMax::Id:
            return UnoccupiedSetbackMax::kMetadataEntry;
        case EmergencyHeatDelta::Id:
            return EmergencyHeatDelta::kMetadataEntry;
        case ACType::Id:
            return ACType::kMetadataEntry;
        case ACCapacity::Id:
            return ACCapacity::kMetadataEntry;
        case ACRefrigerantType::Id:
            return ACRefrigerantType::kMetadataEntry;
        case ACCompressorType::Id:
            return ACCompressorType::kMetadataEntry;
        case ACErrorCode::Id:
            return ACErrorCode::kMetadataEntry;
        case ACLouverPosition::Id:
            return ACLouverPosition::kMetadataEntry;
        case ACCoilTemperature::Id:
            return ACCoilTemperature::kMetadataEntry;
        case ACCapacityformat::Id:
            return ACCapacityformat::kMetadataEntry;
        case PresetTypes::Id:
            return PresetTypes::kMetadataEntry;
        case ScheduleTypes::Id:
            return ScheduleTypes::kMetadataEntry;
        case NumberOfPresets::Id:
            return NumberOfPresets::kMetadataEntry;
        case NumberOfSchedules::Id:
            return NumberOfSchedules::kMetadataEntry;
        case NumberOfScheduleTransitions::Id:
            return NumberOfScheduleTransitions::kMetadataEntry;
        case NumberOfScheduleTransitionPerDay::Id:
            return NumberOfScheduleTransitionPerDay::kMetadataEntry;
        case ActivePresetHandle::Id:
            return ActivePresetHandle::kMetadataEntry;
        case ActiveScheduleHandle::Id:
            return ActiveScheduleHandle::kMetadataEntry;
        case Presets::Id:
            return Presets::kMetadataEntry;
        case Schedules::Id:
            return Schedules::kMetadataEntry;
        case SetpointHoldExpiryTimestamp::Id:
            return SetpointHoldExpiryTimestamp::kMetadataEntry;
        case MaxThermostatSuggestions::Id:
            return MaxThermostatSuggestions::kMetadataEntry;
        case ThermostatSuggestions::Id:
            return ThermostatSuggestions::kMetadataEntry;
        case CurrentThermostatSuggestion::Id:
            return CurrentThermostatSuggestion::kMetadataEntry;
        case ThermostatSuggestionNotFollowingReason::Id:
            return ThermostatSuggestionNotFollowingReason::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::Thermostat::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::Thermostat::Commands;
        switch (commandId)
        {
        case SetpointRaiseLower::Id:
            return SetpointRaiseLower::kMetadataEntry;
        case SetWeeklySchedule::Id:
            return SetWeeklySchedule::kMetadataEntry;
        case GetWeeklySchedule::Id:
            return GetWeeklySchedule::kMetadataEntry;
        case ClearWeeklySchedule::Id:
            return ClearWeeklySchedule::kMetadataEntry;
        case SetActiveScheduleRequest::Id:
            return SetActiveScheduleRequest::kMetadataEntry;
        case SetActivePresetRequest::Id:
            return SetActivePresetRequest::kMetadataEntry;
        case AddThermostatSuggestion::Id:
            return AddThermostatSuggestion::kMetadataEntry;
        case RemoveThermostatSuggestion::Id:
            return RemoveThermostatSuggestion::kMetadataEntry;
        case AtomicRequest::Id:
            return AtomicRequest::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
