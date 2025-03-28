// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster GeneralDiagnostics (cluster code: 51/0x33)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace GeneralDiagnostics {

inline constexpr ClusterId Id = 0x00000033;

namespace Attributes {
namespace NetworkInterfaces {
inline constexpr AttributeId Id = 0x00000000;
} // namespace NetworkInterfaces
namespace RebootCount {
inline constexpr AttributeId Id = 0x00000001;
} // namespace RebootCount
namespace UpTime {
inline constexpr AttributeId Id = 0x00000002;
} // namespace UpTime
namespace TotalOperationalHours {
inline constexpr AttributeId Id = 0x00000003;
} // namespace TotalOperationalHours
namespace BootReason {
inline constexpr AttributeId Id = 0x00000004;
} // namespace BootReason
namespace ActiveHardwareFaults {
inline constexpr AttributeId Id = 0x00000005;
} // namespace ActiveHardwareFaults
namespace ActiveRadioFaults {
inline constexpr AttributeId Id = 0x00000006;
} // namespace ActiveRadioFaults
namespace ActiveNetworkFaults {
inline constexpr AttributeId Id = 0x00000007;
} // namespace ActiveNetworkFaults
namespace TestEventTriggersEnabled {
inline constexpr AttributeId Id = 0x00000008;
} // namespace TestEventTriggersEnabled
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
namespace TestEventTrigger {
inline constexpr CommandId Id = 0x00000000;
} // namespace TestEventTrigger
namespace TimeSnapshot {
inline constexpr CommandId Id = 0x00000001;
} // namespace TimeSnapshot
namespace PayloadTestRequest {
inline constexpr CommandId Id = 0x00000003;
} // namespace PayloadTestRequest
namespace TimeSnapshotResponse {
inline constexpr CommandId Id = 0x00000002;
} // namespace TimeSnapshotResponse
namespace PayloadTestResponse {
inline constexpr CommandId Id = 0x00000004;
} // namespace PayloadTestResponse
} // namespace Commands

namespace Events {
namespace HardwareFaultChange {
inline constexpr EventId Id = 0x00000000;
} // namespace HardwareFaultChange
namespace RadioFaultChange {
inline constexpr EventId Id = 0x00000001;
} // namespace RadioFaultChange
namespace NetworkFaultChange {
inline constexpr EventId Id = 0x00000002;
} // namespace NetworkFaultChange
namespace BootReason {
inline constexpr EventId Id = 0x00000003;
} // namespace BootReason
} // namespace Events

} // namespace GeneralDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
