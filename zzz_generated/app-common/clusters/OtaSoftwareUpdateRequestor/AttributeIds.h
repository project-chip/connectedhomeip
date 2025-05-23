// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster OtaSoftwareUpdateRequestor (cluster code: 42/0x2A)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OtaSoftwareUpdateRequestor {
namespace Attributes {
namespace DefaultOTAProviders {
inline constexpr AttributeId Id = 0x00000000;
} // namespace DefaultOTAProviders

namespace UpdatePossible {
inline constexpr AttributeId Id = 0x00000001;
} // namespace UpdatePossible

namespace UpdateState {
inline constexpr AttributeId Id = 0x00000002;
} // namespace UpdateState

namespace UpdateStateProgress {
inline constexpr AttributeId Id = 0x00000003;
} // namespace UpdateStateProgress

namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList

namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList

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
} // namespace OtaSoftwareUpdateRequestor
} // namespace Clusters
} // namespace app
} // namespace chip
