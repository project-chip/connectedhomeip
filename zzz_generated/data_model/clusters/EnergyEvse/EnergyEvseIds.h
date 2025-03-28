// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster EnergyEvse (cluster code: 153/0x99)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

inline constexpr ClusterId Id = 0x00000099;

namespace Attributes {
namespace State {
inline constexpr AttributeId Id = 0x00000000;
} // namespace State
namespace SupplyState {
inline constexpr AttributeId Id = 0x00000001;
} // namespace SupplyState
namespace FaultState {
inline constexpr AttributeId Id = 0x00000002;
} // namespace FaultState
namespace ChargingEnabledUntil {
inline constexpr AttributeId Id = 0x00000003;
} // namespace ChargingEnabledUntil
namespace DischargingEnabledUntil {
inline constexpr AttributeId Id = 0x00000004;
} // namespace DischargingEnabledUntil
namespace CircuitCapacity {
inline constexpr AttributeId Id = 0x00000005;
} // namespace CircuitCapacity
namespace MinimumChargeCurrent {
inline constexpr AttributeId Id = 0x00000006;
} // namespace MinimumChargeCurrent
namespace MaximumChargeCurrent {
inline constexpr AttributeId Id = 0x00000007;
} // namespace MaximumChargeCurrent
namespace MaximumDischargeCurrent {
inline constexpr AttributeId Id = 0x00000008;
} // namespace MaximumDischargeCurrent
namespace UserMaximumChargeCurrent {
inline constexpr AttributeId Id = 0x00000009;
} // namespace UserMaximumChargeCurrent
namespace RandomizationDelayWindow {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace RandomizationDelayWindow
namespace NextChargeStartTime {
inline constexpr AttributeId Id = 0x00000023;
} // namespace NextChargeStartTime
namespace NextChargeTargetTime {
inline constexpr AttributeId Id = 0x00000024;
} // namespace NextChargeTargetTime
namespace NextChargeRequiredEnergy {
inline constexpr AttributeId Id = 0x00000025;
} // namespace NextChargeRequiredEnergy
namespace NextChargeTargetSoC {
inline constexpr AttributeId Id = 0x00000026;
} // namespace NextChargeTargetSoC
namespace ApproximateEVEfficiency {
inline constexpr AttributeId Id = 0x00000027;
} // namespace ApproximateEVEfficiency
namespace StateOfCharge {
inline constexpr AttributeId Id = 0x00000030;
} // namespace StateOfCharge
namespace BatteryCapacity {
inline constexpr AttributeId Id = 0x00000031;
} // namespace BatteryCapacity
namespace VehicleID {
inline constexpr AttributeId Id = 0x00000032;
} // namespace VehicleID
namespace SessionID {
inline constexpr AttributeId Id = 0x00000040;
} // namespace SessionID
namespace SessionDuration {
inline constexpr AttributeId Id = 0x00000041;
} // namespace SessionDuration
namespace SessionEnergyCharged {
inline constexpr AttributeId Id = 0x00000042;
} // namespace SessionEnergyCharged
namespace SessionEnergyDischarged {
inline constexpr AttributeId Id = 0x00000043;
} // namespace SessionEnergyDischarged
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

namespace Commands {
namespace Disable {
inline constexpr CommandId Id = 0x00000001;
} // namespace Disable
namespace EnableCharging {
inline constexpr CommandId Id = 0x00000002;
} // namespace EnableCharging
namespace EnableDischarging {
inline constexpr CommandId Id = 0x00000003;
} // namespace EnableDischarging
namespace StartDiagnostics {
inline constexpr CommandId Id = 0x00000004;
} // namespace StartDiagnostics
namespace SetTargets {
inline constexpr CommandId Id = 0x00000005;
} // namespace SetTargets
namespace GetTargets {
inline constexpr CommandId Id = 0x00000006;
} // namespace GetTargets
namespace ClearTargets {
inline constexpr CommandId Id = 0x00000007;
} // namespace ClearTargets
namespace GetTargetsResponse {
inline constexpr CommandId Id = 0x00000000;
} // namespace GetTargetsResponse
} // namespace Commands

namespace Events {
namespace EVConnected {
inline constexpr EventId Id = 0x00000000;
} // namespace EVConnected
namespace EVNotDetected {
inline constexpr EventId Id = 0x00000001;
} // namespace EVNotDetected
namespace EnergyTransferStarted {
inline constexpr EventId Id = 0x00000002;
} // namespace EnergyTransferStarted
namespace EnergyTransferStopped {
inline constexpr EventId Id = 0x00000003;
} // namespace EnergyTransferStopped
namespace Fault {
inline constexpr EventId Id = 0x00000004;
} // namespace Fault
namespace Rfid {
inline constexpr EventId Id = 0x00000005;
} // namespace Rfid
} // namespace Events

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
