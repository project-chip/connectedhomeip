// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AccountLogin (cluster code: 1294/0x50E)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AccountLogin {

inline constexpr ClusterId Id = 0x0000050E;

namespace Attributes {
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
namespace GetSetupPIN {
inline constexpr CommandId Id = 0x00000000;
} // namespace GetSetupPIN
namespace Login {
inline constexpr CommandId Id = 0x00000002;
} // namespace Login
namespace Logout {
inline constexpr CommandId Id = 0x00000003;
} // namespace Logout
namespace GetSetupPINResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace GetSetupPINResponse
} // namespace Commands

namespace Events {
namespace LoggedOut {
inline constexpr EventId Id = 0x00000000;
} // namespace LoggedOut
} // namespace Events

} // namespace AccountLogin
} // namespace Clusters
} // namespace app
} // namespace chip
