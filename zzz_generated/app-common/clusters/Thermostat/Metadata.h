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
inline constexpr DataModel::AttributeEntry kMetadataEntry(LocalTemperature::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace LocalTemperature
namespace OutdoorTemperature {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OutdoorTemperature::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OutdoorTemperature
namespace Occupancy {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Occupancy::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Occupancy
namespace AbsMinHeatSetpointLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AbsMinHeatSetpointLimit::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AbsMinHeatSetpointLimit
namespace AbsMaxHeatSetpointLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AbsMaxHeatSetpointLimit::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AbsMaxHeatSetpointLimit
namespace AbsMinCoolSetpointLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AbsMinCoolSetpointLimit::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AbsMinCoolSetpointLimit
namespace AbsMaxCoolSetpointLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AbsMaxCoolSetpointLimit::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AbsMaxCoolSetpointLimit
namespace PICoolingDemand {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PICoolingDemand::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PICoolingDemand
namespace PIHeatingDemand {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PIHeatingDemand::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PIHeatingDemand
namespace HVACSystemTypeConfiguration {
inline constexpr DataModel::AttributeEntry kMetadataEntry(HVACSystemTypeConfiguration::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace HVACSystemTypeConfiguration
namespace LocalTemperatureCalibration {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LocalTemperatureCalibration::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace LocalTemperatureCalibration
namespace OccupiedCoolingSetpoint {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OccupiedCoolingSetpoint::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace OccupiedCoolingSetpoint
namespace OccupiedHeatingSetpoint {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OccupiedHeatingSetpoint::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace OccupiedHeatingSetpoint
namespace UnoccupiedCoolingSetpoint {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UnoccupiedCoolingSetpoint::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kOperate);
} // namespace UnoccupiedCoolingSetpoint
namespace UnoccupiedHeatingSetpoint {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UnoccupiedHeatingSetpoint::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kOperate);
} // namespace UnoccupiedHeatingSetpoint
namespace MinHeatSetpointLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinHeatSetpointLimit::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace MinHeatSetpointLimit
namespace MaxHeatSetpointLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxHeatSetpointLimit::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace MaxHeatSetpointLimit
namespace MinCoolSetpointLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinCoolSetpointLimit::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace MinCoolSetpointLimit
namespace MaxCoolSetpointLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxCoolSetpointLimit::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace MaxCoolSetpointLimit
namespace MinSetpointDeadBand {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinSetpointDeadBand::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace MinSetpointDeadBand
namespace RemoteSensing {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RemoteSensing::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace RemoteSensing
namespace ControlSequenceOfOperation {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ControlSequenceOfOperation::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace ControlSequenceOfOperation
namespace SystemMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SystemMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace SystemMode
namespace ThermostatRunningMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ThermostatRunningMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ThermostatRunningMode
namespace StartOfWeek {
inline constexpr DataModel::AttributeEntry kMetadataEntry(StartOfWeek::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace StartOfWeek
namespace NumberOfWeeklyTransitions {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfWeeklyTransitions::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace NumberOfWeeklyTransitions
namespace NumberOfDailyTransitions {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfDailyTransitions::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace NumberOfDailyTransitions
namespace TemperatureSetpointHold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TemperatureSetpointHold::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace TemperatureSetpointHold
namespace TemperatureSetpointHoldDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TemperatureSetpointHoldDuration::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace TemperatureSetpointHoldDuration
namespace ThermostatProgrammingOperationMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ThermostatProgrammingOperationMode::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kManage);
} // namespace ThermostatProgrammingOperationMode
namespace ThermostatRunningState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ThermostatRunningState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ThermostatRunningState
namespace SetpointChangeSource {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SetpointChangeSource::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SetpointChangeSource
namespace SetpointChangeAmount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SetpointChangeAmount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SetpointChangeAmount
namespace SetpointChangeSourceTimestamp {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SetpointChangeSourceTimestamp::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace SetpointChangeSourceTimestamp
namespace OccupiedSetback {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OccupiedSetback::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace OccupiedSetback
namespace OccupiedSetbackMin {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OccupiedSetbackMin::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OccupiedSetbackMin
namespace OccupiedSetbackMax {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OccupiedSetbackMax::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OccupiedSetbackMax
namespace UnoccupiedSetback {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UnoccupiedSetback::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace UnoccupiedSetback
namespace UnoccupiedSetbackMin {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UnoccupiedSetbackMin::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace UnoccupiedSetbackMin
namespace UnoccupiedSetbackMax {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UnoccupiedSetbackMax::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace UnoccupiedSetbackMax
namespace EmergencyHeatDelta {
inline constexpr DataModel::AttributeEntry kMetadataEntry(EmergencyHeatDelta::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace EmergencyHeatDelta
namespace ACType {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ACType::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ACType
namespace ACCapacity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ACCapacity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ACCapacity
namespace ACRefrigerantType {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ACRefrigerantType::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ACRefrigerantType
namespace ACCompressorType {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ACCompressorType::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ACCompressorType
namespace ACErrorCode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ACErrorCode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ACErrorCode
namespace ACLouverPosition {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ACLouverPosition::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ACLouverPosition
namespace ACCoilTemperature {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ACCoilTemperature::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ACCoilTemperature
namespace ACCapacityformat {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ACCapacityformat::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ACCapacityformat
namespace PresetTypes {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(PresetTypes::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace PresetTypes
namespace ScheduleTypes {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ScheduleTypes::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ScheduleTypes
namespace NumberOfPresets {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfPresets::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NumberOfPresets
namespace NumberOfSchedules {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfSchedules::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NumberOfSchedules
namespace NumberOfScheduleTransitions {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfScheduleTransitions::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace NumberOfScheduleTransitions
namespace NumberOfScheduleTransitionPerDay {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfScheduleTransitionPerDay::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace NumberOfScheduleTransitionPerDay
namespace ActivePresetHandle {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ActivePresetHandle::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ActivePresetHandle
namespace ActiveScheduleHandle {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ActiveScheduleHandle::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ActiveScheduleHandle
namespace Presets {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Presets::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, Access::Privilege::kManage);
} // namespace Presets
namespace Schedules {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Schedules::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, Access::Privilege::kManage);
} // namespace Schedules
namespace SetpointHoldExpiryTimestamp {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SetpointHoldExpiryTimestamp::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace SetpointHoldExpiryTimestamp
namespace MaxThermostatSuggestions {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxThermostatSuggestions::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace MaxThermostatSuggestions
namespace ThermostatSuggestions {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ThermostatSuggestions::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ThermostatSuggestions
namespace CurrentThermostatSuggestion {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentThermostatSuggestion::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace CurrentThermostatSuggestion
namespace ThermostatSuggestionNotFollowingReason {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ThermostatSuggestionNotFollowingReason::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace ThermostatSuggestionNotFollowingReason

} // namespace Attributes

namespace Commands {
namespace SetpointRaiseLower {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetpointRaiseLower::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace SetpointRaiseLower
namespace SetWeeklySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetWeeklySchedule::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace SetWeeklySchedule
namespace GetWeeklySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(GetWeeklySchedule::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace GetWeeklySchedule
namespace ClearWeeklySchedule {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ClearWeeklySchedule::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace ClearWeeklySchedule
namespace SetActiveScheduleRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetActiveScheduleRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace SetActiveScheduleRequest
namespace SetActivePresetRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetActivePresetRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace SetActivePresetRequest
namespace AddThermostatSuggestion {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(AddThermostatSuggestion::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace AddThermostatSuggestion
namespace RemoveThermostatSuggestion {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RemoveThermostatSuggestion::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace RemoveThermostatSuggestion
namespace AtomicRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(AtomicRequest::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace AtomicRequest

} // namespace Commands
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
