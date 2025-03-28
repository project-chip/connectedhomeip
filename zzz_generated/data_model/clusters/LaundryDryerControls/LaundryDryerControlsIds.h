// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster LaundryDryerControls (cluster code: 74/0x4A)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LaundryDryerControls {

inline constexpr ClusterId Id = 0x0000004A;

namespace Attributes {
namespace SupportedDrynessLevels {
inline constexpr AttributeId Id = 0x00000000;
} // namespace SupportedDrynessLevels
namespace SelectedDrynessLevel {
inline constexpr AttributeId Id = 0x00000001;
} // namespace SelectedDrynessLevel
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

namespace Events {} // namespace Events

} // namespace LaundryDryerControls
} // namespace Clusters
} // namespace app
} // namespace chip
