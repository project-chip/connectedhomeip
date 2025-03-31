// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster MediaInput (cluster code: 1287/0x507)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaInput {

inline constexpr ClusterId Id = 0x00000507;

namespace Attributes {
namespace InputList {
inline constexpr AttributeId Id = 0x00000000;
} // namespace InputList
namespace CurrentInput {
inline constexpr AttributeId Id = 0x00000001;
} // namespace CurrentInput
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
namespace SelectInput {
inline constexpr CommandId Id = 0x00000000;
} // namespace SelectInput
namespace ShowInputStatus {
inline constexpr CommandId Id = 0x00000001;
} // namespace ShowInputStatus
namespace HideInputStatus {
inline constexpr CommandId Id = 0x00000002;
} // namespace HideInputStatus
namespace RenameInput {
inline constexpr CommandId Id = 0x00000003;
} // namespace RenameInput
} // namespace Commands

namespace Events {} // namespace Events

} // namespace MediaInput
} // namespace Clusters
} // namespace app
} // namespace chip
