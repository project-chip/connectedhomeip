// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Messages (cluster code: 151/0x97)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Messages {

inline constexpr ClusterId Id = 0x00000097;

namespace Attributes {
namespace Messages {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Messages
namespace ActiveMessageIDs {
inline constexpr AttributeId Id = 0x00000001;
} // namespace ActiveMessageIDs
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
namespace PresentMessagesRequest {
inline constexpr CommandId Id = 0x00000000;
} // namespace PresentMessagesRequest
namespace CancelMessagesRequest {
inline constexpr CommandId Id = 0x00000001;
} // namespace CancelMessagesRequest
} // namespace Commands

namespace Events {
namespace MessageQueued {
inline constexpr EventId Id = 0x00000000;
} // namespace MessageQueued
namespace MessagePresented {
inline constexpr EventId Id = 0x00000001;
} // namespace MessagePresented
namespace MessageComplete {
inline constexpr EventId Id = 0x00000002;
} // namespace MessageComplete
} // namespace Events

} // namespace Messages
} // namespace Clusters
} // namespace app
} // namespace chip
