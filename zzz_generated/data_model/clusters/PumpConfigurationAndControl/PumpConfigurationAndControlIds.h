// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster PumpConfigurationAndControl (cluster code: 512/0x200)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PumpConfigurationAndControl {

inline constexpr ClusterId Id = 0x00000200;

namespace Attributes {
namespace MaxPressure {
inline constexpr AttributeId Id = 0x00000000;
} // namespace MaxPressure
namespace MaxSpeed {
inline constexpr AttributeId Id = 0x00000001;
} // namespace MaxSpeed
namespace MaxFlow {
inline constexpr AttributeId Id = 0x00000002;
} // namespace MaxFlow
namespace MinConstPressure {
inline constexpr AttributeId Id = 0x00000003;
} // namespace MinConstPressure
namespace MaxConstPressure {
inline constexpr AttributeId Id = 0x00000004;
} // namespace MaxConstPressure
namespace MinCompPressure {
inline constexpr AttributeId Id = 0x00000005;
} // namespace MinCompPressure
namespace MaxCompPressure {
inline constexpr AttributeId Id = 0x00000006;
} // namespace MaxCompPressure
namespace MinConstSpeed {
inline constexpr AttributeId Id = 0x00000007;
} // namespace MinConstSpeed
namespace MaxConstSpeed {
inline constexpr AttributeId Id = 0x00000008;
} // namespace MaxConstSpeed
namespace MinConstFlow {
inline constexpr AttributeId Id = 0x00000009;
} // namespace MinConstFlow
namespace MaxConstFlow {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace MaxConstFlow
namespace MinConstTemp {
inline constexpr AttributeId Id = 0x0000000B;
} // namespace MinConstTemp
namespace MaxConstTemp {
inline constexpr AttributeId Id = 0x0000000C;
} // namespace MaxConstTemp
namespace PumpStatus {
inline constexpr AttributeId Id = 0x00000010;
} // namespace PumpStatus
namespace EffectiveOperationMode {
inline constexpr AttributeId Id = 0x00000011;
} // namespace EffectiveOperationMode
namespace EffectiveControlMode {
inline constexpr AttributeId Id = 0x00000012;
} // namespace EffectiveControlMode
namespace Capacity {
inline constexpr AttributeId Id = 0x00000013;
} // namespace Capacity
namespace Speed {
inline constexpr AttributeId Id = 0x00000014;
} // namespace Speed
namespace LifetimeRunningHours {
inline constexpr AttributeId Id = 0x00000015;
} // namespace LifetimeRunningHours
namespace Power {
inline constexpr AttributeId Id = 0x00000016;
} // namespace Power
namespace LifetimeEnergyConsumed {
inline constexpr AttributeId Id = 0x00000017;
} // namespace LifetimeEnergyConsumed
namespace OperationMode {
inline constexpr AttributeId Id = 0x00000020;
} // namespace OperationMode
namespace ControlMode {
inline constexpr AttributeId Id = 0x00000021;
} // namespace ControlMode
namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList
namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList
namespace EventList {
inline constexpr AttributeId Id = Globals::Attributes::EventList::Id;
} // namespace EventList
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

namespace Commands {} // namespace Commands

namespace Events {
namespace SupplyVoltageLow {
inline constexpr EventId Id = 0x00000000;
} // namespace SupplyVoltageLow
namespace SupplyVoltageHigh {
inline constexpr EventId Id = 0x00000001;
} // namespace SupplyVoltageHigh
namespace PowerMissingPhase {
inline constexpr EventId Id = 0x00000002;
} // namespace PowerMissingPhase
namespace SystemPressureLow {
inline constexpr EventId Id = 0x00000003;
} // namespace SystemPressureLow
namespace SystemPressureHigh {
inline constexpr EventId Id = 0x00000004;
} // namespace SystemPressureHigh
namespace DryRunning {
inline constexpr EventId Id = 0x00000005;
} // namespace DryRunning
namespace MotorTemperatureHigh {
inline constexpr EventId Id = 0x00000006;
} // namespace MotorTemperatureHigh
namespace PumpMotorFatalFailure {
inline constexpr EventId Id = 0x00000007;
} // namespace PumpMotorFatalFailure
namespace ElectronicTemperatureHigh {
inline constexpr EventId Id = 0x00000008;
} // namespace ElectronicTemperatureHigh
namespace PumpBlocked {
inline constexpr EventId Id = 0x00000009;
} // namespace PumpBlocked
namespace SensorFailure {
inline constexpr EventId Id = 0x0000000A;
} // namespace SensorFailure
namespace ElectronicNonFatalFailure {
inline constexpr EventId Id = 0x0000000B;
} // namespace ElectronicNonFatalFailure
namespace ElectronicFatalFailure {
inline constexpr EventId Id = 0x0000000C;
} // namespace ElectronicFatalFailure
namespace GeneralFault {
inline constexpr EventId Id = 0x0000000D;
} // namespace GeneralFault
namespace Leakage {
inline constexpr EventId Id = 0x0000000E;
} // namespace Leakage
namespace AirDetection {
inline constexpr EventId Id = 0x0000000F;
} // namespace AirDetection
namespace TurbineOperation {
inline constexpr EventId Id = 0x00000010;
} // namespace TurbineOperation
} // namespace Events

} // namespace PumpConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip
