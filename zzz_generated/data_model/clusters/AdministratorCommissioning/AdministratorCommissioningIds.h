// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AdministratorCommissioning (cluster code: 60/0x3C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AdministratorCommissioning {

inline constexpr ClusterId Id = 0x0000003C;

namespace Attributes {
namespace WindowStatus {
inline constexpr AttributeId Id = 0x00000000;
} // namespace WindowStatus
namespace AdminFabricIndex {
inline constexpr AttributeId Id = 0x00000001;
} // namespace AdminFabricIndex
namespace AdminVendorId {
inline constexpr AttributeId Id = 0x00000002;
} // namespace AdminVendorId
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
namespace OpenCommissioningWindow {
inline constexpr CommandId Id = 0x00000000;
} // namespace OpenCommissioningWindow
namespace OpenBasicCommissioningWindow {
inline constexpr CommandId Id = 0x00000001;
} // namespace OpenBasicCommissioningWindow
namespace RevokeCommissioning {
inline constexpr CommandId Id = 0x00000002;
} // namespace RevokeCommissioning
} // namespace Commands

namespace Events {} // namespace Events

} // namespace AdministratorCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
