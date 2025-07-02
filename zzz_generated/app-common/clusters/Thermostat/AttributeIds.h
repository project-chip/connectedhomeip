// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Thermostat (cluster code: 513/0x201)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {
namespace Attributes {
namespace LocalTemperature {
inline constexpr AttributeId Id = 0x00000000;
} // namespace LocalTemperature

namespace OutdoorTemperature {
inline constexpr AttributeId Id = 0x00000001;
} // namespace OutdoorTemperature

namespace Occupancy {
inline constexpr AttributeId Id = 0x00000002;
} // namespace Occupancy

namespace AbsMinHeatSetpointLimit {
inline constexpr AttributeId Id = 0x00000003;
} // namespace AbsMinHeatSetpointLimit

namespace AbsMaxHeatSetpointLimit {
inline constexpr AttributeId Id = 0x00000004;
} // namespace AbsMaxHeatSetpointLimit

namespace AbsMinCoolSetpointLimit {
inline constexpr AttributeId Id = 0x00000005;
} // namespace AbsMinCoolSetpointLimit

namespace AbsMaxCoolSetpointLimit {
inline constexpr AttributeId Id = 0x00000006;
} // namespace AbsMaxCoolSetpointLimit

namespace PICoolingDemand {
inline constexpr AttributeId Id = 0x00000007;
} // namespace PICoolingDemand

namespace PIHeatingDemand {
inline constexpr AttributeId Id = 0x00000008;
} // namespace PIHeatingDemand

namespace HVACSystemTypeConfiguration {
inline constexpr AttributeId Id = 0x00000009;
} // namespace HVACSystemTypeConfiguration

namespace LocalTemperatureCalibration {
inline constexpr AttributeId Id = 0x00000010;
} // namespace LocalTemperatureCalibration

namespace OccupiedCoolingSetpoint {
inline constexpr AttributeId Id = 0x00000011;
} // namespace OccupiedCoolingSetpoint

namespace OccupiedHeatingSetpoint {
inline constexpr AttributeId Id = 0x00000012;
} // namespace OccupiedHeatingSetpoint

namespace UnoccupiedCoolingSetpoint {
inline constexpr AttributeId Id = 0x00000013;
} // namespace UnoccupiedCoolingSetpoint

namespace UnoccupiedHeatingSetpoint {
inline constexpr AttributeId Id = 0x00000014;
} // namespace UnoccupiedHeatingSetpoint

namespace MinHeatSetpointLimit {
inline constexpr AttributeId Id = 0x00000015;
} // namespace MinHeatSetpointLimit

namespace MaxHeatSetpointLimit {
inline constexpr AttributeId Id = 0x00000016;
} // namespace MaxHeatSetpointLimit

namespace MinCoolSetpointLimit {
inline constexpr AttributeId Id = 0x00000017;
} // namespace MinCoolSetpointLimit

namespace MaxCoolSetpointLimit {
inline constexpr AttributeId Id = 0x00000018;
} // namespace MaxCoolSetpointLimit

namespace MinSetpointDeadBand {
inline constexpr AttributeId Id = 0x00000019;
} // namespace MinSetpointDeadBand

namespace RemoteSensing {
inline constexpr AttributeId Id = 0x0000001A;
} // namespace RemoteSensing

namespace ControlSequenceOfOperation {
inline constexpr AttributeId Id = 0x0000001B;
} // namespace ControlSequenceOfOperation

namespace SystemMode {
inline constexpr AttributeId Id = 0x0000001C;
} // namespace SystemMode

namespace ThermostatRunningMode {
inline constexpr AttributeId Id = 0x0000001E;
} // namespace ThermostatRunningMode

namespace StartOfWeek {
inline constexpr AttributeId Id = 0x00000020;
} // namespace StartOfWeek

namespace NumberOfWeeklyTransitions {
inline constexpr AttributeId Id = 0x00000021;
} // namespace NumberOfWeeklyTransitions

namespace NumberOfDailyTransitions {
inline constexpr AttributeId Id = 0x00000022;
} // namespace NumberOfDailyTransitions

namespace TemperatureSetpointHold {
inline constexpr AttributeId Id = 0x00000023;
} // namespace TemperatureSetpointHold

namespace TemperatureSetpointHoldDuration {
inline constexpr AttributeId Id = 0x00000024;
} // namespace TemperatureSetpointHoldDuration

namespace ThermostatProgrammingOperationMode {
inline constexpr AttributeId Id = 0x00000025;
} // namespace ThermostatProgrammingOperationMode

namespace ThermostatRunningState {
inline constexpr AttributeId Id = 0x00000029;
} // namespace ThermostatRunningState

namespace SetpointChangeSource {
inline constexpr AttributeId Id = 0x00000030;
} // namespace SetpointChangeSource

namespace SetpointChangeAmount {
inline constexpr AttributeId Id = 0x00000031;
} // namespace SetpointChangeAmount

namespace SetpointChangeSourceTimestamp {
inline constexpr AttributeId Id = 0x00000032;
} // namespace SetpointChangeSourceTimestamp

namespace OccupiedSetback {
inline constexpr AttributeId Id = 0x00000034;
} // namespace OccupiedSetback

namespace OccupiedSetbackMin {
inline constexpr AttributeId Id = 0x00000035;
} // namespace OccupiedSetbackMin

namespace OccupiedSetbackMax {
inline constexpr AttributeId Id = 0x00000036;
} // namespace OccupiedSetbackMax

namespace UnoccupiedSetback {
inline constexpr AttributeId Id = 0x00000037;
} // namespace UnoccupiedSetback

namespace UnoccupiedSetbackMin {
inline constexpr AttributeId Id = 0x00000038;
} // namespace UnoccupiedSetbackMin

namespace UnoccupiedSetbackMax {
inline constexpr AttributeId Id = 0x00000039;
} // namespace UnoccupiedSetbackMax

namespace EmergencyHeatDelta {
inline constexpr AttributeId Id = 0x0000003A;
} // namespace EmergencyHeatDelta

namespace ACType {
inline constexpr AttributeId Id = 0x00000040;
} // namespace ACType

namespace ACCapacity {
inline constexpr AttributeId Id = 0x00000041;
} // namespace ACCapacity

namespace ACRefrigerantType {
inline constexpr AttributeId Id = 0x00000042;
} // namespace ACRefrigerantType

namespace ACCompressorType {
inline constexpr AttributeId Id = 0x00000043;
} // namespace ACCompressorType

namespace ACErrorCode {
inline constexpr AttributeId Id = 0x00000044;
} // namespace ACErrorCode

namespace ACLouverPosition {
inline constexpr AttributeId Id = 0x00000045;
} // namespace ACLouverPosition

namespace ACCoilTemperature {
inline constexpr AttributeId Id = 0x00000046;
} // namespace ACCoilTemperature

namespace ACCapacityformat {
inline constexpr AttributeId Id = 0x00000047;
} // namespace ACCapacityformat

namespace PresetTypes {
inline constexpr AttributeId Id = 0x00000048;
} // namespace PresetTypes

namespace ScheduleTypes {
inline constexpr AttributeId Id = 0x00000049;
} // namespace ScheduleTypes

namespace NumberOfPresets {
inline constexpr AttributeId Id = 0x0000004A;
} // namespace NumberOfPresets

namespace NumberOfSchedules {
inline constexpr AttributeId Id = 0x0000004B;
} // namespace NumberOfSchedules

namespace NumberOfScheduleTransitions {
inline constexpr AttributeId Id = 0x0000004C;
} // namespace NumberOfScheduleTransitions

namespace NumberOfScheduleTransitionPerDay {
inline constexpr AttributeId Id = 0x0000004D;
} // namespace NumberOfScheduleTransitionPerDay

namespace ActivePresetHandle {
inline constexpr AttributeId Id = 0x0000004E;
} // namespace ActivePresetHandle

namespace ActiveScheduleHandle {
inline constexpr AttributeId Id = 0x0000004F;
} // namespace ActiveScheduleHandle

namespace Presets {
inline constexpr AttributeId Id = 0x00000050;
} // namespace Presets

namespace Schedules {
inline constexpr AttributeId Id = 0x00000051;
} // namespace Schedules

namespace SetpointHoldExpiryTimestamp {
inline constexpr AttributeId Id = 0x00000052;
} // namespace SetpointHoldExpiryTimestamp

namespace MaxThermostatSuggestions {
inline constexpr AttributeId Id = 0x00000053;
} // namespace MaxThermostatSuggestions

namespace ThermostatSuggestions {
inline constexpr AttributeId Id = 0x00000054;
} // namespace ThermostatSuggestions

namespace CurrentThermostatSuggestion {
inline constexpr AttributeId Id = 0x00000055;
} // namespace CurrentThermostatSuggestion

namespace ThermostatSuggestionNotFollowingReason {
inline constexpr AttributeId Id = 0x00000056;
} // namespace ThermostatSuggestionNotFollowingReason

namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList

namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList

namespace AttributeList {
inline constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;
} // namespace AttributeList

namespace FeatureMap {
inline constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;
} // namespace FeatureMap

namespace ClusterRevision {
inline constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;
} // namespace ClusterRevision

} // namespace Attributes
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
