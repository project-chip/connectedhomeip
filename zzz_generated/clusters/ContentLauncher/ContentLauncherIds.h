// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster ContentLauncher (cluster code: 1290/0x50A)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ContentLauncher {

inline constexpr ClusterId Id = 0x0000050A;

namespace Attributes {
namespace AcceptHeader {
inline constexpr AttributeId Id = 0x00000000;
} // namespace AcceptHeader
namespace SupportedStreamingProtocols {
inline constexpr AttributeId Id = 0x00000001;
} // namespace SupportedStreamingProtocols
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
namespace LaunchContent {
inline constexpr CommandId Id = 0x00000000;
} // namespace LaunchContent
namespace LaunchURL {
inline constexpr CommandId Id = 0x00000001;
} // namespace LaunchURL
namespace LauncherResponse {
inline constexpr CommandId Id = 0x00000002;
} // namespace LauncherResponse
} // namespace Commands

namespace Events {} // namespace Events

} // namespace ContentLauncher
} // namespace Clusters
} // namespace app
} // namespace chip
