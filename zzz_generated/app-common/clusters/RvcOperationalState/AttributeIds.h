// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster RvcOperationalState (cluster code: 97/0x61)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace RvcOperationalState {
namespace Attributes {
namespace PhaseList {
inline constexpr AttributeId Id = 0x00000000;
} // namespace PhaseList

namespace CurrentPhase {
inline constexpr AttributeId Id = 0x00000001;
} // namespace CurrentPhase

namespace CountdownTime {
inline constexpr AttributeId Id = 0x00000002;
} // namespace CountdownTime

namespace OperationalStateList {
inline constexpr AttributeId Id = 0x00000003;
} // namespace OperationalStateList

namespace OperationalState {
inline constexpr AttributeId Id = 0x00000004;
} // namespace OperationalState

namespace OperationalError {
inline constexpr AttributeId Id = 0x00000005;
} // namespace OperationalError

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
} // namespace RvcOperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
