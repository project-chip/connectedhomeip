// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster Descriptor (cluster code: 29/0x1D)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Descriptor {

inline constexpr ClusterId Id = 0x0000001D;

namespace Attributes {
namespace DeviceTypeList {
inline constexpr AttributeId Id = 0x00000000;
} // namespace DeviceTypeList
namespace ServerList {
inline constexpr AttributeId Id = 0x00000001;
} // namespace ServerList
namespace ClientList {
inline constexpr AttributeId Id = 0x00000002;
} // namespace ClientList
namespace PartsList {
inline constexpr AttributeId Id = 0x00000003;
} // namespace PartsList
namespace TagList {
inline constexpr AttributeId Id = 0x00000004;
} // namespace TagList
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

} // namespace Descriptor
} // namespace Clusters
} // namespace app
} // namespace chip
