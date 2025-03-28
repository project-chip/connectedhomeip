// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster CommissionerControl (cluster code: 1873/0x751)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissionerControl {

inline constexpr ClusterId Id = 0x00000751;

namespace Attributes {
namespace SupportedDeviceCategories {
inline constexpr AttributeId Id = 0x00000000;
} // namespace SupportedDeviceCategories
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
namespace RequestCommissioningApproval {
inline constexpr CommandId Id = 0x00000000;
} // namespace RequestCommissioningApproval
namespace CommissionNode {
inline constexpr CommandId Id = 0x00000001;
} // namespace CommissionNode
namespace ReverseOpenCommissioningWindow {
inline constexpr CommandId Id = 0x00000002;
} // namespace ReverseOpenCommissioningWindow
} // namespace Commands

namespace Events {
namespace CommissioningRequestResult {
inline constexpr EventId Id = 0x00000000;
} // namespace CommissioningRequestResult
} // namespace Events

} // namespace CommissionerControl
} // namespace Clusters
} // namespace app
} // namespace chip
