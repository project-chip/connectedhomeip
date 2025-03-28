// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster WebRTCTransportRequestor (cluster code: 1364/0x554)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WebRTCTransportRequestor {

inline constexpr ClusterId Id = 0x00000554;

namespace Attributes {
namespace CurrentSessions {
inline constexpr AttributeId Id = 0x00000000;
} // namespace CurrentSessions
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
namespace Offer {
inline constexpr CommandId Id = 0x00000001;
} // namespace Offer
namespace Answer {
inline constexpr CommandId Id = 0x00000002;
} // namespace Answer
namespace ICECandidates {
inline constexpr CommandId Id = 0x00000003;
} // namespace ICECandidates
namespace End {
inline constexpr CommandId Id = 0x00000004;
} // namespace End
} // namespace Commands

namespace Events {} // namespace Events

} // namespace WebRTCTransportRequestor
} // namespace Clusters
} // namespace app
} // namespace chip
