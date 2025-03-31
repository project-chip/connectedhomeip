// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster SmokeCoAlarm (cluster code: 92/0x5C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace SmokeCoAlarm {

inline constexpr ClusterId Id = 0x0000005C;

namespace Attributes {
namespace ExpressedState {
inline constexpr AttributeId Id = 0x00000000;
} // namespace ExpressedState
namespace SmokeState {
inline constexpr AttributeId Id = 0x00000001;
} // namespace SmokeState
namespace COState {
inline constexpr AttributeId Id = 0x00000002;
} // namespace COState
namespace BatteryAlert {
inline constexpr AttributeId Id = 0x00000003;
} // namespace BatteryAlert
namespace DeviceMuted {
inline constexpr AttributeId Id = 0x00000004;
} // namespace DeviceMuted
namespace TestInProgress {
inline constexpr AttributeId Id = 0x00000005;
} // namespace TestInProgress
namespace HardwareFaultAlert {
inline constexpr AttributeId Id = 0x00000006;
} // namespace HardwareFaultAlert
namespace EndOfServiceAlert {
inline constexpr AttributeId Id = 0x00000007;
} // namespace EndOfServiceAlert
namespace InterconnectSmokeAlarm {
inline constexpr AttributeId Id = 0x00000008;
} // namespace InterconnectSmokeAlarm
namespace InterconnectCOAlarm {
inline constexpr AttributeId Id = 0x00000009;
} // namespace InterconnectCOAlarm
namespace ContaminationState {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace ContaminationState
namespace SmokeSensitivityLevel {
inline constexpr AttributeId Id = 0x0000000B;
} // namespace SmokeSensitivityLevel
namespace ExpiryDate {
inline constexpr AttributeId Id = 0x0000000C;
} // namespace ExpiryDate
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
namespace SelfTestRequest {
inline constexpr CommandId Id = 0x00000000;
} // namespace SelfTestRequest
} // namespace Commands

namespace Events {
namespace SmokeAlarm {
inline constexpr EventId Id = 0x00000000;
} // namespace SmokeAlarm
namespace COAlarm {
inline constexpr EventId Id = 0x00000001;
} // namespace COAlarm
namespace LowBattery {
inline constexpr EventId Id = 0x00000002;
} // namespace LowBattery
namespace HardwareFault {
inline constexpr EventId Id = 0x00000003;
} // namespace HardwareFault
namespace EndOfService {
inline constexpr EventId Id = 0x00000004;
} // namespace EndOfService
namespace SelfTestComplete {
inline constexpr EventId Id = 0x00000005;
} // namespace SelfTestComplete
namespace AlarmMuted {
inline constexpr EventId Id = 0x00000006;
} // namespace AlarmMuted
namespace MuteEnded {
inline constexpr EventId Id = 0x00000007;
} // namespace MuteEnded
namespace InterconnectSmokeAlarm {
inline constexpr EventId Id = 0x00000008;
} // namespace InterconnectSmokeAlarm
namespace InterconnectCOAlarm {
inline constexpr EventId Id = 0x00000009;
} // namespace InterconnectCOAlarm
namespace AllClear {
inline constexpr EventId Id = 0x0000000A;
} // namespace AllClear
} // namespace Events

} // namespace SmokeCoAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
