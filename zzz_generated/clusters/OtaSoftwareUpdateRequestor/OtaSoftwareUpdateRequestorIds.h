// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster OtaSoftwareUpdateRequestor (cluster code: 42/0x2A)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OtaSoftwareUpdateRequestor {

inline constexpr ClusterId Id = 0x0000002A;

namespace Attributes {
namespace DefaultOTAProviders {
inline constexpr AttributeId Id = 0x00000000;
} // namespace DefaultOTAProviders
namespace UpdatePossible {
inline constexpr AttributeId Id = 0x00000001;
} // namespace UpdatePossible
namespace UpdateState {
inline constexpr AttributeId Id = 0x00000002;
} // namespace UpdateState
namespace UpdateStateProgress {
inline constexpr AttributeId Id = 0x00000003;
} // namespace UpdateStateProgress
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
namespace AnnounceOTAProvider {
inline constexpr CommandId Id = 0x00000000;
} // namespace AnnounceOTAProvider
} // namespace Commands

namespace Events {
namespace StateTransition {
inline constexpr EventId Id = 0x00000000;
} // namespace StateTransition
namespace VersionApplied {
inline constexpr EventId Id = 0x00000001;
} // namespace VersionApplied
namespace DownloadError {
inline constexpr EventId Id = 0x00000002;
} // namespace DownloadError
} // namespace Events

} // namespace OtaSoftwareUpdateRequestor
} // namespace Clusters
} // namespace app
} // namespace chip
