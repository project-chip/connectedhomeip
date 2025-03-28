// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Chime (cluster code: 1366/0x556)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Chime {

inline constexpr ClusterId Id = 0x00000556;

namespace Attributes {
namespace InstalledChimeSounds {
inline constexpr AttributeId Id = 0x00000000;
} // namespace InstalledChimeSounds
namespace SelectedChime {
inline constexpr AttributeId Id = 0x00000001;
} // namespace SelectedChime
namespace Enabled {
inline constexpr AttributeId Id = 0x00000002;
} // namespace Enabled
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
namespace PlayChimeSound {
inline constexpr CommandId Id = 0x00000000;
} // namespace PlayChimeSound
} // namespace Commands

namespace Events {} // namespace Events

} // namespace Chime
} // namespace Clusters
} // namespace app
} // namespace chip
