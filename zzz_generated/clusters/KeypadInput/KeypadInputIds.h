// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster KeypadInput (cluster code: 1289/0x509)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace KeypadInput {

inline constexpr ClusterId Id = 0x00000509;

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
namespace SendKey {
inline constexpr CommandId Id = 0x00000000;
} // namespace SendKey
namespace SendKeyResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace SendKeyResponse
} // namespace Commands

namespace Events {} // namespace Events

} // namespace KeypadInput
} // namespace Clusters
} // namespace app
} // namespace chip
