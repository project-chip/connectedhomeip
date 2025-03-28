// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster PushAvStreamTransport (cluster code: 1365/0x555)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

inline constexpr ClusterId Id = 0x00000555;

namespace Attributes {
namespace SupportedContainerFormats {
inline constexpr AttributeId Id = 0x00000000;
} // namespace SupportedContainerFormats
namespace SupportedIngestMethods {
inline constexpr AttributeId Id = 0x00000001;
} // namespace SupportedIngestMethods
namespace CurrentConnections {
inline constexpr AttributeId Id = 0x00000002;
} // namespace CurrentConnections
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
namespace AllocatePushTransport {
inline constexpr CommandId Id = 0x00000000;
} // namespace AllocatePushTransport
namespace DeallocatePushTransport {
inline constexpr CommandId Id = 0x00000002;
} // namespace DeallocatePushTransport
namespace ModifyPushTransport {
inline constexpr CommandId Id = 0x00000003;
} // namespace ModifyPushTransport
namespace SetTransportStatus {
inline constexpr CommandId Id = 0x00000004;
} // namespace SetTransportStatus
namespace ManuallyTriggerTransport {
inline constexpr CommandId Id = 0x00000005;
} // namespace ManuallyTriggerTransport
namespace FindTransport {
inline constexpr CommandId Id = 0x00000006;
} // namespace FindTransport
namespace AllocatePushTransportResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace AllocatePushTransportResponse
namespace FindTransportResponse {
inline constexpr CommandId Id = 0x00000007;
} // namespace FindTransportResponse
} // namespace Commands

namespace Events {
namespace PushTransportBegin {
inline constexpr EventId Id = 0x00000000;
} // namespace PushTransportBegin
namespace PushTransportEnd {
inline constexpr EventId Id = 0x00000001;
} // namespace PushTransportEnd
} // namespace Events

} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
