// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Thermostat (cluster code: 513/0x201)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Thermostat/ThermostatIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace Thermostat {
namespace Metadata {

inline constexpr uint32_t kRevision = 7;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kLocalTemperatureEntry = {
    .attributeId    = Thermostat::Attributes::LocalTemperature::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOutdoorTemperatureEntry = {
    .attributeId    = Thermostat::Attributes::OutdoorTemperature::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOccupancyEntry = {
    .attributeId    = Thermostat::Attributes::Occupancy::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAbsMinHeatSetpointLimitEntry = {
    .attributeId    = Thermostat::Attributes::AbsMinHeatSetpointLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAbsMaxHeatSetpointLimitEntry = {
    .attributeId    = Thermostat::Attributes::AbsMaxHeatSetpointLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAbsMinCoolSetpointLimitEntry = {
    .attributeId    = Thermostat::Attributes::AbsMinCoolSetpointLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kAbsMaxCoolSetpointLimitEntry = {
    .attributeId    = Thermostat::Attributes::AbsMaxCoolSetpointLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPICoolingDemandEntry = {
    .attributeId    = Thermostat::Attributes::PICoolingDemand::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPIHeatingDemandEntry = {
    .attributeId    = Thermostat::Attributes::PIHeatingDemand::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kHVACSystemTypeConfigurationEntry = {
    .attributeId    = Thermostat::Attributes::HVACSystemTypeConfiguration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kLocalTemperatureCalibrationEntry = {
    .attributeId    = Thermostat::Attributes::LocalTemperatureCalibration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kOccupiedCoolingSetpointEntry = {
    .attributeId    = Thermostat::Attributes::OccupiedCoolingSetpoint::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kOccupiedHeatingSetpointEntry = {
    .attributeId    = Thermostat::Attributes::OccupiedHeatingSetpoint::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kUnoccupiedCoolingSetpointEntry = {
    .attributeId    = Thermostat::Attributes::UnoccupiedCoolingSetpoint::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kUnoccupiedHeatingSetpointEntry = {
    .attributeId    = Thermostat::Attributes::UnoccupiedHeatingSetpoint::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kMinHeatSetpointLimitEntry = {
    .attributeId    = Thermostat::Attributes::MinHeatSetpointLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kMaxHeatSetpointLimitEntry = {
    .attributeId    = Thermostat::Attributes::MaxHeatSetpointLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kMinCoolSetpointLimitEntry = {
    .attributeId    = Thermostat::Attributes::MinCoolSetpointLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kMaxCoolSetpointLimitEntry = {
    .attributeId    = Thermostat::Attributes::MaxCoolSetpointLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kMinSetpointDeadBandEntry = {
    .attributeId    = Thermostat::Attributes::MinSetpointDeadBand::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kRemoteSensingEntry = {
    .attributeId    = Thermostat::Attributes::RemoteSensing::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kControlSequenceOfOperationEntry = {
    .attributeId    = Thermostat::Attributes::ControlSequenceOfOperation::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kSystemModeEntry = {
    .attributeId    = Thermostat::Attributes::SystemMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kThermostatRunningModeEntry = {
    .attributeId    = Thermostat::Attributes::ThermostatRunningMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kStartOfWeekEntry = {
    .attributeId    = Thermostat::Attributes::StartOfWeek::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfWeeklyTransitionsEntry = {
    .attributeId    = Thermostat::Attributes::NumberOfWeeklyTransitions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfDailyTransitionsEntry = {
    .attributeId    = Thermostat::Attributes::NumberOfDailyTransitions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTemperatureSetpointHoldEntry = {
    .attributeId    = Thermostat::Attributes::TemperatureSetpointHold::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kTemperatureSetpointHoldDurationEntry = {
    .attributeId    = Thermostat::Attributes::TemperatureSetpointHoldDuration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kThermostatProgrammingOperationModeEntry = {
    .attributeId    = Thermostat::Attributes::ThermostatProgrammingOperationMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kThermostatRunningStateEntry = {
    .attributeId    = Thermostat::Attributes::ThermostatRunningState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSetpointChangeSourceEntry = {
    .attributeId    = Thermostat::Attributes::SetpointChangeSource::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSetpointChangeAmountEntry = {
    .attributeId    = Thermostat::Attributes::SetpointChangeAmount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSetpointChangeSourceTimestampEntry = {
    .attributeId    = Thermostat::Attributes::SetpointChangeSourceTimestamp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOccupiedSetbackEntry = {
    .attributeId    = Thermostat::Attributes::OccupiedSetback::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kOccupiedSetbackMinEntry = {
    .attributeId    = Thermostat::Attributes::OccupiedSetbackMin::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOccupiedSetbackMaxEntry = {
    .attributeId    = Thermostat::Attributes::OccupiedSetbackMax::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kUnoccupiedSetbackEntry = {
    .attributeId    = Thermostat::Attributes::UnoccupiedSetback::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kUnoccupiedSetbackMinEntry = {
    .attributeId    = Thermostat::Attributes::UnoccupiedSetbackMin::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kUnoccupiedSetbackMaxEntry = {
    .attributeId    = Thermostat::Attributes::UnoccupiedSetbackMax::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kEmergencyHeatDeltaEntry = {
    .attributeId    = Thermostat::Attributes::EmergencyHeatDelta::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kACTypeEntry = {
    .attributeId    = Thermostat::Attributes::ACType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kACCapacityEntry = {
    .attributeId    = Thermostat::Attributes::ACCapacity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kACRefrigerantTypeEntry = {
    .attributeId    = Thermostat::Attributes::ACRefrigerantType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kACCompressorTypeEntry = {
    .attributeId    = Thermostat::Attributes::ACCompressorType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kACErrorCodeEntry = {
    .attributeId    = Thermostat::Attributes::ACErrorCode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kACLouverPositionEntry = {
    .attributeId    = Thermostat::Attributes::ACLouverPosition::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kACCoilTemperatureEntry = {
    .attributeId    = Thermostat::Attributes::ACCoilTemperature::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kACCapacityformatEntry = {
    .attributeId    = Thermostat::Attributes::ACCapacityformat::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kPresetTypesEntry = {
    .attributeId    = Thermostat::Attributes::PresetTypes::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kScheduleTypesEntry = {
    .attributeId    = Thermostat::Attributes::ScheduleTypes::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfPresetsEntry = {
    .attributeId    = Thermostat::Attributes::NumberOfPresets::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfSchedulesEntry = {
    .attributeId    = Thermostat::Attributes::NumberOfSchedules::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfScheduleTransitionsEntry = {
    .attributeId    = Thermostat::Attributes::NumberOfScheduleTransitions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNumberOfScheduleTransitionPerDayEntry = {
    .attributeId    = Thermostat::Attributes::NumberOfScheduleTransitionPerDay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kActivePresetHandleEntry = {
    .attributeId    = Thermostat::Attributes::ActivePresetHandle::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kActiveScheduleHandleEntry = {
    .attributeId    = Thermostat::Attributes::ActiveScheduleHandle::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPresetsEntry = {
    .attributeId    = Thermostat::Attributes::Presets::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kSchedulesEntry = {
    .attributeId    = Thermostat::Attributes::Schedules::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kSetpointHoldExpiryTimestampEntry = {
    .attributeId    = Thermostat::Attributes::SetpointHoldExpiryTimestamp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kSetpointRaiseLowerEntry = {
    .commandId       = Thermostat::Commands::SetpointRaiseLower::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kSetWeeklyScheduleEntry = {
    .commandId       = Thermostat::Commands::SetWeeklySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kGetWeeklyScheduleEntry = {
    .commandId       = Thermostat::Commands::GetWeeklySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kClearWeeklyScheduleEntry = {
    .commandId       = Thermostat::Commands::ClearWeeklySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kSetActiveScheduleRequestEntry = {
    .commandId       = Thermostat::Commands::SetActiveScheduleRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kSetActivePresetRequestEntry = {
    .commandId       = Thermostat::Commands::SetActivePresetRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kAtomicRequestEntry = {
    .commandId       = Thermostat::Commands::AtomicRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};

} // namespace Commands
} // namespace Metadata
} // namespace Thermostat
} // namespace clusters
} // namespace app
} // namespace chip
