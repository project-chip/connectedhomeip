// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster OvenMode (cluster code: 73/0x49)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OvenMode {

inline constexpr ClusterId Id = 0x00000049;

namespace Attributes {
namespace SupportedModes {
inline constexpr AttributeId Id = 0x00000000;
} // namespace SupportedModes
namespace CurrentMode {
inline constexpr AttributeId Id = 0x00000001;
} // namespace CurrentMode
namespace StartUpMode {
inline constexpr AttributeId Id = 0x00000002;
} // namespace StartUpMode
namespace OnMode {
inline constexpr AttributeId Id = 0x00000003;
} // namespace OnMode
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
namespace ChangeToMode {
inline constexpr CommandId Id = 0x00000000;
} // namespace ChangeToMode
namespace ChangeToModeResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace ChangeToModeResponse
} // namespace Commands

namespace Events {} // namespace Events

} // namespace OvenMode
} // namespace Clusters
} // namespace app
} // namespace chip
