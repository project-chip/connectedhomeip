// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Thermostat (cluster code: 513/0x201)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Thermostat/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

inline constexpr uint32_t kRevision = 7;

namespace Attributes {
namespace LocalTemperature {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LocalTemperature::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LocalTemperature
namespace OutdoorTemperature {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OutdoorTemperature::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OutdoorTemperature
namespace Occupancy {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Occupancy::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Occupancy
namespace AbsMinHeatSetpointLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AbsMinHeatSetpointLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AbsMinHeatSetpointLimit
namespace AbsMaxHeatSetpointLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AbsMaxHeatSetpointLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AbsMaxHeatSetpointLimit
namespace AbsMinCoolSetpointLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AbsMinCoolSetpointLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AbsMinCoolSetpointLimit
namespace AbsMaxCoolSetpointLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AbsMaxCoolSetpointLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace AbsMaxCoolSetpointLimit
namespace PICoolingDemand {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PICoolingDemand::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PICoolingDemand
namespace PIHeatingDemand {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PIHeatingDemand::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PIHeatingDemand
namespace HVACSystemTypeConfiguration {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = HVACSystemTypeConfiguration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace HVACSystemTypeConfiguration
namespace LocalTemperatureCalibration {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LocalTemperatureCalibration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace LocalTemperatureCalibration
namespace OccupiedCoolingSetpoint {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OccupiedCoolingSetpoint::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace OccupiedCoolingSetpoint
namespace OccupiedHeatingSetpoint {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OccupiedHeatingSetpoint::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace OccupiedHeatingSetpoint
namespace UnoccupiedCoolingSetpoint {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = UnoccupiedCoolingSetpoint::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace UnoccupiedCoolingSetpoint
namespace UnoccupiedHeatingSetpoint {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = UnoccupiedHeatingSetpoint::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace UnoccupiedHeatingSetpoint
namespace MinHeatSetpointLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MinHeatSetpointLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace MinHeatSetpointLimit
namespace MaxHeatSetpointLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxHeatSetpointLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace MaxHeatSetpointLimit
namespace MinCoolSetpointLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MinCoolSetpointLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace MinCoolSetpointLimit
namespace MaxCoolSetpointLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxCoolSetpointLimit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace MaxCoolSetpointLimit
namespace MinSetpointDeadBand {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MinSetpointDeadBand::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace MinSetpointDeadBand
namespace RemoteSensing {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RemoteSensing::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace RemoteSensing
namespace ControlSequenceOfOperation {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ControlSequenceOfOperation::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ControlSequenceOfOperation
namespace SystemMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SystemMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace SystemMode
namespace ThermostatRunningMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThermostatRunningMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ThermostatRunningMode
namespace StartOfWeek {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = StartOfWeek::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace StartOfWeek
namespace NumberOfWeeklyTransitions {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfWeeklyTransitions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfWeeklyTransitions
namespace NumberOfDailyTransitions {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfDailyTransitions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfDailyTransitions
namespace TemperatureSetpointHold {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TemperatureSetpointHold::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace TemperatureSetpointHold
namespace TemperatureSetpointHoldDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TemperatureSetpointHoldDuration::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace TemperatureSetpointHoldDuration
namespace ThermostatProgrammingOperationMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThermostatProgrammingOperationMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ThermostatProgrammingOperationMode
namespace ThermostatRunningState {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ThermostatRunningState::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ThermostatRunningState
namespace SetpointChangeSource {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SetpointChangeSource::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SetpointChangeSource
namespace SetpointChangeAmount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SetpointChangeAmount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SetpointChangeAmount
namespace SetpointChangeSourceTimestamp {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SetpointChangeSourceTimestamp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SetpointChangeSourceTimestamp
namespace OccupiedSetback {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OccupiedSetback::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace OccupiedSetback
namespace OccupiedSetbackMin {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OccupiedSetbackMin::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OccupiedSetbackMin
namespace OccupiedSetbackMax {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OccupiedSetbackMax::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OccupiedSetbackMax
namespace UnoccupiedSetback {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = UnoccupiedSetback::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace UnoccupiedSetback
namespace UnoccupiedSetbackMin {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = UnoccupiedSetbackMin::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace UnoccupiedSetbackMin
namespace UnoccupiedSetbackMax {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = UnoccupiedSetbackMax::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace UnoccupiedSetbackMax
namespace EmergencyHeatDelta {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EmergencyHeatDelta::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace EmergencyHeatDelta
namespace ACType {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ACType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ACType
namespace ACCapacity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ACCapacity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ACCapacity
namespace ACRefrigerantType {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ACRefrigerantType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ACRefrigerantType
namespace ACCompressorType {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ACCompressorType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ACCompressorType
namespace ACErrorCode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ACErrorCode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ACErrorCode
namespace ACLouverPosition {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ACLouverPosition::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ACLouverPosition
namespace ACCoilTemperature {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ACCoilTemperature::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ACCoilTemperature
namespace ACCapacityformat {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ACCapacityformat::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ACCapacityformat
namespace PresetTypes {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PresetTypes::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PresetTypes
namespace ScheduleTypes {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ScheduleTypes::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ScheduleTypes
namespace NumberOfPresets {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfPresets::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfPresets
namespace NumberOfSchedules {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfSchedules::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfSchedules
namespace NumberOfScheduleTransitions {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfScheduleTransitions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfScheduleTransitions
namespace NumberOfScheduleTransitionPerDay {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = NumberOfScheduleTransitionPerDay::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NumberOfScheduleTransitionPerDay
namespace ActivePresetHandle {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActivePresetHandle::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActivePresetHandle
namespace ActiveScheduleHandle {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ActiveScheduleHandle::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ActiveScheduleHandle
namespace Presets {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Presets::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace Presets
namespace Schedules {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Schedules::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace Schedules
namespace SetpointHoldExpiryTimestamp {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SetpointHoldExpiryTimestamp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SetpointHoldExpiryTimestamp

} // namespace Attributes

namespace Commands {
namespace SetpointRaiseLower {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetpointRaiseLower::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SetpointRaiseLower
namespace SetWeeklySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetWeeklySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace SetWeeklySchedule
namespace GetWeeklySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = GetWeeklySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace GetWeeklySchedule
namespace ClearWeeklySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ClearWeeklySchedule::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace ClearWeeklySchedule
namespace SetActiveScheduleRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetActiveScheduleRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SetActiveScheduleRequest
namespace SetActivePresetRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = SetActivePresetRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace SetActivePresetRequest
namespace AtomicRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = AtomicRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace AtomicRequest

} // namespace Commands
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
